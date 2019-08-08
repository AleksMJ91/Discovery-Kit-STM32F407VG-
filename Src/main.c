/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "i2s.h"
#include "spi.h"
#include "usb_host.h"
#include "gpio.h"

#include "Dac.h"

void SystemClock_Config(void);
void MX_USB_HOST_Process(void);
void InitTIM6();

_Bool MountUsbFlash();
_Bool OpenRootDirectrory();
_Bool ReadFileInfo(FILINFO* fileInfo);
_Bool IsNotWave(const char fileName[13]);

uint32_t FileRead( uint16_t* buf, uint32_t sizeBuf, FSIZE_t offset);
_Bool OpenFileRead(const char fileName[100]);

#define AUDIO_SAMPLES_AMOUNT	8192
uint16_t Audio_Buffer[AUDIO_SAMPLES_AMOUNT];
uint16_t* bufP = Audio_Buffer;
uint16_t bufferCount = 0;

volatile uint32_t readedBytes;
_Bool dmaTransferFinish = 0;

uint8_t DacVol = 10;
//**********User button*************************
int btnPressCnt = 0;
_Bool BtnClickLong = FALSE;
_Bool BtnClickShort = FALSE;

//*********************USB**********************
extern ApplicationTypeDef Appli_state;
FATFS USBDISKFatFs;           /* File system object for USB disk logical drive */
FIL MyFile;                   /* File object */
FILINFO fInfo;
DIR	pDir;
volatile FSIZE_t fileOffset;
//**********************************************

//Обработка прерывания таймера TIM6
void TIM6_DAC_IRQHandler(void)
{
    TIM6->SR &= ~TIM_SR_UIF; //Сбрасываем флаг прерывания

    if(HAL_GPIO_ReadPin(GPIOA, USER_BUTON) == GPIO_PIN_SET)
    	btnPressCnt ++;
    else
    {
    	if(btnPressCnt >= 2)
    		BtnClickShort = TRUE;

    	btnPressCnt = 0;
    }

     if(btnPressCnt >= 6)
    	BtnClickLong  = TRUE;
}
//DMA TX IRQ
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s3)
{
	 if(hi2s3->Instance == SPI3)
	 {
		 dmaTransferFinish = 1;
		 HAL_GPIO_TogglePin(GPIOD, GREEN_LED);
	 }
}
//DMA 1/2 TX IRQ
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
//	 if(hi2s->Instance == SPI3)
//	 {
//		 audioProcessEnd = 1;
////		 //Первая половина отыгрына - играем вторую
//		 bufP = Audio_Buffer;
//		 HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_15);
//		 HAL_I2S_Transmit_DMA(hi2s, &Audio_Buffer[readedBytes / 2], readedBytes / 2);
//	 }
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  // MCU Configuration
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_USB_HOST_Init();
  InitTIM6();

  DacInitialize();

  // wait for preparing usb otg
  while(Appli_state != APPLICATION_READY)
    MX_USB_HOST_Process();

  //Open  usb, read root directory
  if(!MountUsbFlash() || !OpenRootDirectrory())
  {
    	//error detected, need to restart
    	HAL_GPIO_WritePin(GPIOD, GREEN_LED, GPIO_PIN_SET);
    	//Infinite loop
    	while(1)
    		;
  }
  //Infinite loop
  while(1)
  {
    //take next file info
    ReadFileInfo(&fInfo);
    //we get the last file in list, need to return to start
    if( fInfo.fname[0] == 0)
    {
    	ReadFileInfo(NULL);
        //take next file info
        ReadFileInfo(&fInfo);
    }

    if(IsNotWave(fInfo.fname))
    	continue;

    if(!OpenFileRead( fInfo.fname))
    	continue;

    readedBytes = FileRead(Audio_Buffer, AUDIO_SAMPLES_AMOUNT * 2, 0);
	fileOffset  = readedBytes;
	 //отправляем играть 1 половину массива
	HAL_I2S_Transmit_DMA(&hi2s3, Audio_Buffer, AUDIO_SAMPLES_AMOUNT/ 2);
	bufP = &Audio_Buffer[AUDIO_SAMPLES_AMOUNT/ 2];
	bufferCount = 1;
    //loop for file size
    while(!f_eof(&MyFile))
    {
		 MX_USB_HOST_Process();

		 if(BtnClickLong)
		 {
			 HAL_GPIO_TogglePin(GPIOD, BLUE_LED);
			 BtnClickLong = FALSE;
			 DacSetVol(++ DacVol);
		 }
		 if(BtnClickShort)
		 {
			 HAL_GPIO_TogglePin(GPIOD, RED_LED);
			 BtnClickShort = FALSE;

			 DacSetVol(-- DacVol);
		 }


		 if(dmaTransferFinish)
		 {
			 //Blink the orange led
			 HAL_GPIO_TogglePin(GPIOD, ORANGE_LED);
			 //reset flag
			 dmaTransferFinish  = 0;
			 //Continue transferring
			 HAL_I2S_Transmit_DMA(&hi2s3, bufP, AUDIO_SAMPLES_AMOUNT/ 2);
			 if(bufferCount == 1)
			 {
				 bufP = &Audio_Buffer[AUDIO_SAMPLES_AMOUNT/ 2];
				 bufferCount = 0;
			 }
			 else
			 {
				 bufP = &Audio_Buffer[0];
				 bufferCount = 1;
			 }

			 readedBytes = FileRead(bufP, AUDIO_SAMPLES_AMOUNT, fileOffset);
			 fileOffset += readedBytes;
		 }
    }

    //close current file
    f_close(&MyFile);
  }
}

