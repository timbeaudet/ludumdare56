///
/// @file
/// @details Contains component / logic for the finish area where the player will finish a level/track and get a time.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_FinishAreaComponent_hpp
#define LudumDare56_FinishAreaComponent_hpp

#include "../../ludumdare56.hpp"
#include "../../game_state/race_session_state.hpp"
#include "../../game_state/object_state.hpp"

namespace LudumDare56::GameState
{

	class ZoneFinishComponent : public ComponentState
	{
	public:
		ZoneFinishComponent(ObjectState& object, const TrackBundler::Component& component);
		virtual ~ZoneFinishComponent(void);

		virtual void OnAwake(void) override;
		virtual void OnDestroy(void) override;

		virtual void OnSimulate(void) override;

	private:
		String mNextTrackName;
	};

};	//namespace LudumDare56::GameState

#endif /* LudumDare56_FinishAreaComponent_hpp */
