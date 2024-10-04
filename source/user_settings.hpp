///
/// @file
/// @details A place to hold configuration/preferences/settings/values for the player.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_UserSettings_hpp
#define LudumDare56_UserSettings_hpp

//Do Not Include ludumdare56.hpp or any other LudumDare56 headers that have a chance of doing so.

#include <turtle_brains/core/tb_dynamic_structure.hpp>

#include <array>

namespace LudumDare56
{
	typedef TurtleBrains::Core::tbString String;

	namespace Settings
	{
		inline String Fullscreen(void) { return "fullscreen"; } //Boolean
		inline String VerticalSync(void) { return "vertical_sync"; } //Boolean
		inline String ShowShadows(void) { return "show_shadows"; } //Boolean
		inline String ShowReflections(void) { return "show_reflections"; } //Boolean
		inline String SinglePassShadows(void) { return "single_pass_shadows"; } //Boolean
		inline String ShowParticles(void) { return "show_particles"; } //Boolean
		inline String ShowBloom(void) { return "show_bloom"; } //Boolean
		inline String ShowAmbientOcclusion(void) { return "show_ao"; } //Boolean

		inline String WindowPositionX(void) { return "window_position_x"; } //Integer
		inline String WindowPositionY(void) { return "window_position_y"; } //Integer
		inline String WindowWidth(void) { return "window_width"; } //Integer
		inline String WindowHeight(void) { return "window_height"; } //Integer
		inline String SuperSampling(void) { return "super_sampling"; } //Integer

		inline String FieldOfView(void) { return "field_of_view"; } //Float
		inline String MusicVolume(void) { return "music_volume"; } //Float
		inline String SoundVolume(void) { return "sound_volume"; } //Float
		inline String ShakeIntensity(void) { return "shake_intensity"; } //Float
		inline String InterfaceAspectRatio(void) { return "interface_aspect_ratio"; } //Float

		inline String ControlSteering(void) { return "control_steering"; } //String
		inline String ControlSteeringInverted(void) { return "control_steering_inverted"; } //Boolean
		inline String ControlSteeringDeadzone(void) { return "control_steering_deadzone"; } //Float
		inline String ControlThrottle(void) { return "control_throttle"; } //String
		inline String ControlThrottleInverted(void) { return "control_throttle_inverted"; } //Boolean
		inline String ControlBrake(void) { return "control_brake"; } //String
		inline String ControlBrakeInverted(void) { return "control_brake_inverted"; } //Boolean

		inline String ControlReset(void) { return "control_reset"; } //String
		inline String ControlShiftUp(void) { return "control_shift_up"; } //String
		inline String ControlShiftDown(void) { return "control_shift_down"; } //String
		inline String ControlHandbrake(void) { return "control_handbrake"; } //String
	};

	///
	/// @details This object is a table of key-value pairs for various user settings. There is a primary/global settings
	///   object TheUserSettings() that gets loaded and saved each run, however this is an object rather than a namespace
	///   or singleton to support launch parameters using the settings, or developer settings etc.
	///
	/// @note 2023-10-24: We tried adding a "settings accessor object" which effectively had private constructors and
	///   static functions like Settings::VerticalSyncKey() and Settings::VerticalSync() which made the key and accessor
	///   object. This would need to also have a Setting::VerticalSync(UserSettings& s) to allow launchSettings and
	///   other non-global settings objects to be used. The benefit added was too minimal to keep, at least at this time.
	///
	///   So we settled on returning the Key from a faux string-enum which will prevent typos and mismatched names for
	///   the same user settings; see/use Settings namespace above.
	///
	class UserSettings
	{
	public:
		UserSettings(void);
		~UserSettings(void);

		bool GetBoolean(const String& keyName, const bool defaultValue = false) const;
		tbCore::int64 GetInteger(const String& keyName, const tbCore::int64 defaultValue = 0) const;
		float GetFloat(const String& keyName, const float defaultValue = 0.0f) const;
		String GetString(const String& keyName, const String& defaultValue = "") const;

		void SetBoolean(const String& keyName, const bool value);
		void SetInteger(const String& keyName, const tbCore::int64 value);
		void SetFloat(const String& keyName, const float value);
		void SetString(const String& keyName, const String& value);

		bool HasKey(const String& keyName);
		void DeleteKey(const String& keyName);

		///
		/// @param filename Should be JUST the filename not the whole path.
		///
		/// @note This will attempt to load a file directly from the CurrentWorkingDirectory and if that fails try
		///   loading the file from SaveDirectory(). Internally this will set a flag to ensure the local vs save
		///   directory is used when saving the file.
		///
		void LoadSettings(const String& filename);
		void SaveSettings(const String& filename) const;

		void CreateDefaultSettings(void);

	private:
		tbCore::DynamicStructure mUserSettings;
		bool mIsLocalSettings;
	};	//namespace UserSettings

	UserSettings& TheUserSettings(void);

};	//namespace LudumDare56

#endif /* LudumDare56_UserSettings_hpp */
