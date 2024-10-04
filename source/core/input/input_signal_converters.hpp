///
/// @file
/// @details Acouple simple InputSignalConverters that may eventually be moved directly into TurtleBrains.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef Core_InputSignalConverters_hpp
#define Core_InputSignalConverters_hpp

#include <turtle_brains/game/tb_input_action.hpp>

namespace TyreBytes
{
	namespace Core
	{
		namespace Input
		{

			class SignalConverter : public tbGame::InputSignalConverterInterface
			{
			public:
				SignalConverter(void);
				virtual ~SignalConverter(void);

				virtual float DigitalToAnalogConverter(bool digitalValue, float deltaTime) override;
				virtual bool AnalogToDigitalConverter(float analogValue, float deltaTime) override;
				virtual float AnalogToAnalogConverter(float analogValue, float deltaTime) override;
			private:
				float mAnalogValue;
			};

			class SteeringSignalConverter : public SignalConverter
			{
			public:
				SteeringSignalConverter(float minimum, float maximum);
				virtual ~SteeringSignalConverter(void);

				virtual float AnalogToAnalogConverter(float analogValue, float deltaTime) override;
				virtual float InvertAnalogValue(float analogValue) override;

			private:
				float mMinimum;
				float mMaximum;
			};

		}; //namespace Input
	};	//namespace Core
}; //namespace TyreBytes

#endif /* Core_InputSignalConverters_hpp */
