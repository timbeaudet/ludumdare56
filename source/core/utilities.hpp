///
/// @file
/// @details Provides a simple way to load up and hold the triangles of a mesh for collision purposes.
/// @history Some of this code was started in 2018 for RallyOfRockets.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef Core_Utilities_hpp
#define Core_Utilities_hpp

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_string.hpp>
#include <turtle_brains/core/tb_dynamic_structure.hpp>

#include <fstream>
#include <vector>

namespace TyreBytes
{
	namespace Core
	{
		namespace Utilities
		{

			template <typename Type> inline void WriteBinary(const Type& object, std::ofstream& outputFile)
			{
				outputFile.write(reinterpret_cast<const char*>(&object), sizeof(Type));
			}

			inline void WriteBinary(const void* object, size_t size, std::ofstream& outputFile)
			{
				outputFile.write(reinterpret_cast<const char*>(object), size);
			}

			template <typename Type> inline void ReadBinary(Type& object, std::ifstream& inputFile)
			{
				inputFile.read(reinterpret_cast<char*>(&object), sizeof(Type));
			}

			inline void ReadBinary(void* object, size_t size, std::ifstream& inputFile)
			{
				inputFile.read(reinterpret_cast<char*>(object), size);
			}

			template <typename Type> inline Type ReadBinary(std::ifstream& inputFile)
			{
				Type object;
				ReadBinary(object, inputFile);
				return object;
			}

			std::vector<unsigned char> LoadBinaryFileContents(const tbCore::tbString& filePath);

			tbCore::tbString LoadFileContentsToString(const tbCore::tbString& filePath, bool trimTrailingWhitespace = false);
			bool SaveStringContentToFile(const tbCore::tbString& filePath, const tbCore::tbString& stringContents);

			class DateTime
			{
			public:
				DateTime(void);
				explicit DateTime(const tbCore::tbString& iso8601);
				bool operator<(const DateTime& other) const;
				bool operator<=(const DateTime& other) const;
				bool operator>(const DateTime& other) const;
				bool operator>=(const DateTime& other) const;
				bool operator==(const DateTime& other) const;
				bool operator!=(const DateTime& other) const;

				bool IsValid(void) const;
				operator bool(void) const;

				static DateTime TimeNow(void);
				static DateTime StartOfDay(const DateTime& dateTime);
				static DateTime EndOfDay(const DateTime& dateTime);

			private:
				int mYear = 0;
				int mMonth = 0;
				int mDay = 0;
				int mHour = 0;
				int mMinute = 0;
				int mSecond = 0;
			};

		};	//namespace Utilities
	};	//namespace Core
};	//namespace TyreBytes

#endif /* Core_Utilities_hpp */
