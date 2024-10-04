///
/// @file
/// @details Creates a developer tool command-line system to register and parse commands from the user.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef Undefined_CommandSystem_hpp
#define Undefined_CommandSystem_hpp

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_string.hpp>
#include <turtle_brains/core/tb_dynamic_structure.hpp>
#include <turtle_brains/core/tb_noncopyable.hpp>

#include <vector>
#include <list>
#include <unordered_map>

namespace tbImplementation { class CommandManagerCreator; }

namespace TurtleBrains
{
	namespace Development
	{
		class CommandManager;
		class Command;

		enum class ParameterType
		{
			Integer,
			String,
			Float,
			Boolean
		};

		class CommandDefinition
		{
		public:
			CommandDefinition(const tbCore::tbString& commandName, const tbCore::tbString& commandDescription);
			CommandDefinition(const tbCore::tbString& commandName, const std::vector<tbCore::tbString>& commandDescriptions);
			virtual ~CommandDefinition(void);

			void Enable(void);
			void Disable(void);

		protected:
			void AddSynopsis(const tbCore::tbString& synopsis);
			void AddSynopsis(const std::vector<tbCore::tbString>& synopsis);

			void AddParameter(const tbCore::tbString& parameterName, const ParameterType& parameterType);
			void AddOption(const tbCore::tbString& optionName, const std::vector<tbCore::tbString>& descriptions);
			void AddOption(const tbCore::tbString& optionName, const tbCore::tbString& parameterName,
				const ParameterType& parameterType, const std::vector<tbCore::tbString>& descriptions);
			//This expects the option to have been added first.
			void AddParameterToOption(const tbCore::tbString& optionName, const tbCore::tbString& parameterName, const ParameterType& parameterType);
			//void AddOption(const tbCore::tbString& optionName, const ParameterType& parameterType, const std::vector<tbCore::tbString>& descriptions);
			//void AddOption(const tbCore::tbString& optionName, const std::vector<ParameterType>& parameterTypes, const std::vector<tbCore::tbString>& descriptions);

			virtual void OnRunCommand(Command& command) = 0;

		private:
			friend class CommandManager;
			friend class Command;

			struct Parameter
			{
				tbCore::tbString mName;
				ParameterType mType;
			};

			struct Option
			{
				tbCore::tbString mName;
				std::vector<tbCore::tbString> mDescriptions;
				std::vector<Parameter> mParameters;
			};

			tbCore::tbString mCommandName;
			std::vector<tbCore::tbString> mDescriptions;
			std::vector<tbCore::tbString> mSynopsis;

			std::vector<Parameter> mParameters;
			std::vector<Option> mOptions;
			bool mIsEnabled;
		};

		class Command
		{
		public:
			Command(void);
			~Command();

			bool HasOption(const tbCore::tbString& optionName) const;
			bool GetOptionAsBoolean(const tbCore::tbString& optionName, const tbCore::tbString& parameterName) const;
			tbCore::tbString GetOptionAsString(const tbCore::tbString& optionName, const tbCore::tbString& parameterName) const;
			int GetOptionAsInteger(const tbCore::tbString& optionName, const tbCore::tbString& parameterName) const;
			float GetOptionAsFloat(const tbCore::tbString& optionName, const tbCore::tbString& parameterName) const;

			size_t GetParameterCount(void) const;
			tbCore::tbString GetParameter(size_t parameterIndex) const;

			// bool HasParameter(const tbCore::tbString& parameterName) const;
			// bool GetParameterAsBoolean(const tbCore::tbString& parameterName) const;
			// tbCore::tbString GetParameterAsString(const tbCore::tbString& parameterName) const;
			// int GetParameterAsInteger(const tbCore::tbString& parameter) const;
			// float GetParameterAsFloat(const tbCore::tbString& parameter) const;

			const tbCore::tbString& GetOriginalEntry(void) const { return mOriginalEntry; }

		private:
			friend class CommandManager;

			tbCore::DynamicStructure FindOptionParameter(const tbCore::tbString& optionName, const tbCore::tbString& parameterName) const;

			struct OptionParameter
			{
				tbCore::tbString mParameterName;
				tbCore::DynamicStructure mParameterValue;
			};

			std::vector<tbCore::DynamicStructure> mParameters;
			//std::unordered_map<tbCore::tbString, tbCore::DynamicStructure> mParameters;
			std::unordered_map<tbCore::tbString, std::vector<OptionParameter>> mOptions;
			tbCore::tbString mOriginalEntry;
		};

		class CommandManager : tbCore::Noncopyable
		{
		public:
			void ExecuteCommand(const tbCore::tbString& commandEntry);
			void DisplayHelp(const tbCore::tbString& commandName = "") const;

			void AddCommandDefinition(CommandDefinition* definition);
			void RemoveCommandDefinition(CommandDefinition* definition);

		private:
			bool DoesCommandFitSynopsis(const Command& command, const CommandDefinition& commandDefinition) const;

			CommandManager(void);
			~CommandManager(void);

			friend class tbImplementation::CommandManagerCreator;

			std::list<CommandDefinition*> mCommandDefinitions;
		};

		CommandManager& TheCommandManager(void);
		//extern CommandManager& theCommandManager;

	}; /* namespace Development */
}; /* namespace TurtleBrains */


namespace tbDevelopment = TurtleBrains::Development;

#endif /* Undefined_CommandSystem_hpp */
