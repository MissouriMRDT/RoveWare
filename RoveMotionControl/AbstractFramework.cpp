#include "AbstractFramework.h"
#include "RoveMotionUtilities.h"
#include "RoveBoard.h"

bool MotionAxis::verifyInput(long inputToVerify)
{
  switch(inType)
  {
    case InputSpeed:
    return SPEED_MIN <= inputToVerify && inputToVerify <= SPEED_MAX;
    case InputPosition:
    return POS_MIN <= inputToVerify && inputToVerify <= POS_MAX;
    case InputPowerPercent:
    return POWERPERCENT_MIN <= inputToVerify && inputToVerify <= POWERPERCENT_MAX;
    case InputTorque:
    return TORQUE_MIN <= inputToVerify && inputToVerify <= TORQUE_MAX;
    case InputVoltage:
    return VOLT_MIN <= inputToVerify && inputToVerify <= VOLT_MAX;
    default:
      return inputToVerify == 0;
  }
}

bool MotionAxis::switchModules(ValueType newInputType, IOConverter* newAlgorithm)
{
  if((newInputType == newAlgorithm->inType) && (controller1->inType == newAlgorithm->outType))
  {
    manip = newAlgorithm;
    inType = newInputType;
    
    algorithmUsed = true;

    return(true);
  }
  else
  {
    return(false);
  }
}

bool MotionAxis::switchModules(ValueType newInputType, IOConverter* newAlgorithm, OutputDevice* newDevice)
{
  if((newInputType == newAlgorithm->inType) && (newDevice->inType == newAlgorithm->outType))
   {
     manip = newAlgorithm;
     inType = newInputType;
     controller1 = newDevice;

     algorithmUsed = true;

     return(true);
   }
   else
   {
     return(false);
   }
}

bool MotionAxis::switchModules(ValueType newInputType, OutputDevice* newDevice)
{
  bool valid = false;

  if(algorithmUsed)
  {
    if(((newInputType == manip->inType) && newDevice->inType == manip->outType))
    {
      valid = true;
    }
  }
  else if((newInputType == newDevice->inType))
  {
    valid = true;
  }

  if(!valid)
  {
    return false;
  }
  else
  {
    inType = newInputType;
    controller1 = newDevice;
    return(true);
  }
}

bool MotionAxis::removeIOConverter(ValueType newInputType)
{
  if(!algorithmUsed)
  {
    return(true);
  }
  else if(newInputType == controller1->inType)
  {
    inType = newInputType;
    manip = 0;
    algorithmUsed = false;

    return(true);
  }
  else
  {
    return(false);
  }
}

bool MotionAxis::removeIOConverter(ValueType newInputType, OutputDevice* newDevice)
{
  if(newInputType == newDevice->inType)
  {
    inType = newInputType;
    manip = 0;
    algorithmUsed = false;
    controller1 = newDevice;

    return(true);
  }
  else
  {
    return(false);
  }
}

void MotionAxis::useStopcap(StopcapMechanism* stopcapToUse)
{
  stopcap = stopcapToUse;
  stopcapUsed = true;
}

bool MotionAxis::handleStopCap(long *move, ValueType valueType)
{
  if(!stopcapUsed)
  {
    return true;
  }

  StopcapStatus status = stopcap->getStopcapStatus();

  if(status == StopcapStatus_FullStop)
  {
    return false;
  }
  else if(status != StopcapStatus_None)
  {
    if(valueType == InputPowerPercent || valueType == InputTorque || valueType == InputSpeed || valueType == InputVoltage)
    {
      if(status == StopcapStatus_OnlyPositive && *move < 0)
      {
        *move = 0;
      }
      else if(status == StopcapStatus_OnlyNegative && *move > 0)
      {
        *move = 0;
      }
    }
    else
    {
      debugFault("motion axis handle stop cap: logic not implemented");
    }
  }

  return true;
}

bool IOConverter::addSupportingAlgorithm(IOConverter* support)
{
  if(outType == support->outType && inType == support->inType)
  {
    supportingAlgorithm = support;
    supportUsed = true;
    return true;
  }
  else
  {
    return false;
  }
}

void IOConverter::persistantSupport(bool persistant)
{
  supportIsPersistant = persistant;
}
