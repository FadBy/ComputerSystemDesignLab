/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "melody.h"
#include "melodies_data.h"
extern TIM_HandleTypeDef htim6;
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define MAX_INPUT_LENGTH 512
#define MAX_MELODIES 4

// I2C keypad (SDK-1.1) parameters
#define KB_I2C_ADDRESS        (0xE2)          // 8-bit address
#define KB_I2C_READ_ADDRESS   (KB_I2C_ADDRESS | 1)
#define KB_I2C_WRITE_ADDRESS  (KB_I2C_ADDRESS & ~1)
#define KB_INPUT_REG          (0x0)
#define KB_OUTPUT_REG         (0x1)
#define KB_CONFIG_REG         (0x3)
#define KB_KEY_DEBOUNCE_MS    50U
#define KB_I2C_TIMEOUT        10U
#define MODE_BTN_PORT         GPIOC
#define MODE_BTN_PIN          GPIO_PIN_15

void print_string(char* s) {
	if (s == NULL) return;
	int len = strlen(s);
	if (len == 0) return;
	HAL_UART_Transmit(&huart6, (uint8_t *)s, len, HAL_MAX_DELAY);
}

static Melody* standard_melodies[MAX_MELODIES] = {NULL};
static Melody* user_melody = NULL;

// Keyboard state
static uint32_t kb_last_press_ms = 0;
static int kb_last_index = -1;
static uint8_t kb_row_hits[4] = {0};
static uint8_t kb_col_hits[3] = {0};
static uint32_t mode_btn_last_ms = 0;
static uint8_t mode_btn_last = 1;
static uint8_t keyboard_test_mode = 0;

// Key mapping: indices 1..12 -> chars used by app
static const char kb_map_music[12] = {
	'1','2','3',
	'4','5','6',
	'7','8','9',
	':','0','\r'
};

// Scan matrix via I2C expander; return key index 1..12 or -1 if none/invalid
static int kb_scan_index(void) {
	const uint32_t now = HAL_GetTick();
	if (now - kb_last_press_ms < KB_KEY_DEBOUNCE_MS) {
		return -1;
	}

	uint8_t tmp = 0;
	uint8_t reg_buffer = 0xFF;
	int detected_index = -1;
	int found = 0;

	if (HAL_I2C_Mem_Write(&hi2c1, KB_I2C_WRITE_ADDRESS, KB_OUTPUT_REG, 1, &tmp, 1, KB_I2C_TIMEOUT) != HAL_OK) {
		return -1;
	}

	for (int row = 0; row < 4; row++) {
		uint8_t cfg = (uint8_t)~(1U << row);
		if (HAL_I2C_Mem_Write(&hi2c1, KB_I2C_WRITE_ADDRESS, KB_CONFIG_REG, 1, &cfg, 1, KB_I2C_TIMEOUT) != HAL_OK) {
			continue;
		}
		HAL_Delay(1); // settle
		if (HAL_I2C_Mem_Read(&hi2c1, KB_I2C_READ_ADDRESS, KB_INPUT_REG, 1, &reg_buffer, 1, KB_I2C_TIMEOUT) != HAL_OK) {
			continue;
		}

		uint8_t col_code = reg_buffer >> 4; // upper 4 bits are columns
		int col = -1;
		switch (col_code) {
			case 6: col = 0; break; // 0b0110
			case 5: col = 1; break; // 0b0101
			case 3: col = 2; break; // 0b0011
			default: col = -1; break;
		}

		if (col != -1) {
			found++;
			kb_row_hits[row]++;
			kb_col_hits[col]++;
			if (found == 1) {
				detected_index = row * 3 + col + 1; // 1..12
			}
		}
	}

	int row_sum = kb_row_hits[0]+kb_row_hits[1]+kb_row_hits[2]+kb_row_hits[3];
	int col_sum = kb_col_hits[0]+kb_col_hits[1]+kb_col_hits[2];
	memset(kb_row_hits, 0, sizeof(kb_row_hits));
	memset(kb_col_hits, 0, sizeof(kb_col_hits));

	// Если ни одной валидной клавиши — сбросить last и выйти
	if (found == 0) {
		kb_last_index = -1;
		return -1;
	}

	if (found != 1 || row_sum != 1 || col_sum != 1) {
		kb_last_index = -1;
		return -1; // multi-press or no key
	}

	if (detected_index == kb_last_index) {
		return -1; // held key, ignore repeat
	}

	kb_last_index = detected_index;
	kb_last_press_ms = now;
	return detected_index;
}

