/*
 * RoveCommSerial.h
 *
 *  Created on: Oct 19, 2017
 *      Author: drue
 *
 *  Library for doing RoveComm transfers between boards over Serial. Library automatically handles rovecomm communication features
 *  such as packet parsing, data retreiving, error checking with crc16 standard, and error correction in case messages got out of order.
 *
 *  Dependencies: RoveUart.h
 */

#ifndef ROVECOMMSERIAL_H_
#define ROVECOMMSERIAL_H_

#include <stdint.h>
#include "../Roveboard Msp432 builder/Roveboard/standardized_API/RoveUart.h"

typedef roveUART_Handle RoveCommSerialHandle;

#define RoveCommError_DataCorrupted 2
#define RoveCommError_Success       3

//overview: starts roveCommSerial with a uart.
//inputs:   uart_index:   Index of the uart module to use
//          baud_rate:    baud rate of the uart to use
//          txPin:        transmitting pin to be used by the uart. Must be associated with the uart; check your device's roveUart.h file for info
//          rxPin:        receiving pin to be used by the uart. Must be associated with the uart; check your device's roveUart.h file for info
//          maxDataBytes: The maximum amount of bytes this board is expected to receive or transmit as data from a rovecomm message
//
//returns:  a handle to the now initialized rovecomm instance for this uart.
//Note:     You can call this multiple times for different uart modules if you wish to do rovecomm data exchange on more than one module
RoveCommSerialHandle roveCommSerial_BeginSingle(uint16_t uart_index, uint16_t baud_rate, uint16_t txPin, uint16_t rxPin, uint16_t maxDataBytes);

//overview: gets a rovecomm message currently in our receive buffer, if there is one.
//inputs:   handle:     handle of the rovecomm instance to use
//          dataID:     return-by-pointer argument that will contain the message's dataID upon return, or 0 if there wasn't a message waiting.
//          size:       return-by-pointer argument that will contain how many data bytes were in the message
//          data:       return-by-pointer argument that will contain the messages data. What datatype (char, float, etc) depends on the dataId;
//                      usually you initialize data before calling this function to an unsigned character array bigger than the biggest message,
//                      and cast the return value to whatever data type is used by the dataID.
//
//returns:  RoveCommError_Success if operation encountered no problems, RoveCommError_BuffTooSmall if
//          we received a message larger than the serial buffer could handle, and RoveCommError_DataCorrupted if the message got
//          mangled in transmit
uint8_t roveCommSerial_GetMsg(RoveCommSerialHandle handle, uint16_t* dataID, uint8_t* size, void* data);

//overview: sends a rovecomm message across the uart line
//inputs:   handle:     handle of the rovecomm instance to use
//          dataID:     return-by-pointer argument that will contain the message's dataID upon return, or 0 if there wasn't a message waiting.
//          size:       return-by-pointer argument that will contain how many data bytes were in the message
//          data:       return-by-pointer argument that will contain the messages data. What datatype (char, float, etc) depends on the dataId;
//                      usually you initialize data before calling this function to an unsigned character array bigger than the biggest message,
//                      and cast the return value to whatever data type is used by the dataID.
//
//returns:  true if operation encountered no problems, false if a problem was encountered and will usually need a couple of more function calls
//          to correct itself
bool roveCommSerial_SendMsg(RoveCommSerialHandle handle, uint16_t dataID, const uint8_t size, const void* data);


#endif /* ROVECOMMSERIAL_H_ */
