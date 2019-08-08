/*
 * Dac.c
 *
 *  Created on: 1 èþí. 2019 ã.
 *      Author: Aleksey
 */

#include <main.h>
#include "stdio.h"
#include "main.h"
#include "Dac.h"

extern I2C_HandleTypeDef hi2c1;/*I2C1 handle*/
extern I2S_HandleTypeDef hi2s3;

uint8_t val, frequency = 0;
/* Set the Master volume */
void DacSetVol(uint8_t vol)
{
	uint8_t convVol = VOLUME_CONVERT(vol);

	if(vol > 0xE6)
	{
		WriteDacRegValue( 0x20, convVol - 0xE7);
		WriteDacRegValue( 0x21, convVol - 0xE7);
	}
	else
	{
		WriteDacRegValue( 0x20, convVol + 0x19);
		WriteDacRegValue( 0x21, convVol + 0x19);
	}
}

void DacInitialize()
{

	I2S3_Init();

	/*Recommended operations for power up*/
	RecommendedPowerUpSequence();
	//Power Control 2 (Address 04h)
	WriteDacRegValue(DAC_REG_Power_Ctl_2, 0xAF);

	//Playback Control 1 Headphone Analog Gain
	WriteDacRegValue(DAC_REG_Playback_Ctl_1, 0x70);

	//Clocking Control
	WriteDacRegValue(DAC_REG_Clocking_Ctl, 0x81);

	//Interface Control 1 (Address 06h)
	WriteDacRegValue(DAC_REG_Interface_Ctl_1, 0x07);

	//Analog ZC and SR Settings (Address 0Ah)
	WriteDacRegValue(DAC_REG_Analog_ZC_SR_Settings, 0);

	//Limiter Control 1, Min/Max Thresholds (Address 27h)
	WriteDacRegValue(DAC_REG_Limit_Ctl_1_Thresholds, 0);

	//PCMx Volume: PCMA (Address 1Ah)
	WriteDacRegValue(DAC_REG_PCMA_Vol, 0x0A);

	//PCMx Volume: PCMB (Address 1Bh)
	WriteDacRegValue(DAC_REG_PCMB_Vol, 0x0A);

	//Tone Control (Address 1Fh)
	WriteDacRegValue(DAC_REG_Tone_Ctl, 0x0F);

//	I2S_Cmd(SPI3, ENABLE);



//#define BEEP

#ifdef BEEP
	/*
	MSTxVOL[7:0]“Master Volume Control: MSTA (Address 20h) & MSTB (Address 21h)” on page 51
	PCMxVOL[6:0]“PCMx Volume: PCMA (Address 1Ah) & PCMB (Address 1Bh)” on page 47
	OFFTIME[2:0] “Beep Off Time” on page 48
	ONTIME[3:0]“Beep On Time” on page 48
	FREQ[3:0]“Beep Frequency” on page 47
	BEEP[1:0]“Beep Configuration” on page 49
	BEEPMIXDIS“Beep Mix Disable” on page 49
	BPVOL[4:0]“Beep Volume” on page 49
	*/

	/* Keep Codec powered OFF */
	WriteDacRegValue(0x02, 0x01);
	/* SPK always OFF & HP always ON */
	WriteDacRegValue(0x04, 0xAF);
    /* Set the Master volume */
	WriteDacRegValue(0x20, 0xE6);
	WriteDacRegValue(0x21, 0xE6);
	/* Enable the PassThrough on AIN1A and AIN1B */
	WriteDacRegValue(0x08, 0x01);
	WriteDacRegValue(0x09, 0x01);
	/* Route the analog input to the HP line */
	WriteDacRegValue(0x0E, 0xC0);
	/* Set the Passthough volume */
	WriteDacRegValue(0x14, 0x00);
	WriteDacRegValue(0x15, 0x00);
	//MSTxVOL[7:0]“Master Volume Control: MSTA (Address 20h) & MSTB (Address 21h)”
	WriteDacRegValue(0x20, 0x18);
	WriteDacRegValue(0x21, 0x18);
	//PCMxVOL[6:0]“PCMx Volume: PCMA (Address 1Ah) & PCMB (Address 1Bh)” on page 47
	WriteDacRegValue(0x1A, 0x18);
	WriteDacRegValue(0x1B, 0x18);
	//~1.23 s sec and -10 db
	WriteDacRegValue(DAC_REG_BEEP_Vol_Off_Time, 0x08);
	//~1.20 s and 888.89 Hz
	WriteDacRegValue(DAC_REG_BEEP_Freq_On_Time, 0x65);
	//Multiple mode
	WriteDacRegValue(DAC_REG_BEEP_Tone_Cfg, 0x80);
	// Power on the Codec
	WriteDacRegValue(DAC_REG_Power_Ctl_1, 0x9E);



#else
    WriteDacRegValue(0x00, 0x99);
    WriteDacRegValue(0x47, 0x80);
    WriteDacRegValue(DAC_REG_VP_Battery_Level, 0xFF);
    WriteDacRegValue(DAC_REG_VP_Battery_Level, 0x7F);
    WriteDacRegValue(0x00, 0x00);
    WriteDacRegValue(DAC_REG_Power_Ctl_2, 0xAF);
    WriteDacRegValue(DAC_REG_Playback_Ctl_1, 0x60);
    WriteDacRegValue(DAC_REG_Clocking_Ctl, 0x81);
    WriteDacRegValue(DAC_REG_Interface_Ctl_1, 0x04);
    WriteDacRegValue(DAC_REG_Analog_ZC_SR_Settings, 0x00);
    WriteDacRegValue(0x27, 0x00);
    WriteDacRegValue(DAC_REG_PCMA_Vol, 0x0A);
    WriteDacRegValue(DAC_REG_PCMB_Vol, 0x0A);

    /* Set the Speaker Mono mode */
    WriteDacRegValue(0x0F , 0x06);

    /* Disable the analog soft ramp */
    WriteDacRegValue(0x0A, 0x00);

   /* Disable the digital soft ramp */
    WriteDacRegValue(0x0E, 0x04);

   /* Disable the limiter attack level */
    WriteDacRegValue(0x27, 0x00);

   /* Adjust Bass and Treble levels */
    WriteDacRegValue(0x1F, 0x0F);

	//MSTxVOL[7:0]“Master Volume Control: MSTA (Address 20h) & MSTB (Address 21h)”
	WriteDacRegValue(0x20, 0xA9);
	WriteDacRegValue(0x21, 0xA9);

    WriteDacRegValue(DAC_REG_Tone_Ctl, 0x0F);
    WriteDacRegValue(DAC_REG_Power_Ctl_1, 0x9E);

#endif

		// Power on the Codec
	WriteDacRegValue(DAC_REG_Power_Ctl_1, 0x9E);
}

