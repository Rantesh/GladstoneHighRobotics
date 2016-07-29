#include "WPILib.h"
#include "iostream"
#include "fstream"

class MecanumDefaultCode : public IterativeRobot
{
	int counter = 0;
	int recharge = 0;
	int target = 0;
	double i=0;
	double elevateRamp=0;
	double high = 0;
	double low = 0;
	bool isMovingUp = false;
	bool moveDownClosed = false;
	bool moveUpClosed = false;

	/*
	 * Initializing the file instream and outstream.
	 * The instream will put things into the volatile (changing with every restart) code.
	 * The outstream will take things out of the volatile code and put it into the file.
	 */
	std::ifstream instream;
	std::ofstream outstream;
	// Variables used to determine the values of the five different levels.
	float level[4];

	void getLevels()
	{
		/*
		 * getLevels() opens a file, puts the values of every level onto
		 * volatile variables which can then be used in the code.
		 */
		instream.open("/home/lvuser/Levels.txt");
		instream >> level[0] >> level[1] >> level[2] >> level[3];
		instream.close();
	}
	void setLevels()
	{
		/*
		 * setLevels() opens a file, puts the values of all the levels into the file, and then closes it.
		 * NOTE: Never use setLevels() before you use getLevels() for the first time in your code!
		 * If you do we will have to rewrite every single level.
		 */
		outstream.open("/home/lvuser/Levels.txt");
		outstream << level[0] << "\n" << level[1] << "\n" << level[2] << "\n" << level[3] << "\n";
		outstream.close();
	}
	void manipIO(){
		// Controls the input and output wheels for the manipulator
		if (driveStick->GetRawButton(5)){
			manipTalon.Set(i);
			boulderVic->Set(i);
			if (i<1){
				i=i+0.05;
			}
		}
		else if (driveStick->GetRawAxis(2)){
			manipTalon.Set(-i);
			boulderVic->Set(-i);
			if (i<1){
				i=i+0.05;
			}
		}
		else {
			manipTalon.Set(0);
			boulderVic->Set(0);
			i=0;
		}
	}
	void resetLevels(){
		/*
		 * Resets the level system for the manipulator.
		 * Press select to reset encoder position to 0.
		 * Press up on the POV and select to set the current position to level 3.
		 * Press left on the POV and select to set the current position to level 2.
		 * Press down on the POV and select to set the current position to level 0.
		 * Press right on the POV and select to set the current position to level 1.
		 */
		if (driveStick->GetRawButton(7) || driveStick->GetRawAxis(8)){
			switch(driveStick->GetPOV()){
				case(-1):
					manipTalon.SetPosition(0);
					break;
				case(0):
					level[3] = manipTalon.GetPosition();
					break;
				case(90):
					level[2] = manipTalon.GetPosition();
					break;
				case(180):
					level[0] = manipTalon.GetPosition();
					break;
				case(270):
					level[1] = manipTalon.GetPosition();
					break;
			}
			/*
			 * Resets the PERMANENT level system for the manipulator.  If you are only pressing select it will be a volatile change.
			 */
			if (driveStick->GetRawButton(8)){
				setLevels();
			}
		}
	}
	int deadband(int i){
		/*
		 * Function: deadband
		 * Task: create a deadband for the joysticks on the controller
		 * Input: i
		 * Outputs 0 if within the range or a value between -1 to 1 depending on i
		 */

		int range = 0.1;

		// if i is above the range return a value between 0 and 1
		if (i >= range){
			return (i - 1)/ (1-range) +1;
		}
		// if -i is above the range return a value between 0 and -1
		else if (i <= -range){
			return (i + 1)/ (1 - range)-1;
		}
		// else return 0
		else{
			return 0;
		}
	}
	void manipElevate(){
		// Raises and lowers manipulator
		if (driveStick->GetRawButton(6)) {
			manipTalon.Set(elevateRamp);
			if (elevateRamp < 0.7){
				elevateRamp = elevateRamp + 0.05;
			}
		}
		else if (driveStick->GetRawAxis(3)){
			manipTalon.Set(-elevateRamp);
			if (elevateRamp < 0.7){
				elevateRamp = elevateRamp + 0.05;
			}
		}
/*		else if (manipTalon.IsFwdLimitSwitchClosed() || manipTalon.IsRevLimitSwitchClosed()){
			manipTalon.Set(0);
			elevateRamp = 0;
			isMovingUp = false;


		 	// Changes the level limits of the manipTalon object
			if (manipTalon.GetPosition() <= level[0] + 0.05 && manipTalon.GetPosition() >= level[0] - 0.05){
				manipTalon.ConfigSoftPositionLimits(level[0], level[1]);
			}
			else if (manipTalon.GetPosition() <= level[1] + 0.05 && manipTalon.GetPosition() >= level[1] - 0.05){
			manipTalon.ConfigSoftPositionLimits(level[0], level[2]);
			}
			else if (manipTalon.GetPosition() <= level[2] + 0.05 && manipTalon.GetPosition() >= level[2] - 0.05){
			manipTalon.ConfigSoftPositionLimits(level[1], level[3]);
			}
			else if (manipTalon.GetPosition() <= level[3] + 0.05 && manipTalon.GetPosition() >= level[3] - 0.05){
			manipTalon.ConfigSoftPositionLimits(level[2], level[3]);
			}
		}
		 */
		else {
			manipTalon.Set(0);
			elevateRamp = 0;
		}
	}
	void servoControl(){
		// Servo control
		if (driveStick->GetRawButton(3)){
			Servant->Set(1);
		}
		else{
			Servant->Set(0);
		}
	}
	void manipElevateB(){
		if (driveStick->GetRawButton(1) && target < 3 && recharge < 0){
			target++;
			recharge = 500;
		}
		else if (driveStick->GetRawButton(2) && target > 0 && recharge < 0){
			target--;
			recharge = 500;
		}
		recharge--;

		if (manipTalon.GetEncPosition() <= level[0]){
			moveDownClosed = true;
		}
		else if (manipTalon.GetEncPosition() >= level[3]){
			moveUpClosed = true;
		}
		else if (manipTalon.GetEncPosition() > level[target] - 0.05 && !moveDownClosed){
			manipTalon.Set(elevateRamp);
			elevateRamp = elevateRamp - 0.05;
		}
		else if (manipTalon.GetEncPosition() < level[target] + 0.05 && !moveUpClosed && !topLimit->Get()){
			manipTalon.Set(elevateRamp);
			elevateRamp = elevateRamp + 0.05;
		}
		else {
			manipTalon.Set(0);
			elevateRamp = 0;
		}
	}
	void manipElevateVic(){
		// Raises and lowers manipulator
		if (driveStick->GetRawButton(6)) {
			manipVic->Set(elevateRamp);
			manipTalon.Set(elevateRamp);
			if (elevateRamp < 0.7){
				elevateRamp = elevateRamp + 0.05;
			}
		}
		else if (driveStick->GetRawAxis(3)){
			manipVic->Set(elevateRamp);
			manipTalon.Set(elevateRamp);
			if (elevateRamp < 0.7){
				elevateRamp = elevateRamp - 0.05;
			}
		}
		else {
			elevateRamp = 0;
			manipVic->Set(elevateRamp);
			manipTalon.Set(elevateRamp);
		}
	}

