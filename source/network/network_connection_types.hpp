///
/// @file
/// @details StrongInteger typedefs for the Safe and Fast connections in LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_NetworkConnectionTypes_hpp
#define LudumDare56_NetworkConnectionTypes_hpp

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_typed_integer.hpp>

namespace LudumDare56
{
	namespace Network
	{

		enum class SafeConnectionType : tbCore::uint8 { };
		typedef tbCore::TypedInteger<SafeConnectionType> SafeConnection;

		enum class FastConnectionType : tbCore::uint8 { };
		typedef tbCore::TypedInteger<FastConnectionType> FastConnection;

		enum class ConnectionType
		{
			Safe, Fast
		};

	};	//namespace Network
};	//namespace LudumDare56

//--------------------------------------------------------------------------------------------------------------------//

#endif /* LudumDare56_NetworkConnectionTypes_hpp */
