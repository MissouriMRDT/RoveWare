/*
 * SingleStopLimitSwitch.h
 *
 *  Created on: May 24, 2018
 *      Author: drue
 */

#ifndef ROVEWARE_ROVEMOTIONCONTROL_STOPCAPMECHANISMS_SINGLESTOPLIMITSWITCH_H_
#define ROVEWARE_ROVEMOTIONCONTROL_STOPCAPMECHANISMS_SINGLESTOPLIMITSWITCH_H_

#include "../AbstractFramework.h"
#include "../RoveMotionControl.h"
#include <stdint.h>

class SingleStopLimitSwitch : public StopcapMechanism
{
  private:
    bool switchLogicHigh;
    uint16_t switchPin;

  public:
    StopcapStatus getStopcapStatus();
    SingleStopLimitSwitch(uint16_t pin, bool isLogicHigh);
};



#endif /* ROVEWARE_ROVEMOTIONCONTROL_STOPCAPMECHANISMS_SINGLESTOPLIMITSWITCH_H_ */
