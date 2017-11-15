/*
 * RoveCommSerial.cpp
 *
 *  Created on: Oct 19, 2017
 *      Author: drue
 */
#include "RoveCommSerial.h"
#include "RoveBoard.h"
#include <string.h>

#define ROVECOMMSERIAL_HEADER_LENGTH    8
#define ROVECOMMSERIAL_START         0xFE
#define ROVECOMMSERIAL_END           0xEF

#define START_OFFSET        0
#define LENGTH_OFFSET       1
#define FLAGS_OFFSET        2
#define DATAID_OFFSET_MSB   3
#define DATAID_OFFSET_LSB   4
#define CRC_OFFSET_MSB      5
#define CRC_OFFSET_LSB      6
#define END_OFFSET          7

#define ROVECOMM_ACKNOWLEDGE_FLAG   1
#define ROVECOMM_PING               0x0001
#define ROVECOMM_PING_REPLY         0x0002
#define ROVECOMM_ACKNOWLEDGE_MSG    0x0006

static bool RoveCommParseMsg(uint8_t* buffer, uint16_t* dataID, uint8_t * size, void* data, uint8_t* flags);
static void RoveCommHandleSystemMsg(RoveCommSerialHandle uartHandle, uint8_t* buffer, uint16_t* dataID, uint8_t* size, void* data, uint8_t* flags);
static uint16_t crc16(const uint8_t* data_p, uint8_t length);

RoveCommSerialHandle roveCommSerial_BeginSingle(uint16_t uart_index, uint16_t baud_rate, uint16_t txPin, uint16_t rxPin, uint16_t maxDataBytes)
{
  roveUART_Handle uartHandle;

  uartHandle = roveUartOpen(uart_index, baud_rate, txPin, rxPin);
  roveUartSetBufferLength(uartHandle, maxDataBytes + ROVECOMMSERIAL_HEADER_LENGTH + 1);

  return uartHandle;
}

uint8_t roveCommSerial_GetMsg(RoveCommSerialHandle uartHandle, uint16_t* dataID, uint8_t* size, void* data)
{
  uint8_t flags = 0;
  uint8_t dummy;

  *dataID = 0;
  *size = 0;

  //Check to see if we have messages. If we do have messages in the buffer and the first ISN'T the start byte, then we're out of order
  //and we should clear out the buffer until we either do find one or it's empty
  if(roveUartAvailable(uartHandle) == 0)
  {
    return RoveCommError_Success;
  }
  else
  {
    //if we have a data packet that doesn't start at the start message, discard bytes until we find it
    if(roveUartPeek(uartHandle) != ROVECOMMSERIAL_START)
    {
      uint8_t i;
      uint8_t bytesInReadBuff = roveUartAvailable(uartHandle);
      bool startFound = false;
      for(i = 0; i < bytesInReadBuff; i++)
      {
        roveUartRead(uartHandle, &dummy, 1); //discard incorrect start packet
        if(roveUartPeek(uartHandle) == ROVECOMMSERIAL_START)
        {
          startFound = true;
          break;
        }
      }

      if(!startFound)
      {
        return RoveCommError_Success;
      }
    }

    //we have a potential start message at the top of the buffer. Now let's verify that a) there's enough byte left in the buffer to form
    //a full message. We do this by adding up what we assume is the size byte and the header length b) If they go beyond our buffer length,
    //we assume it's not the correct size byte and discard. If they do fit within our buffer but we don't have that many bytes available yet,
    //assume rest of the message isn't here yet c) if that's all false, then check to see if there's an END byte at the expected end of the message. If there is, then we have
    //a complete package. Otherwise discard.
    int sizeOfMessage = roveUartPeek(uartHandle, 1);
    if(ROVECOMMSERIAL_HEADER_LENGTH > roveUartAvailable(uartHandle) || roveUartPeek(uartHandle, ROVECOMMSERIAL_HEADER_LENGTH -1) != ROVECOMMSERIAL_END)
    {
      //incorrect header packet, discard and try again next time
      roveUartRead(uartHandle, &dummy, 1);
      return RoveCommError_Success;
    }
    else if(sizeOfMessage == -1)
    {
      return RoveCommError_Success;
    }
    else if(sizeOfMessage + ROVECOMMSERIAL_HEADER_LENGTH > roveUartGetBufferLength(uartHandle))
    {
      //discard incorrect start packet, try again next time
      roveUartRead(uartHandle, &dummy, 1);
      return RoveCommError_Success;
    }
    else if(sizeOfMessage + ROVECOMMSERIAL_HEADER_LENGTH > roveUartAvailable(uartHandle))
    {
      //rest of the message hasn't arrived yet, try again next time
      return RoveCommError_Success;
    }
  }


  //we have both a complete header packet, and (according to the size number in the header) a complete data payload, so we have a
  //complete transfer waiting for us in memory. Note that we actually removed one part of the packet, the Begin packet, in order to use
  //peek() to see what the size of the message is. So we should re-insert the begin packet since the rest of the system expects it to be there
  //for error check calculations
  uint8_t packetSize = roveUartPeek(uartHandle, 1) + ROVECOMMSERIAL_HEADER_LENGTH;
  uint8_t buffer[packetSize];
  bool dataValid;

  roveUartRead(uartHandle, buffer, packetSize);

  dataValid = RoveCommParseMsg(buffer, dataID, size, data, &flags);
  if(dataValid)
  {
    RoveCommHandleSystemMsg(uartHandle, buffer, dataID, size, data, &flags);
    return RoveCommError_Success;
  }
  else
  {
    *dataID = 0;
    *size = 0;
    return RoveCommError_DataCorrupted;
  }
}

static bool RoveCommParseMsg(uint8_t* buffer, uint16_t* dataID, uint8_t * size, void* data, uint8_t* flags)
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
    return false;
  }
  else
  {
    return true;
  }
}

bool roveCommSerial_SendMsg(RoveCommSerialHandle handle, uint16_t dataID, const uint8_t size, const void* data)
{
  uint8_t flags = 0;
  uint16_t crc = 0;
  uint8_t buffer[size + ROVECOMMSERIAL_HEADER_LENGTH];
  crc = crc16((uint8_t*)data, size);

  buffer[START_OFFSET] = ROVECOMMSERIAL_START;
  buffer[LENGTH_OFFSET] = size;
  buffer[FLAGS_OFFSET] = flags;
  buffer[DATAID_OFFSET_MSB] = dataID >> 8;
  buffer[DATAID_OFFSET_LSB] = dataID & 0x00FF;
  buffer[CRC_OFFSET_MSB] = crc >> 8;
  buffer[CRC_OFFSET_LSB] = crc & 0x00FF;
  buffer[END_OFFSET] = ROVECOMMSERIAL_END;

  memcpy(&(buffer[ROVECOMMSERIAL_HEADER_LENGTH]), data, size);

  roveUartWrite(handle, buffer, size + ROVECOMMSERIAL_HEADER_LENGTH);

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
    roveCommSerial_SendMsg(uartHandle, ROVECOMM_ACKNOWLEDGE_MSG, sizeof(uint16_t), dataID);
  }

  switch (*dataID)
  {
    case ROVECOMM_PING:
      roveCommSerial_SendMsg(uartHandle, ROVECOMM_PING_REPLY, sizeof(uint8_t), (uint8_t)(0));
      break;
    case ROVECOMM_PING_REPLY:
      break;
    case ROVECOMM_ACKNOWLEDGE_MSG:
      break;
    default:
      return;
  }

  //if it didn't return, it means the message was a system message. The user doesn't care about those, so
  //make it look like they received nothing
  *dataID = 0;
  *size = 0;
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
