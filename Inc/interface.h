/*
 * interface.h
 *
 *  Created on: 4 sept. 2022 y.
 *      Author: illialykhoshvai
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include "pt/pt.h"

#define CNTRL_1 0
#define DATA 4
#define STATUS_1 5
#define STATUS_2 6

typedef enum {
	write,
	read
} addressEnding_t;

typedef enum {
	ackFailure,
	ackComplete
} ackResult_t;

// low-level
PT_THREAD(startCondtionPT(struct pt* pt, uint32_t* i2c));
PT_THREAD(sendBytePT(struct pt* pt, uint32_t* i2c, uint8_t data));
PT_THREAD(sendArrayPT(struct pt* pt, uint32_t* i2c, uint8_t* data, uint8_t length));
PT_THREAD(sendAdressPT(struct pt* pt, uint32_t* i2c, uint8_t* returnValue, uint8_t address, addressEnding_t operation));
PT_THREAD(readArrayPT(struct pt* pt, uint32_t* i2c, uint8_t* rByte, uint8_t length));
// C-C-C-Combo
// universal functions
PT_THREAD(iicWritePT(struct pt* pt, uint32_t* i2c, uint8_t slaveAddress, uint8_t* regToWrite, uint8_t length));
PT_THREAD(iicCombinedPT(struct pt* pt, uint32_t* i2c, uint8_t slaveAddress, uint8_t regToRead, uint8_t* returnByte, uint8_t length));

#endif /* INTERFACE_H_ */
