///
/// @file
/// @details Acouple simple InputSignalConverters that may eventually be moved directly into TurtleBrains.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "input_signal_converters.hpp"

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Input::SignalConverter::SignalConverter(void) :
	tbGame::InputSignalConverterInterface(),
	mAnalogValue(0.0f)
{
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Input::SignalConverter::~SignalConverter(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

float TyreBytes::Core::Input::SignalConverter::DigitalToAnalogConverter(bool digitalValue, float deltaTime)
{
	const float pressRate(2.0f * deltaTime);
	const float releaseRate(2.0f * deltaTime);

	if (true == digitalValue)
	{
		mAnalogValue += pressRate;
		if (mAnalogValue > 1.0f)
		{
			mAnalogValue = 1.0f;
		}
	}
	else
	{
		mAnalogValue -= releaseRate;
		if (mAnalogValue < 0.0f)
		{
			mAnalogValue = 0.0f;
		}
	}

	return mAnalogValue;
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Input::SignalConverter::AnalogToDigitalConverter(float analogValue, float /*deltaTime*/)
{
	return (analogValue > 0.5f) ? true : false;
}

//--------------------------------------------------------------------------------------------------------------------//

float TyreBytes::Core::Input::SignalConverter::AnalogToAnalogConverter(float analogValue, float /*deltaTime*/)
{
	return analogValue;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Input::SteeringSignalConverter::SteeringSignalConverter(float minimum, float maximum) :
	SignalConverter(),
	mMinimum(minimum),
	mMaximum(maximum)
{
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Input::SteeringSignalConverter::~SteeringSignalConverter(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

float TyreBytes::Core::Input::SteeringSignalConverter::AnalogToAnalogConverter(float analogValue, float /*deltaTime*/)
{
	if (mMinimum > mMaximum)
	{
		return tbMath::Clamp(1.0f - (analogValue - mMaximum) / (mMinimum - mMaximum), 0.0f, 1.0f);
	}
	return tbMath::Clamp((analogValue - mMinimum) / (mMaximum - mMinimum), 0.0f, 1.0f);
}

//--------------------------------------------------------------------------------------------------------------------//

float TyreBytes::Core::Input::SteeringSignalConverter::InvertAnalogValue(float analogValue)
{
	//tb_error("rorError: Does not make sense to invert a steering axis.");
	return 1.0f - analogValue;
};

//--------------------------------------------------------------------------------------------------------------------//
