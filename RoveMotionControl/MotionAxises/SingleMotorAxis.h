#ifndef ROVEJOINTCONTROL_SINGLEMOTORJOINT_H_
#define ROVEJOINTCONTROL_SINGLEMOTORJOINT_H_

#include "../AbstractFramework.h"
#include "../RoveMotionUtilities.h"

//represents an arm axis that just has one motor controlling its motion.
//see the readme.md for more info.
class SingleMotorAxis : public MotionAxis
{
	public:

    //Overview: creates axis interface without an IO Converter, IE where output is passed straight to the output device.
    //
    //Inputs:   inputType: What kind of movement this axis should be controlled by, such as speed or position input.
    //          cont: The output device controlling the first motor on this axis
		SingleMotorAxis(ValueType inputType, OutputDevice * cont);

    //Overview: creates the axis interface with an IOConverter to translate commands for the output devices.
		//
		//Inputs: inputType: What kind of movement this axis should be controlled by, such as speed or position input.
    //        alg: the IOConverter used by this axis
    //        cont: The first output device controlling the first motor on this axis
		SingleMotorAxis(ValueType inputType, IOConverter *alg, OutputDevice* cont);

		~SingleMotorAxis();

		//Overview: runs control algorithm for a axis so that it moves.
		//
		//Inputs: input: a long that represents the desired movement. Value constraints and meaning depend on the inputType.
    //               For example, if this axis runs off of speed input then the values are constrained between
		//               SPEED_MIN and SPEED_MAX, and otherwise for the other types.
		//
    //returns: The status of attempting to control this axis. Such as if the output is running, if it's complete, if there was an error, etc
		AxisControlStatus runOutputControl(const long movement);
    
    //tells the axis to halt. Note that this won't keep the axis from moving if called again;
    //use disable for that
    void stop();
    
    //turns the axis off; it will stop moving until enabled
    void disableAxis();
    
    //turns the axis on after being disabled
    void enableAxis();
};

#endif
