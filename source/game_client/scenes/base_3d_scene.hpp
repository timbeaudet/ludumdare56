///
/// @file
/// @details This is the base for all scenes that will be requiring online connection with the GameServer. If the
///   connection is lost the game will return to Title with a message.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_BaseOnlineScene_hpp
#define LudumDare56_BaseOnlineScene_hpp

#include "../../core/development/time_profiler.hpp"
#include "../../game_state/race_session_state.hpp"

#include <ice/game/ice_game_scene.hpp>
#include <ice/graphics/ice_visualization.hpp>

#include <vector>
#include <memory>

namespace LudumDare56
{
	namespace GameClient
	{

		class Base3dScene : public iceGame::GameScene
		{
		public:
			static void StartAsDebugServer(void);
			static void DestroyConnection(void);

			Base3dScene(void);
			virtual ~Base3dScene(void);

		protected:
			static bool sIsDebugging;
			static bool sIsDebuggingPhysics;

			void SimulateGameState(void);
			void SimulateNetworkAndGameState(void);

			virtual void OnSimulate(void) override;
			virtual void OnUpdate(const float deltaTime) override;
			virtual void OnPerspectiveRender(void) const override;
			virtual void OnOrthographicRender(void) const override;
			virtual void OnInterfaceRender(void) const override;
			virtual void OnOpen(void) override;
			virtual void OnClose(void) override;

			void UpdateDeveloperStuffs(const float deltaTime);
			bool IsDeveloperConsoleOpen(void) const;
			virtual void DisplayDeveloperConsole(void) const;

#if defined(development_build)
			typedef TyreBytes::Core::Development::FrameProfiler::Channel TimingChannel;
			mutable TyreBytes::Core::Development::FrameProfiler mProfiler;
			#define ludumdare56_start_timer(timer) { mProfiler.Start(timer); }
			#define ludumdare56_stop_timer(timer) { mProfiler.Stop(timer); }
#else
			#define ludumdare56_start_timer(timer) ;
			#define ludumdare56_stop_timer(timer) ;
#endif /* development_build */

		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_BaseOnlineScene_hpp */
