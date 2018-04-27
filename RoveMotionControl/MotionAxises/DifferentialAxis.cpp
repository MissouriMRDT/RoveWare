#include "DifferentialAxis.h"
#include "../RoveMotionUtilities.h"

DifferentialAxis::DifferentialAxis(DifferentialType axisType, ValueType inputType, IOConverter *alg, OutputDevice* cont1, OutputDevice* cont2)
  : MotionAxis(inputType, alg, cont1), controller2(cont2), coupled(false), thisAxisType(axisType), motorOneVirtualPower(0), motorTwoVirtualPower(0)
{
  //checks to make sure the passed arguments all work with each other, that is that the algorithm's input type
  //is the same as what the user is putting in, and that the algorithm's output value type is what the output device
  //expects to take in, etc
  if((inputType == alg->inType) && (cont1->inType == alg->outType) && (cont1->inType == cont2->inType))
  {
    validConstruction = true;
  }
  else
  {
    validConstruction = false;
  }
}

DifferentialAxis::DifferentialAxis(DifferentialType axisType, ValueType inputType, OutputDevice* cont1, OutputDevice* cont2)
  : MotionAxis(inputType, cont1), controller2(cont2), coupled(false), thisAxisType(axisType), motorOneVirtualPower(0), motorTwoVirtualPower(0)
{
  //checks to make sure the passed arguments all work with each other, IE that the two output devices both take the same type.
  if((inputType == cont1->inType) && (cont1->inType == cont2->inType))
  {
    validConstruction = true;
  }
  else
  {
    validConstruction = false;
  }
}

DifferentialAxis::~DifferentialAxis()
{
  //destructor deliberately left blank; we don't want to delete the pointers to other classes as the main program could still be handling them
}

AxisControlStatus DifferentialAxis::runOutputControl(const long movement)
{
  long mov; //var used as interrum value since algorithm can change the value
  bool motionComplete;
  AxisControlStatus returnStatus;
  
  if(!enabled)
  {
    returnStatus = DeviceDisabled;
  }
  else if(verifyInput(movement) == false)
  {
    returnStatus = InvalidInput;
  }
  else if(!validConstruction)
  {
    returnStatus = InvalidConstruction;
  }
  else
  {
  	//runs the algorithm on the input if there is one. Else it just passes the output directly to the output device
    if(algorithmUsed)
    {
      mov = manip->runAlgorithm(movement, &motionComplete);
    }
    else
    {
      mov = movement;
      motionComplete = true;
    }
    
    //if motionComplete returned false but movement is 0, that's an indication that an error state occured
    if(motionComplete == false && mov == 0)
    {
      controller1->stop();
      controller2->stop();
      returnStatus = AlgorithmError;
    }

    //check to see if stopcap has demanded we stop. As well, we pass it mov so that it can modify it
    //if the stopcaps demand that we modify the move value
    else if(!handleStopCap(&mov, controller1->inType))
    {
      controller1->stop();
      controller2->stop();
      returnStatus = StopcapActivated;
    }
    else
    {
      if(motionComplete == true)
      {
        returnStatus = OutputComplete;
      }
      else
      {
        returnStatus = OutputRunning;
      }

      if(thisAxisType == DifferentialTilt)
      {
        motorOneVirtualPower = mov;
        motorTwoVirtualPower = mov;
      }
      else
      {
        motorOneVirtualPower = -mov;
        motorTwoVirtualPower = mov;
      }

      int motorOneTruePower = motorOneVirtualPower;
      int motorTwoTruePower = motorTwoVirtualPower;

      //this only happens if this axis has been coupled with another axis.
      //The coupled logic will modify the power calculated by the algorithm
      //to allow smooth tilting and rotating at the same time.
      if(coupled && controller1->inType == InputPowerPercent)
      {
        motorOneTruePower += coupledAxis->motorOneVirtualPower;
        if (motorOneTruePower > POWERPERCENT_MAX)
        {
          motorOneTruePower = POWERPERCENT_MAX;
        }
        else if (motorOneTruePower < POWERPERCENT_MIN)
        {
          motorOneTruePower = POWERPERCENT_MIN;
        }

        motorTwoTruePower += coupledAxis->motorTwoVirtualPower;
        if (motorTwoTruePower > POWERPERCENT_MAX)
        {
          motorTwoTruePower = POWERPERCENT_MAX;
        }
        else if (motorTwoTruePower < POWERPERCENT_MIN)
        {
          motorTwoTruePower = POWERPERCENT_MIN;
        }
      }

      controller1->move(motorOneTruePower);
      controller2->move(motorTwoTruePower);
    }
  }

  return(returnStatus);
}
  
void DifferentialAxis::stop()
{
  controller1->stop();
  controller2->stop();
  motorOneVirtualPower = 0;
  motorTwoVirtualPower = 0;
  
  if(coupled)
  {
    coupledAxis->motorOneVirtualPower = 0;
    coupledAxis->motorTwoVirtualPower = 0;
  }
}

void DifferentialAxis::disableAxis()
{
  stop();
  //since disabling controllers would affect both axiss if this is a differential axis,
  //don't turn off the devices. Just stop the axis until enable is called
  if(!coupled)
  {
    controller1->setPower(false);
    controller2->setPower(false);
  }
  enabled = false;
}

bool DifferentialAxis::switchDifferentialModules(ValueType newInputType, IOConverter* newAlgorithm, OutputDevice* newDevice1, OutputDevice* newDevice2)
{
  if((newInputType == newAlgorithm->inType) && (newDevice1->inType == newAlgorithm->outType) && (newDevice2->inType == newDevice1->inType))
  {
   manip = newAlgorithm;
   inType = newInputType;
   controller1 = newDevice1;
   controller2 = newDevice2;

   algorithmUsed = true;

   return(true);
  }
  else
  {
   return(false);
  }
}

void DifferentialAxis::enableAxis()
{
  //since enabling controllers would affect both axiss if this is a differential axis,
  //don't turn on the devices. Just enable this axis to move
  if(!coupled)
  {
    controller1->setPower(true);
    controller2->setPower(true);
  }
  enabled = true;
}

void DifferentialAxis::pairDifferentialAxis(DifferentialAxis* otherAxis)
{
  //differential logic only works if the paired axiss are different types
  if(thisAxisType != otherAxis->thisAxisType)
  {
    this->coupledAxis = otherAxis;
    this->coupled = true;
    otherAxis->coupledAxis = this;
    otherAxis->coupled = true;
  }
  else
  {
    //catch this in a debug infinite loop, so the user knows they set up their axiss wrong
    while(1)
    {
    }
  }
}
