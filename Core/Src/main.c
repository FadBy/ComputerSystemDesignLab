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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//#define TICK_TIME 50
//#define LONG_PRESS_TIME 300

//int press_duration = 0;
//int is_pressed = 0;

enum InputType {
	FirstNumber,
	SecondNumber
};

enum InputType current_input_type = FirstNumber;

int first_number = 0;
int second_number = 0;
char operator;
int result;
char current_char;

short convert_char_to_short(char c) {
	return c - '0';
}

void process_non_digit_error() {
	// TODO turn on red led
}

int is_operator_sign(char c) {
	return c == '+' || c == '-' || c == '/' || c == '*';
}

int is_equal_sign(char c) {
	return c == '=';
}

void process_input() {
	switch (current_input_type) {
		case FirstNumber:
			if (is_operator_sign(current_char)) {
				if (first_number > 0) {
					process_operator_sign(current_char);
					print_char(current_char);
				}
			}
			else if (isdigit(current_char)) {
				if (first_number < 10000) {
					increase_number(current_char, &first_number);
					print_char(current_char);
				}
			}
			break;
		case SecondNumber:
			if (is_equal_sign(current_char)) {
				if (second_number > 0) {
					print_char(current_char);
					process_result(operator);
				}
			}
			else if (isdigit(current_char)) {
				if (second_number < 10000) {
					increase_number(current_char, &second_number);
					print_char(current_char);
				}
			}
			break;
	}	
}

void process_operator_sign(char c) {
	operator = c;
	current_input_type = SecondNumber;
}

void process_result(char operator) {
	switch (operator) {
		case '+':
			result = first_number + second_number;
			break;
		case '-':
			result = first_number - second_number;
			break;
		case '*':
			result = first_number * second_number;
			break;
		case '/':
			result = first_number / second_number;
			break;
		default:
			break;
	}
	first_number = 0;
	second_number = 0;
	current_input_type = FirstNumber;
	print_result(result);
	print_newline();
}

void increase_number(char c, int* number) {
	int digit = convert_char_to_short(c);
	*number = *number * 10;
	*number = *number + digit;
}

void print_char(char c) {
	HAL_UART_Transmit(&huart6, (uint8_t *) &c, 1, 10);
}

void print_result(int r) {
    char s[20];
    int len = sprintf(s, "%d", r);
    HAL_UART_Transmit(&huart6, (uint8_t *)s, len, 10);
}

void print_newline() {
	print_char('\n');
}

void receive_input() {
	HAL_UART_Receive(&huart6, (uint8_t *) &current_char, 1, 1);
}

int input_received() {
	return current_char != '\0';
}

//int counter = 0;

//int overcrowd = 0;
//void overcrowd_visual(){
//	if (counter == -1) {
//		counter = 3;
//		if (overcrowd > 0) {
//			overcrowd--;
//		}
//	}
//	else if (counter == 4) {
//		counter = 0;
//		overcrowd++;
//	}
//	else {
//		return;
//	}
//	if (overcrowd == 0) {
//		return;
//	}
//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13 | GPIO_PIN_14, GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13 | GPIO_PIN_14, GPIO_PIN_SET);
//	HAL_Delay(250);
//	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_RESET);
//	for(int i=0; i<overcrowd;i++){
//		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
//		HAL_Delay(250);
//		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
//		HAL_Delay(250);
//	}
//	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13, GPIO_PIN_RESET);
//}
//
//int get_long_press() {
//	return press_duration >= 300 && !is_pressed;
//}
//
//int get_short_press() {
//	return press_duration > 0 && press_duration < 300 && !is_pressed;
//}
//
//void sync_button_state() {
//	int button_state = !HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
//
//	if (is_pressed) {
//		press_duration += TICK_TIME;
//	}
//	else {
//		press_duration = 0;
//	}
//
//	is_pressed = button_state;
//}
//
//void process_increment() {
//	counter++;
//}
//
//void process_decrement() {
//	counter--;
//}
//
//void display_counter() {
//	int small_bit = counter % 2;
//	int big_bit = (counter / 2) % 2;
//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, small_bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, big_bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
//}

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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
	  receive_input();
	  if (input_received()) {
		  process_input();
	  }
	  current_char = '\0';
//	  HAL_UART_Transmit(&huart6, (uint8_t *) s, sizeof(s), 10);
//	  HAL_Delay(1000);
//	  sync_button_state();
//	  if (get_short_press()) {
//		  process_increment();
//		  overcrowd_visual();
//	  }
//	  if (get_long_press()) {
//		  process_decrement();
//		  overcrowd_visual();
//	  }
//	  display_counter();
//	  HAL_Delay(TICK_TIME);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
