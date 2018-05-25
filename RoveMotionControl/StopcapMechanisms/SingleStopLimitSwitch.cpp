/*
 * SingleStopLimitSwitch.cpp
 *
 *  Created on: May 24, 2018
 *      Author: drue
 */

#include "SingleStopLimitSwitch.h"
#include "RoveBoard.h"

SingleStopLimitSwitch::SingleStopLimitSwitch(uint16_t pin, bool isLogicHigh)
: switchLogicHigh(isLogicHigh), switchPin(pin),
  StopcapMechanism(){}

StopcapStatus SingleStopLimitSwitch::getStopcapStatus()
{
  if(digitalPinRead(switchPin) == switchLogicHigh)
  {
    return StopcapStatus_FullStop;
  }
  else
  {
    return StopcapStatus_None;
  }
}



