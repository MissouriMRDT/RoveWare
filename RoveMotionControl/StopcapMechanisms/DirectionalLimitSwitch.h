/*
 * DirectionalLimitSwitch.h
 *
 *  Created on: Apr 27, 2018
 *      Author: drue
 */

#ifndef ROVEWARE_ROVEMOTIONCONTROL_STOPCAPMECHANISMS_DIRECTIONALLIMITSWITCH_H_
#define ROVEWARE_ROVEMOTIONCONTROL_STOPCAPMECHANISMS_DIRECTIONALLIMITSWITCH_H_

#include "../AbstractFramework.h"
#include "../RoveMotionControl.h"
#include <stdint.h>

class DirectionalLimitSwitch : StopcapMechanism
{
  private:
    bool switchLogicHigh;
    uint16_t switchPin;
    bool directionThatsAlwaysAllowed;

  public:
    StopcapStatus getStopcapStatus();

    DirectionalLimitSwitch(uint16_t pin, bool isLogicHigh, bool allowableDirection);
};



#endif /* ROVEWARE_ROVEMOTIONCONTROL_STOPCAPMECHANISMS_DIRECTIONALLIMITSWITCH_H_ */
