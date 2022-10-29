/*
 * hardware.h
 *
 *  Created on: 3 Sept. 2022
 *      Author: illialykhoshvai
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_

//#define FAST_I2C_MODE

void initHW(void);

#define CLEAR_TIM4_IFLAG do { TIM4->SR &= ~TIM_SR_UIF; } while(0)

#endif /* HARDWARE_H_ */
