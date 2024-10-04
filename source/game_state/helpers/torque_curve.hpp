///
/// @file
/// @details Create a Torque/Power curve for an engine to lookup how much torque it applies at a given RPM.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_TorqueCurve_hpp
#define LudumDare56_TorqueCurve_hpp

#include <ice/physics/ice_physical_types.hpp>

#include <vector>

namespace LudumDare56
{
	namespace GameState
	{

		class TorqueCurve
		{
		public:
			static TorqueCurve MiataTorqueCurve(void);

			TorqueCurve(void);
			~TorqueCurve(void);

			///
			/// @details Inserts a point for the curve to follow a more realistic torque/power curve of an internal combustion engine.
			///
			/// @param engineSpeedRPM Must be a positive value representing the speed of the engine in revolutions-per-minute.
			/// @param torque Must be a positive value representing the torque produced at engineSpeedRPM.
			///
			/// @note Cannot be called once the TorqueCurve object has been normalized or an error condition will be triggered.
			///
			void AddPlotPoint(const icePhysics::Scalar engineSpeedRPM, const icePhysics::Scalar torque);

			///
			/// @details Finds the maximum torque value in the table and normalizes all values to be within 0.0 to 1.0.
			///
			void NormalizeTorqueCurve(void);

			///
			/// @details Will return true if the TorqueTable has been normalized, "set in stone."
			///
			inline bool IsNormalized(void) const { return mIsNormalized; }

			///
			/// @details Returns the maximum amount of torque in Nm (Newton-meters) of the engine.
			///
			icePhysics::Scalar GetMaximumTorque(void) const;

			///
			/// @details Returns the maximum torque output of the engine at the given engine speed in Nm (Newton-meters).
			///
			icePhysics::Scalar GetOutputTorque(const icePhysics::Scalar engineSpeedRPM) const;

			icePhysics::Scalar GetMaximumRPM(void) const;

		private:

			///
			/// @details Returns a value from 0 to 1 representing a percentage of the maximum torque at this given engine speed.
			///
			icePhysics::Scalar GetOutputValue(const icePhysics::Scalar engineSpeedRPM) const;

			static const size_t kTorqueTableSize = 16;

			typedef std::pair<icePhysics::Scalar, icePhysics::Scalar> PlotPoint; //RPM, NormalizedTorque
			std::vector<PlotPoint> mTorqueTable;
			icePhysics::Scalar mMaximumTorque;  //In Nm
			bool mIsNormalized;
		};

	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_TorqueCurve_hpp */
