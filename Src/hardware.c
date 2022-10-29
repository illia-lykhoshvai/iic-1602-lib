/*
 * hardware.c
 *
 *  Created on: 3 Sept. 2022
 *      Author: illialykhoshvai
 */

#include "stm32f4xx.h"

#include "main.h"
#include "hardware.h"

void initRCC(void);
void initWDG(void);
void initGPIO(void);
void initTIM(void);
void initI2C(void);

void initHW(void) {
	initRCC();
	initWDG();
	initGPIO();
	initTIM();
	initI2C();
}

void initRCC(void) {
// HSI is already enabled and ready
// configure PLL
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM_Msk + RCC_PLLCFGR_PLLN_Msk);
	RCC->PLLCFGR |= (8 << RCC_PLLCFGR_PLLM_Pos) + (80 << RCC_PLLCFGR_PLLN_Pos);
// turn on PLL
	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY_Msk));
// configure flash for 80mhz hclk
	FLASH->ACR |= FLASH_ACR_LATENCY_2WS;
// selects PLL as input to SysClk
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
	while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));
	SystemCoreClockUpdate();
}

void initWDG(void) {

}

void initGPIO(void) {
//  i2c1_scl => PB8
// 	i2c1_sda => PB9
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->AFR[1] |= ((4 << 0) + (4 << 4)); // AF4 for i2c-dedicated pins
	GPIOB->MODER &= ~(GPIO_MODER_MODE8 + GPIO_MODER_MODE9);
	GPIOB->MODER |= (GPIO_MODER_MODE8_1 + GPIO_MODER_MODE9_1);
	GPIOB->OTYPER |= (GPIO_OTYPER_OT8 + GPIO_OTYPER_OT9);
	GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR8 + GPIO_OSPEEDER_OSPEEDR9);
}

void initTIM(void) {
	// 5ms int. base time, tclk = 80mhz
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	TIM4->CR1 |= TIM_CR1_ARPE;
	TIM4->DIER |= TIM_DIER_UIE;
	TIM4->PSC = 800 - 1;
	TIM4->ARR = 100 - 1;
	TIM4->CR1 = TIM_CR1_CEN;
	NVIC_EnableIRQ(TIM4_IRQn);
}

void initI2C(void) { // I2C1 is used
/// master mode:
/// 1. program peripheral input clk in i2c_cr2 reg. to generate correct timings
/// 2. configure clock control reg.
/// 3. configure the rise time reg.
/// 4. program the i2c_cr1 reg. to enable the peripth
/// 5. set the start bit in the i2c_cr1 reg to generate start condit.
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // 40mhz @ APB1
	DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_I2C1_SMBUS_TIMEOUT;
	I2C1->CR2 |= (40 << I2C_CR2_FREQ_Pos);
#ifdef FAST_I2C_MODE // fm mode = 400khz
	I2C1->CCR = I2C_CCR_FS + I2C_CCR_DUTY + (4 << I2C_CCR_CCR_Pos);
	I2C1->TRISE = (8 + 1); // rise = 200ns
#else // sm mode = 100khz
	I2C1->CCR = 200;
	I2C1->TRISE = ((40/4) + 1); // rise = 250ns
#endif
	I2C1->CR1 |= I2C_CR1_PE;
}
