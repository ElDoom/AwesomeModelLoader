#pragma once
#include<mutex>
#include "Config.h"

extern std::mutex joystickMutex;

extern GLFWwindow* window;

struct JoystickState
{
	int axisCount; //how many axes there are for this joystick...yes, "axes" is the plural of "axis", I know it looks funny
	const float *axisStates; //array of axisCount floats, between -1 and 1 in value
	int buttonCount; //how many buttons there are for this joystick
	const unsigned char *buttonStates; //array of buttonCount unsigned chars, will either be GLFW_PRESS or GLFW_RELEASE in value
};

class Joystick
{
public:
	bool foundJoystick = false;
	JoystickState joystickState;
	int joystickNumber;
	float joystickPositionAxis1 = 0.f;
	float joystickPositionAxis2 = 0.f;
	float joystickPositionAxis4 = 0.f;
	float joystickPositionAxis3 = 0.f;

	void ProcessJoystickAxis(float &joystickAxis, Configuration configuration);
	bool PollJoystick();
	void DoJoystick(void);
	bool Init();
};