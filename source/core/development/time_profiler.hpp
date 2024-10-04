///
/// @file
/// @details Provide a small object to assist in performance tracking and monitoring.
/// @history File first started with Rally of Rockets in late 2021, then Trailing Brakes Racing Simulator and beyond.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef Core_TimeProfiler_hpp
#define Core_TimeProfiler_hpp
#if defined(development_build)

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_string.hpp>
#include <turtle_brains/system/tb_system_timer.hpp>

#include <array>

namespace TyreBytes
{
	namespace Core
	{
		namespace Development
		{

			class TimeProfiler
			{
			public:
				static void ImGuiShowPerformance(const Development::TimeProfiler& simulateTimer,
					const Development::TimeProfiler& updateTimer, const Development::TimeProfiler& renderTimer);

				explicit TimeProfiler(const tbCore::tbString& name);
				virtual ~TimeProfiler(void);

				///
				/// @note Every 100 calls this will Reset the minimum and maximum peaks.
				///
				void Start(void);
				void Stop(void);

				///
				/// @note This is called automatically every 100 calls to Start().
				///
				void Reset(bool resetEver = false);

				tbCore::uint64 GetMaximumPeak(void) const { return mMaximumPeak; }
				tbCore::uint64 GetMinimumPeak(void) const { return mMinimumPeak; }
				tbCore::uint64 GetMaximumPeakEver(void) const { return mMaximumPeakEver; }
				tbCore::uint64 GetMinimumPeakEver(void) const { return mMinimumPeakEver; }

				tbCore::uint64 GetExpectedPeak(void) const { return mExpectedPeak; }

				tbCore::uint64 GetAverageTime(void) const { return mAverageTime; }
				tbCore::uint64 GetLastTime(void) const { return mLastDeltaTime; }

				void SetExpectedPeak(const tbCore::uint64 expectedPeak) { mExpectedPeak = expectedPeak; }

			private:
				const tbCore::tbString mName;
				tbSystem::Timer::Timer mTimer;

				tbCore::uint64 mExpectedPeak;
				tbCore::uint64 mMaximumPeak;
				tbCore::uint64 mMinimumPeak;
				tbCore::uint64 mMaximumPeakEver;
				tbCore::uint64 mMinimumPeakEver;
				tbCore::uint64 mAverageTime;
				tbCore::uint64 mLastDeltaTime;

				tbCore::uint64 mTotalTimeEver;
				tbCore::uint64 mAverageTimeEver;
				int mEverLapCount;

				tbCore::uint64 mTotalTime;
				int mLapCount;
			};

			class FrameProfiler
			{
			public:
				enum Channel { kSimulate, kUpdate, kRender, kTotalChannels };

				FrameProfiler(void);
				~FrameProfiler(void);

				void Start(Channel channel);
				void Stop(Channel channel);
				void Reset(bool resetEver = false);

				static void ImGuiShowPerformance(const Development::FrameProfiler& profiler);

			private:
				std::array<TimeProfiler, kTotalChannels> mTimers;
			};

			enum ProfileDisplayType { Hidden, Small, Graph };
			void ProfileDisplay(TimeProfiler& profiler, ProfileDisplayType& type);

		};	//namespace Development
	};	//namespace Core
};	//namespace TyreBytes

#endif /* development_build */
#endif /* Core_TimeProfiler_hpp */
