///
/// @file
/// @details Manage all the racecars in the simulation on both game and server side.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "../game_state/racecar_state.hpp"
#include "../game_state/racetrack_state.hpp"
#include "../game_state/driver_state.hpp"
#include "../game_state/helpers/torque_curve.hpp"
#include "../game_state/events/racecar_events.hpp"
#include "../game_state/racecar_controller_interface.hpp"

#include "../logging.hpp"

#include <array>

namespace
{
	using LudumDare56::GameState::PhysicsModels::PhysicsModel;

	const PhysicsModel kGenericPhysicsModel = PhysicsModel::ExtremelyBasic;
	const PhysicsModel kTheBestPhysicsModel = PhysicsModel::ExtremeDrifting; //This is intended for "Tim's Awesome Car" / Tier3 support.

	std::vector<PhysicsModel> thePhysicsModels = {
		PhysicsModel::ExtremelyFast, PhysicsModel::ExtremelyBasic, PhysicsModel::ExtremeDrifting, PhysicsModel::ExtremelyBasic, PhysicsModel::NullModel
	};

	typedef std::array<LudumDare56::GameState::RacecarState, LudumDare56::GameState::kNumberOfRacecars> RacecarArray;
	RacecarArray& TheRacecarArray(void)
	{
		static RacecarArray theRacecarArray;
		return theRacecarArray;
	}
};

PhysicsModel GetRacecarPhysicsModel(tbCore::uint8 carID);

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::RacecarState& LudumDare56::GameState::RacecarState::Get(const RacecarIndex racecarIndex)
{
	return TheRacecarArray()[racecarIndex];
}

LudumDare56::GameState::RacecarState& LudumDare56::GameState::RacecarState::GetMutable(const RacecarIndex racecarIndex)
{
	return TheRacecarArray()[racecarIndex];
}

// 2024-10-06: Yes I'm aware GameState shouldn't being doing sounds, that should be the GameClient... but its a jam.
tbAudio::AudioController theStartCueController;
std::vector<tbAudio::AudioController> theCrashSounds;

std::array<tbAudio::AudioController, 3> theEngineControllers;


