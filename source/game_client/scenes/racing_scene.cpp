///
/// @file
/// @details This scene is the primary racing scene for the LudumDare56 project.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../../game_client/scenes/racing_scene.hpp"
#include "../../game_client/scenes/scene_manager.hpp"
#include "../../game_client/entities_2d/mouse_hiding_entity.hpp"
#include "../../game_client/entities_2d/lap_time_popup_entity.hpp"
#include "../../game_client/entities_2d/start_procedure_lights_entity.hpp"
#include "../../game_client/graphics_2d/racecar_name_tag.hpp"
#include "../../game_client/player_racecar_controller.hpp"
#include "../../game_state/race_session_state.hpp"
#include "../../game_state/racecar_state.hpp"
#include "../../game_state/racetrack_state.hpp"
#include "../../game_state/timing_and_scoring_state.hpp"
#include "../../game_state/ai/artificial_driver_controller.hpp"
#include "../../game_state/events/driver_events.hpp"
#include "../../game_state/events/racecar_events.hpp"
#include "../../game_state/events/timing_events.hpp"
#include "../../game_state/events/race_session_events.hpp"
#include "../../game_server/game_server.hpp"
#include "../../network/network_handlers.hpp"
#include "../../network/network_packets.hpp"
#include "../../network/network_manager.hpp"
#include "../../core/utilities.hpp"
#include "../../logging.hpp"

#include <turtle_brains/network/tb_http_request.hpp>
#include <turtle_brains/network/tb_http_response.hpp>

#include <ice/graphics/ice_cascaded_shadow_maps.hpp>

LudumDare56::GameClient::RacingScene::GameMode LudumDare56::GameClient::RacingScene::sGameMode(GameMode::Singleplayer);

namespace
{	// @note: The player driver/racecar indices will always remain invalid in the MultiplayerHost game mode.
	LudumDare56::GameState::DriverIndex thePlayerDriverIndex = LudumDare56::GameState::InvalidDriver();
	LudumDare56::GameState::RacecarIndex thePlayerRacecarIndex = LudumDare56::GameState::InvalidRacecar();
};

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacingScene::SetGameMode(const GameMode mode)
{
	sGameMode = mode;
}

LudumDare56::GameState::DriverIndex LudumDare56::GameClient::RacingScene::GetPlayerDriverIndex(void) { return thePlayerDriverIndex; }
LudumDare56::GameState::RacecarIndex LudumDare56::GameClient::RacingScene::GetPlayerRacecarIndex(void) { return thePlayerRacecarIndex; }

