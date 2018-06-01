#ifndef ROVEJOINTCONTROL_ABSTRACTFRAMEWORK_H_
#define ROVEJOINTCONTROL_ABSTRACTFRAMEWORK_H_

#include "RoveMotionUtilities.h"

class MotionAxis;
class IOConverter;
class OutputDevice;
class FeedbackDevice;
class StopcapMechanism;

class DifferentialAxis;
class SingleMotorAxis;

//Primary interface class for controlling the axis; manages the other classes and
//computes any calculations that depend on the nature of the axis itself
//see README.md for more info
class MotionAxis
{
  protected:

    //expected input type from the user
    ValueType inType;

    //If the output device doesn't naturally understand the user's input type, then this will convert the values
    IOConverter* manip;

    //pointer to the output device instance which is passed in when creating the framework interface
    //called in runOutputControl
    OutputDevice* controller1;

    //pointer to the stopcap mechanism the axis has been told to use
    StopcapMechanism* stopcap;

    //tracks whether or not the parameters passed in the axis constructor were valid
    bool validConstruction;
    
    //tracks whether or not the user has passed in an algorithm to use; if not, commands are passed directly to output
    bool algorithmUsed;
    
    //tracks whether or not the user has passed in a stopcap to use
    bool stopcapUsed;

    //tracks whether or not the axis has been enabled for operation
    bool enabled;

    //Overview: function that checks to see if the user put in a proper input value when calling the runOutputControl function
    //
    //returns:  true if the input is in a valid range, false if it's not
    bool verifyInput(long inputToVerify);

    //Overview: Function that will check to see the status of the attached stop cap (if there is one) and modify the
    //          move value accordingly.
    //inputs:   Move: A pointer to the move value that the Axis of motion wants to pass to its output device(s)
    //          valueType: What type of value move represents
    //returns:  True if the axis is cleared to pass the move value to its outputDevice(s), false if the axis needs to
    //          issue a stop command
    bool handleStopCap(long *move, ValueType valueType);

    MotionAxis(ValueType in, IOConverter* alg, OutputDevice* dev)
    : enabled(true), inType(in), algorithmUsed(true), manip(alg), controller1(dev), stopcapUsed(false) {};

    MotionAxis(ValueType in, OutputDevice* dev)
    : enabled(true), inType(in), algorithmUsed(false), manip(0), controller1(dev), stopcapUsed(false) {};

  public:

    //Overview: Runs the output control for this axis, IE making it move or checking
    //          to see if it needs to move. In other words, update the output control loops.
    //          Must pass an integer for this implementation.
    //
    //returns:  The status of attempting to update the controls on this axis.
    //          Such as if the output is running, if it's complete, errors, etc
    virtual AxisControlStatus runOutputControl(const long movement) = 0;
    
    //Overview: replaces the current axis's algorithm component with a different one
    //
    //inputs:   the new type of input the axis is going to be given when runOutputControl is called.
    //          the new algorithm module
    //
    //returns:  true if swap was successful, false if not and previous settings retained
    //
    //warning:  not thread safe
    bool switchModules(ValueType newInputType, IOConverter* newAlgorithm);

    //Overview: replaces the current axis's algorithm component and outputDevice components with different ones
    //
    //input:    the new type of input the axis is going to be given when runOutputControl is called
    //          the new algorithm module
    //          the new OutputDevice module
    //
    //returns:  true if swap was successful, false if not and previous settings retained
    //
    //warning:  not thread safe
    bool switchModules(ValueType newInputType, IOConverter* newAlgorithm, OutputDevice* newDevice);

    //Overview: replaces the current axis's outputDevice components with a different one
    //
    //input:    the new type of input the axis is going to be given when runOutputControl is called
    //          the new OutputDevice module
    //
    //returns:  true if swap was successful, false if not and previous settings retained
    //
    //warning:  not thread safe
    bool switchModules(ValueType newInputType, OutputDevice* newDevice);

    //Overview: tells axis stop using an IOConverter, IE values should be passes straight to the output.
    //
    //inputs:   the new type of input the axis is going to be given when runOutputControl is called
    //
    //returns:  true if swap was successful, false if not and previous settings retained
    //
    //warning:  not thread safe
    bool removeIOConverter(ValueType newInputType);
    
    //Overview: Tells axis to use a stopcapMechanism, this will tell the axis to listen to the passed mechanism for
    //          when it should stop moving such as limit switches.
    //inputs:   The stopcap to use
    //
    //returns:  fuck all
    //
    //warning:  not thread safe.
    void useStopcap(StopcapMechanism* stopcapToUse);

    //Overview: Tells axis to stop using a stopcapMechanism.
    //inputs:   fuck all
    //
    //returns:  fuck all, cause fuck you that's why
    //
    //warning: not thread safe.
    void removeStopcap();

    //Overview: tells axis to stop using an IOConverter, IE values should be passes straight to the output,
    //          and swap the axis's outputDevice with a different one.
    //
    //inputs:   the new type of input the axis is going to be given when runOutputControl is called the new OutputDevice module
    //
    //returns:  true if swap was successful, false if not and previous settings retained
    //
    //warning: not thread safe
    bool removeIOConverter(ValueType newInputType, OutputDevice* newDevice);

    //tells the axis to halt. Note that this won't keep the axis from moving if called again;
    //use disable for that
    virtual void stop() = 0;
    
    //turns the axis off; it will stop moving until enabled
    virtual void disableAxis() = 0;
    
    //turns the axis on after being disabled
    virtual void enableAxis() = 0;
};

