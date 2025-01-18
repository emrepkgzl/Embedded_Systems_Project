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
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t Rx_data[8];
uint8_t watering_time;
uint8_t instant_watering_time = 1;
uint8_t waiting_time;
char message[3];
uint32_t channel1, channel2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM6_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM7_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Mikrosaniye mertebesinde bekleme yapan fonksiyonu tanimla */
void delay (uint16_t time)
{
	/* Timer sayacini sifirla */
	__HAL_TIM_SET_COUNTER(&htim6, 0);
	/* Sayac fonksiyona girilen degere ulasana kadar bekle */
	while ((__HAL_TIM_GET_COUNTER(&htim6))<time);
}

/* DHT sensorundan gelen verileri okumak icin degiskenleri olustur */
uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t SUM, RH, TEMP;

float Temperature = 0;
float Humidity = 0;
uint8_t Presence = 0;

/* Istenilen pini output olarak ayarlamak icin gerekli fonksiyonu tanimla */
void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

/* Istenilen pini input olarak ayarlamak icin gerekli fonksiyonu tanimla */
void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

/* DHT sicaklik ve nem sensorunun bagli oldugu pin ve portu tanimla */
#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_1

/* DHT sensorunun yanit vermesi icin gerekli verileri gonderecek fonksiyonu tanimla */
void DHT11_Start (void)
{
	/* DHT ye bagli pini output olarak ayarla */
	Set_Pin_Output (DHT11_PORT, DHT11_PIN);
	/* Pine logic low degeri gonder */
	HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 0);
	/* DHT satasheet inde belirtildigi uzere 18ms bekle */
	delay (18000);
	/* Pine logic high degeri gonder */
    HAL_GPIO_WritePin (DHT11_PORT, DHT11_PIN, 1);
    /* 20 mikrosaniye bekle */
	delay (20);
	/* DHT ye bagli pini input olarak ayarla */
	Set_Pin_Input(DHT11_PORT, DHT11_PIN);
}

/* DHT sensorunun yanitini kontrol edecek fonksiyonu tanimla */
uint8_t DHT11_Check_Response (void)
{
	uint8_t Response = 0;
	/* 40 mikrosaniye bekle */
	delay (40);
	/* Pin degeri logic low olursa asagida verilenleri yap */
	if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))
	{
		/* 80 mikrosaniye bekle */
		delay (80);
		/* Alinan yaniti degiskene kaydet */
		if ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN))) Response = 1;
		else Response = -1;
	}
	/* Pin degeri logic low olana kadar bekle */
	while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)));

	/* Yanit degerini geri dondur */
	return Response;
}

