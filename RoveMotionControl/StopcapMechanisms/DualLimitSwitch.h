/*
 * DualLimitSwitch.h
 *
 *  Created on: Apr 27, 2018
 *      Author: drue
 */

#ifndef ROVEWARE_ROVEMOTIONCONTROL_STOPCAPMECHANISMS_DIRECTIONALLIMITSWITCH_H_
#define ROVEWARE_ROVEMOTIONCONTROL_STOPCAPMECHANISMS_DIRECTIONALLIMITSWITCH_H_

#include "../AbstractFramework.h"
#include "../RoveMotionControl.h"
#include <stdint.h>

class DualLimitSwitch : public StopcapMechanism
{
  private:
    bool switchLogicHigh;
    uint16_t positiveDirectionSwitchPin;
    uint16_t negativeDirectionSwitchPin;

  public:
    StopcapStatus getStopcapStatus();
    DualLimitSwitch(uint16_t negativePin, uint16_t positivePin, bool isLogicHigh);
};



#endif /* ROVEWARE_ROVEMOTIONCONTROL_STOPCAPMECHANISMS_DIRECTIONALLIMITSWITCH_H_ */
