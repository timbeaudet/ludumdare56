///
/// @file
/// @details Some implementation details regarding the racetrack that should be shared only within GameState.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacetrackImplementation_hpp
#define LudumDare56_RacetrackImplementation_hpp

#include <ice/physics/ice_bounding_volumes.hpp>

#include <vector>

namespace LudumDare56
{
	namespace GameState
	{
		namespace Implementation
		{

			struct TrackNode
			{
				icePhysics::BoundingPlane mLeadingPlane;
				icePhysics::BoundingPlane mTrailingPlane;
				icePhysics::BoundingPlane mLeftPlane;
				icePhysics::BoundingPlane mRightPlane;
			};

			typedef std::vector<TrackNode> TrackNodeContainer;

			const TrackNodeContainer& TheTrackNodes(void);
			TrackNodeContainer& TheMutableTrackNodes(void);

		};	//namespace Implementation
	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_RacetrackImplementation_hpp */