tbMath::Matrix4 LudumDare56::GameClient::RacingScene::GetWorldToProjection(void)
{
	const iceGraphics::Camera& camera = theSceneManager->GetSceneAs<RacingScene>(SceneId::kRacingScene).GetCamera();
	return camera.GetWorldToView() * camera.GetViewToProjection();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacingScene::RacingScene(void) :
	Base3dScene(),
	mResetAction(tbApplication::tbKeySpace),
	mToggleInfoAction(tbApplication::tbKeyN),

	mCamera(),

	mRacetrack(),
	mRacecarArray(),
	mRacecarTachometer(GameState::InvalidRacecar()),
	mRacecarSpeedometer(GameState::InvalidRacecar()),
	mRacecarStandings(GameState::InvalidRacecar()),
	mSettingsScreen()
{
	iceGraphics::EnvironmentMapSettings environmentMapSettings;
	environmentMapSettings.mMapSize = 1024;
	environmentMapSettings.mAlwaysRender = false;
	EnableEnvironmentMap(0, environmentMapSettings);

	//TODO: LudumDare56: Enable UserSettings for Bloom, and SuperSampling, AO etc etc
	//SetSuperSampling(UserSettings::GetSettings().mSuperSampling);

	tbCore::uint8 racecarIndex = 0;
	for (RacecarGraphic& racecar : mRacecarArray)
	{
		racecar.SetRacecarIndex(racecarIndex);
		racecar.SetVisible(false);
		++racecarIndex;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameClient::RacingScene::~RacingScene(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacingScene::OpenUserSettings(void)
{
	AddEntity(mSettingsScreen);
}

//--------------------------------------------------------------------------------------------------------------------//

#define enable_or_disable_setting(setting, enableCode, disableCode) \
	if (true == TheUserSettings().GetBoolean(setting)) { enableCode; } \
	else { disableCode; }

void LudumDare56::GameClient::RacingScene::UpdateUserSettings(void)
{
	iceGraphics::ShadowSettings shadowSettings;
	shadowSettings.mAttemptSinglePass = TheUserSettings().GetBoolean(Settings::SinglePassShadows(), true);

	enable_or_disable_setting(Settings::ShowBloom(), EnableBloom(), DisableBloom());
	enable_or_disable_setting(Settings::ShowAmbientOcclusion(), EnableAmbientOcclusion(), DisableAmbientOcclusion());
	enable_or_disable_setting(Settings::ShowShadows(), EnableShadows(shadowSettings), DisableShadows());
	enable_or_disable_setting(Settings::ShowParticles(), EnableParticles(), DisableParticles());
	//enable_or_disable_setting(Settings::ShowReflections(), (), DisableParticles());

	SetSuperSampling(tbCore::RangedCast<tbGraphics::PixelSpace>(TheUserSettings().GetInteger(Settings::SuperSampling(), 2)));
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::GameClient::RacingScene::IsDriving(void) const
{
	return GameState::IsValidRacecar(thePlayerRacecarIndex);
}

//--------------------------------------------------------------------------------------------------------------------//
bool paused = false;

void LudumDare56::GameClient::RacingScene::OnSimulate(void)
{
	ludumdare56_start_timer(TimingChannel::kSimulate);

	Base3dScene::OnSimulate();

	if (true == mResetAction.IsPressed() && true == IsDriving())
	{
		GameState::RaceSessionState::PlaceCarOnGrid(GameState::RacecarState::GetMutable(thePlayerRacecarIndex));
	}
	if (true == mToggleInfoAction.IsPressed())
	{
		RacecarGraphic::sDisplayCarNumbers = !RacecarGraphic::sDisplayCarNumbers;
	}

	if (GameMode::Singleplayer == sGameMode)
	{
		if (false == paused)
		{
			SimulateGameState();
		}

#if defined(development_build)
		{	//Developer Mode Debugging Junk, possible want to throw away!
			if (true == tbApplication::Input::IsKeyPressed(tbApplication::tbKeyP))
			{
				paused = !paused;
			}
			else if (true == tbApplication::Input::IsKeyDown(tbApplication::tbKey0))
			{
				static int counter = 0;
				++counter;
				paused = (counter % 4) ? true : false;
			}
			else if (true == tbApplication::Input::IsKeyReleased(tbApplication::tbKey0))
			{
				paused = false;
			}
		}
#endif /* development_build */
	}
	else
	{
		SimulateNetworkAndGameState();
	}

	if (true == IsDriving())
	{
		mCamera.SetCameraMode(CameraController::CameraMode::kDrivingCamera);
		mCamera.SetViewedRacecarIndex(thePlayerRacecarIndex);
	}

	mCamera.Simulate();

	ludumdare56_stop_timer(TimingChannel::kSimulate);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacingScene::OnUpdate(const float deltaTime)
{
	ludumdare56_start_timer(TimingChannel::kUpdate);

	// @note 2023-10-25: Update the camera before doing anything so that any Graphic objects will be able to use the
	//   GetWorldToProjection() function properly. This is to fix nametags and other graphics from bouncing around.
	mCamera.SetMovementSpeed(50.0f);
	mCamera.Update(GetCamera(), deltaTime);

	const GameState::RacecarIndex viewedRacecar = mCamera.GetViewedRacecarIndex();
	mRacecarTachometer.SetRacecarIndex(viewedRacecar);
	mRacecarSpeedometer.SetRacecarIndex(viewedRacecar);
	mRacecarStandings.SetRacecarIndex(viewedRacecar);
	Base3dScene::OnUpdate(deltaTime);

	mRacetrack.Update(deltaTime);

	for (RacecarGraphic& racecar : mRacecarArray)
	{
		racecar.Update(deltaTime);
	}

	if (false == mSettingsScreen.IsDisplayingSettings() && true == tbApplication::Input::IsKeyReleased(tbApplication::tbKeyEscape))
	{
#if defined(development_build)
		if (false == GetQuickPlayRacetrackPath().empty())
		{	//Just quit for quick play.
			theSceneManager->QuitGame();
		}
#endif /* development_build */

		if (GameMode::Multiplayer == sGameMode && true == IsDriving())
		{	//Jump into spectator mode before just exiting the server.
			Network::SendSafePacket(Network::CreateTinyPacket(Network::PacketType::DriverLeavesRacecar, thePlayerDriverIndex));
		}
		else
		{
			theSceneManager->ChangeToScene(SceneId::kTitleScene);
		}
	}

	ludumdare56_stop_timer(TimingChannel::kUpdate);
	ludumdare56_start_timer(TimingChannel::kRender);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacingScene::OnPerspectiveRender(void) const
{
	Base3dScene::OnPerspectiveRender();
	mRacetrack.RenderDebug();

#if defined(development_build)
	static iceGraphics::Visualization aiVisuals;

	if (true == sIsDebuggingPhysics)
	{	//Physics / GameState debugging
		GameState::RaceSessionState::RenderDebug();

		GameState::ArtificialDriverController::SetDebugVisualizer(&aiVisuals);
		aiVisuals.Render();
	}

	if (false == paused)
	{
		aiVisuals.ClearVisuals();
	}
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacingScene::OnOrthographicRender(void) const
{
	Base3dScene::OnOrthographicRender();
	ludumdare56_stop_timer(TimingChannel::kRender);
	DisplayDeveloperConsole();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacingScene::OnOpen(void)
{
	Base3dScene::OnOpen();
	UpdateUserSettings();

	mCamera.SetToDefaults(tbMath::Vector3::Zero(), tbMath::Vector3(20.0f, 20.0f, 20.0f));

	AddEntity(new MouseHidingEntity());
	AddGraphic(mRacecarTachometer);
	AddGraphic(mRacecarSpeedometer);
	AddEntity(mRacecarStandings);

	switch (sGameMode)
	{
	case GameMode::Singleplayer: {
		tb_always_log(LogGame::Always() << "LudumDare56 Singleplayer");

		GameState::RaceSessionState::Create(true);
		GameState::RaceSessionState::SetSessionPhase(GameState::RaceSessionState::SessionPhase::kPhasePractice);

		thePlayerDriverIndex = GameState::RaceSessionState::DriverEnterCompetition(GameState::DriverLicense("singleplayer", "Player One"));
		tb_error_if(0 != thePlayerDriverIndex, "Expected driver index of 0 when in local practice mode.");

		const icePhysics::Matrix4 gridToWorld = GameState::RacetrackState::GetGridToWorld(GameState::GridIndex(0));
		thePlayerRacecarIndex = GameState::RaceSessionState::DriverEnterRacecar(thePlayerDriverIndex);
		tb_error_if(0 != thePlayerRacecarIndex, "Expected player racecar index of 0 when in local practice mode.");
		GameState::RacecarState::GetMutable(thePlayerRacecarIndex).SetRacecarMeshID(0);
		GameState::RacecarState::GetMutable(thePlayerRacecarIndex).SetVehicleToWorld(gridToWorld);
		UpdateControllerBindings();

		for (tbCore::byte index = 1; index < GameState::kNumberOfRacecars && index < GameState::kNumberOfDrivers; ++index)
		{
			const icePhysics::Matrix4 botGridToWorld = GameState::RacetrackState::GetGridToWorld(static_cast<GameState::GridIndex>(index));
			const GameState::DriverLicense botLicense = GameState::DriverLicense("singleplayer", "Bot " + tb_string(static_cast<int>(index)));
			const GameState::DriverIndex botDriver = GameState::RaceSessionState::DriverEnterCompetition(botLicense);
			const GameState::RacecarIndex botRacecar = GameState::RaceSessionState::DriverEnterRacecar(botDriver);
			GameState::RacecarState::GetMutable(botRacecar).SetVehicleToWorld(botGridToWorld);
			GameState::RacecarState::GetMutable(botRacecar).SetRacecarMeshID(1);
			GameState::RacecarState::GetMutable(botRacecar).SetRacecarController(new GameState::ArtificialDriverController(botDriver, botRacecar));
		}
		break; }

	default: {
		tb_error("LudumDare56 Disabled Multiplayer.");
		break; }
	};

	GameState::RaceSessionState::AddEventListener(*this);
	GameState::TimingState::AddEventListener(*this);

	for (GameState::RacecarState& racecar : GameState::RacecarState::AllMutableRacecars())
	{
		AddGraphic(new RacecarNameTag(racecar.GetRacecarIndex()));
		racecar.AddEventListener(*this);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacingScene::OnClose(void)
{
	for (GameState::RacecarState& racecar : GameState::RacecarState::AllMutableRacecars())
	{
		racecar.RemoveEventListener(*this);
	}

	GameState::TimingState::RemoveEventListener(*this);
	GameState::RaceSessionState::RemoveEventListener(*this);

	ClearEntities();
	ClearGraphics();

	mRacetrack.SetVisible(false);
	for (RacecarGraphic& racecar : mRacecarArray)
	{
		racecar.SetVisible(false);
	}

	switch (sGameMode)
	{
	case GameMode::Singleplayer: {
		GameState::RaceSessionState::Destroy();
		break; }
	case GameMode::MultiplayerHost: {
		GameServer::ShutdownServer();
		break; }
	case GameMode::Multiplayer: {
		GameState::RaceSessionState::Destroy();
		Network::DestroyConnection(Network::DisconnectReason::Graceful);
		break; }
	};

	Base3dScene::OnClose();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacingScene::OnHandleEvent(const TyreBytes::Core::Event& event)
{
	switch (event.GetID())
	{
	case GameState::Events::RaceSession::RaceSessionPhaseChanged: {
		const auto& phaseChangeEvent = event.As<GameState::Events::RaceSessionPhaseChangeEvent>();
		if (GameMode::Singleplayer == sGameMode && 0 == phaseChangeEvent.mPhaseTimer &&
			GameState::RaceSessionState::SessionPhase::kPhaseGrid == phaseChangeEvent.mSessionPhase)
		{
			GameState::RaceSessionState::SetSessionPhase(GameState::RaceSessionState::SessionPhase::kPhaseGrid, 250);
		}
		else
		{	//Either Multiplayer or the second time through where the timer actually starts counting down,
			//   timer == 0 is special 'recursive' case so that Multiplayer server can set the worstLatency.
			AddEntity(new StartProcedureLightsEntity());
		}
		break; }


	case GameState::Events::Driver::DriverEntersCompetition: {
		const GameState::Events::DriverEvent& eventData = event.As<GameState::Events::DriverEvent>();
		if (eventData.mDriverIndex == thePlayerDriverIndex)
		{
			if (GameMode::Multiplayer == sGameMode && GameState::InvalidRacecar() == thePlayerRacecarIndex)
			{

			}
		}
		break; }
	case GameState::Events::Racecar::DriverEntersRacecar: {
		const GameState::Events::RacecarSeatEvent& eventData = event.As<GameState::Events::RacecarSeatEvent>();
		if (eventData.mDriverIndex == thePlayerDriverIndex)
		{
			tb_error_if(GameMode::Singleplayer == sGameMode && thePlayerRacecarIndex != eventData.mRacecarIndex,
				"Error: Why is there mismatch of (single) player RacecarIndex?");
			tb_error_if(GameMode::Multiplayer == sGameMode && Network::GetClientHandler().GetRacecarIndexForPlayer() != eventData.mRacecarIndex,
				"Error: Why is there mismatch of (multi) player RacecarIndex?");

			thePlayerRacecarIndex = eventData.mRacecarIndex;
			UpdateControllerBindings();
		}
		break; }
	case GameState::Events::Racecar::DriverLeavesRacecar: {
		const GameState::Events::RacecarSeatEvent& eventData = event.As<GameState::Events::RacecarSeatEvent>();
		if (eventData.mDriverIndex == thePlayerDriverIndex)
		{
			tb_error_if(GameMode::Singleplayer == sGameMode, "Error: Player should always have a racecar in singleplayer mode.");
			thePlayerRacecarIndex = GameState::InvalidRacecar();
		}

		break; }


	case GameState::Events::Timing::ResetTimingResults: {
		tb_always_log(LogGame::Error() << "RacingScene detected Timing and Scoring Reset Competition!");
		break; }
	case GameState::Events::Timing::CompletedLapResult: {
		const auto& lapResultEvent = event.As<GameState::Events::TimingEvent>();
		tb_debug_log(LogGame::Debug() << "RacingScene detected lap result:\n\tDriver: " << lapResultEvent.mDriverName <<
			"\n\tLapTime: " << tbCore::String::TimeToString(lapResultEvent.mLapTime) << "\n\tOn Lap: " <<
			static_cast<int>(lapResultEvent.mLapNumber));

		if (lapResultEvent.mDriverLicense == GameState::DriverState::Get(thePlayerDriverIndex).GetLicense() &&
			lapResultEvent.mDriverName == GameState::DriverState::Get(thePlayerDriverIndex).GetName())
		{
			AddEntity(new LapTimePopupEntity(lapResultEvent.mLapTime));
		}
		break; }
	};
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameClient::RacingScene::UpdateControllerBindings(void)
{
	if (GameState::IsValidRacecar(thePlayerRacecarIndex))
	{	//Player may not have joined the track yet, so they might not have a racecar.
		GameState::RacecarState::GetMutable(thePlayerRacecarIndex).SetRacecarController(new PlayerRacecarController());
	}
}

//--------------------------------------------------------------------------------------------------------------------//
