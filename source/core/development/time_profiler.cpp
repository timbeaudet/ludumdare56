///
/// @file
/// @details Provide a small object to assist in performance tracking and monitoring.
/// @history File first started with Rally of Rockets in late 2021, then Trailing Brakes Racing Simulator and beyond.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#if defined(development_build)

#include "time_profiler.hpp"
#include "tb_imgui_implementation.hpp"

#include <turtle_brains/core/tb_defines.hpp>
#include <turtle_brains/core/debug/tb_debug_logger.hpp>

#include <vector>

//--------------------------------------------------------------------------------------------------------------------//

const tbCore::uint64 kInfinity(0xFFFFFFFFFFFFFFFF);

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Development::TimeProfiler::TimeProfiler(const tbCore::tbString& name) :
	mName(name),
	mTimer(),
	mExpectedPeak(kInfinity), //This will get setup by the FrameProfiler::ctor for Simulate, Update and Render timers.
	mMaximumPeak(0),
	mMinimumPeak(kInfinity),
	mMaximumPeakEver(0),
	mMinimumPeakEver(kInfinity),
	mAverageTime(0),
	mLastDeltaTime(0),
	mTotalTimeEver(0),
	mAverageTimeEver(0),
	mEverLapCount(0),
	mTotalTime(0),
	mLapCount(0)
{
	mTimer.Reset();
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Development::TimeProfiler::~TimeProfiler(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::TimeProfiler::Start(void)
{
	if (mLapCount > 100)
	{
		Reset();
	}

	if (0 == (mEverLapCount % 1000))
	{
		tb_log("%s average: %s\n", mName.c_str(), (tbCore::ToString<float>(static_cast<float>(GetMaximumPeak()) / 1000.0f) + " ms").c_str());
	}

	mTimer.Reset();
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::TimeProfiler::Stop(void)
{
	mTimer.Update();

	mLastDeltaTime = static_cast<tbCore::uint64>(mTimer.GetDeltaTime() * 1000 * 1000);

	if (mLastDeltaTime > mMaximumPeakEver)
	{
		mMaximumPeakEver = mLastDeltaTime;
	}

	if (mLastDeltaTime < mMinimumPeakEver)
	{
		mMinimumPeakEver = mLastDeltaTime;
	}

	if (mLastDeltaTime > mMaximumPeak)
	{
		mMaximumPeak = mLastDeltaTime;
	}

	if (mLastDeltaTime < mMinimumPeak)
	{
		mMinimumPeak = mLastDeltaTime;
	}

	++mLapCount;
	mTotalTime += mLastDeltaTime;
	mAverageTime = mTotalTime / mLapCount;

	++mEverLapCount;
	mTotalTimeEver += mLastDeltaTime;
	mAverageTimeEver = mTotalTimeEver / mEverLapCount;
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::TimeProfiler::Reset(bool resetEver)
{
	if (true == resetEver)
	{
		mMaximumPeakEver = 0;
		mMinimumPeakEver = kInfinity;
		mTotalTimeEver = 0;
		mAverageTimeEver = 0;
		mEverLapCount = 0;
	}

	mMaximumPeak = 0;
	mMinimumPeak = kInfinity;
	mLapCount = 0;
}

//--------------------------------------------------------------------------------------------------------------------//

void PushBackOrShiftForward(std::vector<float>& container, float value, size_t maximumSize)
{
	if (container.size() < maximumSize)
	{
		container.push_back(value);
	}
	else
	{
		for (size_t index = 0; index < container.size() - 1; ++index)
		{
			container[index] = container[index + 1];
		}
		container.back() = value;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::TimeProfiler::ImGuiShowPerformance(const TimeProfiler& simulateTimer,
	const TimeProfiler& updateTimer, const TimeProfiler& renderTimer)
{
#if defined(development_build)
	/// @note When ImGui says RGBA, they actually mean ABGR as in input (seems like endianness flips).
	const ImVec4 kErrorColor = ImVec4(ImColor(0xFF257EED));


	if (true == ImGui::CollapsingHeader("Peformance", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const size_t maximumSize(100);
		static std::vector<float> simulate;
		static std::vector<float> update;
		static std::vector<float> render;
		static std::vector<float> total;

		//TODO: RallyOfRockets: 2022-05-20: The simulate graph is a very rough representation because Simulate() may
		//  be called 0 to N times per frame, and this function is currently called once a frame. This means with
		//  higher framerate the previous simulate times will get used multiple times, and with lower framerates some
		//  of the simulate times will be skipped if it ran more than once in a frame.
		PushBackOrShiftForward(simulate, static_cast<float>(simulateTimer.GetLastTime()), maximumSize);
		PushBackOrShiftForward(update, static_cast<float>(updateTimer.GetLastTime()), maximumSize);
		PushBackOrShiftForward(render, static_cast<float>(renderTimer.GetLastTime()), maximumSize);
		PushBackOrShiftForward(total, static_cast<float>(simulateTimer.GetLastTime() + updateTimer.GetLastTime() + renderTimer.GetLastTime()), maximumSize);

		bool peakingTooHigh = false;

		{
			int pushedColors = 0;
			if (simulateTimer.GetMaximumPeak() > simulateTimer.GetExpectedPeak())
			{
				ImGui::PushStyleColor(ImGuiCol_Text, kErrorColor);
				ImGui::PushStyleColor(ImGuiCol_PlotLines, kErrorColor);
				pushedColors = 2;
				peakingTooHigh = true;
			}

			ImGui::PlotLines(("Simulate: " + tbCore::ToString<float>(static_cast<float>(simulateTimer.GetMaximumPeak()) / 1000.0f) + " ms").c_str(),
				simulate.data(), tbCore::size(simulate.size()), 0, nullptr, 0.0f, static_cast<float>(simulateTimer.GetExpectedPeak()), ImVec2(0, 40.0f));
			ImGui::PopStyleColor(pushedColors);
		}

		{
			int pushedColors = 0;
			if (updateTimer.GetMaximumPeak() > updateTimer.GetExpectedPeak())
			{
				ImGui::PushStyleColor(ImGuiCol_Text, kErrorColor);
				ImGui::PushStyleColor(ImGuiCol_PlotLines, kErrorColor);
				pushedColors = 2;
				peakingTooHigh = true;
			}

			ImGui::PlotLines(("Update: " + tbCore::ToString<float>(static_cast<float>(updateTimer.GetMaximumPeak()) / 1000.0f) + " ms").c_str(),
				update.data(), tbCore::size(update.size()), 0, nullptr, 0.0f, static_cast<float>(updateTimer.GetExpectedPeak()), ImVec2(0, 40.0f));
			ImGui::PopStyleColor(pushedColors);
		}

		{
			int pushedColors = 0;
			if (renderTimer.GetMaximumPeak() > renderTimer.GetExpectedPeak())
			{
				ImGui::PushStyleColor(ImGuiCol_Text, kErrorColor);
				ImGui::PushStyleColor(ImGuiCol_PlotLines, kErrorColor);
				pushedColors = 2;
				peakingTooHigh = true;
			}

			ImGui::PlotLines(("Render: " + tbCore::ToString<float>(static_cast<float>(renderTimer.GetMaximumPeak()) / 1000.0f) + " ms").c_str(),
				render.data(), tbCore::size(render.size()), 0, nullptr, 0.0f, static_cast<float>(renderTimer.GetExpectedPeak()), ImVec2(0, 40.0f));
			ImGui::PopStyleColor(pushedColors);
		}

		{
			int pushedColors = 0;
			if (true == peakingTooHigh)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, kErrorColor);
				ImGui::PushStyleColor(ImGuiCol_PlotLines, kErrorColor);
				pushedColors = 2;
			}
			ImGui::PlotLines(("Total: " + tbCore::ToString<float>(static_cast<float>(simulateTimer.GetMaximumPeak() + updateTimer.GetMaximumPeak() + renderTimer.GetMaximumPeak()) / 1000.0f) + " ms").c_str(),
				total.data(), tbCore::size(total.size()), 0, nullptr, 0.0f, 16000.0f, ImVec2(0, 40.0f));
			ImGui::PopStyleColor(pushedColors);
		}
	}
#else
	tb_unused(simulateTimer);
	tb_unused(updateTimer);
	tb_unused(renderTimer);
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Development::FrameProfiler::FrameProfiler(void) :
	mTimers{ TimeProfiler("Simulate"), TimeProfiler("Update"), TimeProfiler("Render"), }
{
	//const tbCore::uint64 tenthsOfAMillisecond = 100;
	const tbCore::uint64 millisecond = 1000;
	mTimers[Channel::kSimulate].SetExpectedPeak(2 * millisecond);
	mTimers[Channel::kUpdate].SetExpectedPeak(1 * millisecond);
	mTimers[Channel::kRender].SetExpectedPeak(8 * millisecond);
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Development::FrameProfiler::~FrameProfiler(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::FrameProfiler::Start(Channel channel)
{
	mTimers[channel].Start();
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::FrameProfiler::Stop(Channel channel)
{
	mTimers[channel].Stop();
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::FrameProfiler::Reset(bool resetEver)
{
	for (TimeProfiler& timer : mTimers)
	{
		timer.Reset(resetEver);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::FrameProfiler::ImGuiShowPerformance(const FrameProfiler& profiler)
{
	TimeProfiler::ImGuiShowPerformance(profiler.mTimers[kSimulate],
		profiler.mTimers[kUpdate], profiler.mTimers[kRender]);
}

//--------------------------------------------------------------------------------------------------------------------//

#endif /* development_build */
