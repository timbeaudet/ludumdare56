///
/// @file
/// @details Wrapping up the details of implementation of Dear ImGui for TurtleBrains.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef Core_TurtleBrainsImGui_hpp
#define Core_TurtleBrainsImGui_hpp
#if defined(development_build) && !defined(ludumdare56_headless_build)

#include "../../dependencies/imgui/imgui.h"

#include <turtle_brains/core/tb_string.hpp>

namespace TyreBytes
{
	namespace Core
	{
		namespace Development
		{

			namespace tbImGui
			{

				void Initialize(const tbCore::tbString& imguiFilepath);
				void Cleanup(void);
				void UpdateFrame(const float deltaTime);
				void RenderFrame(void);

				bool WantsInput(void);

			};	//namespace tbImGui

		};	//namespace Development
	};	//namespace Core
};	//namespace TyreBytes

#endif /* development_build && !defined(ludumdare56_headless_build) */
#endif /* Core_TurtleBrainsImGui_hpp */
