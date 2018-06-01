#include "SingleMotorAxis.h"
#include "../RoveMotionUtilities.h"

SingleMotorAxis::SingleMotorAxis(ValueType inputType, IOConverter *alg, OutputDevice* cont) : MotionAxis(inputType, alg, cont)
{
  //checks to make sure the passed arguments all work with each other, that is that the algorithm's input type is the same as what the user is putting in, and
  //that the algorithm's output value type is what the output device expects to take in, etc
  if((inputType == alg->inType) && (cont->inType == alg->outType))
  {
    validConstruction = true;
  }
  else
  {
    validConstruction = false;
  }
}

SingleMotorAxis::SingleMotorAxis(ValueType inputType, OutputDevice* cont) : MotionAxis(inputType, cont)
{
  //checks to make sure the passed arguments all work with each other, that is that device's input type is the same as what the user is putting in
  if(inputType == cont->inType)
  {
    validConstruction = true;
  }
  else
  {
    validConstruction = false;
  }
}

SingleMotorAxis::~SingleMotorAxis()
{
  //deliberately left blank; we don't want to destroy the pointers to other classes as the main program could still be handling them
}

AxisControlStatus SingleMotorAxis::runOutputControl(const long movement)
{
	long mov; //var used as interrum value since algorithm can change the value
  IOConverter_Status converterStatus;
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
  	//calls algorithm if there's one used. If not, output passed directly to output device
    if(algorithmUsed)
    {
      mov = manip->runAlgorithm(movement, &converterStatus);
    }
    else
    {
      mov = movement;
      converterStatus.flags = IOConverter_Complete;
    }
    
    if(converterStatus.flags != IOConverter_Complete && converterStatus.flags != IOConverter_RunAgain)
    {
      controller1->stop();

      if(converterStatus.flags == IOConverter_AlgorithmFail)
      {
        returnStatus = AlgorithmError;
      }
      else if(converterStatus.flags == IOConverter_FeedbackFail)
      {
        returnStatus = FeedbackError;
      }
    }

    //check to see if stopcap has demanded we stop. As well, we pass it mov so that it can modify it
    //if the stopcaps demand that we modify the move value
    else if(!handleStopCap(&mov, controller1->inType))
    {
      controller1->stop();
      returnStatus = StopcapActivated;
    }
    else
    {
      if(converterStatus.flags == IOConverter_Complete)
      {
        returnStatus = OutputComplete;
      }
      else
      {
        returnStatus = OutputRunning;
      }

      //moves device with output decided on by the algorithm
      controller1->move(mov);
    }
  }

	return(returnStatus);
}

void SingleMotorAxis::stop()
{
  controller1->stop();
}

void SingleMotorAxis::disableAxis()
{
  controller1->setPower(false);
  enabled = false;
}

void SingleMotorAxis::enableAxis()
{
  controller1->setPower(true);
  enabled = true;
}