/* DHT sensorunden gelen verileri okuyacak fonksiyonu tanimla */
uint8_t DHT11_Read (void)
{
	/* Gerekli degiskenleri tanimla */
	uint8_t i,j;
	for (j=0;j<8;j++)
	{
		/* Pin degeri logic low olana kadar bekle */
		while (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)));
		/* 40 mikrosaniye bekle */
		delay (40);
		/* Pin degeri logic low olursa asagida verilenleri yap */
		if (!(HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)))
		{
			/* Logic low degeri geldiginde dongu degerine gore biti kaydirip logic and ile kaydet */
			i&= ~(1<<(7-j));
		}
		/* Logic high degeri geldiginde dongu degerine gore biti kaydirip logic or ile kaydet */
		else i|= (1<<(7-j));
		/* Pin degeri logic high olana kadar bekle */
		while ((HAL_GPIO_ReadPin (DHT11_PORT, DHT11_PIN)));
	}
	/* Elde edilen degeri geri dondur */
	return i;
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
  MX_TIM6_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  /* Timer, uart kesmesi ve adc kesmesini baslat */
  HAL_TIM_Base_Start(&htim6);
  HAL_UART_Receive_IT(&huart2, Rx_data, 3);
  HAL_ADC_Start_IT(&hadc1);

  /* Role pinlerini logic high seviyesine al */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_SET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	    /* DHT sensorunu baslat */
	    DHT11_Start();
	    /* Sensor yanitini kontrol et */
	    Presence = DHT11_Check_Response();
	    /* Gelen sicaklik ve nem degerlerini degiskenlere kaydet */
	    Rh_byte1 = DHT11_Read ();
	    Rh_byte2 = DHT11_Read ();
	    Temp_byte1 = DHT11_Read ();
	    Temp_byte2 = DHT11_Read ();

	    /* 1,5sn bekle */
	   	HAL_Delay(1500);

	   	/* Gonderilen verilerin anlasilmasi icin basina veriye ozel byte degeri ekle */
		message[0] = 123;
		/* Integer bolmesi yaparak iki basamakli degerin onlar basamagini diziye kaydet */
		message[1] = (channel1 / 10);
		/* Mod alarak iki basamakli degerin onlar basamagini diziye kaydet */
		message[2] = (channel1 % 10);

		/* 1,5sn bekle */
		HAL_Delay(1500);

		/* Diziyi UART ile ESP tarafina gonder */
		HAL_UART_Transmit(&huart2, (uint8_t*)message, 3, HAL_MAX_DELAY);

		/* Gonderilen verilerin anlasilmasi icin basina veriye ozel byte degeri ekle */
		message[0] = 124;
		/* Integer bolmesi yaparak iki basamakli degerin onlar basamagini diziye kaydet */
		message[1] = (channel2 / 10);
		/* Mod alarak iki basamakli degerin onlar basamagini diziye kaydet */
		message[2] = (channel2 % 10);

		/* Diziyi UART ile ESP tarafina gonder */
		HAL_UART_Transmit(&huart2, (uint8_t*)message, 3, HAL_MAX_DELAY);

		/* 1,5sn bekle */
	   	HAL_Delay(1500);
	   	HAL_ADC_Start_IT(&hadc1);

	   	/* Gonderilen verilerin anlasilmasi icin basina veriye ozel byte degeri ekle */
	   	message[0] = 125;
	   	/* Integer bolmesi yaparak iki basamakli degerin onlar basamagini diziye kaydet */
	   	message[1] = (Rh_byte1 / 10);
	   	/* Mod alarak iki basamakli degerin onlar basamagini diziye kaydet */
	   	message[2] = (Rh_byte1 % 10);

	   	/* 1,5sn bekle */
	   	HAL_Delay(1500);

	   	/* Diziyi UART ile ESP tarafina gonder */
	   	HAL_UART_Transmit(&huart2, (uint8_t*)message, 3, HAL_MAX_DELAY);

	   	/* Gonderilen verilerin anlasilmasi icin basina veriye ozel byte degeri ekle */
		message[0] = 126;
		/* Integer bolmesi yaparak iki basamakli degerin onlar basamagini diziye kaydet */
		message[1] = (Temp_byte1 / 10);
		/* Mod alarak iki basamakli degerin onlar basamagini diziye kaydet */
		message[2] = (Temp_byte1 % 10);

		/* Diziyi UART ile ESP tarafina gonder */
	    HAL_UART_Transmit(&huart2, (uint8_t*)message, 3, HAL_MAX_DELAY);

	    /* UART kesmesini tekrardan aktiflestir */
	    HAL_UART_Receive_IT(&huart2, Rx_data, 3);
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
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 50-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 0xffff-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 60000-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 50000-1;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PE10 PE11 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    /* Conversion modda pinleri okumak icin static sayac degiskenini olustur */
	static uint8_t counter = 1;
	if(counter == 0)
	{
		/* 12 bitlik adc degerini yuzde olarak hesaplayip channel1 degiskenine kaydet */
		channel1 = 100 - ((HAL_ADC_GetValue(&hadc1)*10)/409);
		counter = 1;
	}
	else
	{
		/* 12 bitlik adc degerini yuzde olarak hesaplayip channel2 degiskenine kaydet */
		channel2 = 100 - ((HAL_ADC_GetValue(&hadc1)*10)/409);
		counter = 0;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	/* interrupti tekrardan aktiflestir */
	HAL_UART_Receive_IT(&huart2, Rx_data, 3);
	/* hangi limit oldugunu kontrol et */
	if((Rx_data[0] - '0') == 1)
	{
		/* char dan int e donustur */
		/*watering_time = (Rx_data[1] - '0') * 10;
		watering_time += (Rx_data[2] - '0');*/
	}
	else if((Rx_data[0] - '0') == 2)
	{
		/* char dan int e donustur */
		/*waiting_time = (Rx_data[1] - '0') * 10;
		waiting_time += (Rx_data[2] - '0');
		if(waiting_time != 0)
		{
			__HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
			__HAL_TIM_SET_COUNTER(&htim7, 0);
			HAL_TIM_Base_Start_IT(&htim7);
		}*/
	}
	else if((Rx_data[0] - '0') == 3)
	{
		/* char dan int e donustur */
		/*instant_watering_time = (Rx_data[1] - '0') * 10;
		instant_watering_time += (Rx_data[2] - '0');*/
	}
	else if((Rx_data[0] - '0') == 4)
	{
		if((Rx_data[1] - '0') == 1)
		{
			//do instant watering
			/*HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_RESET);
			HAL_Delay(instant_watering_time * 1000);
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);*/
		}
		else
		{
			//do nothing
		}
		if((Rx_data[2] - '0') == 1)
		{
			//turn on fan
			//HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
		}
		else
		{
			//turn off fan
			//HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
		}
	}

	//HAL_UART_Receive_IT(&huart2, Rx_data, 3);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
	waiting_time--;
	if(waiting_time <= 0)
	{
		__HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
		__HAL_TIM_SET_COUNTER(&htim7, 0);
		HAL_TIM_Base_Stop_IT(&htim7);
	}
}
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

#ifdef  USE_FULL_ASSERT
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