static char kb_poll_char(void) {
	int idx = kb_scan_index();
	if (idx <= 0 || idx > 12) return 0;
	return kb_map_music[idx - 1];
}

static void kb_clear_last(void) {
	kb_last_index = -1;
}

static void handle_mode_button(void) {
	uint8_t state = HAL_GPIO_ReadPin(MODE_BTN_PORT, MODE_BTN_PIN);
	uint32_t now = HAL_GetTick();
	if (state != mode_btn_last && (now - mode_btn_last_ms) > 50U) {
		mode_btn_last = state;
		mode_btn_last_ms = now;
		if (state == GPIO_PIN_RESET) { // release (active-low)
			keyboard_test_mode = !keyboard_test_mode;
			if (keyboard_test_mode) {
				print_string("Keyboard test mode ON\r\n");
			} else {
				print_string("Application mode ON\r\n");
			}
		}
	}
}


static Melody* parse_user_melody_string(const char* input) {
    if (!input || strlen(input) == 0) {
        return NULL;
    }

    size_t note_count = 1;
    for (const char* p = input; *p; p++) {
        if (*p == ',') note_count++;
    }

    Melody* melody = melody_create(note_count, "User melody");
    if (!melody) return NULL;

    char* input_copy = strdup(input);
    if (!input_copy) {
        melody_free(melody);
        return NULL;
    }

    char* token = strtok(input_copy, ",");
    size_t index = 0;

    while (token && index < note_count) {
        uint32_t frequency = 0, duration = 0, pause = 0;
        if (sscanf(token, "%lu:%lu:%lu", &frequency, &duration, &pause) >= 2) {
            melody->notes[index].frequency = frequency;
            melody->notes[index].duration = duration;
            melody->notes[index].pause = pause;
            index++;
        }
        token = strtok(NULL, ",");
    }

    melody->count = index;
    free(input_copy);
    return melody;
}

static void load_standard_melodies(void) {
    standard_melodies[0] = get_happy_birthday();
    standard_melodies[1] = get_jingle_bells();
    standard_melodies[2] = get_elochka();
    standard_melodies[3] = get_imperial_march();
}

int duration = 0;
int melody_pause = 0;
int i=0;
int melody_playing =0;
Melody* melody = NULL;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim) {
	if (htim->Instance != TIM6 || !melody_playing) {
		return;
	}

	if (melody_pause > 0) {
		melody_pause--;
		return;
	}

	if (melody->notes[i].frequency == 0) {
		htim1.Instance->CCR1 = 0;
	} else {
		htim1.Instance->ARR = 180000000 / ((melody->notes[i].frequency) * htim1.Instance->PSC) - 1;
		htim1.Instance->CCR1 = htim1.Instance->ARR >> 1;
	}
	
	duration++;
	
	if (duration >= melody->notes[i].duration) {
		duration = 0;
		i++;
		htim1.Instance->CCR1 = 0;
		
		if (i >= melody->count) {
			i = 0;
			melody_playing = 0;
		} else {
			melody_pause = melody->notes[i].pause;
		}
	}
}

static void play_melody_with_message() {
    print_string("Playing melody\r\n");

    i=0;
    duration = 0;
    melody_playing=1;
    melody_pause = 0;
}
char receive_input(void) {
    // 1) keypad first
    char key = kb_poll_char();
    if (key) {
    	return key;
    }

    // 2) UART (non-blocking)
    char c;
    HAL_StatusTypeDef status = HAL_UART_Receive(&huart6, (uint8_t*)&c, 1, 50);
    if (status == HAL_OK) {
        return c;
    }
    return 0;
}

