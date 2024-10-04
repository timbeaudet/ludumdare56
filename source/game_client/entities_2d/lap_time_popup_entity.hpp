///
/// @file
/// @details Displays the most recent laptime for a few moments for the player to see what they achieved.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_LapTimePopupEntity_hpp
#define LudumDare56_LapTimePopupEntity_hpp

#include "../../game_state/race_session_state.hpp"
#include "../../core/event_system.hpp"

#include <turtle_brains/game/tb_entity.hpp>
#include <turtle_brains/game/tb_game_timer.hpp>
#include <turtle_brains/graphics/tb_text.hpp>

namespace LudumDare56
{
	namespace GameClient
	{

		class LapTimePopupEntity : public tbGame::Entity
		{
		public:
			LapTimePopupEntity(const tbCore::uint32 lapTime);
			virtual ~LapTimePopupEntity(void);

		protected:
			virtual void OnUpdate(const float deltaTime) override;

		private:
			tbGraphics::Text mLapTimeText;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_LapTimePopupEntity_hpp */
