///
/// @file
/// @details Store the identity information of a driver to give name/license id etc.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_DriverLicense_hpp
#define LudumDare56_DriverLicense_hpp

#include <turtle_brains/core/tb_string.hpp>

namespace LudumDare56
{
	namespace GameState
	{

		struct DriverLicense
		{
			explicit DriverLicense(const tbCore::tbString& identifier = "", const tbCore::tbString& name = "", const bool isModerator = false) :
				mIdentifier(identifier),
				mName(name),
				mIsModerator(isModerator)
			{
			}

			tbCore::tbString mIdentifier = ""; //UniqueIdentifier for the driver, aka license in many places.
			tbCore::tbString mName = "";
			bool mIsModerator = false;

			static DriverLicense Invalid(void) { return DriverLicense(); }
		};

	};	//namespace GameState
};	//namespace LudumDare56

#endif /* LudumDare56_DriverLicense_hpp */