static void handle_setup_menu(void) {
	print_string("=== Setup Menu ===\r\n");
	print_string("Enter notes one per line: frequency:duration[:pause]\r\n");
	print_string("Example: 440:500:100\r\n");
	print_string("Press Enter to add note; press Enter twice to finish\r\n");

	Note temp_notes[MAX_USER_NOTES];
	size_t note_count = 0;
	char note_buf[64] = {0};
	size_t note_len = 0;
	uint8_t last_enter = 0;

	while (1) {
		char c = receive_input();
		if (c == 0) continue;

		if (c == '\n' || c == '\r') {
			if (note_len == 0) {
				if (last_enter) {
					// finish input
					break;
				} else {
					last_enter = 1;
					print_string("\r\n"); // echo empty line
					continue;
				}
			}
			// parse current note
			note_buf[note_len] = '\0';
			uint32_t f=0,d=0,p=0;
			int scanned = sscanf(note_buf, "%lu:%lu:%lu", &f, &d, &p);
			if (scanned < 2) {
				print_string("\r\nError: note format must be freq:dur[:pause]\r\n");
				// reset current buffer
				note_len = 0;
				memset(note_buf, 0, sizeof(note_buf));
				last_enter = 0;
				continue;
			}
			if (note_count >= MAX_USER_NOTES) {
				print_string("\r\nError: too many notes\r\n");
				note_len = 0;
				memset(note_buf, 0, sizeof(note_buf));
				last_enter = 0;
				continue;
			}
			temp_notes[note_count].frequency = f;
			temp_notes[note_count].duration  = d;
			temp_notes[note_count].pause     = (scanned >= 3) ? p : 0;
			note_count++;

			print_string("\r\nNote added\r\n");

			// reset for next note
			note_len = 0;
			memset(note_buf, 0, sizeof(note_buf));
			last_enter = 0;
			continue;
		}
		
		if (note_len >= sizeof(note_buf) - 1) {
			print_string("\r\nError: note text too long\r\n");
			note_len = 0;
			memset(note_buf, 0, sizeof(note_buf));
			last_enter = 0;
			continue;
		}
		
		if (c >= 32 && c <= 126) {
			note_buf[note_len++] = c;
			char char_str[2] = {c, '\0'};
			print_string(char_str);
		}
	}

	// build melody
	if (note_count == 0) {
		print_string("Input is empty, exiting setup menu\r\n");
		return;
	}

	if (user_melody) {
		melody_free(user_melody);
	}
	user_melody = melody_create(note_count, "User melody");
	if (!user_melody) {
		print_string("Error: cannot allocate melody\r\n");
		return;
	}
	for (size_t k = 0; k < note_count; ++k) {
		user_melody->notes[k] = temp_notes[k];
	}
	user_melody->count = note_count;

	char count_str[32];
	sprintf(count_str, "%lu", (unsigned long)user_melody->count);
	print_string("User melody saved successfully! (");
	print_string(count_str);
	print_string(" notes)\r\n");
	print_string("Press '5' to play it\r\n");
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART6_UART_Init();
  MX_TIM6_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  print_string("=== Musical Box ===\r\n");
  print_string("Commands:\r\n");
  print_string("  '1' - Play Happy Birthday\r\n");
  print_string("  '2' - Play Jingle Bells\r\n");
  print_string("  '3' - Play V lesu rodilas elochka\r\n");
  print_string("  '4' - Play Imperial March\r\n");
  print_string("  '5' - Play user melody\r\n");
  print_string("  Enter - Setup menu\r\n");
  print_string("Ready!\r\n");

  load_standard_melodies();

  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  while (1)
  {
	handle_mode_button();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    char c = receive_input();
	if (c == 0) {
		continue;
	}

	if (keyboard_test_mode) {
		char buf[16];
		snprintf(buf, sizeof(buf), "Key: %c\r\n", c);
		print_string(buf);
		kb_clear_last();
		continue;
	}

	// Проверяем, не играет ли сейчас мелодия
	if (melody_playing) {
		continue;
	}

	if (c >= '1' && c <= '4') {
		int melody_index = c - '1';
		if (melody_index < MAX_MELODIES && standard_melodies[melody_index]) {
			melody = standard_melodies[melody_index];
			play_melody_with_message();
		}
	}
	else if (c == '5') {
		if (user_melody) {
			melody = user_melody;
			play_melody_with_message();
		} else {
			print_string("No user melody loaded. Use Enter to enter setup menu.\r\n");
		}
	}
	else if (c == '\n' || c == '\r') {
		handle_setup_menu();
	}
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
