///
/// @file
/// @details Entry point of the LudumDare56 project.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_LudumDare56_hpp
#define LudumDare56_LudumDare56_hpp

#include "user_settings.hpp"

#include <turtle_brains/tb_application_kit.hpp>
#include <turtle_brains/tb_audio_kit.hpp>
#include <turtle_brains/tb_core_kit.hpp>
#include <turtle_brains/tb_debug_kit.hpp>
#include <turtle_brains/tb_game_kit.hpp>
#include <turtle_brains/tb_graphics_kit.hpp>
#include <turtle_brains/tb_math_kit.hpp>
#include <turtle_brains/tb_development_kit.hpp>

#include <ice/graphics/ice_mesh.hpp>
#include <ice/graphics/ice_light.hpp>
#include <ice/graphics/ice_camera.hpp>
#include <ice/graphics/ice_renderer.hpp>
#include <ice/graphics/ice_shader_manager.hpp>
#include <ice/graphics/ice_visualization.hpp>
#include <ice/graphics/ice_graphic.hpp>
#include <ice/graphics/ice_render_queue.hpp>

#include <ice/physics/ice_physical_types.hpp>

#include <cstdint>

#if defined(development_build) && defined(ludumdare56_headless_build)
#error A headless_build cannot enable development_build,
#endif

namespace LudumDare56
{
	typedef TurtleBrains::Core::tbString String;
	typedef TurtleBrains::Math::Vector2 Vector2;
	typedef TurtleBrains::Math::Vector3 Vector3;
	typedef TurtleBrains::Math::Vector4 Vector4;
	typedef TurtleBrains::Math::Matrix3 Matrix3;
	typedef TurtleBrains::Math::Matrix4 Matrix4;
	typedef TurtleBrains::Math::Angle Angle;

	typedef InternalCombustion::Physics::Scalar iceScalar;
	typedef InternalCombustion::Physics::Vector2 iceVector2;
	typedef InternalCombustion::Physics::Vector3 iceVector3;
	typedef InternalCombustion::Physics::Vector4 iceVector4;
	typedef InternalCombustion::Physics::Matrix3 iceMatrix3;
	typedef InternalCombustion::Physics::Matrix4 iceMatrix4;
	typedef InternalCombustion::Physics::Angle iceAngle;

	typedef std::unique_ptr<InternalCombustion::Graphics::Graphic> GraphicPtr;

	typedef TurtleBrains::Application::Key Key;
	namespace Input = TurtleBrains::Game::Input;

	typedef tbCore::uint32 MillisecondTimer;

	static const MillisecondTimer kFixedTimeMS(10);
	static const float kFixedTime(0.01f);
	static const Vector3 kTheZeroVector = Vector3::Zero();

	inline Vector3 WorldUp(void) { return Vector3(0.0f, 1.0f, 0.0f); }

	inline Vector3 Right(void) { return Vector3(1.0f, 0.0f, 0.0f); }
	inline Vector3 Up(void) { return Vector3(0.0f, 1.0f, 0.0f); }
	inline Vector3 Forward(void) { return Vector3(0.0f, 0.0f, -1.0f); }
	inline Vector3 Left(void) { return -Right(); }
	inline Vector3 Down(void) { return -Up(); }
	inline Vector3 Backward(void) { return -Forward(); }

	String GetSaveDirectory(void);
	String GetTwitchClientID(void);
	String GetPatreonClientID(void);
	String GetYouTubeClientID(void);

	String GetQuickPlayRacetrackPath(void);

	UserSettings ParseLaunchParameters(int argumentCount, const char* argumentValues[]);

}; /* namespace LudumDare56 */

#endif /* LudumDare56_LudumDare56_hpp */
