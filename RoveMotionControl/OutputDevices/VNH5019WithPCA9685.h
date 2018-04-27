/*
 * VNH5019WithPCA9685.h
 *
 *  Created on: Mar 16, 2018
 *      Author: drue
 */

#ifndef ROVEJOINTCONTROL_VNH5019WITHPCA9685_H_
#define ROVEJOINTCONTROL_VNH5019WITHPCA9685_H_

#include "../AbstractFramework.h"
#include "RoveBoard.h"

//This class represents the hardware setup where potentially several VNH5019 motor drivers are being controlled by a single
//PCA9685 pwm driver, the former getting its pwm signals (which represent magnitude of speed) from the latter, and the user
//directly talking to the latter. Each instance of this class represents a single VNH5019 driver that's being controlled, with
//the instances all sharing information on the PCA so that they all know how to talk to it.
class VNH5019WithPCA9685 : public OutputDevice
{
  protected:

     RoveI2C_Handle i2cHandle;

     //move function which passes in power percent (which is converted to phase and PWM) to move device
     void move(const long movement);

     //tells the device to power on or off.
     void setPower(bool powerOn);

     //tells device to stop
     void stop();

     const uint8_t ChipAddress;
     const uint8_t MotorIndex;
     const uint8_t InaPin;
     const uint8_t InbPin;
     const uint8_t I2cModule;
     const uint8_t DataPin;
     const uint8_t ClockPin;

     long currentMove;

   public:


     /*constructor for when the i2c not-enable pin on the pca is not used.
      * inputs:
      *        chipAdd: the 7 bit chip address for the PCA
      *        motorInd: the index of which motor this instance of the class is moving, since the pca controls multiple the class
      *                  needs to know which one an instance is specifically moving. IE 0 for this is motor 0, 1 for motor one, etc.
      *                  Note: 0 index based. Can go up to 15.
      *        motorInaPin: the pin that controls the VNH's ina pin.
      *        motorInbPin: the pin that controls the VNH's inb pin.
      *        i2cIndex: index of which i2c module is to be used with the i2c pins. See roveboard's i2c.h
      *        clock pin: i2c clock pin to be used to talk to the pca
      *        data pin: i2c data pin to be used to talk to the pca
      *        inverted: Whether or not the motor is 'upside down' IE whether or not to invert move commands in terms of direction.
     */
     VNH5019WithPCA9685(uint8_t chipAdd, uint8_t motorInd, uint8_t motorInaPin, uint8_t motorInbPin, uint8_t i2cModuleIndex, uint8_t clockPin, uint8_t dataPin, bool inverted);

     //returns the last commanded value the instance of the class received.
     long getCurrentMove();

};



#endif /* ROVEJOINTCONTROL_VNH5019WITHPCA9685_H_ */
