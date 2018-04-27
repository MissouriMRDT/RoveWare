/*
 * DirectionalLimitSwitch.cpp
 *
 *  Created on: Apr 27, 2018
 *      Author: drue
 */

#include "DirectionalLimitSwitch.h"
#include "RoveBoard.h"

DirectionalLimitSwitch::DirectionalLimitSwitch(uint16_t pin, bool isLogicHigh, bool allowableDirection)
: switchLogicHigh(isLogicHigh), switchPin(pin), directionThatsAlwaysAllowed(allowableDirection) {}

StopcapStatus DirectionalLimitSwitch::getStopcapStatus()
{
  if(digitalPinRead(switchPin) == switchLogicHigh)
  {
    if(directionThatsAlwaysAllowed)
    {
      return StopcapStatus_OnlyPositive;
    }
    else
    {
      return StopcapStatus_OnlyNegative;
    }
  }

  return StopcapStatus_None;
}
