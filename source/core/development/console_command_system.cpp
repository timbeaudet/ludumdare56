///
/// @file
/// @details Creates a developer tool command-line system to register and parse commands from the user.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "console_command_system.hpp"
#include "developer_console.hpp"

#include <turtle_brains/core/unit_test/tb_unit_test.hpp>
#include <turtle_brains/core/unit_test/tb_test_provider.hpp>
#include <turtle_brains/core/debug/tb_debug_logger.hpp>

#include <algorithm>
#include <queue>

namespace
{
	const tbCore::tbString kSpacing("    ");
};

namespace tbImplementation
{
	class CommandManagerCreator
	{
	public:
		CommandManagerCreator(void) :
			mCommandManager()
		{
		}

		~CommandManagerCreator(void)
		{
		}

		tbDevelopment::CommandManager mCommandManager;
	};
};

TurtleBrains::Development::CommandManager& TurtleBrains::Development::TheCommandManager(void)
{
	static tbImplementation::CommandManagerCreator theCommandManagerCreator;
	return theCommandManagerCreator.mCommandManager;
}

//TurtleBrains::Development::CommandManager& TurtleBrains::Development::theCommandManager(theCommandManagerCreator.mCommandManager);

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TurtleBrains::Development::CommandDefinition::CommandDefinition(const tbCore::tbString& commandName, const tbCore::tbString& commandDescription) :
	mCommandName(commandName),
	mDescriptions{ commandDescription },
	mSynopsis{ },
	mIsEnabled(false)
{
	Enable();
}

//--------------------------------------------------------------------------------------------------------------------//

TurtleBrains::Development::CommandDefinition::CommandDefinition(const tbCore::tbString& commandName, const std::vector<tbCore::tbString>& commandDescriptions) :
	mCommandName(commandName),
	mDescriptions{ commandDescriptions },
	mSynopsis{ },
	mIsEnabled(false)
{
	Enable();
}

//--------------------------------------------------------------------------------------------------------------------//