	Victor *manipVic;
	CANTalon manipTalon;
	Servo *Servant;
	Joystick *driveStick;
	RobotDrive *drive;
	Victor *boulderVic;
	DigitalInput *topLimit;

public:
	MecanumDefaultCode(void) : manipTalon(0)
	{
/*		manipTalon.SetFeedbackDevice(CANTalon::QuadEncoder);  // Tells the Talon that it is dealing with a Quadrature Encoder
		manipTalon.SetPosition(0);    // Set Encoder position to 0
*/		manipTalon.DisableSoftPositionLimits();		// Is supposed to disable encoder limits
//		manipTalon.ConfigLimitMode(CANSpeedController::kLimitMode_SrxDisableSwitchInputs);	// Is supposed to disable all limits
		manipVic = new Victor(4);
		Servant = new Servo(3);
		driveStick = new Joystick(0);
		drive = new RobotDrive(0,1);
		boulderVic = new Victor(2);
		topLimit = new DigitalInput(0);

	}

	void RobotInit()
	{
		getLevels();
	}

	void AutonomousInit()
	{
		// This will set the first level to the value of the encoder.
		// Level[0] = manipTalon.GetEncPosition();
		counter = 0;
	}

	void AutonomousPeriodic(void)
	{
		if (counter < 1000) {
			drive->ArcadeDrive(0.5,0);
			counter++;
		}
		else {
			drive->ArcadeDrive(0);
		}
	}

	void TeleopPeriodic(void)
	{
		// Basic arcade drive using both joysticks
//		drive->ArcadeDrive(driveStick->GetRawAxis(1),-driveStick->GetRawAxis(4));

		// Basic tank drive
		drive->TankDrive(driveStick->GetRawAxis(5),driveStick->GetRawAxis(1));

		manipIO();
		servoControl();
	}
};
START_ROBOT_CLASS(MecanumDefaultCode);
