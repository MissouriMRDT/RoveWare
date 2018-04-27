#ifndef DIFFERENTIALAXIS_H_
#define DIFFERENTIALAXIS_H_

#include "../AbstractFramework.h"
#include "../RoveMotionUtilities.h"

//this enum represents the different degrees of motion that a differential axis can provide. Each class instance will service
//one degree of movement.
enum DifferentialType
{
  //This describes moving the differential axis so that the shaft moves up and down
  DifferentialTilt,

  //This describes moving the differential axis so that the shaft rotates around
  DifferentialRotate
};

//Represents axiss that are differential in set up (ask a mechanical if you don't know). These kinds of axiss have at least two separate degrees
//of motion that can be used at the same time. In software, each degree of motion gets its own class instance so that they can be operated
//independantly or at the same time as desired.
//See README.md for more information.
class DifferentialAxis : public MotionAxis
{
	protected:

		OutputDevice* controller2;
    
    //the other axis with which this axis is coupled with
    DifferentialAxis* coupledAxis;

    DifferentialType thisAxisType;

    //tracks whether or not the Differential axis is coupled with another Differential axis; physically they always are,
    //but if for some reason the user wants to only use one motion on the differential axis, then they can choose
    //to not couple the axis with another.
    bool coupled;
    
	public:

    //variables to store the power of either motor; useful if this is in coupled mode, as the movements of one axis motion
    //affect the other so virtual power variables are used to track what this specific moton axis is trying
    //to move at, to compare with the other axis this is coupled with
    int motorOneVirtualPower;
    int motorTwoVirtualPower;
    
		//Overview: creates the axis interface with an IOConverter to translate commands for the output devices.
    //          Note both output devices have to have the same input type.
    //
    //Inputs:   DifferentialType: Whether this differnetial axis has tilt motion or rotate motion
		//          inputType: What kind of movement this axis should be controlled by, such as speed or position input.
		//          alg: the IOConverter used by this axis
		//          cont1: The first output device controlling the first motor on this axis
		//          cont2: The second output device controlling the second motor on this axis
		DifferentialAxis(DifferentialType axisType, ValueType inputType, IOConverter *alg, OutputDevice* cont1, OutputDevice* cont2);

		//Overview: creates axis interface without an IO Converter, IE where output is passed straight to the output device
		//
		//Input:    DifferentialType: Whether this differnetial axis has tilt motion or rotate motion
		//          Note both output devices are assumed to have the same input type
		//          inputType: What kind of movement this axis should be controlled by, such as speed or position input.
		//          cont1: The first output device controlling the first motor on this axis
		//          cont2: The second output device controlling the second motor on this axis
		DifferentialAxis(DifferentialType axisType, ValueType inputType, OutputDevice* cont1, OutputDevice* cont2);

		~DifferentialAxis();

		//Overview: runs algorithm for moving two motors together so that it moves the axis, rotating it or tilting it.
		//input:    movement, a long that represents the desired movement. Value constraints and meaning depend on the inputType.
		//          For example, if this axis runs off of speed input then the values are constrained between SPEED_MIN and SPEED_MAX
    //returns:  The status of attempting to control this axis. Such as if the output is running, if it's complete, if there was an error, etc
		AxisControlStatus runOutputControl(const long movement);

		//Overview: replaces the current axis's algorithm and two output device components with different ones
    //
    //inputs:   the new type of input the axis is going to be given when runOutputControl is called.
    //          the new algorithm module
		//          the two new output devices
    //
    //returns:  true if swap was successful, false if not and previous settings retained
    //
    //warning:  not thread safe
    bool switchDifferentialModules(ValueType newInputType, IOConverter* newAlgorithm, OutputDevice* newDevice1, OutputDevice* newDevice2);
    
    //Overview: couples this differential axis with another; differential axiss
    //          are intertwined so need to interact with each other to properly gauge output power
    //
    //Inputs:   The other differential axis to couple this one with. If this axis is a rotate axis then the other must be a tilt
    //          and vice versa; they can't be the same type.
    void pairDifferentialAxis(DifferentialAxis* otherAxis);
    
    //Overview: tells the axis to halt. Note that this won't keep the axis from moving if called again;
    //          use disable for that
    void stop();
    
    //Overview: turns the axis off; it will stop moving until enabled
    void disableAxis();
    
    //Overview: turns the axis on after being disabled
    void enableAxis();
};

#endif
