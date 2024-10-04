///
/// @file
/// @details Provides a simple way to load up and hold the triangles of a mesh for collision purposes.
/// @history Some of this code was started in 2018 for RallyOfRockets.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "utilities.hpp"

#include <turtle_brains/core/debug/tb_debug_logger.hpp>

#include <fstream>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <cctype>
#include <algorithm>

//--------------------------------------------------------------------------------------------------------------------//

std::vector<unsigned char> TyreBytes::Core::Utilities::LoadBinaryFileContents(const tbCore::tbString& filePath)
{
	std::vector<unsigned char> data;

	std::ifstream inputFile(filePath, std::ios::binary);
	if (true == inputFile.is_open())
	{
		inputFile.seekg(0, std::ios::end);
		data.resize(static_cast<size_t>(inputFile.tellg()));
		inputFile.seekg(0, std::ios::beg);

		ReadBinary(data.data(), data.size(), inputFile);
		inputFile.close();
		return data;
	}

	return data;
}

//--------------------------------------------------------------------------------------------------------------------//

tbCore::tbString TyreBytes::Core::Utilities::LoadFileContentsToString(const tbCore::tbString& filePath, bool trimTrailingWhitespace)
{
	std::ifstream inputFile(filePath);
	if (true == inputFile.is_open())
	{
		tbCore::tbString contents;

		inputFile.seekg(0, std::ios::end);
		contents.reserve(static_cast<size_t>(inputFile.tellg()));
		inputFile.seekg(0, std::ios::beg);

		contents.assign((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());

		if (true == trimTrailingWhitespace)
		{
			tbCore::String::TrimTrailingWhitespaceInPlace(contents);
		}

		inputFile.close();
		return contents;
	}

	return "";
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Utilities::SaveStringContentToFile(const tbCore::tbString& filePath, const tbCore::tbString& stringContents)
{
	std::ofstream outputFile(filePath);
	if (true == outputFile.is_open())
	{
		outputFile << stringContents;
		outputFile.close();
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Utilities::DateTime TyreBytes::Core::Utilities::DateTime::TimeNow(void)
{
	//auto now = std::chrono::system_clock::now();
	//auto itt = std::chrono::system_clock::to_time_t(now);
	//std::ostringstream ss;
	//ss << std::put_time(gmtime(&itt), "%FT%TZ");
	//return DateTime(ss.str());

	time_t t = std::time(nullptr);
	std::tm* tm = gmtime(&t);

	DateTime now;
	now.mYear = tm->tm_year + 1900;
	now.mMonth = tm->tm_mon + 1;
	now.mDay = tm->tm_mday;
	now.mHour = tm->tm_hour;
	now.mMinute = tm->tm_min;
	now.mSecond = tm->tm_sec;
	return now;
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Utilities::DateTime TyreBytes::Core::Utilities::DateTime::StartOfDay(const DateTime& dateTime)
{
	DateTime dayStart = dateTime;
	dayStart.mHour = 0;
	dayStart.mMinute = 0;
	dayStart.mSecond = 0;
	return dayStart;
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Utilities::DateTime TyreBytes::Core::Utilities::DateTime::EndOfDay(const DateTime& dateTime)
{
	DateTime dayEnd= dateTime;
	dayEnd.mHour = 23;
	dayEnd.mMinute = 59;
	dayEnd.mSecond = 59;
	return dayEnd;
}


//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Utilities::DateTime::DateTime(void)
{
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Utilities::DateTime::DateTime(const tbCore::tbString& iso8601)
{	//"%Y-%m-%dT%H:%M:%SZ%z"
	sscanf(iso8601.c_str(), "%d-%d-%dT%d:%d:%dZ", &mYear, &mMonth, &mDay, &mHour, &mMinute, &mSecond);
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Utilities::DateTime::operator<(const DateTime& other) const
{
	if (mYear > other.mYear) { return false; }
	if (mYear < other.mYear) { return true; }

	if (mMonth > other.mMonth) { return false; }
	if (mMonth < other.mMonth) { return true; }

	if (mDay > other.mDay) { return false; }
	if (mDay < other.mDay) { return true; }

	if (mHour > other.mHour) { return false; }
	if (mHour < other.mHour) { return true; }

	if (mMinute > other.mMinute) { return false; }
	if (mMinute < other.mMinute) { return true; }

	return (mSecond < other.mSecond);
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Utilities::DateTime::operator<=(const DateTime& other) const
{
	return (*this < other || *this == other);
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Utilities::DateTime::operator>(const DateTime& other) const
{
	return (other < *this);
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Utilities::DateTime::operator>=(const DateTime& other) const
{
	return (other < *this || *this == other);
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Utilities::DateTime::operator==(const DateTime& other) const
{
	return (mYear == other.mYear && mMonth == other.mMonth && mDay == other.mDay &&
		mHour == other.mHour && mMinute == other.mMinute && mSecond == other.mSecond);
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Utilities::DateTime::operator!=(const DateTime& other) const
{
	return false == (*this == other);
}

//--------------------------------------------------------------------------------------------------------------------//

bool TyreBytes::Core::Utilities::DateTime::IsValid(void) const
{
	return (0 != mYear || 0 != mMonth || 0 != mDay || 0 != mHour || 0 != mMinute || 0 != mSecond);
}

//--------------------------------------------------------------------------------------------------------------------//

TyreBytes::Core::Utilities::DateTime::operator bool(void) const
{
	return IsValid();
}

//--------------------------------------------------------------------------------------------------------------------//

//This could be a function to remove content between two tokens, including the tokens themselves. Could be useful for
//stripping out comments "//" to "\n" or even "/*" to "*/"...  Might be useful:

//const tbCore::tbString header = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\n\r\n";
//tbCore::tbString modifiedData = header + Core::Utilities::LoadFileContentsToString("data/web_overlay/stream_overlay.html");
//std::vector<std::pair<std::string, std::string>> tagsToRemove = {
//	{ "<!--non-stream-start-->" , "<!--non-stream-final-->" },
//	{ "/*non-stream-start*/" , "/*non-stream-final*/" },
//};
//
//for (const auto& tagToRemove : tagsToRemove)
//{
//	size_t startIndex = modifiedData.find(tagToRemove.first);
//	size_t finalIndex = modifiedData.find(tagToRemove.second);
//	while (startIndex != std::string::npos && finalIndex != std::string::npos)
//	{
//		modifiedData = modifiedData.substr(0, startIndex) + modifiedData.substr(finalIndex + tagToRemove.second.size());
//		startIndex = modifiedData.find(tagToRemove.first);
//		finalIndex = modifiedData.find(tagToRemove.second);
//	}
//}
//
//SendData(modifiedData);

//--------------------------------------------------------------------------------------------------------------------//
