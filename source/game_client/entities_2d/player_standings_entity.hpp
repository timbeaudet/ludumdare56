///
/// @file
/// @details Displays the most recent laptime for a few moments for the player to see what they achieved.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_PlayerStandingsEntity_hpp
#define LudumDare56_PlayerStandingsEntity_hpp

#include "../../game_state/race_session_state.hpp"

#include <turtle_brains/game/tb_entity.hpp>
#include <turtle_brains/game/tb_game_timer.hpp>
#include <turtle_brains/graphics/tb_text.hpp>

namespace LudumDare56
{
	namespace GameClient
	{

		class PlayerStandingsEntity : public tbGame::Entity
		{
		public:
			PlayerStandingsEntity(const GameState::RacecarIndex racecarIndex);
			virtual ~PlayerStandingsEntity(void);

			void SetRacecarIndex(const GameState::RacecarIndex racecarIndex);

		protected:
			virtual void OnSimulate(void) override;
			virtual void OnUpdate(const float deltaTime) override;

		private:
			tbGraphics::Text mStandingsText;
			GameState::RacecarIndex mRacecarIndex;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_PlayerStandingsEntity_hpp */
