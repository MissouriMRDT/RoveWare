/*
 * RoveCommSerial.h
 *
 *  Created on: Oct 19, 2017
 *      Author: drue
 */

#ifndef ROVECOMMSERIAL_H_
#define ROVECOMMSERIAL_H_

#include <stdint.h>
#include "prototype_API/RoveUart.h"

typedef roveUART_Handle RoveCommSerialHandle;

RoveCommSerialHandle roveCommSerial_BeginSingle(uint16_t uart_index, uint16_t baud_rate, uint16_t txPin, uint16_t rxPin);
bool roveCommSerial_GetMsg(RoveCommSerialHandle handle, uint16_t* dataID, uint8_t* size, void* data);
bool roveCommSerial_SendMsg(RoveCommSerialHandle handle, uint16_t dataID, uint8_t size, const void* data);

bool roveCommSerial_SetSerialBufferSize(RoveCommSerialHandle handle, uint16_t newBufferSize);
uint16_t roveCommSerial_GetSerialBufferSize(RoveCommSerialHandle handle);


#endif /* ROVECOMMSERIAL_H_ */
