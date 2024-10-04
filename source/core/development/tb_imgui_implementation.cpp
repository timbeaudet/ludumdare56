///
/// @file
/// @details Wrapping up the details of implementation of Dear ImGui for TurtleBrains.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "tb_imgui_implementation.hpp"

#include <turtle_brains/core/tb_opengl.hpp>
#include <turtle_brains/graphics/tb_graphic.hpp>
#include <turtle_brains/graphics/tb_texture_manager.hpp>
#include <turtle_brains/graphics/implementation/tbi_renderer.hpp>
#include <turtle_brains/game/tb_input_action.hpp>

#if defined(development_build) && !defined(ludumdare56_headless_build)

tbGraphics::TextureHandle theImGuiTexture(tbGraphics::InvalidTexture());

namespace
{
	tbCore::tbString theImGuiSettingsFile;
};

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::tbImGui::Initialize(const tbCore::tbString& imguiFilepath)
{
	theImGuiSettingsFile = imguiFilepath;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = theImGuiSettingsFile.c_str();

	// Setup style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	unsigned char* pixelData = nullptr;
	int width = 0;
	int height = 0;
	io.Fonts->GetTexDataAsRGBA32(&pixelData, &width, &height);
	theImGuiTexture = tbGraphics::theTextureManager.CreateTextureFromPixelData(
		static_cast<tbGraphics::PixelSpace>(width), static_cast<tbGraphics::PixelSpace>(height), tbGraphics::TextureFormat::ColorARGB8, pixelData);
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::tbImGui::Cleanup(void)
{
	//ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::tbImGui::UpdateFrame(const float deltaTime)
{
//	if (!g_FontTexture)
//		ImGui_ImplGlfwGL3_CreateDeviceObjects();

	ImGuiIO& io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	io.DisplaySize = ImVec2(static_cast<float>(tbGraphics::ScreenWidth()), static_cast<float>(tbGraphics::ScreenHeight()));
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	io.DeltaTime = deltaTime;

	// Setup inputs
	// (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
	if (true)// glfwGetWindowAttrib(g_Window, GLFW_FOCUSED))
	{
		// Set OS mouse position if requested (only used when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
		if (io.WantSetMousePos)
		{
			tbGame::Input::SetMousePosition(tbMath::Vector2(io.MousePos.x, io.MousePos.y));
		}
		else
		{
			tbMath::Vector2 mousePosition(tbGame::Input::GetMousePosition());
			io.MousePos = ImVec2(mousePosition.x, mousePosition.y);
		}
	}
	else
	{
		io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
	}

	io.MouseWheel = tbApplication::Input::GetMouseWheelDeltaPrecise();

	io.MouseDown[0] = tbGame::Input::IsKeyDown(tbApplication::tbMouseLeft);
	io.MouseDown[1] = tbGame::Input::IsKeyDown(tbApplication::tbMouseRight);
	io.MouseDown[2] = tbGame::Input::IsKeyDown(tbApplication::tbMouseMiddle);

	io.KeyAlt = tbGame::Input::IsKeyDown(tbApplication::tbKeyAltModifier);
	io.KeyCtrl = tbGame::Input::IsKeyDown(tbApplication::tbKeyControlModifier);
	io.KeyShift = tbGame::Input::IsKeyDown(tbApplication::tbKeyShiftModifier);

	for (int i = 0; i < tbApplication::tbKeyMax; ++i)
	{
		io.KeysDown[tbApplication::tbKey0 + i] = tbApplication::Input::IsKeyDown(static_cast<tbApplication::Key>(tbApplication::tbKey0 + i));
	}

	io.KeyMap[ImGuiKey_Tab] = tbApplication::tbKeyTab;
	io.KeyMap[ImGuiKey_LeftArrow] = tbApplication::tbKeyLeft;
	io.KeyMap[ImGuiKey_RightArrow] = tbApplication::tbKeyRight;
	io.KeyMap[ImGuiKey_UpArrow] = tbApplication::tbKeyUp;
	io.KeyMap[ImGuiKey_DownArrow] = tbApplication::tbKeyDown;
	io.KeyMap[ImGuiKey_PageUp] = tbApplication::tbKeyPageUp;
	io.KeyMap[ImGuiKey_PageDown] = tbApplication::tbKeyPageDown;
	io.KeyMap[ImGuiKey_Home] = tbApplication::tbKeyHome;
	io.KeyMap[ImGuiKey_End] = tbApplication::tbKeyEnd;
	io.KeyMap[ImGuiKey_Insert] = tbApplication::tbKeyInsert;
	io.KeyMap[ImGuiKey_Delete] = tbApplication::tbKeyDelete;
	io.KeyMap[ImGuiKey_Backspace] = tbApplication::tbKeyBackspace;
	io.KeyMap[ImGuiKey_Space] = tbApplication::tbKeySpace;
	io.KeyMap[ImGuiKey_Enter] = tbApplication::tbKeyEnter;
	io.KeyMap[ImGuiKey_Escape] = tbApplication::tbKeyEscape;
	io.KeyMap[ImGuiKey_A] = tbApplication::tbKeyA;
	io.KeyMap[ImGuiKey_C] = tbApplication::tbKeyC;
	io.KeyMap[ImGuiKey_V] = tbApplication::tbKeyV;
	io.KeyMap[ImGuiKey_X] = tbApplication::tbKeyX;
	io.KeyMap[ImGuiKey_Y] = tbApplication::tbKeyY;
	io.KeyMap[ImGuiKey_Z] = tbApplication::tbKeyZ;

	for (ImWchar i = 0; i < 10; ++i)
	{
		if (tbApplication::Input::IsKeyPressed(static_cast<tbApplication::Key>(tbApplication::tbKey0 + i)))
		{
			io.KeysDown[tbApplication::tbKey0 + i] = true;
			io.AddInputCharacter('0' + i);
		}

		if (tbApplication::Input::IsKeyPressed(static_cast<tbApplication::Key>(tbApplication::tbKeyNumpad0 + i)))
		{
			io.KeysDown[tbApplication::tbKeyNumpad0 + i] = true;
			io.AddInputCharacter('0' + i);
		}
	}

	for (ImWchar i = 0; i < 26; ++i)
	{
		if (tbApplication::Input::IsKeyPressed(static_cast<tbApplication::Key>(tbApplication::tbKeyA + i)))
		{
			io.AddInputCharacter((io.KeyShift) ? ('A' + i) : ('a' + i));
		}
	}

	if (tbApplication::Input::IsKeyPressed(tbApplication::tbKeySpace)) { io.AddInputCharacter(' '); }
	if (tbApplication::Input::IsKeyPressed(tbApplication::tbKeyPeriod)) { io.AddInputCharacter('.'); }
	if (tbApplication::Input::IsKeyPressed(tbApplication::tbKeyNumpadPeriod)) { io.AddInputCharacter('.'); }
	if (tbApplication::Input::IsKeyPressed(tbApplication::tbKeyMinus)) { io.AddInputCharacter((io.KeyShift) ? '_' : '-'); }
	if (tbApplication::Input::IsKeyPressed(tbApplication::tbKeyPlus)) { io.AddInputCharacter((io.KeyShift) ? '+' : '='); }
	if (tbApplication::Input::IsKeyPressed(tbApplication::tbKeyNumpadAdd)) { io.AddInputCharacter('+'); }
	if (tbApplication::Input::IsKeyPressed(tbApplication::tbKeyNumpadSubtract)) { io.AddInputCharacter('-'); }
	if (tbApplication::Input::IsKeyPressed(tbApplication::tbKeyNumpadMultiply)) { io.AddInputCharacter('*'); }
	if (tbApplication::Input::IsKeyPressed(tbApplication::tbKeyNumpadDivide)) { io.AddInputCharacter('/'); }


	//// Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
	//if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0 && glfwGetInputMode(g_Window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
	//{
	//	ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
	//	if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
	//	{
	//		glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	//	}
	//	else
	//	{
	//		glfwSetCursor(g_Window, g_MouseCursors[cursor] ? g_MouseCursors[cursor] : g_MouseCursors[ImGuiMouseCursor_Arrow]);
	//		glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	//	}
	//}
//
//	// Gamepad navigation mapping [BETA]
//	memset(io.NavInputs, 0, sizeof(io.NavInputs));
//	if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)
//	{
//		// Update gamepad inputs
//#define MAP_BUTTON(NAV_NO, BUTTON_NO)       { if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS) io.NavInputs[NAV_NO] = 1.0f; }
//#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1) { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); if (v > 1.0f) v = 1.0f; if (io.NavInputs[NAV_NO] < v) io.NavInputs[NAV_NO] = v; }
//		int axes_count = 0, buttons_count = 0;
//		const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
//		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
//		MAP_BUTTON(ImGuiNavInput_Activate, 0);     // Cross / A
//		MAP_BUTTON(ImGuiNavInput_Cancel, 1);     // Circle / B
//		MAP_BUTTON(ImGuiNavInput_Menu, 2);     // Square / X
//		MAP_BUTTON(ImGuiNavInput_Input, 3);     // Triangle / Y
//		MAP_BUTTON(ImGuiNavInput_DpadLeft, 13);    // D-Pad Left
//		MAP_BUTTON(ImGuiNavInput_DpadRight, 11);    // D-Pad Right
//		MAP_BUTTON(ImGuiNavInput_DpadUp, 10);    // D-Pad Up
//		MAP_BUTTON(ImGuiNavInput_DpadDown, 12);    // D-Pad Down
//		MAP_BUTTON(ImGuiNavInput_FocusPrev, 4);     // L1 / LB
//		MAP_BUTTON(ImGuiNavInput_FocusNext, 5);     // R1 / RB
//		MAP_BUTTON(ImGuiNavInput_TweakSlow, 4);     // L1 / LB
//		MAP_BUTTON(ImGuiNavInput_TweakFast, 5);     // R1 / RB
//		MAP_ANALOG(ImGuiNavInput_LStickLeft, 0, -0.3f, -0.9f);
//		MAP_ANALOG(ImGuiNavInput_LStickRight, 0, +0.3f, +0.9f);
//		MAP_ANALOG(ImGuiNavInput_LStickUp, 1, +0.3f, +0.9f);
//		MAP_ANALOG(ImGuiNavInput_LStickDown, 1, -0.3f, -0.9f);
//#undef MAP_BUTTON
//#undef MAP_ANALOG
//		if (axes_count > 0 && buttons_count > 0)
//			io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
//		else
//			io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
//	}

	// Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
	ImGui::NewFrame();
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::Development::tbImGui::RenderFrame(void)
{
	ImGui::Render();

	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	ImGuiIO& io = ImGui::GetIO();
	int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width == 0 || fb_height == 0)
		return;

	ImDrawData* draw_data = ImGui::GetDrawData();
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	// Backup GL state
	GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
	glActiveTexture(GL_TEXTURE0);
	GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
	GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
	GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
	GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#if !defined(tb_web)
	GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
	GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
	GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
	GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
	GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
	GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
	GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
	glEnable(GL_BLEND);
	//glEnable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
#if !defined(tb_web)
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif /* tb_web */

	// Setup viewport, orthographic projection matrix
	glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
	// const float ortho_projection[4][4] =
	// {
	// 	{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
	// 	{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
	// 	{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
	// 	{ -1.0f,                  1.0f,                   0.0f, 1.0f },
	// };

//	glUseProgram(g_ShaderHandle);
//--	glUniform1i(g_AttribLocationTex, 0);
//	glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);

	//2022-04-30: There was a warning here that glBindSampler would always be true, so I've commented out the original
	//  line and just called the glBindSampler(), this will probably break on some obscure platform, mobile, etc...
#if defined(tb_web)
	glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#else
	if (glBindSampler) glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif

	// Recreate the VAO every time
	// (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts, and we don't track creation/deletion of windows so we don't have an obvious key to use to cache them.)
	GLuint vao_handle = 0;
	glGenVertexArrays(1, &vao_handle);
	glBindVertexArray(vao_handle);
//	glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);

	unsigned int vertexBuffer(0);
	unsigned int elementsBuffer(0);
	glGenBuffers(1, &elementsBuffer);
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	//Set the positional attributes.
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));

	glEnableVertexAttribArray(0);

	//Set the color attributes.
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
	glEnableVertexAttribArray(1);

	//Set the texture coordinate attributes.
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
	glEnableVertexAttribArray(2);

	std::vector<tbImplementation::Renderer::Vertex2D> verts;

	// Draw
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const ImDrawIdx* idx_buffer_offset = 0;

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				//Essentially during the creation we could set the fonts texture (which is void*) to a TurtleBrains texture handle
				//and then cast it back here. Since we are only using a single font texture, currently just using the handle as is.
				//io.Fonts->TexID = (void *)(intptr_t)g_FontTexture; //In Create.
				//tbGraphics::theTextureManager.BindTexture(pcmd->TextureId);
				tb_error_if(nullptr != pcmd->TextureId, "If more than one texture id is needed, see comment.");
				tbGraphics::theTextureManager.BindTexture(theImGuiTexture);
				glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
			}
			idx_buffer_offset += pcmd->ElemCount;
		}
	}
	//glDeleteVertexArrays(1, &vao_handle);

	// Restore modified GL state
	glUseProgram(last_program);
	glBindTexture(GL_TEXTURE_2D, last_texture);

	//2022-04-30: There was a warning here that glBindSampler would always be true, so I've commented out the original
	//  line and just called the glBindSampler(), this will probably break on some obscure platform, mobile, etc...
#if defined(tb_web)
	glBindSampler(0, last_sampler);
#else
	if (glBindSampler) glBindSampler(0, last_sampler);
#endif

	glActiveTexture(last_active_texture);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vertexBuffer);

	glBindVertexArray(last_vertex_array);
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
	if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#if !defined(tb_web)
	glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif /* tb_web */
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

bool TyreBytes::Core::Development::tbImGui::WantsInput(void)
{
	return ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse;
}

//--------------------------------------------------------------------------------------------------------------------//

#endif /* development_build && !defined(ludumdare56_headless_build) */
