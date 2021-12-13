#pragma once
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

class LowPassFilter
{
public:

	LowPassFilter() :
		output(0),
		ePow(0)
	{
	}

	LowPassFilter(double iCutOffFrequency, double iDeltaTime) :
		output(0),
		ePow(1.0 - exp(-iDeltaTime * 2 * M_PI * iCutOffFrequency))
	{
		if (iDeltaTime <= 0)
		{
			ePow = 0;
		}
		if (iCutOffFrequency <= 0)
		{
			ePow = 0;
		}
	}

	double update(double input)
	{
		return output += (input - output) * ePow;
	}

	double update(double input, double deltaTime, double cutoffFrequency)
	{
		reconfigureFilter(deltaTime, cutoffFrequency); //Changes ePow accordingly.
		return output += (input - output) * ePow;
	}

	void reconfigureFilter(double deltaTime, double cutoffFrequency)
	{
		if (deltaTime <= 0)
		{
			ePow = 0;
		}
		if (cutoffFrequency <= 0)
		{
			ePow = 0;
		}
		ePow = 1 - exp(-deltaTime * 2 * M_PI * cutoffFrequency);
	}

private:
	double output;
	double ePow;
};