//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarState::RacecarState(void) :
	mCreatures(),
	mPhysicsModel(new PhysicsModels::NullPhysicsModel()),
	mController(new NullRacecarController()),
	mPhysicalWorld(nullptr),
	mElapsedTime(tbGame::GameTimer::Zero()),
	mPreviousPosition(iceVector3::Zero()),
	mSwarmToWorld(iceMatrix4::Identity()),
	mOnTrackCounter(0),
	mSwarmHealth(kNumberOfCreatures),
	mRacecarIndex(InvalidRacecar()),
	mDriverIndex(InvalidDriver()),
	mRacecarMeshID(0),
	mIsOnTrack(false),
	mIsVisible(false),
	mRacecarFinished(false),
	mCreatureFinished(false),
	mJustResetted(false)
{
	theStartCueController = tbAudio::theAudioManager.PlayEvent("audio_events", "start_countdown");
	theStartCueController.Stop();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarState::~RacecarState(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Create(icePhysics::World& physicalWorld)
{
	tb_error_if(false == IsValidRacecar(mRacecarIndex), "Expected the RacecarIndex to be valid by Create().");

	mPhysicalWorld = &physicalWorld;
	mPhysicsModel = PhysicsModels::Instantiate(physicalWorld, GetRacecarPhysicsModel(mRacecarMeshID));

	RaceSessionState::PlaceCarOnGrid(*this);

	ResetRacecar(GetVehicleToWorld());

	theEngineControllers = std::array<tbAudio::AudioController, 3>{
		tbAudio::theAudioManager.PlayEvent("audio_events", "engine_1"),
		tbAudio::theAudioManager.PlayEvent("audio_events", "engine_2"),
		tbAudio::theAudioManager.PlayEvent("audio_events", "engine_3")
	};
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Destroy(icePhysics::World& /*physicalWorld*/)
{
	mPhysicalWorld = nullptr;
	mPhysicsModel.reset(new PhysicsModels::NullPhysicsModel());

	theStartCueController.Stop();
	for (tbAudio::AudioController& controller : theEngineControllers)
	{
		controller.Stop();
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::ResetRacecar(const iceMatrix4& vehicleToWorld)
{
	SetVehicleToWorld(vehicleToWorld);
	mPhysicsModel->ResetRacecarForces();

	//const iceScalar range = 5.0f;

	const std::array<iceVector3, 200> placementSpots = {
		iceVector3(-1.36860, -0.65165, 0.77294), iceVector3(-0.98504, -0.64692, 0.17409), iceVector3(1.11580, -0.64314, 1.54648), iceVector3(1.18518, -0.64033, 1.97437), iceVector3(-2.37489, -0.63850, 0.00899), iceVector3(-1.99235, -0.65166, -0.22435), iceVector3(1.21515, -0.64692, -0.97703), iceVector3(-1.21260, -0.64315, -2.03696), iceVector3(-1.70956, -0.64033, -0.96846), iceVector3(0.92643, -0.63850, -0.92804), iceVector3(1.84888, -0.65165, 1.45760), iceVector3(0.07323, -0.64692, -2.86069), iceVector3(-0.49374, -0.64314, 0.91751), iceVector3(0.71278, -0.64033, 0.64649), iceVector3(0.20085, -0.63850, -2.40368), iceVector3(-0.35677, -0.65165, 0.64018), iceVector3(0.82492, -0.64692, 1.28794), iceVector3(-0.38625, -0.64315, -0.88058), iceVector3(0.36404, -0.64034, -1.96735), iceVector3(0.76700, -0.63850, -0.65134), iceVector3(-0.99507, -0.65166, -0.84158), iceVector3(1.81115, -0.64692, 0.08916), iceVector3(-1.43317, -0.64315, -2.42901), iceVector3(-2.19207, -0.64033, 0.96841), iceVector3(1.16939, -0.63850, 0.58052), iceVector3(-1.93505, -0.65165, 1.37666), iceVector3(1.54930, -0.64692, 1.76241), iceVector3(-0.92806, -0.64315, -1.27877), iceVector3(-0.48589, -0.64033, -1.20852), iceVector3(1.06970, -0.63850, -2.60238), iceVector3(-0.84129, -0.65166, -1.76139), iceVector3(-0.04981, -0.64692, 1.59510), iceVector3(-1.31765, -0.64314, 0.00285), iceVector3(1.85050, -0.64033, 0.53248), iceVector3(0.26363, -0.63850, -0.56551), iceVector3(-2.82462, -0.65166, -0.31072), iceVector3(-0.76666, -0.64692, 1.06474), iceVector3(0.38100, -0.64315, -1.53343), iceVector3(1.39273, -0.64033, -0.68879), iceVector3(2.29714, -0.63850, 0.73809), iceVector3(2.73295, -0.65166, -0.79922), iceVector3(-1.15021, -0.64692, -0.56206), iceVector3(-2.33943, -0.64314, 0.49562), iceVector3(1.43973, -0.64033, 1.27839), iceVector3(-0.61551, -0.63850, -0.68020), iceVector3(-1.19719, -0.65166, -1.57719), iceVector3(0.71147, -0.64692, 0.99463), iceVector3(0.10054, -0.64314, 1.07960), iceVector3(-0.45331, -0.64033, -0.20579), iceVector3(-1.95868, -0.63850, -0.63144), iceVector3(-0.01615, -0.65166, 0.06482), iceVector3(-1.97189, -0.64692, 0.19355), iceVector3(-2.51847, -0.64315, -1.32589), iceVector3(-1.35807, -0.64033, -0.81375), iceVector3(0.95051, -0.63850, -1.25064), iceVector3(-1.87393, -0.65166, -2.14219), iceVector3(-2.57297, -0.64692, 1.20950), iceVector3(-1.72424, -0.64314, 0.96717), iceVector3(2.06612, -0.64033, 1.10949), iceVector3(0.92609, -0.63850, 0.38873), iceVector3(2.19109, -0.65166, -0.88149), iceVector3(0.46854, -0.64692, -0.78856), iceVector3(-0.08465, -0.64314, -0.75582), iceVector3(0.11186, -0.64033, 0.78510), iceVector3(-0.47185, -0.63850, -2.80628), iceVector3(1.70516, -0.65165, 0.92422), iceVector3(-0.97773, -0.64692, -2.67253), iceVector3(-2.75270, -0.64314, 0.71670), iceVector3(2.27139, -0.64033, -1.69169), iceVector3(0.36678, -0.63850, 2.80875), iceVector3(2.36384, -0.65165, 1.57328), iceVector3(-0.70817, -0.64692, 1.42463), iceVector3(0.98636, -0.64314, 0.83006), iceVector3(1.51838, -0.64033, -0.37971), iceVector3(-1.49150, -0.63850, -1.30648), iceVector3(-0.02073, -0.65165, 1.99212), iceVector3(0.70403, -0.64692, -0.13308), iceVector3(0.67187, -0.64315, -1.40555), iceVector3(0.25692, -0.64033, 0.49774), iceVector3(2.30346, -0.63850, -0.47285), iceVector3(0.20555, -0.65165, 1.38404), iceVector3(-0.44172, -0.64692, 1.93477), iceVector3(-2.22349, -0.64315, -1.75075), iceVector3(-0.75077, -0.64033, -0.22931), iceVector3(-0.00991, -0.63850, 0.00752), iceVector3(-0.37823, -0.65166, -0.52706), iceVector3(0.04966, -0.64692, -1.63639), iceVector3(0.11269, -0.64315, -1.32710), iceVector3(-1.62387, -0.64033, 1.72876), iceVector3(0.39103, -0.63850, -0.12956), iceVector3(-0.73410, -0.65166, -1.01636), iceVector3(-0.29987, -0.64692, -2.39769), iceVector3(0.70880, -0.64315, -1.78273), iceVector3(1.06937, -0.64033, -1.63604), iceVector3(-1.31536, -0.63850, 0.31333), iceVector3(0.87282, -0.65166, -0.36313), iceVector3(-0.64656, -0.64692, -1.45006), iceVector3(1.21522, -0.64314, 0.20482), iceVector3(-0.46335, -0.64034, -2.00548), iceVector3(-1.91998, -0.63850, 0.60491), iceVector3(1.89599, -0.65166, -0.27452), iceVector3(1.03620, -0.64692, -0.04434), iceVector3(0.53401, -0.64314, -0.42221), iceVector3(0.55586, -0.64033, 1.44465), iceVector3(1.95038, -0.63850, -2.06638), iceVector3(-0.80551, -0.65166, -2.24203), iceVector3(-0.44802, -0.64692, 1.25399), iceVector3(-0.65095, -0.64314, 2.76289), iceVector3(0.55071, -0.64033, 0.44427), iceVector3(0.68710, -0.63850, -2.25917), iceVector3(1.22823, -0.65166, -0.28820), iceVector3(0.36971, -0.64692, -1.16780), iceVector3(2.77787, -0.64314, 0.15631), iceVector3(0.43135, -0.64033, 1.14523), iceVector3(-1.46886, -0.63850, 1.28693), iceVector3(1.34133, -0.65166, -1.34707), iceVector3(-1.20104, -0.64692, 1.04275), iceVector3(2.30940, -0.64314, -0.05281), iceVector3(0.18468, -0.64033, -0.26713), iceVector3(1.97463, -0.63850, -1.22592), iceVector3(-1.11213, -0.65165, 0.54677), iceVector3(0.77083, -0.64692, 0.16033), iceVector3(-2.70980, -0.64315, -0.84238), iceVector3(-0.15722, -0.64033, 1.28464), iceVector3(2.81169, -0.63850, -0.30533), iceVector3(-1.21569, -0.65165, 2.05981), iceVector3(1.46315, -0.64692, -0.06218), iceVector3(-1.20817, -0.64314, 1.62497), iceVector3(0.76133, -0.64033, 1.74957), iceVector3(-1.53982, -0.63850, 0.51993), iceVector3(1.51085, -0.65165, 0.24870), iceVector3(-1.91813, -0.64692, -1.40594), iceVector3(1.73169, -0.64315, -1.53514), iceVector3(1.80426, -0.64033, -0.65640), iceVector3(0.18927, -0.63850, 2.39576), iceVector3(-1.58553, -0.65165, 2.34818), iceVector3(0.17818, -0.64692, -0.91352), iceVector3(-0.38185, -0.64314, 1.57558), iceVector3(-2.17721, -0.64033, -1.00018), iceVector3(-0.01218, -0.63850, 0.00300), iceVector3(-0.19039, -0.65166, -0.22104), iceVector3(-1.61282, -0.64692, -0.19293), iceVector3(0.17547, -0.64314, 0.23395), iceVector3(-0.69051, -0.64033, 0.65728), iceVector3(-0.20511, -0.63850, -1.39346), iceVector3(-0.35774, -0.65166, -1.62953), iceVector3(1.59468, -0.64692, -1.01092), iceVector3(-0.29892, -0.64314, 2.34976), iceVector3(-2.37140, -0.64033, -0.49532), iceVector3(1.27331, -0.63850, 2.50947), iceVector3(-0.03574, -0.65166, -2.01858), iceVector3(2.78050, -0.64692, 0.61733), iceVector3(0.64986, -0.64315, -1.06703), iceVector3(0.41911, -0.64033, 0.80610), iceVector3(-2.29379, -0.63850, 1.64058), iceVector3(-1.13324, -0.65165, 2.59941), iceVector3(-0.12957, -0.64692, -1.07504), iceVector3(-1.32384, -0.64314, -0.32498), iceVector3(-0.05863, -0.64033, 0.51943), iceVector3(-0.35246, -0.63850, 0.05640), iceVector3(1.67097, -0.65165, 2.26898), iceVector3(-0.45094, -0.64692, 0.36250), iceVector3(1.09517, -0.64314, 1.12706), iceVector3(1.53238, -0.64034, -2.39232), iceVector3(0.46056, -0.63850, 0.15011), iceVector3(-1.60860, -0.65166, 0.16265), iceVector3(-2.83122, -0.64692, 0.20300), iceVector3(-0.15298, -0.64314, 0.25077), iceVector3(0.60112, -0.64034, -2.76597), iceVector3(0.01885, -0.63850, -0.00811), iceVector3(-0.15098, -0.65165, 2.83859), iceVector3(-0.66335, -0.64692, 0.09536), iceVector3(-0.83124, -0.64314, 1.79356), iceVector3(-0.00099, -0.64033, -0.00579), iceVector3(1.11280, -0.63850, -0.60569), iceVector3(2.60745, -0.65165, 1.12492), iceVector3(-1.97391, -0.64692, 2.03793), iceVector3(-0.99175, -0.64314, 1.28916), iceVector3(1.34088, -0.64033, 0.87060), iceVector3(-0.79045, -0.63850, 0.39158), iceVector3(-0.97759, -0.65165, 0.82551), iceVector3(-1.60908, -0.64692, -1.77765), iceVector3(-0.01723, -0.64314, -0.03254), iceVector3(2.22055, -0.64033, 0.33500), iceVector3(0.82175, -0.63850, 2.21947), iceVector3(-0.18240, -0.65165, 0.90521), iceVector3(0.31166, -0.64692, 1.69518), iceVector3(0.43650, -0.64314, 2.06972), iceVector3(-1.17425, -0.64033, -1.09062), iceVector3(0.81656, -0.63850, 2.69503), iceVector3(1.47180, -0.65165, 0.54769), iceVector3(-0.83187, -0.64692, -0.50755), iceVector3(2.53020, -0.64315, -1.27340), iceVector3(-1.01614, -0.64033, -0.15236), iceVector3(-1.55236, -0.63850, -0.54323), iceVector3(1.11363, -0.65166, -2.09838), iceVector3(2.04721, -0.64692, 1.96108), iceVector3(1.49474, -0.64315, -1.85873), iceVector3(-0.03715, -0.64033, -0.47281), iceVector3(-0.76971, -0.63850, 2.25746),
	};

	iceScalar creatureY = vehicleToWorld.GetPosition().y + 0.06f;

	CreatureIndex creatureIndex = 0;
	for (Creature& creature : mCreatures)
	{
		//const iceMatrix4 creatureToVehicle = iceMatrix4::Translation(tbMath::RandomFloat(-range, range),
		//	0.0f, tbMath::RandomFloat(-range, range));

		const iceMatrix4 creatureToVehicle = iceMatrix4::Translation(placementSpots[creatureIndex]);

		creature.mCreatureToWorld = creatureToVehicle * vehicleToWorld;
		creature.mCreatureToWorld.SetPosition(creature.mCreatureToWorld.GetPosition().x, creatureY, creature.mCreatureToWorld.GetPosition().z);
		creature.mPreviousPosition = creature.mCreatureToWorld.GetPosition();
		creature.mVelocity = iceVector3::Zero();
		creature.mIsOnTrack = true;
		creature.mIsAlive = true;
		creature.mIsRacing = true;

		++creatureIndex;
	}

	mPreviousPosition = vehicleToWorld.GetPosition();

	if (RaceSessionState::GetWorldTimer() < 100 && true == theStartCueController.IsComplete())
	{
		theStartCueController.Play();
	}
	else if (RaceSessionState::GetWorldTimer() > 5000)
	{
		tb_debug_log("World Timer: " << RaceSessionState::GetWorldTimer());
		tbAudio::theAudioManager.PlayEvent("audio_events", "start");
	}

	mElapsedTime = 0;
	mRacecarFinished = false;
	mCreatureFinished = false;
	mJustResetted = true;
	mSwarmHealth = kNumberOfCreatures;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarController(RacecarControllerInterface* controller)
{
	mController.reset((nullptr == controller) ? new NullRacecarController() : controller);
}

//--------------------------------------------------------------------------------------------------------------------//

const LudumDare56::GameState::RacecarControllerInterface& LudumDare56::GameState::RacecarState::GetRacecarController(void) const
{
	tb_error_if(nullptr == mController, "Expected the controller to always be valid, even if NullController.");
	return *mController;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::RacecarControllerInterface& LudumDare56::GameState::RacecarState::GetMutableRacecarController(void)
{
	tb_error_if(nullptr == mController, "Expected the controller to always be valid, even if NullController.");
	return *mController;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarMeshID(const tbCore::uint8 racecarMeshID)
{
	if (racecarMeshID != mRacecarMeshID)
	{
		mRacecarMeshID = racecarMeshID;

		if (nullptr != mPhysicalWorld)
		{
			if (true == IsRacecarInUse())
			{
				mPhysicsModel->SetEnabled(false);
			}

			mPhysicsModel = PhysicsModels::Instantiate(*mPhysicalWorld, GetRacecarPhysicsModel(mRacecarMeshID));

			if (true == IsRacecarInUse())
			{
				mPhysicsModel->SetEnabled(true);
			}
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarIndex(const RacecarIndex& racecarIndex)
{
	mRacecarIndex = racecarIndex;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetRacecarDriver(const DriverIndex driverIndex)
{
	const DriverIndex previousDriverIndex = mDriverIndex;
	if (true == IsValidDriver(previousDriverIndex))
	{
		SendEvent(Events::RacecarSeatEvent(Events::Racecar::DriverLeavesRacecar, mDriverIndex, mRacecarIndex));
	}

	mDriverIndex = driverIndex;

	if (true == IsValidDriver(mDriverIndex))
	{
		if (false == IsValidDriver(previousDriverIndex))
		{
			mPhysicsModel->SetEnabled(true);
		}

		RaceSessionState::PlaceCarOnGrid(*this);

		SendEvent(Events::RacecarSeatEvent(Events::Racecar::DriverEntersRacecar, mDriverIndex, mRacecarIndex));
	}
	else if (true == IsValidDriver(previousDriverIndex))
	{
		mPhysicsModel->SetEnabled(false);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetBodyToWorld(void) const
{
	return mPhysicsModel->GetBodyToWorld();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetWheelToWorld(const size_t wheelIndex) const
{
	return mPhysicsModel->GetWheelToWorld(wheelIndex);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::OnRacecarFinished(void)
{
	mRacecarFinished = true;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::OnCreatureFinished(const CreatureIndex& creatureIndex)
{
	mCreatureFinished = true;
	mCreatures[creatureIndex].mIsRacing = false;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetCreatureToWorld(const CreatureIndex& creatureIndex) const
{
	return mCreatures[creatureIndex].mCreatureToWorld;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetSwarmToWorld(void) const
{
	return mSwarmToWorld;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::GetSwarmVelocity(void) const
{
	return mSwarmVelocity;
};

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Matrix4 LudumDare56::GameState::RacecarState::GetVehicleToWorld(void) const
{
	return mPhysicsModel->GetVehicleToWorld();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetVehicleToWorld(const iceMatrix4& vehicleToWorld)
{
	mPhysicsModel->SetVehicleToWorld(vehicleToWorld);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::GetAngularVelocity(void) const
{
	return mPhysicsModel->GetAngularVelocity();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetAngularVelocity(const iceVector3& angularVelocity)
{
	mPhysicsModel->SetAngularVelocity(angularVelocity);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::GetLinearVelocity(void) const
{
	return mPhysicsModel->GetLinearVelocity();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::SetLinearVelocity(const iceVector3& linearVelocity)
{
	mPhysicsModel->SetLinearVelocity(linearVelocity);
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Scalar LudumDare56::GameState::RacecarState::GetEngineSpeed(void) const
{
	return mPhysicsModel->GetEngineSpeed();
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::GameState::Gear LudumDare56::GameState::RacecarState::GetShifterPosition(void) const
{
	return mPhysicsModel->GetShifterPosition();
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Simulate(void)
{
	//if (true == tbApplication::Input::IsKeyDown(tbApplication::tbKeyP))
	//{
	//	tb_debug_log("\n\n\n\n\n-------------------\n");
	//	tb_debug_log("std::vector<> = { " << tbCore::Debug::ContinueEntry());

	//	const iceMatrix4 worldToVehicle = GetVehicleToWorld().ComputeInverse();
	//	for (const Creature& creature : mCreatures)
	//	{
	//		const iceVector3 positionWorld = creature.mCreatureToWorld.GetPosition();
	//		const iceVector3 positionVehicle = worldToVehicle.TransformCoordinate(positionWorld);
	//		tb_debug_log("iceVector3(" << positionVehicle << "),\n" << tbCore::Debug::ContinueEntry());
	//	}

	//	tb_debug_log("}\n\n");
	//}

	mPreviousPosition = GetVehicleToWorld().GetPosition();

	if (false == mRacecarFinished && false == HasLost())
	{
		if (false == mJustResetted)
		{
			mElapsedTime.IncrementStep();
		}
		mJustResetted = false;

		mController->UpdateControls();
		mPhysicsModel->Simulate(*mController);
	}
	else
	{
		BrakeOnlyRacecarController brakesController;
		mPhysicsModel->Simulate(brakesController);

		mPhysicsModel->SetLinearVelocity(mPhysicsModel->GetLinearVelocity() * 0.5f * kFixedTime);
	}

	SimulateCreatureSwarm();

	for (size_t index = 0; index < theCrashSounds.size(); /* in loop */)
	{
		if (true == theCrashSounds[index].IsComplete())
		{
			theCrashSounds[index] = theCrashSounds.back();
			theCrashSounds.pop_back();
		}
		else
		{
			++index;
		}
	}

	float percentage = tbMath::Clamp(mElapsedTime.GetPercentageOf(400), 0.0f, 1.0f);
	if (true == HasWon() || true == HasLost())
	{
		percentage = 0.0f;
	}

	for (tbAudio::AudioController& controller : theEngineControllers)
	{
		controller.SetVolume(percentage);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::RenderDebug(void) const
{
#if !defined(ludumdare56_headless_build)
	if (true == IsRacecarInUse())
	{
		mPhysicsModel->DebugRender();
	}
#endif /* ludumdare56_headless_build */
}

//--------------------------------------------------------------------------------------------------------------------//

std::vector<tbCore::uint8> LudumDare56::GameState::RacecarState::GetAvailableCars(bool /*isSubbed*/, bool /*isTier3*/)
{
	std::vector<tbCore::uint8> cars;
	cars.push_back(0);
	cars.push_back(1);
	cars.push_back(2);
	cars.push_back(3);
	return cars;
}

//--------------------------------------------------------------------------------------------------------------------//

PhysicsModel GetRacecarPhysicsModel(tbCore::uint8 /*carID*/)
{
	//return PhysicsModel::ExtremeDrifting;
	return PhysicsModel::ExtremelyBasic;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString LudumDare56::GameState::RacecarState::GetCarFilepath(tbCore::uint8 carID)
{
	const tbCore::tbString pathToRacecars = "data/meshes/racecars/";
	std::vector<String> availableRacecars = {
		pathToRacecars + "formula_blue.msh",
		pathToRacecars + "formula_red.msh",
		pathToRacecars + "formula_yellow.msh",
		pathToRacecars + "formula_pink.msh",
	};

	if (carID >= availableRacecars.size())
	{	//Return generic racecar.
		return availableRacecars[0];
	}

	return availableRacecars[carID];
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

//namespace
//{
	//icePhysics::Scalar kCohesionDistance = 7.0f;   //more like a visible range.
	//icePhysics::Scalar kSeparationDistance = 3.0f; //more like a in my personal space.

	//icePhysics::Scalar kAvoidFactor = 0.2f;    //separation
	//icePhysics::Scalar kMatchingFactor = 0.2f; //alignment
	//icePhysics::Scalar kCenteringFactor = 0.2f; //cohesion
	//icePhysics::Scalar kTargetFactor = 2.0;

	//icePhysics::Scalar kMaximumVelocity = 2.0;


icePhysics::Scalar kCohesionDistance = 0.0f;   //more like a visible range.
icePhysics::Scalar kSeparationDistance = 1.664f; //more like a in my personal space.

icePhysics::Scalar kAvoidFactor = 5.0f;    //separation
icePhysics::Scalar kMatchingFactor = 1.449f; //alignment
icePhysics::Scalar kCenteringFactor = 0.913f; //cohesion
icePhysics::Scalar kTargetFactor = 40.0;

icePhysics::Scalar kMaximumVelocity = 80.0;
icePhysics::Scalar kVelocityDrag = 0.89;
icePhysics::Scalar kTargetRange = -3.5;
icePhysics::Scalar kTargetSpeed = 0.5;

//When target is stationary / in range.
//icePhysics::Scalar kCohesionDistance = 1.0f;   //more like a visible range.
//icePhysics::Scalar kSeparationDistance = 0.5f; //more like a in my personal space.
//
//icePhysics::Scalar kAvoidFactor = 2.0f;    //separation
//icePhysics::Scalar kMatchingFactor = 1.25f; //alignment
//icePhysics::Scalar kCenteringFactor = 0.913f; //cohesion
//icePhysics::Scalar kTargetFactor = 0.25;
//
//icePhysics::Scalar kMaximumVelocity = 80.0;
//icePhysics::Scalar kVelocityDrag = 0.89;
//};

float calculatePitch(float speed, float maxPitch) {
	const float minSpeed = 1.0;
	const float maxSpeed = 35.0;
	const float minPitch = 0.75;

	// Calculate the slope using the maxPitch value
	float slope = (maxPitch - minPitch) / (maxSpeed - minSpeed);
	return slope * (speed - minSpeed) + minPitch;
}

void LudumDare56::GameState::RacecarState::SimulateCreatureSwarm(void)
{
	//if (true == HasLost())
	//{
	//	for (Creature& creature : mCreatures)
	//	{
	//		if (true == creature.mIsAlive)
	//		{
	//  Ideally we would add drag to any alive creatures, and stop them from moving around. But then I realized
	//  if they are alive, but under gravity, or on/off track etc... yuck. Also at time of writing we are testing
	//  all creature dead to lose, so nobody is alive anyway.
	//		}
	//	}
	//}

#if defined(tb_debug_build)
	//static int skipFrames = 5;
	static int skipFrames = 1;
#else
	//1 disables, since all indices will be mod == 0. 2 = skip 1 frame, even/odds...
	static int skipFrames = 1;
#endif
	static int dumdumFrameCounter = 0;
	++dumdumFrameCounter;
	dumdumFrameCounter = dumdumFrameCounter % skipFrames;

	iceVector3 targetPosition = GetVehicleToWorld().GetPosition();
	targetPosition.y = 0.0f;

	const iceScalar targetSpeed = GetLinearVelocity().Magnitude();

	//tb_debug_log(LogState::Info() << "Target position: " << targetPosition);

	int creatureCount = 0;
	iceVector3 swarmPosition = iceVector3::Zero();
	mSwarmVelocity = iceVector3::Zero();

	//bool first = true;
	CreatureIndex creatureIndex = 0;
	mSwarmHealth = 0;

	std::array<std::pair<iceVector3, CreatureIndex>, 3> positionCountArray = {
		std::pair<iceVector3, CreatureIndex>{ iceVector3::Zero(), 0 },
		std::pair<iceVector3, CreatureIndex>{ iceVector3::Zero(), 0 },
		std::pair<iceVector3, CreatureIndex>{ iceVector3::Zero(), 0 },
	};

	for (Creature& creature : mCreatures)
	{
		creature.mPreviousPosition = creature.mCreatureToWorld.GetPosition();
		if (false == creature.mIsAlive)
		{
			++creatureIndex;
			continue;
		}

		if (false == creature.mIsRacing)
		{
			++mSwarmHealth;
			++creatureIndex;
			continue;
		}

		iceScalar fraction = 0.0;
		iceVector3 intersectionPoint = iceVector3::Zero();

		//if (true == mPhysicalWorld->HackyAPI_CastRayToGlobalCollider(GetVehicleToWorld().GetPosition() +
		if (true == mPhysicalWorld->HackyAPI_CastRay(GetVehicleToWorld().GetPosition() +
			Vector3::Up() * 1.0f, Vector3::Down(), intersectionPoint, fraction) && fraction < 1.1)
		{
			iceMatrix4 modifiedVehicleToWorld = GetVehicleToWorld();
			const iceVector3 oldPosition = GetVehicleToWorld().GetPosition();

			iceVector3 position = oldPosition;
			position.y = intersectionPoint.y + 0.01f;
			modifiedVehicleToWorld.SetPosition(position);
			SetVehicleToWorld(modifiedVehicleToWorld);
		}

		// TODO: LudumDare56: 2024-10-05: We might want to go implement the Spline Collider to take in a specific collider
		//   mesh instead of forcing visuals.
		if (creatureIndex % skipFrames == dumdumFrameCounter)
		{
			//if (true == mPhysicalWorld->HackyAPI_CastRayToGlobalCollider(creature.mCreatureToWorld.GetPosition() +
			if (true == mPhysicalWorld->HackyAPI_CastRay(creature.mCreatureToWorld.GetPosition() +
				Vector3::Up() * 2.0f, Vector3::Down(), intersectionPoint, fraction) && fraction < 2.10)
			{
				creature.mIsOnTrack = true;
				//creature.mVelocity.y = 0.0;

				const iceVector3 oldPosition = creature.mCreatureToWorld.GetPosition();
				iceVector3 position = creature.mCreatureToWorld.GetPosition();
				position.y = intersectionPoint.y + 0.01f;
				creature.mCreatureToWorld.SetPosition(position);

				creature.mVelocity.y = position.y - oldPosition.y;
			}
			else
			{
			//	creature.mIsAlive = false; //To insta-kill when 'getting an offtrack' Don't do up here, we might be flying!
				creature.mIsOnTrack = false;

				iceVector3 position = creature.mCreatureToWorld.GetPosition();

				iceVector3 at = iceVector3::Zero();
				if (true == icePhysics::LineSegmentToPlaneCollision(position, position + iceVector3::Down() * 0.005, iceVector3::Zero(), iceVector3::Up(), at))
				{
					creature.Die(); //To insta-kill when 'getting an offtrack'
				}
			}
		}

		const CreatureIndex engineChannel = creatureIndex % theEngineControllers.size();
		if (positionCountArray[engineChannel].second < 1)
		{
			positionCountArray[engineChannel].first += creature.mVelocity;
			positionCountArray[engineChannel].second += 1;
		}


		if (false == mIsOnTrack)
		{
			creature.mVelocity.y += -10.0f * kFixedTime;
			if (creature.mCreatureToWorld.GetPosition().y <= -0.01f)
			{
				creature.Die();
				++creatureIndex;
				continue;
			}
		}
		else if (creature.mVelocity.y < 0.0f)
		{
			creature.mVelocity.y = 0.0;
		}

		icePhysics::Scalar visibleDistance = kCohesionDistance;
		icePhysics::Scalar bubbleDistance = kSeparationDistance;

		if (creature.mCreatureToWorld.GetPosition().DistanceTo(targetPosition) < kTargetRange &&
			targetSpeed < kTargetSpeed)
		{
			visibleDistance = 1.0;
			bubbleDistance = 0.5;
		}

		const iceVector3 alignment = CalculateAlignment(creature, visibleDistance);
		const iceVector3 cohesion = CalculateCohesion(creature, visibleDistance);
		const iceVector3 separation = CalculateSeparation(creature, bubbleDistance);
		const iceVector3 closeSeparation = CalculateSeparation(creature, bubbleDistance / 4.0f);

		creature.Move(targetPosition, targetSpeed, alignment, cohesion, separation, GetVehicleToWorld());

		//if (first)
		//{
		//	first = false;
		//	tb_debug_log(LogState::Info() << "   alignment: " << alignment);
		//	tb_debug_log(LogState::Info() << "   cohesion: " << cohesion);
		//	tb_debug_log(LogState::Info() << "   separation: " << separation);
		//}

		swarmPosition += creature.mCreatureToWorld.GetPosition();
		mSwarmVelocity += creature.mVelocity;
		mSwarmVelocity.y = 0.0f;

		++mSwarmHealth;
		++creatureCount;
		++creatureIndex;
	}

	if (creatureCount == 0)
	{	//Should never happen if we have minimum health be 20 or 40 creatures...
		mSwarmToWorld = GetVehicleToWorld();
	}
	else
	{
		swarmPosition /= creatureCount;
		mSwarmVelocity /= creatureCount;

		mSwarmToWorld.SetPosition(swarmPosition);

		iceScalar swarmSpeed = 0.0;
		iceVector3 direction = iceVector3::Normalize(mSwarmVelocity, swarmSpeed);
		if (swarmSpeed < 0.4)
		{
			direction = iceVector3::Forward();
		}

		const iceVector3 right = Vector3::Cross(direction, Vector3::Up());
		mSwarmToWorld.SetBasis(0, right);
		mSwarmToWorld.SetBasis(1, Vector3::Up());
		mSwarmToWorld.SetBasis(2, -direction);


		//Do engine audio
		size_t engineChannel = 0;
		for (tbAudio::AudioController& controller : theEngineControllers)
		{
			iceVector3& average = positionCountArray[engineChannel].first;
			const CreatureIndex& count = positionCountArray[engineChannel].second;

			if (0 == count)
			{
				if (false == controller.IsComplete())
				{
					controller.Stop();
				}
			}
			else
			{
				average /= count;
				average.y = 0.0f;
				const float speed = static_cast<float>(average.Magnitude());

				tb_debug_log("channel: " << engineChannel << "   speed: " << speed << "  count: " << +count);


				//1 = 0.75
				//35 = 3.75


				//Fluffy made that math happen
				controller.Play();
				//controller.SetPitch(tbMath::Clamp(0.0882f * speed + 0.6618f, 0.75f, 4.0f));
				controller.SetPitch(calculatePitch(speed, 1.5));
			}

			++engineChannel;
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Creature::Move(const iceVector3& targetPosition, const iceScalar targetSpeed,
	const iceVector3& alignment, const iceVector3& cohesion, const iceVector3& separation, const iceMatrix4& vehicleToWorld)
{
	iceVector3 position = mCreatureToWorld.GetPosition();

	iceScalar distanceToTarget = 0.0;
	const iceVector3 directionToTarget = iceVector3::Normalize(targetPosition - position, distanceToTarget);
	//const iceVector3 directionToTarget = (targetPosition - position);

	icePhysics::Scalar avoidFactor = kAvoidFactor;
	icePhysics::Scalar centerFactor = kCenteringFactor;
	icePhysics::Scalar matchFactor = kMatchingFactor;
	icePhysics::Scalar targetFactor = kTargetFactor;

	if (distanceToTarget < kTargetRange && targetSpeed < kTargetSpeed)
	{
		avoidFactor = 2.0f;    //separation
		matchFactor = 1.25f; //alignment
		centerFactor = 0.913f; //cohesion
		targetFactor = 0.25;

		//icePhysics::Scalar kMaximumVelocity = 80.0;
		//icePhysics::Scalar kVelocityDrag = 0.89;
	}

	iceScalar speed = 0.0;
	if (true == mIsOnTrack)
	{
		mVelocity -= mVelocity * kVelocityDrag * kFixedTime;

		// Ignore any Y from swarm behavior.
		iceVector3 flatVelocity = mVelocity;
		flatVelocity += ((cohesion * centerFactor) + (separation * avoidFactor) + (alignment * matchFactor) +
			directionToTarget * targetFactor) * kFixedTime;
		flatVelocity.y = 0.0;

		speed = flatVelocity.Magnitude();
		if (speed > kMaximumVelocity)
		{
			flatVelocity = flatVelocity.GetNormalized() * kMaximumVelocity;
		}

		mVelocity.x = flatVelocity.x;
		mVelocity.z = flatVelocity.z;
	}

	position += mVelocity * kFixedTime;

	mCreatureToWorld.SetPosition(position);

	const iceVector3 direction = (speed > 0.4) ? mVelocity.GetNormalized() : -vehicleToWorld.GetBasis(2);
	const iceVector3 right = Vector3::Cross(direction, Vector3::Up());
	mCreatureToWorld.SetBasis(0, right);
	mCreatureToWorld.SetBasis(1, Vector3::Up());
	mCreatureToWorld.SetBasis(2, -direction);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::GameState::RacecarState::Creature::Die(void)
{
	mIsAlive = false;

	if (theCrashSounds.size() < 5)
	{
		theCrashSounds.push_back(tbAudio::theAudioManager.PlayEvent("audio_events", "crash"));
	}
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::CalculateCohesion(const Creature& creature, const iceScalar visibleDistance) const
{
	int count = 0;
	iceVector3 averagePosition = iceVector3::Zero();

	for (const Creature& otherCreature : mCreatures)
	{
		if (&otherCreature == &creature || false == otherCreature.mIsAlive)
		{	//Don't look at ourself or unalived creatures!
			continue;
		}

		const iceScalar distance = creature.mCreatureToWorld.GetPosition().DistanceTo(otherCreature.mCreatureToWorld.GetPosition());
		if (distance < visibleDistance)
		{
			averagePosition += otherCreature.mCreatureToWorld.GetPosition();
			++count;
		}
	}

	if (count > 0)
	{
		averagePosition = averagePosition / count;
		return averagePosition - creature.mCreatureToWorld.GetPosition();
	}

	return iceVector3::Zero();
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::CalculateSeparation(const Creature& creature, const iceScalar separationDistance) const
{
	iceVector3 separation = iceVector3::Zero();
	for (const Creature& otherCreature : mCreatures)
	{
		if (&otherCreature == &creature || false == otherCreature.mIsAlive)
		{	//Don't look at ourself or unalived creatures!
			continue;
		}

		iceScalar distance = 0.0;
		iceVector3 separationDirection = iceVector3::Normalize(creature.mCreatureToWorld.GetPosition() - otherCreature.mCreatureToWorld.GetPosition(), distance);
		if (distance < separationDistance)
		{
			// The source article said this; but we actually need to invert it so that we separate more strongly from
			//   the creatures that are closer than the creatures that are near the separation 'border'.
			//	separation += creature.mCreatureToWorld.GetPosition() - otherCreature.mCreatureToWorld.GetPosition();
			separation += separationDirection * (separationDistance - distance);
		}
	}

	return separation;
}

//--------------------------------------------------------------------------------------------------------------------//

icePhysics::Vector3 LudumDare56::GameState::RacecarState::CalculateAlignment(const Creature& creature, const iceScalar visibleDistance) const
{
	int count = 0;
	iceVector3 averageVelocity;

	for (const Creature& otherCreature : mCreatures)
	{
		if (&otherCreature == &creature || false == otherCreature.mIsAlive)
		{	//Don't look at ourself or unalived creatures!
			continue;
		}

		const iceScalar distance = creature.mCreatureToWorld.GetPosition().DistanceTo(otherCreature.mCreatureToWorld.GetPosition());
		if (distance < visibleDistance)	//used as 'visual range' in https://vanhunteradams.com/Pico/Animal_Movement/Boids-algorithm.html
		{
			averageVelocity += otherCreature.mVelocity;
			++count;
		}
	}

	if (count > 0)
	{
		averageVelocity = averageVelocity / count;
		return averageVelocity - creature.mVelocity;
	}

	return iceVector3::Zero();
}

//--------------------------------------------------------------------------------------------------------------------//
