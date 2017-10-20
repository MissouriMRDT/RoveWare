/*
 * RoveCommSerial.cpp
 *
 *  Created on: Oct 19, 2017
 *      Author: drue
 */
#include "RoveCommSerial.h"
#include "RoveBoard.h"
#include <string.h>

#define ROVECOMMSERIAL_HEADER_LENGTH 4

#define LENGTH_OFFSET       0
#define FLAGS_OFFSET        1
#define DATAID_OFFSET_MSB   2
#define DATAID_OFFSET_LSB   3


#define ROVECOMM_ACKNOWLEDGE_FLAG   1
#define ROVECOMM_PING               0x0001
#define ROVECOMM_PING_REPLY         0x0002
#define ROVECOMM_SUBSCRIBE          0x0003
#define ROVECOMM_UNSUBSCRIBE        0x0004
#define ROVECOMM_FORCE_UNSUBSCRIBE  0x0005
#define ROVECOMM_ACKNOWLEDGE_MSG    0x0006

static void RoveCommParseMsg(uint8_t* buffer, uint16_t* dataID, uint8_t * size, void* data, uint8_t* flags);
static void RoveCommHandleSystemMsg(RoveCommSerialHandle uartHandle, uint8_t* buffer, uint16_t* dataID, uint8_t* size, void* data, uint8_t* flags);

RoveCommSerialHandle roveCommSerial_BeginSingle(uint16_t uart_index, uint16_t baud_rate, uint16_t txPin, uint16_t rxPin)
{
  return roveUartOpen(uart_index, baud_rate, txPin, rxPin);
}

bool roveComm_GetMsg(RoveCommSerialHandle uartHandle, uint16_t* dataID, uint8_t* size, void* data)
{
  uint8_t flags = 0;

  *dataID = 0;
  *size = 0;

  //if there's not even enough bytes available for a header segment, we either haven't received anything or
  //we haven't received a complete package
  if(roveUartAvailable(uartHandle) < ROVECOMMSERIAL_HEADER_LENGTH)
  {
    return true;
  }

  //if we do have a header packet, check to see how many more bytes we expect in this transmission. Size of the data payload in bytes
  //is the first thing in the header, so just use uartPeek to see it. If we have a complete header but don't yet have the complete
  //data payload, then we haven't received a complete package
  else if(roveUartPeek(uartHandle) +  ROVECOMMSERIAL_HEADER_LENGTH < roveUartAvailable(uartHandle))
  {
    uint16_t uartBuffLength = roveUartGetBufferLength(uartHandle);
    if(uartBuffLength < roveUartPeek(uartHandle) +  ROVECOMMSERIAL_HEADER_LENGTH)
    {
      //this message is actually bigger than the serial buffer, we can't process it
      debugFault("RoveComm get message: Attempting to receive a message bigger than the uart buffer will allow for");

      return false;
    }
    else
    {
      return true;
    }
  }

  //we have both a complete header packet, and (according to the size number in the header) a complete data payload, so we have a
  //complete transfer waiting for us in memory
  else
  {
    uint8_t packetSize = roveUartPeek(uartHandle) + ROVECOMMSERIAL_HEADER_LENGTH;
    uint8_t buffer[packetSize];

    roveUartRead(uartHandle, buffer, packetSize);
    RoveCommParseMsg(buffer, dataID, size, data, &flags);
    RoveCommHandleSystemMsg(uartHandle, buffer, dataID, size, data, &flags);

    return true;
  }
}

static void RoveCommParseMsg(uint8_t* buffer, uint16_t* dataID, uint8_t * size, void* data, uint8_t* flags)
{
  *dataID = (((uint16_t)(buffer[DATAID_OFFSET_MSB]) << 8) | buffer[DATAID_OFFSET_LSB] );
  *size = buffer[LENGTH_OFFSET];
  *flags = buffer[FLAGS_OFFSET];

  memcpy(data, &(buffer[ROVECOMMSERIAL_HEADER_LENGTH]), *size);
}

bool roveComm_SendMsg(RoveCommSerialHandle uartHandle, uint16_t dataID, uint8_t size, const void* data)
{
  uint8_t flags = 0;
  uint8_t packetSize = size + ROVECOMMSERIAL_HEADER_LENGTH;
  uint8_t buffer[packetSize];

  buffer[LENGTH_OFFSET] = size;
  buffer[FLAGS_OFFSET] = flags;
  buffer[DATAID_OFFSET_MSB] = dataID >> 8;
  buffer[DATAID_OFFSET_LSB] = dataID & 0x00FF;

  memcpy(&(buffer[ROVECOMMSERIAL_HEADER_LENGTH]), data, size);

  roveUartWrite(uartHandle, buffer, packetSize);

  return true;
}

static void RoveCommHandleSystemMsg(RoveCommSerialHandle uartHandle, uint8_t* buffer, uint16_t* dataID, uint8_t* size, void* data, uint8_t* flags)
{
  if(*flags & ROVECOMM_ACKNOWLEDGE_FLAG != 0)
  {
    roveComm_SendMsg(uartHandle, ROVECOMM_ACKNOWLEDGE_MSG, sizeof(uint16_t), dataID);
  }

  switch (*dataID)
  {
    case ROVECOMM_PING:
      roveComm_SendMsg(uartHandle, ROVECOMM_PING_REPLY, sizeof(uint8_t), (uint8_t)(0));
      break;
    case ROVECOMM_PING_REPLY:
      break;
    case ROVECOMM_SUBSCRIBE:
      break;
    case ROVECOMM_UNSUBSCRIBE:
      break;
    case ROVECOMM_ACKNOWLEDGE_MSG:
      break;
    case ROVECOMM_FORCE_UNSUBSCRIBE:
      break;
    default:
      return;
  }

  //if it didn't return, it means the message was a system message. The user doesn't care about those, so
  //make it look like they received nothing
  *dataID = 0;
  *size = 0;
}

bool roveCommSerial_SetSerialBufferSize(RoveCommSerialHandle uartHandle, uint16_t newBufferSize)
{
  roveUartSetBufferLength(uartHandle, newBufferSize);
  return true;
}

uint16_t roveCommSerial_GetSerialBufferSize(RoveCommSerialHandle uartHandle)
{
  return roveUartGetBufferLength(uartHandle);
}