//_______________________________________________________________
//Инициализация TIM6 (прерывания)
void InitTIM6()
{
	__HAL_RCC_TIM6_CLK_ENABLE();

	//По умолчанию частота шины 24 МГц при использовании кварца 8 МГц
	//Настройка делителя на 1000 "тиков" в секунду
	TIM6->PSC   = 24000 - 1;// всегда минус 1
	//Отработка прерывания раз в секунду
	TIM6->ARR   = 300;//1000;
	//Разрешения прерывание от таймера
	TIM6->DIER |= TIM_DIER_UIE;
	//Запуск таймера
	TIM6->CR1  |= TIM_CR1_CEN;
	//Разрешение TIM6_DAC_IRQn прерывания
	NVIC_EnableIRQ(TIM6_DAC_IRQn);
}

uint32_t FileRead( uint16_t* buf, uint32_t sizeBuf, FSIZE_t offset)
{
                                       /* FatFs function common result code */
	  uint32_t bytesread = sizeBuf;                     /* File write/read counts */

	if(offset)
		f_lseek(&MyFile, offset);

	 f_read(&MyFile, buf, sizeBuf, (void *)&bytesread);

	 return bytesread;
}


_Bool MountUsbFlash()
{
	return (f_mount(&USBDISKFatFs, (TCHAR const*)USBHPath, 0) == FR_OK);
}

_Bool OpenRootDirectrory()
{
	return ( f_opendir(&pDir, "/") == FR_OK);
}

_Bool ReadFileInfo(FILINFO* fileInfo)
{
	return (f_readdir(&pDir, fileInfo) == FR_OK);
}

_Bool IsNotWave(const char fileName[13])
{
	for(int i = 0; i < 13; i ++)
		if(fileName[i] == '.')
			if((fileName[i + 1] == 'w' && fileName[i + 2] == 'a' && fileName[i + 3] == 'v') ||
					(fileName[i + 1] == 'W' && fileName[i + 2] == 'A' && fileName[i + 3] == 'V'))
				return FALSE;

	return TRUE;
}

_Bool OpenFileRead(const char fileName[100])
{
	FRESULT res = FR_DISK_ERR;              /* FatFs function common result code */
	res = f_open(&MyFile, fileName, FA_READ);

	return (res == FR_OK)? 1 : 0;
}
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
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
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 192;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