void I2S3_Init()
{

       hi2s3.Instance = SPI3;

        /* Disable I2S block */

       __HAL_I2S_DISABLE(&hi2s3);

	 hi2s3.Init.AudioFreq   = 48000;

	 hi2s3.Init.Standard    = 0;

       HAL_I2S_DeInit(&hi2s3);

      HAL_I2S_Init(&hi2s3);
}

//_____________________________________________________________________
//Write DAC selected register value
HAL_StatusTypeDef WriteDacRegValue( uint16_t reg, uint8_t value)
{
	return HAL_I2C_Mem_Write(&hi2c1, i2c_Dac_Adress, reg, I2C_MEMADD_SIZE_8BIT, &value,  1, 0x1000);
}
//_____________________________________________________________________
//Read DAC selected register value
uint8_t ReadDacRegValue(I2C_TypeDef* I2Cx, uint16_t reg)
{
	uint8_t value;
	if(HAL_I2C_Mem_Read(&hi2c1, i2c_Dac_Adress, reg,
			I2C_MEMADD_SIZE_8BIT, &value, 1, 0x1000) == HAL_OK)
		return value;
	else
		return 0xFF;
}
//_____________________________________________________________________
//Recomended operations for start the device
void RecommendedPowerUpSequence()
{
	/*Reset device*/
	HAL_GPIO_WritePin(GPIOD, GPIO_RESET_DAC, GPIO_PIN_SET);
	/*Wait for stability*/
	HAL_Delay(100);

	WriteDacRegValue(DAC_REG_Playback_Ctl_1, 0x01); //Headphone Analog Gain

	WriteDacRegValue(DAC_REG_NULL, 0x99);

	WriteDacRegValue(0x47, 0x80);

	WriteDacRegValue(0x32, 0xFF);

	WriteDacRegValue(0x32, 0x7F);

	WriteDacRegValue(DAC_REG_NULL, 0);
}
