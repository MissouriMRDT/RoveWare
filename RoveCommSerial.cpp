/*
 * RoveCommSerial.cpp
 *
 *  Created on: Oct 19, 2017
 *      Author: drue
 */
#include "RoveCommSerial.h"
#include "RoveBoard.h"
#include <string.h>

#define ROVECOMMSERIAL_START         0xFE


#define START_OFFSET        0
#define LENGTH_OFFSET       1
#define FLAGS_OFFSET        2
#define DATAID_OFFSET_MSB   3
#define DATAID_OFFSET_LSB   4
#define CRC_OFFSET_MSB      5
#define CRC_OFFSET_LSB      6

#define ROVECOMM_ACKNOWLEDGE_FLAG   1
#define ROVECOMM_PING               0x0001
#define ROVECOMM_PING_REPLY         0x0002
#define ROVECOMM_SUBSCRIBE          0x0003
#define ROVECOMM_UNSUBSCRIBE        0x0004
#define ROVECOMM_FORCE_UNSUBSCRIBE  0x0005
#define ROVECOMM_ACKNOWLEDGE_MSG    0x0006

static void RoveCommParseMsg(uint8_t* buffer, uint16_t* dataID, uint8_t * size, void* data, uint8_t* flags);
static void RoveCommHandleSystemMsg(RoveCommSerialHandle uartHandle, uint8_t* buffer, uint16_t* dataID, uint8_t* size, void* data, uint8_t* flags);
static uint16_t crc16(const uint8_t* data_p, uint8_t length);

static bool receiveHeaderStarted = false;

RoveCommSerialHandle roveCommSerial_BeginSingle(uint16_t uart_index, uint16_t baud_rate, uint16_t txPin, uint16_t rxPin)
{
  return roveUartOpen(uart_index, baud_rate, txPin, rxPin);
}

bool roveComm_GetMsg(RoveCommSerialHandle uartHandle, uint16_t* dataID, uint8_t* size, void* data)
{
  uint8_t flags = 0;

  *dataID = 0;
  *size = 0;

  //if we haven't started the process of receiving a packet yet (IE we just finished retrieving one or program is on startup)
  //then look to see if you can find one. If we do have messages in the buffer and the first ISN'T the start byte, then we're out of order
  //and we should clear out the buffer until we either do find one or it's empty
  if(!receiveHeaderStarted)
  {
    if(roveUartAvailable(uartHandle) == 0)
    {
      return true;
    }

    else if(roveUartPeek(uartHandle) != ROVECOMMSERIAL_START)
    {
      uint8_t dummy;
      uint8_t i;
      uint8_t bytesInReadBuff = roveUartAvailable(uartHandle);
      for(i = 0; i < bytesInReadBuff; i++)
      {
        roveUartRead(uartHandle, &dummy, 1); //discard incorrect start packet
        if(roveUartPeek(uartHandle) == ROVECOMMSERIAL_START)
        {
          receiveHeaderStarted = true;
          break;
        }
      }
    }

    else
    {
      receiveHeaderStarted = true;
    }
  }

  if(!receiveHeaderStarted)
  {
    return true;
  }

  //if we do have the beginning of a header packet, check to see how many more bytes we expect in this transmission. If there's nothing left in the buffer, then obviously
  //we don't have the rest of the packet yet
  else if(roveUartAvailable(uartHandle) == 0)
  {
    return true;
  }

  //Size of the data payload in bytes is the first thing in the header, so just use uartPeek to see it.
  //If we have more bytes available but less than what the size we expect then we haven't received a complete package
  else if(roveUartPeek(uartHandle) + ROVECOMMSERIAL_HEADER_LENGTH < roveUartAvailable(uartHandle))
  {
    uint16_t uartBuffLength = roveUartGetBufferLength(uartHandle);
    if(uartBuffLength < roveUartPeek(uartHandle) +  ROVECOMMSERIAL_HEADER_LENGTH)
    {
      //this message is actually bigger than the serial buffer, we can't process it. Restart process
      receiveHeaderStarted = false;
      return false;
    }
    else
    {
      //we can hold what's to come, it's just not here yet
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

    receiveHeaderStarted = false; //restart process
    return true;
  }
}

static void RoveCommParseMsg(uint8_t* buffer, uint16_t* dataID, uint8_t * size, void* data, uint8_t* flags)
{
  uint16_t crcIn;
  uint16_t crc;
  *dataID = (((uint16_t)(buffer[DATAID_OFFSET_MSB]) << 8) | buffer[DATAID_OFFSET_LSB] );
  *size = buffer[LENGTH_OFFSET];
  *flags = buffer[FLAGS_OFFSET];
  crcIn = (((uint16_t)(buffer[CRC_OFFSET_MSB]) << 8) | buffer[CRC_OFFSET_LSB] );

  memcpy(data, &(buffer[ROVECOMMSERIAL_HEADER_LENGTH]), *size);

  crc = crc16((uint8_t*)data, *size);

  if(crc != crcIn)
  {
    *dataID = 0;
    *size = 0;
  }
}

bool roveComm_SendMsg(RoveCommSerialHandle uartHandle, uint16_t dataID, uint8_t size, const void* data)
{
  uint8_t flags = 0;
  uint16_t crc = 0;
  uint8_t packetSize = size + ROVECOMMSERIAL_HEADER_LENGTH;
  uint8_t buffer[packetSize];
  crc = crc16((uint8_t*)data, size);

  buffer[START_OFFSET] = ROVECOMMSERIAL_START;
  buffer[LENGTH_OFFSET] = size;
  buffer[FLAGS_OFFSET] = flags;
  buffer[DATAID_OFFSET_MSB] = dataID >> 8;
  buffer[DATAID_OFFSET_LSB] = dataID & 0x00FF;
  buffer[CRC_OFFSET_MSB] = crc >> 8;
  buffer[CRC_OFFSET_LSB] = crc & 0x00FF;

  memcpy(&(buffer[ROVECOMMSERIAL_HEADER_LENGTH]), data, size);

  roveUartWrite(uartHandle, buffer, packetSize);
  return true;
}

static void RoveCommHandleSystemMsg(RoveCommSerialHandle uartHandle, uint8_t* buffer, uint16_t* dataID, uint8_t* size, void* data, uint8_t* flags)
{
  if(*dataID == 0)
  {
    return;
  }

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

void roveCommSerial_SetSerialBufferSize(RoveCommSerialHandle uartHandle, uint16_t newBufferSize)
{
  roveUartSetBufferLength(uartHandle, newBufferSize);
}

uint16_t roveCommSerial_GetSerialBufferSize(RoveCommSerialHandle uartHandle)
{
  return roveUartGetBufferLength(uartHandle);
}

static uint16_t crc16(const uint8_t* data_p, uint8_t length)
{
    uint8_t x;
    uint16_t crc = 0xFFFF;

    while (length > 0) //dont ask, from internet
    {
      length--;
      x = crc >> 8 ^ *data_p++;
      x ^= x>>4;
      crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x <<5)) ^ ((unsigned short)x);
    }
    return crc;
}
