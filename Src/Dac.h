/*
 * Dac.h
 *
 *  Created on: 1 θών. 2019 γ.
 *      Author: Aleksey
 */

#ifndef DAC_H_
#define DAC_H_

#define VOLUME_CONVERT(Volume)    (((Volume) > 100)? 100:((uint8_t)(((Volume) * 255) / 100)))

#define GPIO_RESET_DAC GPIO_PIN_4


#define i2c_Dac_Adress	 ((uint16_t)(0x94))
#define gpio_Audio_Sda 	 GPIO_Pin_9
#define gpio_Audio_Scl 	 GPIO_Pin_6

#define gpioAF_Audio_Sda 	 GPIO_PinSource9
#define gpioAF_Audio_Scl 	 GPIO_PinSource6

typedef enum
{
	I2C_10KHz = 10000,
	I2C_50KHz = 50000,
	I2C_100KHz = 100000

}I2C_FREQUENCY;

typedef enum
{
	DAC_REG_NULL					= 0x00,
	DAC_REG_ID	 					= 0x01,
	DAC_REG_Power_Ctl_1 		    = 0x02,
	//Reserved 0x03
	DAC_REG_Power_Ctl_2 			= 0x04,
	DAC_REG_Clocking_Ctl			= 0x05,
	DAC_REG_Interface_Ctl_1			= 0x06,
	DAC_REG_Interface_Ctl_2			= 0x07,
	DAC_REG_Passthrough_A_Select	= 0x08,
	DAC_REG_Passthrough_B_Select	= 0x09,
	DAC_REG_Analog_ZC_SR_Settings	= 0x0A,
	//Reserved 0x0B
	DAC_REG_Passthrough_Gang_Control = 0x0C,
	DAC_REG_Playback_Ctl_1			 = 0x0D,
	DAC_REG_Misc_Ctl				 = 0x0E,
	DAC_REG_Playback_Ctl_2			 = 0x0F,
	DAC_REG_Passthrough_A_Vol 		 = 0x14,
	DAC_REG_Passthrough_B_Vol 		 = 0x15,
	DAC_REG_PCMA_Vol 				 = 0x1A,
	DAC_REG_PCMB_Vol				 = 0x1B,
	DAC_REG_BEEP_Freq_On_Time		 = 0x1C,
	DAC_REG_BEEP_Vol_Off_Time		 = 0x1D,
	DAC_REG_BEEP_Tone_Cfg			 = 0x1E,
	DAC_REG_Tone_Ctl 				 = 0x1F,
	DAC_REG_Master_A_Vol,
	DAC_REG_Master_B_Vol,
	DAC_REG_Headphone_A_Volume,
	DAC_REG_Headphone_B_Volume,
	DAC_REG_Speaker_A_Volume,
	DAC_REG_Speaker_B_Volume,
	DAC_REG_Channel_Mixer_Swap,
	DAC_REG_Limit_Ctl_1_Thresholds,
	DAC_REG_Limit_Ctl_2_Release_Rate,
	DAC_REG_Limiter_Attack_Rate,
	DAC_REG_Overflow_Clock_Status = 0x2E,
	DAC_REG_Battery_Compensation,
	DAC_REG_VP_Battery_Level,
	DAC_REG_Speaker_Status,
	DAC_REG_Charge_Pump_Frequency = 0x34

}DAC_CS43L22_REG_NAME;

void DacInitialize();
void DacSetVol(uint8_t vol);
void DacChangeFreq();
void DacInitI2C();
void DacInitGpio();
void I2S3_Init();

void RecommendedPowerUpSequence();
HAL_StatusTypeDef WriteDacRegValue( uint16_t reg, uint8_t value);
uint8_t ReadDacRegValue(I2C_TypeDef* I2Cx, uint16_t reg);

#endif /* DAC_H_ */
