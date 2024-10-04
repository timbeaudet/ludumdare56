///
/// @file
/// @details Provide some information on the version of LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_Version_hpp
#define LudumDare56_Version_hpp

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_string.hpp>

namespace LudumDare56
{
	namespace Version
	{

		constexpr tbCore::uint32 Major(void)
		{
#if defined(build_major)
			return tbCore::uint32(build_major);
#else
			return 0;
#endif
		}

		constexpr tbCore::uint32 Minor(void)
		{
#if defined(build_minor)
			return tbCore::uint32(build_minor);
#else
			return 0;
#endif
		}

		constexpr tbCore::uint32 Patch(void)
		{
#if defined(build_patch)
			return tbCore::uint32(build_patch);
#else
			return 0;
#endif
		}

		inline tbCore::tbString BuildTag(void)
		{
#if defined(tb_headless_build)
			const static tbCore::tbString headlessTag("-headless");
#else
			const static tbCore::tbString headlessTag("");
#endif

#if defined(tb_debug_build)
			return tb_string(headlessTag + "-debug");
#elif defined(tb_development_build)
			return tb_string(headlessTag + "-dev");
#elif defined(tb_release_build)
			return tb_string(headlessTag + "-rel");
#elif defined(tb_public_build)
			return tb_string(headlessTag + "");
#else
  #error Unknown build specificiers.
#endif
		}

		inline tbCore::tbString VersionString(void)
		{
			const tbCore::tbString separator(".");

			return tb_string(Major()) + separator + tb_string(Minor()) + separator + tb_string(Patch()) + BuildTag();
		}

		inline tbCore::tbString ProjectVersionString(void)
		{
			return tb_string("LudumDare56 v") + VersionString();
		}

	};	//namespace Version
};	//namespace LudumDare56

#endif /* LudumDare56_Version_hpp */
