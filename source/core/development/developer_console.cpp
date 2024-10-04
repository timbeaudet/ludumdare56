///
/// @file
/// @details This is a small terminal/console emulator using ImGui for developer modes.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "developer_console.hpp"

#include <turtle_brains/core/tb_defines.hpp>
#include <turtle_brains/core/debug/tb_debug_logger.hpp>

#if defined(development_build)

#include "tb_imgui_implementation.hpp"


#if !defined(ludumdare56_headless_build)

#include "console_command_system.hpp"

#include <array>
#include <list>

//--------------------------------------------------------------------------------------------------------------------//

namespace
{
	const tbCore::tbString kCommandPrefix("dev$ ");
	bool hackFocus = false;
}

class Terminal
{
public:
	std::array<char, 2048> mInputBuffer;
	std::list<tbCore::tbString> mInputHistory;
	std::list<tbCore::tbString> mTerminalHistory;
	bool mAutoScroll;
	bool mScrollToBottom;
	bool mIsOpened;

	Terminal(void) :
		mInputBuffer{ 0, },
		mInputHistory(),
		mTerminalHistory(),
		mAutoScroll(true),
		mScrollToBottom(true),
		mIsOpened(false)
	{
	}

	~Terminal(void)
	{
	}

	void ClearInputBuffer(void)
	{
		memset(mInputBuffer.data(), 0, mInputBuffer.size());
	}

	void ClearLog(void)
	{
		mTerminalHistory.clear();
		mScrollToBottom = true;
	}

	void AddLog(const tbCore::tbString& logMessage)
	{
		tb_debug_log("DevConsole: " << logMessage);

		mTerminalHistory.push_back(logMessage);
		if (true == mAutoScroll)
		{
            mScrollToBottom = true;
		}
	}

    static void  Strtrim(char* str)
	{
		char* str_end = str + strlen(str);
		while (str_end > str && str_end[-1] == ' ')
		{
			--str_end;
		}
		*str_end = '\0';
	}

	void DisplayTerminal(void)
	{
        ImGui::SetNextWindowSize(ImVec2(780, 340), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowCollapsed(!mIsOpened);

        if (false == ImGui::Begin("Developer Terminal"))
        {
            ImGui::End();
            return;
        }

        const float footerHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeight), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::Selectable("Clear"))
			{
				ClearLog();
			}
            ImGui::EndPopup();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1)); // Tighten spacing

        for (const auto& itemString : mTerminalHistory)
        {
            const char* item = itemString.c_str();

			ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			if (strstr(item, "error")) { textColor = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); }

			ImGui::PushStyleColor(ImGuiCol_Text, textColor);
			ImGui::TextUnformatted(item);
			ImGui::PopStyleColor();
        }

        if (true == mScrollToBottom)
		{
            ImGui::SetScrollHereY(1.0f);
		}

        mScrollToBottom = false;
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool reclaim_focus = false;
        if (ImGui::InputText("Input", mInputBuffer.data(), mInputBuffer.size(),
			ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
			&TextEditCallbackStub, (void*)this))
        {
            char* commandString = mInputBuffer.data();
            Strtrim(commandString);
            if (commandString[0])
			{
		        AddLog(kCommandPrefix + commandString);
				tbDevelopment::TheCommandManager().ExecuteCommand(commandString);
			}
            strcpy(commandString, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus || hackFocus)
		{
			hackFocus = false;
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		}

        ImGui::End();
	}

	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
    {
        Terminal* terminal = reinterpret_cast<Terminal*>(data->UserData);
        return terminal->TextEditCallback(data);
    }

	int TextEditCallback(ImGuiInputTextCallbackData* data)
	{
		tb_unused(data);

		//Look at the example for example on how to select through command history with pressing up/down or tab-completion
		//with various events that arrive through the callback.
		return 0;
	}
};

namespace
{
	Terminal theTerminal;
	std::array<char, 2048> theMessageBuffer;
};

#endif /* not ludumdare56_headless_build */
#endif /* development_build */

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::ToggleDeveloperConsole(void)
{
#if defined(development_build) && !defined(ludumdare56_headless_build)
	theTerminal.mIsOpened = !theTerminal.mIsOpened;
	if (true == theTerminal.mIsOpened)
	{
		hackFocus = true;
	}
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::DisplayTerminal(void)
{
#if defined(development_build) && !defined(ludumdare56_headless_build)
	theTerminal.DisplayTerminal();
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::AddLog(const tbCore::tbString& message)
{
#if defined(development_build) && !defined(ludumdare56_headless_build)
	theTerminal.AddLog(message);
#else
	tb_unused(message);
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::AddLog(const char* format, ...)
{
#if defined(development_build) && !defined(ludumdare56_headless_build)
	va_list arguments;
	va_start(arguments, format);

	memset(theMessageBuffer.data(), 0, theMessageBuffer.size());
	vsprintf(theMessageBuffer.data(), format, arguments);
	theTerminal.AddLog(tbCore::ToString(theMessageBuffer.data()));

	va_end(arguments);
#else
	tb_unused(format);
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::InitializeDevelopmentTools(const tbCore::tbString& saveDirectory)
{
#if defined(development_build)
	Development::tbImGui::Initialize(saveDirectory + "imgui.ini");
#else
	tb_unused(saveDirectory);
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//

void TyreBytes::Core::CleanupDevelopmentTools(void)
{
#if defined(development_build)
	Development::tbImGui::Cleanup();
#endif /* development_build */
}

//--------------------------------------------------------------------------------------------------------------------//