//Represents a device (or any general method) used for getting sensory information.
//see README.md for more info
class FeedbackDevice
{
  protected:
    FeedbackDevice(ValueType type)
    : fType(type) {};

    ValueType fType;

  public:

    //Overview: returns the sensor's feedback information, such as last read position or speed, depending on
    //          what the feedback type is. Public for everything to see, if desired
    //
    //returns:  a value representing feedback, the range of the value depends on what input type this device returns.
    //          For example, if this device returns speed feedback, the values shall be in the range between SPEED_MIN and SPEED_MAX
    virtual long getFeedback() = 0;

    //Overview: checks to see what the status is of the feedback device. Call getFeedback() to update the status, then
    //          call this to see if the operation had any issues.
    virtual FeedbackDevice_Status getFeedbackStatus() = 0;

    ValueType getFeedbackType() { return fType; }
};

//represents the device (or any general method) used for physically causing movement, such as a motor controller.
//see README.md for more info
class OutputDevice
{
  //Axis interface needs access to its functions
  friend class SingleMotorAxis;
  friend class DifferentialAxis;
  friend class MotionAxis;

  public:
    
    //gets the current moving value of the device. 
    //return value depends on what value type the device operates on; a speed value for speed devices, etc.
    virtual long getCurrentMove() = 0;

  protected:

    //overview: calls the device to move the motor, based on the passed value of movement.
    //
    //Input:    Can be values based on any of the input types as defined in the enum above, and the ranges for these values
    //          should stay within the max and min constants for each type
    virtual void move(const long movement) =  0;

    //expected input that the output device wants.
    ValueType inType;

    //used for if the specific controller is mounted backwards on the motor axis.
    //True means invert the signal (backwards) False means just send the signal
    bool invert;

    //tracks whether or not the device is enabled/powered on by the axis manager
    bool enabled;
    
    OutputDevice(ValueType type, bool inv)
    : enabled(true), inType(type), invert(inv) {};
    
    //tells device to stop moving
    virtual void stop() = 0;
    
    //tells the device to power on or off.
    virtual void setPower(bool powerOn) = 0;
};

//Represents any algorithm used for converting one type of data (such as position) to another (such as velocity).
//See the README.md for more info
class IOConverter
{
  friend class SingleMotorAxis;
  friend class DifferentialAxis;
  friend class MotionAxis;

  public:

    //Overview: Adds a supporting IOConverter to be used in conjunction with this one; this IOConverter will calculate its own output,
    //          then call the supporting IOConverter and ask for its as well. In this way, multiple algorithms can be
    //          made in parallel and add up with each other.
    //
    //Input:    The supporting algorithm to add.
    //
    //Returns:  True if successfully added, false if there was an issue.
    //
    //Warning:  Supporting algorithm must have the same Input type and Output type as this IOConverter.
    //          Also this function doesn't stack; only one supporting algorithm attached at a time. If you want to use multiple
    //          supporting algorithms, string them to each other.
    bool addSupportingAlgorithm(IOConverter* support);

    //overview: run whatever algorithm this implements, returns value that can be directly passed to an output device
    //
    //input:    int in: representing the input values that need to be converted into output values.
    //                  The specific value constraints depend on the input and output types the algorithm implements; for example
    //                  if an algorithm is supposed to take in speed and output position, then its input is constrained by
    //                  SPEED_MIN and SPEED_MAX, and its output is constrained by POS_MIN and POS_MAX
    //          bool * ret_outputFinished: returned parameter passed by pointer
    //
    //returns:  ret_Output_finished: returns true if the axis has finished its controlled movement and ready to exit the control loop,
    //                               false if it's still in the middle of getting to its desired finished state IE in the middle of
    //                               moving to a desired end position or reaching a desired end speed.
    //          long: The calculated output to directly pass to the output device
    //
    //Note:     If ret_OutputFinished returns false but input returns 0, it's an indication that an error has occured
    virtual long runAlgorithm(const long input, IOConverter_Status * ret_status) = 0;

    //function to be called when class is acting as a support algorithm to another IOConverter.
    //Rather than being the driving force behind the movement, as it is when runAlgorithm is called, this signals to the class that it's
    //acting as a support to another IOConverter and should behave as such.
    //public due to being allowed to be called by all the different IOConverter classes, but
    //not meant to be called by the user directly. C++ needs an internal modifier.
    //
    //inputValue: The input that was passed to the IOConverter chain to act upon
    //calculatedOutput: The output calculated so far in the IOConverter chain, being added with each class's output when passed up the chain.
    //returns: The calculated value to add upon the output that the first IOConverter calculated.
    virtual long addToOutput(const long inputValue, const long calculatedOutput) = 0;

    //Sets whether or not the supporting algorithm coupled with this (if there's one) will continue to output data once the motion is completed.
    //If true, then the supporting algorithm will be allowed to keep compensating for whatever it's compensating for when the motion has stopped.
    void persistantSupport(bool persistant);

    ValueType getInType() { return inType; }
    ValueType getOutType() { return outType; }

  protected:

    IOConverter* supportingAlgorithm;
    bool supportUsed;
    bool supportIsPersistant;

    //the types of values that the algorithm takes in and gives out
    ValueType inType;
    ValueType outType;

    IOConverter(ValueType in, ValueType out)
    : supportUsed(false), supportingAlgorithm(0), inType(in), outType(out), supportIsPersistant(false)
    {};
};

//represents any kind of sensor or mechanism designed to alert the axis of motion that it needs to stop moving, in general
//or in a certain direction, such as a limit switch.
class StopcapMechanism
{
  protected:
    StopcapMechanism() {};

  public:
    virtual StopcapStatus getStopcapStatus() = 0;
};


#endif
