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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "usart.h"
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


void print_char(char c) {
	HAL_UART_Transmit(&huart6, (uint8_t *) &c, 1, 10);
}

void print_string(char* s) {
	int len = strlen(s);
	HAL_UART_Transmit(&huart6, (uint8_t *)s, len, 10);

}
// Глобальные переменные для мелодий
static Melody* standard_melodies[MAX_MELODIES] = {NULL};
static Melody* user_melody = NULL;

// Парсинг пользовательской мелодии из строки формата "440:500:100,880:250:50"
static Melody* parse_user_melody_string(const char* input) {
    if (!input || strlen(input) == 0) {
        return NULL;
    }

    // Подсчитываем количество нот (по количеству запятых + 1)
    size_t note_count = 1;
    for (const char* p = input; *p; p++) {
        if (*p == ',') {
            note_count++;
        }
    }

    // Создаем мелодию с названием "User melody"
    Melody* melody = melody_create(note_count, "User melody");
    if (!melody) {
        return NULL;
    }

    // Парсим строку
    char* input_copy = strdup(input);
    if (!input_copy) {
        melody_free(melody);
        return NULL;
    }

    char* token = strtok(input_copy, ",");
    size_t index = 0;

    while (token && index < note_count) {
        // Парсим формат "frequency:duration:pause"
        uint32_t frequency = 0, duration = 0, pause = 0;

        if (sscanf(token, "%u:%u:%u", &frequency, &duration, &pause) >= 2) {
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

// Загрузка стандартных мелодий
static void load_standard_melodies(void) {
    // Загружаем все захардкоженные мелодии
    standard_melodies[0] = get_happy_birthday();
    standard_melodies[1] = get_jingle_bells();
    standard_melodies[2] = get_elochka();
    standard_melodies[3] = get_imperial_march();
}

// Callback для завершения воспроизведения мелодии
static void melody_completion_callback(void* user_data) {
    const char* message = (const char*)user_data;
    if (message) {
        print_string(message);
        print_string("\r\n");
    }
}
int duration = 0;
int melody_pause = 0;
int i=0;
int melody_playing =0;
Melody* melody = NULL;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim) {
	if (htim->Instance == TIM6) {
		char freq_str[12];

		/*int freq = melody->notes[0].frequency;
						htim1.Instance->ARR = 90000000 / ((freq) * htim1.Instance->PSC) - 1;
						     			htim1.Instance->CCR1 = htim1.Instance->ARR >> 1;
						     			sprintf(freq_str, "%lu", freq);
						     			     			print_string(freq_str);
						     			return;*/
        //print_string("f");
		if (!melody_playing) {
            return;
        }

		char str[1];
		//print_string(sprintf(str, "%d",melody->notes[i].frequency));

		if (melody_pause > 0){
			melody_pause--;
		}else if(melody_pause == 0) {
			//print_string("r");
			if (melody->notes[i].frequency == 0) {
				// Выключаем звук
				htim1.Instance->CCR1 = 0; // 0%
				duration +=1;
				//print_string("m");
			} else {
				// Проигрываем ноты
				duration +=1;
				//int g= ;


				htim1.Instance->ARR = 90000000 / ((melody->notes[i].frequency) * htim1.Instance->PSC) - 1;
     			htim1.Instance->CCR1 = htim1.Instance->ARR >> 1;
     			//sprintf(freq_str, "%lu", htim1.Instance->ARR >> 1);
     			//print_string(freq_str);
			}
			if(duration==melody->notes[i].duration){
				duration=0;
				i++;
				htim1.Instance->CCR1 = 0;
				melody_pause = melody->notes[i].pause;
			}
			//print_string("i");
			if (i == melody->count){
                i = 0;
                //duration = 1;
                melody_playing= 0;
                // Выключаем звук
                htim1.Instance->CCR1 = 0; // 0%
            }
			print_char('\0');
		}
	}
}
// Воспроизведение мелодии с сообщением
static void play_melody_with_message() {
    // Используем имя мелодии, если оно есть

    print_string("Playing melody\r\n");

    i=0;
    duration = 0;
    melody_playing=1;
    melody_pause = 0;
    print_string("b");
    // Запускаем мелодию асинхронно, callback вызовется когда мелодия завершится
    //play_melody(melody, melody_completion_callback, (void*)"Playback finished");
}
char receive_input(void) {
    char c;
    HAL_UART_Receive(&huart6, (uint8_t*)&c, 1, HAL_MAX_DELAY);
    return c;
}
// Обработка меню настройки
static void handle_setup_menu(void) {
	print_string("=== Setup Menu ===\r\n");
	print_string("Enter melody in format: frequency:duration:pause,frequency:duration:pause\r\n");
	print_string("Example: 440:500:100,880:250:50\r\n");
	print_string("Press Enter to finish input\r\n");

    char input_buffer[MAX_INPUT_LENGTH] = {0};
    size_t input_index = 0;
    while (1) {

    	char c=receive_input();

        if (c == 0) {
            // Нет данных, продолжаем ожидание
            continue;
        }

        // Обработка Enter (завершение ввода)
        if (c == '\n' || c == '\r') {
            if (input_index > 0) {
                input_buffer[input_index] = '\0';

                if (user_melody) {
                    melody_free(user_melody);
                }

                user_melody = parse_user_melody_string(input_buffer);

                if (user_melody && user_melody->count > 0) {
                    print_string("User melody saved successfully! (");
                    char count_str[32];
                    sprintf(count_str, "%zu", user_melody->count);
                    print_string(count_str);
                    print_string(" notes)\r\n");
                    print_string("Press '5' to play it\r\n");
                } else {
                    print_string("Error: Invalid melody format\r\n");
                }

                break;
            } else {
                print_string("Input is empty, exiting setup menu\r\n");
                break;
            }
        }
        else if (input_index < MAX_INPUT_LENGTH - 1) {
            if (c >= 32 && c <= 126) {
                input_buffer[input_index++] = c;
                input_buffer[input_index] = '\0';
                char char_str[2] = {c, '\0'};
                print_string(char_str);
            }
        } else {
            print_string("Input buffer full\r\n");
            break;
        }
    }
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  HAL_TIM_Base_Start_IT(&htim1);
//  HAL_TIM_Base_Start_IT(&htim6);



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

  melody = standard_melodies[0];

  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  	  char c= receive_input();
	  	  if (c == 0) {
	  		  // Нет данных
	  		  continue;
	  	  }

	  	  // Проверяем, не играет ли сейчас мелодия
	  	  if (melody_playing) {
	  		  continue;
	  	  }

	  	  // Обработка команд
	  	  if (c >= '1' && c <= '4') {
	  		  int melody_index = c - '1';

	  		  if (melody_index >= 0 && melody_index < MAX_MELODIES && standard_melodies[melody_index]) {
	  			  melody = standard_melodies[melody_index];
	  			  play_melody_with_message();
	  		  } else {
	  			  char msg[64];
	  			  sprintf(msg, "Melody %d not available", melody_index + 1);
	  			  print_string(msg);
	  			  print_string("\r\n");
	  		  }
	  	  }
	  	  else if (c == '5') {
	  		  // Воспроизведение пользовательской мелодии
	  		  if (user_melody) {
	  			  melody= user_melody;
	  			  play_melody_with_message();
	  		  } else {
	  			  print_string("No user melody loaded. Use Enter to enter setup menu.\r\n");
	  		  }
	  	  }
	  	  else if (c == '\n' || c == '\r') {
	  		  // Вход в меню настройки (только если не играет мелодия)
	  		  if (melody_playing) {
	  			  print_string("Cannot enter setup menu while melody is playing\r\n");
	  		  } else {
	  			  handle_setup_menu();
	  		  }
	  	  }
	  	  else {
	  		  // Игнорируем неизвестные команды (не выводим сообщение)
	  		  // Это могут быть служебные символы или символы, которые не нужно обрабатывать
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
