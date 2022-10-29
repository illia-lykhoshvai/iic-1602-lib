/*
 * interface.c
 *
 *  Created on: 4 sept. 2022 y.
 *      Author: illialykhoshvai
 */
#include "stm32f4xx.h"

#include "main.h"
#include "interface.h"

#define SET_ACK(i2c) i2c[CNTRL_1] |= I2C_CR1_ACK
#define RESET_ACK(i2c) i2c[CNTRL_1]&= ~I2C_CR1_ACK

#define STOP_AFTER_BYTE(i2c) i2c[CNTRL_1] |= I2C_CR1_STOP;

void reInitI2C(uint32_t* i2c) {
	i2c[CNTRL_1] &= I2C_CR1_PE;
	i2c[CNTRL_1] |= I2C_CR1_PE;
}

PT_THREAD(startCondtionPT(struct pt* pt, uint32_t* i2c)) {
	PT_BEGIN(pt);
	SET_ACK(i2c);
	i2c[CNTRL_1] |= I2C_CR1_START;
	PT_YIELD_UNTIL(pt, (i2c[STATUS_1] & I2C_SR1_SB));
	(void) I2C1->SR1;
	PT_END(pt);
}

PT_THREAD(sendBytePT(struct pt* pt, uint32_t* i2c, uint8_t data)) {
	PT_BEGIN(pt);
	i2c[4] = data;
	PT_YIELD_UNTIL(pt , (i2c[STATUS_1] & I2C_SR1_TXE));
	PT_END(pt);
}

PT_THREAD(sendArrayPT(struct pt* pt, uint32_t* i2c, uint8_t* data, uint8_t length)) {
	static uint8_t counter;
	PT_BEGIN(pt);
	for (counter = 0; counter < length; counter++) {
		if ( !(counter < (length - 1)) ) {
			STOP_AFTER_BYTE(i2c);
		}
		i2c[DATA] = data[counter];
		PT_YIELD_UNTIL(pt, (i2c[STATUS_1] & I2C_SR1_TXE));
	}
	PT_END(pt);
}

PT_THREAD(sendAdressPT(struct pt* pt, uint32_t* i2c, uint8_t* returnValue, uint8_t address, addressEnding_t operation)) {
	PT_BEGIN(pt);
	i2c[DATA] = (address << 1) | operation;
	PT_YIELD_UNTIL(pt , (i2c[STATUS_1] & (I2C_SR1_ADDR | I2C_SR1_AF) ));
	if (i2c[STATUS_1] & I2C_SR1_AF) {
		i2c[STATUS_1] &= ~I2C_SR1_AF;
		reInitI2C(i2c);
		*returnValue = ackFailure;
	} else {
		*returnValue = ackComplete;
	}
	(void) i2c[STATUS_1];
	(void) i2c[STATUS_2];
	PT_END(pt);
}

PT_THREAD(readArrayPT(struct pt* pt, uint32_t* i2c, uint8_t* rByte, uint8_t length)) {
	static uint8_t counter;
	PT_BEGIN(pt);
	for(counter = 0; counter < length; counter++) {
		if ( !(counter < (length - 1)) ) {
			RESET_ACK(i2c);
			STOP_AFTER_BYTE(i2c);
		}
		PT_YIELD_UNTIL(pt, (i2c[STATUS_1] & I2C_SR1_RXNE));
		rByte[counter] = i2c[DATA];
	}
	PT_END(pt);
}

PT_THREAD(iicWritePT(struct pt* pt, uint32_t* i2c, uint8_t slaveAddress, uint8_t* regToWrite, uint8_t length)) {
	static struct pt temp;
	static uint8_t ackResult;
	PT_BEGIN(pt);
	PT_INIT(&temp);
	PT_YIELD_UNTIL(pt, (startCondtionPT((&temp), i2c) == PT_ENDED) );
	PT_YIELD_UNTIL(pt, (sendAdressPT((&temp), i2c, &ackResult, slaveAddress, write) == PT_ENDED) );
	if (ackResult) {
		PT_YIELD_UNTIL(pt, (sendArrayPT((&temp), i2c, regToWrite, length) == PT_ENDED) );
	}
	PT_END(pt);
}

PT_THREAD(iicCombinedPT(struct pt* pt, uint32_t* i2c, uint8_t slaveAddress, uint8_t regToRead, uint8_t* returnByte, uint8_t length)) {
	static struct pt temp;
	static uint8_t ackResult;
	PT_BEGIN(pt);
	PT_INIT(&temp);
	PT_YIELD_UNTIL(pt, (startCondtionPT((&temp), i2c) == PT_ENDED) );
	PT_YIELD_UNTIL(pt, (sendAdressPT((&temp), i2c, &ackResult, slaveAddress, write) == PT_ENDED) );
	if (ackResult) {
		PT_YIELD_UNTIL(pt, (sendBytePT((&temp), i2c, regToRead) == PT_ENDED) );
		PT_YIELD_UNTIL(pt, (startCondtionPT((&temp), i2c) == PT_ENDED) );
		PT_YIELD_UNTIL(pt, (sendAdressPT((&temp), i2c, &ackResult, slaveAddress, read) == PT_ENDED) );
		if (ackResult) {
			PT_YIELD_UNTIL(pt, (readArrayPT((&temp), i2c, returnByte, length) == PT_ENDED) );
		}
	}
	PT_END(pt);
}