TurtleBrains::Development::CommandDefinition::~CommandDefinition(void)
{
	Disable();
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandDefinition::Enable(void)
{
	if (false == mIsEnabled)
	{
		TheCommandManager().AddCommandDefinition(this);
		mIsEnabled = true;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandDefinition::Disable(void)
{
	if (true == mIsEnabled)
	{
		TheCommandManager().RemoveCommandDefinition(this);
		mIsEnabled = false;
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandDefinition::AddSynopsis(const tbCore::tbString& synopsis)
{
	mSynopsis.push_back(synopsis);
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandDefinition::AddSynopsis(const std::vector<tbCore::tbString>& synopsis)
{
	for (const auto& line : synopsis)
	{
		AddSynopsis(line);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandDefinition::AddParameter(const tbCore::tbString& parameterName, const ParameterType& parameterType)
{
	mParameters.push_back(Parameter{ parameterName, parameterType });
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandDefinition::AddOption(const tbCore::tbString& optionName, const std::vector<tbCore::tbString>& descriptions)
{
	mOptions.push_back( Option{ optionName, descriptions, { } });
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandDefinition::AddOption(const tbCore::tbString& optionName, const tbCore::tbString& parameterName,
	const ParameterType& parameterType, const std::vector<tbCore::tbString>& descriptions)
{
	mOptions.push_back( Option{ optionName, descriptions, { { parameterName, parameterType } } });
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandDefinition::AddParameterToOption(const tbCore::tbString& optionName,
	const tbCore::tbString& parameterName, const ParameterType& parameterType)
{
	for (auto& option : mOptions)
	{
		if (option.mName == optionName)
		{
			option.mParameters.push_back({ parameterName, parameterType });
			return;
		}
	}

	tb_error("Option \"%s\" was not found in the CommandDefinition \"%s\".", optionName.c_str(), mCommandName.c_str());
}

//--------------------------------------------------------------------------------------------------------------------//

// void TurtleBrains::Development::CommandDefinition::AddOption(const tbCore::tbString& optionName, const ParameterType& parameterType,
// 	const std::vector<tbCore::tbString>& descriptions)
// {
// 	mOptions.push_back( Option{ optionName, descriptions, { parameterType } });
// }

//--------------------------------------------------------------------------------------------------------------------//

// void TurtleBrains::Development::CommandDefinition::AddOption(const tbCore::tbString& optionName, const std::vector<ParameterType>& parameterTypes,
// 	const std::vector<tbCore::tbString>& descriptions)
// {
// 	mOptions.push_back( Option{ optionName, descriptions, parameterTypes });
// }

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TurtleBrains::Development::Command::Command(void) :
	mParameters{},
	mOptions{}
{
}

//--------------------------------------------------------------------------------------------------------------------//

TurtleBrains::Development::Command::~Command(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

//This could probably go in a TurtleBrains helper somewhere...
//template<typename Type> bool DoesContainerHave(std::iterator begin, std::iterator end, const Type& item)
// template<typename Type> bool DoesContainerHave(const std::vector<Type>& container, const Type& item)
// {
// 	for (const auto& i : container)
// 	{
// 		if (i == item)
// 		{
// 			return true;
// 		}
// 	}
// 	return false;
// }

//--------------------------------------------------------------------------------------------------------------------//

bool TurtleBrains::Development::Command::HasOption(const tbCore::tbString& optionName) const
{
	return (mOptions.end() == mOptions.find(optionName)) ? false : true;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::DynamicStructure TurtleBrains::Development::Command::FindOptionParameter(const tbCore::tbString& optionName, const tbCore::tbString& parameterName) const
{
	const auto& optionIterator = mOptions.find(optionName);
	if (mOptions.end() != optionIterator)
	{
		for (const auto& parameter : optionIterator->second)
		{
			if (parameter.mParameterName == parameterName)
			{
				return parameter.mParameterValue;
			}
		}
	}

	return tbCore::DynamicStructure::kNullValue;
}

//--------------------------------------------------------------------------------------------------------------------//

bool TurtleBrains::Development::Command::GetOptionAsBoolean(const tbCore::tbString& optionName, const tbCore::tbString& parameterName) const
{
	const tbCore::DynamicStructure& parameterValue = FindOptionParameter(optionName, parameterName);
	return (true == parameterValue.IsNil()) ? false : parameterValue.AsBoolean();
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TurtleBrains::Development::Command::GetOptionAsString(const tbCore::tbString& optionName, const tbCore::tbString& parameterName) const
{
	const tbCore::DynamicStructure& parameterValue = FindOptionParameter(optionName, parameterName);
	return (true == parameterValue.IsNil()) ? "" : parameterValue.AsString();
}

//--------------------------------------------------------------------------------------------------------------------//

int TurtleBrains::Development::Command::GetOptionAsInteger(const tbCore::tbString& optionName, const tbCore::tbString& parameterName) const
{
	const tbCore::DynamicStructure& parameterValue = FindOptionParameter(optionName, parameterName);
	return (true == parameterValue.IsNil()) ? 0 : parameterValue.AsRangedInteger<int>();

}

//--------------------------------------------------------------------------------------------------------------------//

float TurtleBrains::Development::Command::GetOptionAsFloat(const tbCore::tbString& optionName, const tbCore::tbString& parameterName) const
{
	const tbCore::DynamicStructure& parameterValue = FindOptionParameter(optionName, parameterName);
	return (true == parameterValue.IsNil()) ? 0.0f : parameterValue.AsFloat();
}

//--------------------------------------------------------------------------------------------------------------------//

size_t TurtleBrains::Development::Command::GetParameterCount(void) const
{
	return mParameters.size();
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TurtleBrains::Development::Command::GetParameter(size_t parameterIndex) const
{
	return mParameters[parameterIndex].AsString();
}

//--------------------------------------------------------------------------------------------------------------------//

// bool TurtleBrains::Development::Command::HasParameter(const tbCore::tbString& parameterName) const
// {
// 	return (mParameters.end() == mParameters.find(parameterName)) ? false : true;
// }

//--------------------------------------------------------------------------------------------------------------------//

// bool TurtleBrains::Development::Command::GetParameterAsBoolean(const tbCore::tbString& parameterName) const
// {
// 	const auto& parameterIterator = mParameters.find(parameterName);
// 	if (parameterIterator != mParameters.end())
// 	{
// 		return parameterIterator->second.AsBoolean();
// 	}

// 	return false;
// }

//--------------------------------------------------------------------------------------------------------------------//

// tbCore::tbString TurtleBrains::Development::Command::GetParameterAsString(const tbCore::tbString& parameterName) const
// {
// 	const auto& parameterIterator = mParameters.find(parameterName);
// 	if (parameterIterator != mParameters.end())
// 	{
// 		return parameterIterator->second.AsString();
// 	}

// 	return "";
// }

//--------------------------------------------------------------------------------------------------------------------//

// int TurtleBrains::Development::Command::GetParameterAsInteger(const tbCore::tbString& parameterName) const
// {
// 	const auto& parameterIterator = mParameters.find(parameterName);
// 	if (parameterIterator != mParameters.end())
// 	{
// 		return parameterIterator->second.AsInteger();
// 	}

// 	return 0;
// }

//--------------------------------------------------------------------------------------------------------------------//

// float TurtleBrains::Development::Command::GetParameterAsFloat(const tbCore::tbString& parameterName) const
// {
// 	const auto& parameterIterator = mParameters.find(parameterName);
// 	if (parameterIterator != mParameters.end())
// 	{
// 		return parameterIterator->second.AsFloat();
// 	}

// 	return 0.0f;
// }


//--------------------------------------------------------------------------------------------------------------------//

std::vector<tbCore::tbString> SplitStringBy(const tbCore::tbString& inputString, const tbCore::tbString& delimiter)
{
	std::vector<tbCore::tbString> tokens;

	tbCore::tbString brokenString = inputString;
	size_t spot = tbCore::tbString::npos;
	do
	{
		 spot = brokenString.find_first_of(delimiter);
		 tbCore::tbString splitString = brokenString.substr(0, spot);
		 if (false == splitString.empty())
		 {
		 	tokens.push_back(splitString);
		 }

		 if (brokenString.size() > spot + delimiter.size())
		 {
		 	brokenString = brokenString.substr(spot + delimiter.size());
		 }
		 else
		 {
			brokenString = "";
		 }

	} while (tbCore::tbString::npos != spot);

	return tokens;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

class SplitStringByTest : tbCore::UnitTest::TestCaseInterface
{
public:
	SplitStringByTest(void) :
		tbCore::UnitTest::TestCaseInterface("SplitStringByTest")
	{
		mTests.Push({ "hello world", " " }, { "hello", "world" });
		mTests.Push({ "hello  world", " " }, { "hello", "world" });
		mTests.Push({ " hello world", " " }, { "hello", "world" });
		mTests.Push({ "hello world ", " " }, { "hello", "world" });
		mTests.Push({ "    hello  world   ", " " }, { "hello", "world" });
		mTests.Push({ "hello world where are you", " " }, { "hello", "world", "where", "are", "you" });
		mTests.Push({ "", " " }, { });
		mTests.Push({ "      ", " " }, { });
	}

	~SplitStringByTest(void)
	{
	}

protected:
	virtual bool OnRunTest(void) override
	{
		size_t testIndex = 0;
		while (false == mTests.IsComplete())
		{
			const auto& expected = mTests.Result();
			const auto& results = SplitStringBy(mTests.Data().first, mTests.Data().second);

			bool testWentAsExpected = true;
			ExpectedValue(results.size(), expected.size(), "Expected number of results to match.");
			for (size_t index = 0; index < results.size(); ++index)
			{
				if (false == ExpectedValue(results[index], expected[index], "Expected result[%d] contents to match, results(\"%s\") != expected(\"%s\").",
					index, results[index].c_str(), expected[index].c_str()))
				{
					testWentAsExpected = false;
				}
			}

			if (false == testWentAsExpected)
			{
				tb_log("%d RESULTS: ", testIndex);
				for (const auto& str : results) { tb_log("\"%s\" ", str.c_str()); }
				tb_log("\n%d EXPECTD: ", testIndex);
				for (const auto& str : expected) { tb_log("\"%s\" ", str.c_str()); }
				tb_log("\n");
			}

			mTests.Next();
			++testIndex;
		}

		return true;
	}

private:
	tbCore::UnitTest::TestProvider<std::pair<tbCore::tbString, tbCore::tbString>, std::vector<tbCore::tbString>> mTests;
};

SplitStringByTest theSplitStringTest;

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

tbCore::DynamicStructure ParameterValueFromToken(const tbCore::tbString& parameterToken, tbDevelopment::ParameterType parameterType)
{
	switch (parameterType)
	{
	case tbDevelopment::ParameterType::String: return tbCore::DynamicStructure(parameterToken);
	case tbDevelopment::ParameterType::Integer: return tbCore::DynamicStructure(tbCore::FromString<int>(parameterToken));
	case tbDevelopment::ParameterType::Float: return tbCore::DynamicStructure(tbCore::FromString<float>(parameterToken));
	case tbDevelopment::ParameterType::Boolean: return tbCore::DynamicStructure(tbCore::FromString<bool>(parameterToken));
	};

	return tbCore::DynamicStructure::kNullValue;
}

//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TurtleBrains::Development::CommandManager::CommandManager(void)
{
	// CommandDefinition help("help", "Display helpful information about the commands available.");
	// mCommandDefinitions.push_back(help);


	// CommandDefinition perform("perform", {
	// 	"Make an entity behave the way you desire, or show the possible behaviors.",
	// 	"",
	// 	"Some more information about performing"
	// });

	// perform.AddSynopsis("<entity_id> <behavior_name>");
	// perform.AddSynopsis("<entity_name> <behavior_name>");
	// perform.AddSynopsis("--list");

	// perform.AddParameter("entity_id", ParameterType::String);
	// perform.AddParameter("entity_name", ParameterType::String);
	// perform.AddParameter("behavior", ParameterType::String);

	// perform.AddOption("list", {
	// 	"Display a list of behaviors that are available to set."
	// });

	// perform.AddOption("id", "entity_id", ParameterType::String, std::vector<tbCore::tbString>{
	// 	"When used with --list this will filter the behaviors to list only those allowed by the entity with id."
	// });

	// perform.AddOption("type", "entity_type", ParameterType::String, std::vector<tbCore::tbString>{
	// 	"When used with --list this will filter the behaviors to list only those allowed by the entity type specified."
	// });

	// mCommandDefinitions.push_back(perform);
}

//--------------------------------------------------------------------------------------------------------------------//

TurtleBrains::Development::CommandManager::~CommandManager(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandManager::ExecuteCommand(const tbCore::tbString& commandEntry)
{
	const auto tokens = SplitStringBy(commandEntry, " ");
	if (tokens.empty())
	{
		tbDevelopment::AddLog("There was no command to run.");
		return;
	}

	std::queue<tbCore::tbString> tokenQueue;
	for (size_t tokenIndex = 0; tokenIndex < tokens.size(); ++tokenIndex)
	{
		tokenQueue.push(tokens[tokenIndex]);
	}

	const tbCore::tbString commandName(tokenQueue.front());
	tokenQueue.pop();

	bool foundIssueWithEntry = false;
	bool foundCommandDefinition = false;
	for (CommandDefinition* commandDefinition : mCommandDefinitions)
	{
		CommandDefinition& definition = *commandDefinition;
		if (definition.mCommandName == commandName)
		{
			foundCommandDefinition = true;

			Command command;
			command.mOriginalEntry = commandEntry;
			while (false == tokenQueue.empty())
			{
				const tbCore::tbString token = tokenQueue.front();
				tokenQueue.pop();

				if (tbCore::StringStartsWith(token, "--"))
				{
					const tbCore::tbString optionName = token.substr(2);

					bool foundOptionDefinition = false;
					for (const CommandDefinition::Option& optionDefinition : definition.mOptions)
					{
						if (optionDefinition.mName == optionName)
						{
							auto& optionParameters = command.mOptions[optionName];
							for (const auto& parameter : optionDefinition.mParameters)
							{
								if (tokenQueue.empty())
								{
									tbDevelopment::AddLog("Syntax of command incorrect, not enough parameters.");
									return;
								}

								const tbCore::tbString parameterToken = tokenQueue.front();
								tokenQueue.pop();

								optionParameters.push_back({ parameter.mName, ParameterValueFromToken(parameterToken, parameter.mType) });
							}

							foundOptionDefinition = true;
						}
					}

					if (false == foundOptionDefinition)
					{
						tbDevelopment::AddLog("Unknown option '%s' for command '%s'.", token.c_str(), commandName.c_str());
						tbDevelopment::AddLog("    Use 'help %s' for additional information.", commandName.c_str());
						foundIssueWithEntry = true;
						break;
					}
				}
				else
				{
					//NOTE: Since we can't determine WHICH of the parameters this is by name, since a command could have various call-forms
					//   then the parameter tokens are always stored via string, and kept in the order they arrive for position access.
					command.mParameters.push_back(ParameterValueFromToken(token, tbDevelopment::ParameterType::String));
				}
			}

			if (false == foundIssueWithEntry)
			{
				if (true == DoesCommandFitSynopsis(command, definition))
				{
					definition.OnRunCommand(command);
				}
				else
				{
					tbDevelopment::AddLog("Incorrect calling format for command '%s'.", commandName.c_str());
					tbDevelopment::AddLog("    use 'help %s' for additional information.", commandName.c_str());
				}
			}

			break;
		}
	}

	if (false == foundCommandDefinition)
	{
		tbDevelopment::AddLog("Command '%s' was not found.", commandName.c_str());
		tbDevelopment::AddLog("    use 'help' for a list of available commands.");
		tbDevelopment::AddLog("    or 'help <command>' for additional information.");
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void LogHelpInformation(const tbCore::tbString& headerTitle, const std::vector<tbCore::tbString>& informationLines)
{
	tbDevelopment::AddLog("");
	tbDevelopment::AddLog(headerTitle);
	tbDevelopment::AddLog("");
	for (const auto& line : informationLines)
	{
		tbDevelopment::AddLog(kSpacing + line);
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandManager::DisplayHelp(const tbCore::tbString& commandName) const
{
	if (true == commandName.empty())
	{
		tbDevelopment::AddLog("The following commands are available:");
		for (const auto& command : mCommandDefinitions)
		{
			tbDevelopment::AddLog("%s", command->mCommandName.c_str());
		}
	}
	else
	{
		bool foundCommand = false;
		for (const auto& commandPointer : mCommandDefinitions)
		{
			const auto& command = *commandPointer;
			if (command.mCommandName == commandName)
			{
				LogHelpInformation("SYNOPSIS", { });
				if (true == command.mSynopsis.empty())
				{
					tbDevelopment::AddLog(kSpacing + command.mCommandName);
				}
				else
				{
					for (const auto& line : command.mSynopsis)
					{
						tbDevelopment::AddLog(kSpacing + command.mCommandName + " " + line);
					}
				}

				LogHelpInformation("DESCRIPTION", command.mDescriptions);

				if (false == command.mOptions.empty())
				{
					LogHelpInformation("OPTIONS", { });
					for (const auto& option : command.mOptions)
					{
						tbDevelopment::AddLog(kSpacing + "--" + option.mName);
						for (const auto& line : option.mDescriptions)
						{
							tbDevelopment::AddLog(kSpacing + kSpacing + line);
						}
					}
				}

				tbDevelopment::AddLog("");

				foundCommand = true;
				break;
			}
		}

		if (false == foundCommand)
		{
			tbDevelopment::AddLog("Command '%s' was not found.", commandName.c_str());
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandManager::AddCommandDefinition(CommandDefinition* definition)
{
	mCommandDefinitions.push_back(definition);
}

//--------------------------------------------------------------------------------------------------------------------//

void TurtleBrains::Development::CommandManager::RemoveCommandDefinition(CommandDefinition* definition)
{
	mCommandDefinitions.remove(definition);
}

//--------------------------------------------------------------------------------------------------------------------//

bool TurtleBrains::Development::CommandManager::DoesCommandFitSynopsis(const Command& command, const CommandDefinition& commandDefinition) const
{
	std::vector<tbCore::tbString> requiredOptions;
	std::vector<tbCore::tbString> requiredParameters;
	for (const tbCore::tbString& synopsis : commandDefinition.mSynopsis)
	{
		const auto synopsisTokens = SplitStringBy(synopsis, " ");
		requiredOptions.clear();
		requiredParameters.clear();

		for (const auto& synopsisToken : synopsisTokens)
		{
			if (true == tbCore::StringStartsWith(synopsisToken, "--"))
			{
				requiredOptions.push_back(synopsisToken.substr(2));
			}
			else if (true == tbCore::StringStartsWith(synopsisToken, "<"))
			{
				requiredParameters.push_back(synopsisToken.substr(1, synopsisToken.size() - 2));
			}
		}

		bool allRequiredOptions = true;
		for (const auto& option : requiredOptions)
		{
			if (false == command.HasOption(option))
			{
				allRequiredOptions = false;
			}
		}

		if (true == allRequiredOptions && command.GetParameterCount() >= requiredParameters.size())
		{
			return true;
		}
	}

	if (true == commandDefinition.mSynopsis.empty())
	{
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------//

