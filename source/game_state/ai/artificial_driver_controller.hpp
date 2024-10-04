///
/// @file
/// @details A racecar controller for an artifical driver to control the car.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_ArtificialDriverController_hpp
#define LudumDare56_ArtificialDriverController_hpp

#include "ludumdare56.hpp"

#include "../../game_state/race_session_state.hpp"
#include "../../game_state/racetrack_state.hpp"
#include "../../game_state/racecar_controller_interface.hpp"

namespace LudumDare56
{
	namespace GameState
	{
		class RacecarState;
		class DriverState;

		class ArtificialDriverController : public RacecarControllerInterface
		{
		public:
			ArtificialDriverController(const DriverIndex& driverIndex, const RacecarIndex& racecarIndex);
			virtual ~ArtificialDriverController(void);

#if !defined(ludumdare56_headless_build)
			///
			/// @details Sets a visualization to display the debug visuals for ALL artificial drivers. The visualization
			///   will not clear any visuals, so that will be required by whomever renders the visualizer.
			///
			/// @param visualizer Where to visualize the debug information, can be nullptr to disable the visuals.
			///
			static void SetDebugVisualizer(iceGraphics::Visualization* visualizer);
#endif /* !ludumdare56_headless_build */

		protected:
			virtual void OnUpdateControls(void);

		private:
			typedef RacetrackState::TrackNodeIndex TrackNodeIndex;

			TrackNodeIndex FindClosestTrackNode(void);

			const DriverState& mDriver;
			const RacecarState& mRacecar;
		};

	};

};	//namespace LudumDare56

#endif /* LudumDare56_ArtificialDriverController_hpp */
