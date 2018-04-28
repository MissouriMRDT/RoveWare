/*
 * DualLimitSwitch.cpp
 *
 *  Created on: Apr 27, 2018
 *      Author: drue
 */

#include "DualLimitSwitch.h"
#include "RoveBoard.h"

DualLimitSwitch::DualLimitSwitch(uint16_t negativePin, uint16_t positivePin, bool isLogicHigh)
: switchLogicHigh(isLogicHigh), positiveDirectionSwitchPin(positivePin), negativeDirectionSwitchPin(negativePin),
  StopcapMechanism(){}

StopcapStatus DualLimitSwitch::getStopcapStatus()
{
  if(digitalPinRead(negativeDirectionSwitchPin) == switchLogicHigh)
  {
    return StopcapStatus_OnlyPositive;
  }
  else if(digitalPinRead(positiveDirectionSwitchPin) == switchLogicHigh)
  {
    return StopcapStatus_OnlyNegative;
  }
  else
  {
    return StopcapStatus_None;
  }
}
