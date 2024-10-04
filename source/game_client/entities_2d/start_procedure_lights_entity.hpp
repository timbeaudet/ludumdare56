///
/// @file
/// @details Displays a simple 3, 2, 1, GO! for the race to get started!
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_StartProcedureLightsEntity_hpp
#define LudumDare56_StartProcedureLightsEntity_hpp

#include "../../game_state/race_session_state.hpp"

#include "../../core/event_system.hpp"

#include <turtle_brains/game/tb_entity.hpp>
#include <turtle_brains/game/tb_game_timer.hpp>
#include <turtle_brains/graphics/tb_text.hpp>

namespace LudumDare56
{
	namespace GameClient
	{

		class StartProcedureLightsEntity : public tbGame::Entity
		{
		public:
			StartProcedureLightsEntity(void);
			virtual ~StartProcedureLightsEntity(void);

		protected:
			virtual void OnSimulate(void) override;

		private:
			tbGraphics::Text mCountdownText;
			tbGame::GameTimer mAliveTimer;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_StartProcedureLightsEntity_hpp */
