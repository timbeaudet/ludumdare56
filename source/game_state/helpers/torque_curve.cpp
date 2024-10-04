///
/// @file
/// @details Contain and manage the information about each racecar on the server.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "torque_curve.hpp"
#include "../../logging.hpp"

#include <algorithm>

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::TorqueCurve LudumDare56::GameState::TorqueCurve::MiataTorqueCurve(void)
{	//http://www.automobile-catalog.com/curve/1999/1667030/mazda_mx-5_1_9.html
	TorqueCurve curve;
	curve.AddPlotPoint(500, 25.0);
	curve.AddPlotPoint(1000, 75.0);
	curve.AddPlotPoint(1500, 112.0);
	curve.AddPlotPoint(2000, 130.0);
	curve.AddPlotPoint(2500, 137.0);
	curve.AddPlotPoint(3000, 150.0);
	curve.AddPlotPoint(3500, 155.0);
	curve.AddPlotPoint(4000, 158.0);
	curve.AddPlotPoint(4500, 162.0);
	curve.AddPlotPoint(5000, 160.0);
	curve.AddPlotPoint(5500, 159.0);
	curve.AddPlotPoint(6000, 156.5);
	curve.AddPlotPoint(6500, 151.0);
	curve.AddPlotPoint(7000, 127.0);
	curve.AddPlotPoint(7500, 25.0);
	curve.AddPlotPoint(8000, 0.0);
	curve.NormalizeTorqueCurve();
	return curve;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::TorqueCurve::TorqueCurve(void) :
	mTorqueTable(),
	mMaximumTorque(0.0),
	mIsNormalized(false)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::TorqueCurve::~TorqueCurve(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::TorqueCurve::AddPlotPoint(const icePhysics::Scalar engineSpeedRPM, const icePhysics::Scalar torque)
{
	tb_error_if(true == mIsNormalized, "Cannot add more plot points to a table that is already normalized.");
	tb_error_if(engineSpeedRPM < 0.0, "Cannot add plot point for engine speeds less than zero.");
	tb_error_if(torque < 0.0, "Cannot add plot point for torque amounts that are less than zero.");

	auto findItr = std::find_if(mTorqueTable.begin(), mTorqueTable.end(), [engineSpeedRPM](PlotPoint& pt) { return fabs(pt.first - engineSpeedRPM) < 0.1; });
	tb_error_if(mTorqueTable.end() != findItr, "Cannot plot a point on top of another point!");

	PlotPoint pt(engineSpeedRPM, torque);
	mTorqueTable.push_back(pt);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::TorqueCurve::NormalizeTorqueCurve(void)
{
	tb_error_if(true == mTorqueTable.empty(), "Cannot normalize a table without plotted points. Call AddPlotPoint() to make it interesting.");

	std::sort(mTorqueTable.begin(), mTorqueTable.end(), [](PlotPoint& a, PlotPoint& b) { return a.second < b.second; });
	mMaximumTorque = mTorqueTable.back().second;

	std::sort(mTorqueTable.begin(), mTorqueTable.end(), [](PlotPoint& a, PlotPoint& b) { return a.first < b.first; });
	std::for_each(mTorqueTable.begin(), mTorqueTable.end(), [this](PlotPoint& pt) { pt.second /= mMaximumTorque; });

	mIsNormalized = true;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Scalar LudumDare56::GameState::TorqueCurve::GetMaximumTorque(void) const
{
	return mMaximumTorque;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Scalar LudumDare56::GameState::TorqueCurve::GetOutputTorque(const icePhysics::Scalar engineSpeedRPM) const
{
	return GetOutputValue(engineSpeedRPM) * mMaximumTorque;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Scalar LudumDare56::GameState::TorqueCurve::GetOutputValue(const icePhysics::Scalar engineSpeedRPM) const
{
	tb_error_if(false == mIsNormalized, "Cannot get output of a TorqueCurve that has not been normalized. Call NormalizeTorqueCurve().");

	PlotPoint previousPoint = mTorqueTable.front();
	if (engineSpeedRPM < previousPoint.first)
	{	//The RPM of the engine is lower than the lowest in torque table.
		return previousPoint.second;
	}

	for (size_t index(1); index < kTorqueTableSize; ++index)
	{
		const PlotPoint& currentPoint(mTorqueTable[index]);
		const icePhysics::Scalar& currentRPM(currentPoint.first);
		const icePhysics::Scalar& currentTorque(currentPoint.second);

		if (engineSpeedRPM > currentRPM)
		{
			previousPoint = currentPoint;
			continue;
		}

		const icePhysics::Scalar& previousRPM(previousPoint.first);
		const icePhysics::Scalar& previousTorque(previousPoint.second);
		const icePhysics::Scalar percentage = icePhysics::Scalar(1.0 - ((currentRPM - engineSpeedRPM) / (currentRPM - previousRPM)));
		return previousTorque + ((currentTorque - previousTorque) * percentage);
	}

	tb_debug_log(LogPhysics::Warning() << "Value not found for RPM: " << engineSpeedRPM << " in torque table.");
	return mTorqueTable.back().second;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Scalar LudumDare56::GameState::TorqueCurve::GetMaximumRPM(void) const
{
	tb_error_if(false == mIsNormalized, "Cannot get the Maximum RPM of a TorqueCurve that has not been normalized. Call NormalizeTorqueCurve().");
	return mTorqueTable.back().first;
}

//--------------------------------------------------------------------------------------------------------------------//
