///
/// @file
/// @details Some implementation details regarding the racetrack that should be shared only within GameState.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_state/implementation/racetrack_implementation.hpp"
#include "../../ludumdare56.hpp"
#include "../../logging.hpp"

namespace LudumDare56
{
	namespace GameState
	{
		namespace Implementation
		{


		};	//namespace Implementation
	};	//namespace GameState
};	//namespace LudumDare56

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::Implementation::TrackNodeContainer& LudumDare56::GameState::Implementation::TheTrackNodes(void)
{
	return TheMutableTrackNodes();
}

LudumDare56::GameState::Implementation::TrackNodeContainer& LudumDare56::GameState::Implementation::TheMutableTrackNodes(void)
{
	static TrackNodeContainer theRealTrackNodes;
	return theRealTrackNodes;
}

//--------------------------------------------------------------------------------------------------------------------//
