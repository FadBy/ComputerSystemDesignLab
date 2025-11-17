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
#include "usart.h"
#include "gpio.h"

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

enum InputType {
	FirstNumber,
	SecondNumber
};

enum InputType current_input_type = FirstNumber;

int first_number = -1;
int second_number = -1;
char operator;
int result;
char current_char;

int press_duration = 0;
int is_pressed = 0;

int interrupt_enable = 0;

int uart6_receive_finished =0;
int uart6_transmit_ongoing;
static uint8_t uart6_buf;

int transmit_busy = 0;
char transmit_buf[1024];
uint8_t transmit_ptr = 0;
uint8_t transmit_cur = 0;
int t=0;
HAL_StatusTypeDef uart6_start_receive_char_it() {
	uart6_receive_finished =0;
	return HAL_UART_Receive_IT(&huart6, &uart6_buf,1);
}

int uart6_try_get_receive_char(uint8_t *buf) {
	if(uart6_receive_finished) {
		*buf = uart6_buf;
		return 1;
	}
	return 0;
}

HAL_StatusTypeDef uart6_transmit_it(uint8_t *buf, int len) {
	while(uart6_transmit_ongoing);
	uart6_transmit_ongoing = 1;
	return HAL_UART_Transmit_IT(&huart6, buf, len);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if(huart->Instance == USART6)
	{
		uart6_receive_finished = 1;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (transmit_ptr > transmit_cur) {
			HAL_UART_Transmit_IT(&huart6, (uint8_t*) transmit_buf + transmit_cur, sizeof(char));
			transmit_cur++;
		} else {
			transmit_ptr = 0;
			transmit_cur = 0;
			transmit_busy = 0;
		}
}

int error_check(){
	if(first_number>=65536 || second_number>=65536 || result>=32768 || result<=-32768){
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
		print_error();
		HAL_Delay(250);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
		reset_equation();
		return 0;
	}
	return 1;
}

short convert_char_to_short(char c) {
	return c - '0';
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
			if (is_operator_sign(current_char) && first_number!=-1) {
				process_operator_sign(current_char);
				print_char(current_char);
			}
			else if (isdigit(current_char)) {
				if (first_number < 10000) {
					increase_number(current_char, &first_number);
					print_char(current_char);
				}
			}
			error_check();
			break;
		case SecondNumber:
			if (is_equal_sign(current_char) && second_number!=-1) {
				process_result(operator);
				print_char(current_char);

				if (!error_check()) {
					return;
				}
				if (second_number == 0 && operator == '/') {
					second_number = 99999;
					error_check();
					return;
				}
				print_result(result);
				print_newline();
				reset_equation();
			}
			else if (isdigit(current_char)) {
				if (second_number < 10000) {
					increase_number(current_char, &second_number);
					print_char(current_char);
				}
			}
			error_check();
			break;
	}
}

void switch_mode() {
	reset_equation();
	print_string("Mode switched", 13);
	print_newline();
	t=1;
	interrupt_enable = interrupt_enable == 0 ? 1 : 0;
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
}

void reset_equation() {
	first_number = -1;
	second_number = -1;
	result = 0;
	current_input_type = FirstNumber;
	operator = '+';
}

void increase_number(char c, int* number) {
	int digit = convert_char_to_short(c);
	if(*number == -1){
		*number = 0;
	}
	*number = *number * 10;
	*number = *number + digit;
}

void print_char(char c) {
	if (interrupt_enable) {
		if (!transmit_busy) {
			HAL_UART_Transmit_IT(&huart6, (uint8_t*) &c, sizeof(char));
			transmit_busy = 1;
		}
		else transmit_buf[transmit_ptr++] = c;
	 } else {
	  HAL_UART_Transmit(&huart6, (uint8_t *) &c, 1, 10);
	 }
}

void print_result(int r) {
    char s[20];
    int len = sprintf(s, "%d", r);
    print_string(s, len);
}

void print_string(char* s, int len) {
    if (interrupt_enable) {
    	for(int i=0;i<len;i++){
    		print_char(s[i]);
    	}
    } else {
    	HAL_UART_Transmit(&huart6, (uint8_t *)s, len, 10);
    }
}

void print_error() {
	print_newline();
	print_string("error", 5);
	print_newline();
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

int get_press() {
	return press_duration > 0  && !is_pressed;
}

void receive_button_state() {
	int button_state = !HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);

	if (is_pressed) {
		press_duration += HAL_GetTick();
	}
	else {
		press_duration = 0;
	}

	is_pressed = button_state;
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  char d[] = "Hello world\n";
  char c;

  while (1) {
	  receive_button_state();
	  if (get_press()) {
		  switch_mode();
	  }
	  if(interrupt_enable==0){
	      receive_input();
	        if (input_received()) {
	           process_input();
	        }
	     } else{
	    	 if(t==1){
		    	 uart6_start_receive_char_it();
		    	 t=0;
	    	 } else{
			  if(uart6_try_get_receive_char((uint8_t *) &current_char ) )
					 uart6_start_receive_char_it();
			 }

			if (input_received()) {
			  process_input();
			}

	  }
	  current_char = '\0';

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
