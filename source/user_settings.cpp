///
/// @file
/// @details A place to hold configuration/preferences/settings/values for the player.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "user_settings.hpp"
#include "ludumdare56.hpp" //GetSaveDirectory()
#include "logging.hpp"

#include "core/input/key_binder.hpp"

#include <turtle_brains/core/tb_dynamic_structure.hpp>
#include <turtle_brains/application/tb_application_input.hpp>
#include <turtle_brains/system/tb_system_utilities.hpp>
#include <turtle_brains/math/tb_math.hpp>

#include <fstream>

using tbCore::Debug::LogGameplay;

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::UserSettings::CreateDefaultSettings(void)
{
	SetBoolean(Settings::Fullscreen(), false);
	SetBoolean(Settings::VerticalSync(), false);
	SetBoolean(Settings::ShowShadows(), true);
	SetBoolean(Settings::ShowReflections(), false);
	SetBoolean(Settings::SinglePassShadows(), true);
	SetBoolean(Settings::ShowParticles(), true);
	SetBoolean(Settings::ShowBloom(), true);
	SetBoolean(Settings::ShowAmbientOcclusion(), true);
	SetInteger(Settings::WindowPositionX(), 100);
	SetInteger(Settings::WindowPositionY(), 100);
	SetInteger(Settings::WindowWidth(), 1280);
	SetInteger(Settings::WindowHeight(), 720);
	SetInteger(Settings::SuperSampling(), 2);
	SetFloat(Settings::FieldOfView(), 90.0f);
	SetFloat(Settings::MusicVolume(), 0.7f);
	SetFloat(Settings::SoundVolume(), 0.75f);
	SetFloat(Settings::ShakeIntensity(), 1.0f);
	SetFloat(Settings::InterfaceAspectRatio(), 16.0f / 9.0f);
	SetString(Settings::ControlSteering(), "LeftArrow");
	SetBoolean(Settings::ControlSteeringInverted(), false);
	SetFloat(Settings::ControlSteeringDeadzone(), 0.15f);
	SetString(Settings::ControlThrottle(), "UpArrow");
	SetBoolean(Settings::ControlThrottleInverted(), false);
	SetString(Settings::ControlBrake(), "DownArrow");
	SetBoolean(Settings::ControlBrakeInverted(), false);
	SetString(Settings::ControlReset(), "R");
	SetString(Settings::ControlShiftUp(), "A");
	SetString(Settings::ControlShiftDown(), "Z");
	SetString(Settings::ControlHandbrake(), "X");
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::UserSettings& LudumDare56::TheUserSettings(void)
{
	static UserSettings theUserSettings;
	return theUserSettings;
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::UserSettings::UserSettings(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::UserSettings::~UserSettings(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::UserSettings::GetBoolean(const String& keyName, const bool defaultValue) const
{
	return mUserSettings[keyName].AsBooleanWithDefault(defaultValue);
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::int64 LudumDare56::UserSettings::GetInteger(const String& keyName, const tbCore::int64 defaultValue) const
{
	return mUserSettings[keyName].AsIntegerWithDefault(defaultValue);
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::UserSettings::GetFloat(const String& keyName, const float defaultValue) const
{
	return mUserSettings[keyName].AsFloatWithDefault(defaultValue);
}

//--------------------------------------------------------------------------------------------------------------------//

LudumDare56::String LudumDare56::UserSettings::GetString(const String& keyName, const String& defaultValue) const
{
	return mUserSettings[keyName].AsStringWithDefault(defaultValue);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::UserSettings::SetBoolean(const String& keyName, const bool value)
{
	mUserSettings[keyName] = value;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::UserSettings::SetInteger(const String& keyName, const tbCore::int64 value)
{
	mUserSettings[keyName] = value;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::UserSettings::SetFloat(const String& keyName, const float value)
{
	mUserSettings[keyName] = value;
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::UserSettings::SetString(const String& keyName, const String& value)
{
	mUserSettings[keyName] = value;
}

//--------------------------------------------------------------------------------------------------------------------//

bool LudumDare56::UserSettings::HasKey(const String& keyName)
{
	return mUserSettings.HasMember(keyName);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::UserSettings::DeleteKey(const String& keyName)
{
	mUserSettings.RemoveMember(keyName);
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::UserSettings::LoadSettings(const String& filename)
{
	String settingsFilepath = filename; //Try next to executable first.

	bool settingsFileExists = tbSystem::DoesFileExist(settingsFilepath);
	if (true == settingsFileExists)
	{
		mIsLocalSettings = true;
	}
	else
	{
		settingsFilepath = LudumDare56::GetSaveDirectory() + filename;
		settingsFileExists = tbSystem::DoesFileExist(settingsFilepath);
	}

	if (false == settingsFileExists)
	{	//Not loading the file is not exactly an error, but LoadJsonFile would go boom.
		return;
	}

	const tbCore::DynamicStructure settingsData = tbCore::LoadJsonFile(settingsFilepath);
	if (true == settingsData.IsNil())
	{
		tb_always_log(LogGame::Error() << "Failed to load the " << QuotedString(settingsFilepath) << " file.");
		return;
	}

	// 2024-09-03: We can't just assign like: mUserSettings = settingsData; because that will remove any of the defaults
	//   setup by CreateDefaultSettings().
	for (auto& keyValueIterator : settingsData.AsStructure())
	{
		mUserSettings[keyValueIterator.first] = keyValueIterator.second;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LudumDare56::UserSettings::SaveSettings(const String& filename) const
{
	const String settingsFilepath(((mIsLocalSettings) ? "" : LudumDare56::GetSaveDirectory()) + filename);
	if (false == tbCore::SaveJsonFile(settingsFilepath, mUserSettings, true))
	{
		tb_always_log(LogGame::Error() << "Failed to save the settings at: " << QuotedString(settingsFilepath));
	}
}

//--------------------------------------------------------------------------------------------------------------------//
