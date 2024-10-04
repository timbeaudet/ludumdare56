///
/// @file
/// @details This scene is the primary racing scene for the LudumDare56 project.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_RacingScene_hpp
#define LudumDare56_RacingScene_hpp

#include "../../game_client/scenes/base_3d_scene.hpp"
#include "../../game_client/graphics_3d/racetrack_graphic.hpp"
#include "../../game_client/graphics_3d/racecar_graphic.hpp"
#include "../../game_client/graphics_2d/racecar_speedometer.hpp"
#include "../../game_client/graphics_2d/racecar_tachometer.hpp"
#include "../../game_client/entities_2d/settings_screen_entity.hpp"
#include "../../game_client/entities_2d/player_standings_entity.hpp"
#include "../../game_client/camera_controller.hpp"
#include "../../game_state/racecar_controller_interface.hpp"
#include "../../game_state/race_session_state.hpp"
#include "../../ludumdare56.hpp"

#include <turtle_brains/network/tb_socket_connection.hpp>
#include <turtle_brains/network/tb_packet_handler_interface.hpp>

#include <ice/game/ice_game_scene.hpp>
#include <ice/express/ice_gamepad_interface.hpp>
#include <ice/express/icex_cameras.hpp>

#include <track_bundler/track_bundler.hpp>

#include <array>

class ServerPacketHandler;
class ClientPacketHandler;

namespace LudumDare56
{
	namespace GameClient
	{
		class OnlineRacingScene;

		class RacingScene : public Base3dScene, public TyreBytes::Core::EventListener
		{
		public:
			typedef std::array<RacecarGraphic, GameState::kNumberOfRacecars> RacecarArray;

			RacingScene(void);
			virtual ~RacingScene(void);

			enum class GameMode
			{
				Singleplayer,
				Multiplayer,
				MultiplayerHost,
			};

			///
			/// @details Sets the GameMode for the race, whether it would be Singleplayer practice, or Online Multiplayer
			///   etc. This should be called before changing into the RacingScene (or OnlineRacingScene) and remain
			///   unchanged until leaving the RacingScene. Otherwise it could mess up initialization and cleanup.
			///
			static void SetGameMode(const GameMode mode);

			static GameState::DriverIndex GetPlayerDriverIndex(void);
			static GameState::RacecarIndex GetPlayerRacecarIndex(void);
			static tbMath::Matrix4 GetWorldToProjection(void);

			void OpenUserSettings(void);
			void UpdateUserSettings(void);
			void UpdateControllerBindings(void);

		protected:
			static GameMode sGameMode;

			///
			/// @details Returns true if the player is in their car.
			///
			bool IsDriving(void) const;
			virtual void OnSimulate(void) override;

			virtual void OnUpdate(const float deltaTime) override;
			virtual void OnPerspectiveRender(void) const override;
			virtual void OnOrthographicRender(void) const override;
			virtual void OnOpen(void) override;
			virtual void OnClose(void) override;
			virtual void OnHandleEvent(const TyreBytes::Core::Event& event) override;

		private:
			friend class OnlineRacingScene;

			tbGame::InputAction mResetAction;
			tbGame::InputAction mToggleInfoAction;

			CameraController mCamera;

			RacetrackGraphic mRacetrack;
			RacecarArray mRacecarArray;

			RacecarTachometer mRacecarTachometer;
			RacecarSpeedometer mRacecarSpeedometer;
			PlayerStandingsEntity mRacecarStandings;

			//The following things are only valid in Multiplayer.
			SettingsScreenEntity mSettingsScreen;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

//--------------------------------------------------------------------------------------------------------------------//

#endif /* LudumDare56_RacingScene_hpp */
