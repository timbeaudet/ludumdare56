--
-- Premake5 script for creating a Visual Studio, XCode or CodeLite workspace for the project.
--   Requires Premake5 from: http://industriousone.com/
--
-- <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
-----------------------------------------------------------------------------------------------------------------------

newoption {
	trigger = "web",
	description = "Chosen build system to override for web.",
	value = "",
}

newoption {
	trigger = "build-version",
	description = "Version being built, expecting major.minor.patch; --build-version=1.2.3",
	value = "0.0.0",
}

BUILD_VERSION = "0.0.0"

local file = io.open("version.txt", "r")
if (nil ~= file) then
	BUILD_VERSION = file:read("*line")
	file:close()
end

if _OPTIONS["build-version"] ~= nil and _OPTIONS["build-version"] ~= '' then
	BUILD_VERSION = _OPTIONS["build-version"]
end

BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH = string.match(BUILD_VERSION, "(%d+).(%d+).(%d+)")
print("LudumDare56 Version: " .. BUILD_VERSION_MAJOR .. "." .. BUILD_VERSION_MINOR .. "." .. BUILD_VERSION_PATCH)

local WINDOWS_SYSTEM_NAME = "windows"
local LINUX_SYSTEM_NAME = "linux"
local MACOS_SYSTEM_NAME = "macos"
local WEB_SYSTEM_NAME = "web"
--local MACIOS_SYSTEM_NAME = "macios"
--local ANDROID_SYSTEM_NAME = "android"

local PROJECT_NAME = "ludumdare56"
local SYSTEM_NAME = os.target():gsub("macosx", "macos")
if _OPTIONS["web"] then
	SYSTEM_NAME = WEB_SYSTEM_NAME
end

if _ACTION == "clean" then
	os.rmdir("../build/" .. WINDOWS_SYSTEM_NAME)
	os.rmdir("../build/" .. LINUX_SYSTEM_NAME)
	os.rmdir("../build/" .. MACOS_SYSTEM_NAME)
	os.rmdir("../build/" .. WEB_SYSTEM_NAME)
end

local TB_PLATFORM_DEFINE = "tb_" .. SYSTEM_NAME
local SCRIPT_EXTENSION = ".sh"

local TYRE_BYTES_DIRECTORY = "C:/development/tyre_bytes/"
local TURTLE_BRAINS_DIRECTORY = TYRE_BYTES_DIRECTORY .. "turtle_brains/"
if os.isdir("externals/turtle_brains") then
	TURTLE_BRAINS_DIRECTORY = "externals/turtle_brains/"
end

local INTERNAL_COMBUSTION_DIRECTORY = TYRE_BYTES_DIRECTORY .. "ice/"
if os.isdir("externals/ice") then
	INTERNAL_COMBUSTION_DIRECTORY = "externals/ice/"
end

local TRACK_BUILDER_DIRECTORY = TYRE_BYTES_DIRECTORY .. "track_builder/"
if os.isdir("externals/track_builder") then
	TRACK_BUILDER_DIRECTORY = "externals/track_builder/"
end

------------------------------------------------------------------------------------------------------------------------

workspace (PROJECT_NAME)
configurations { "debug", "development", "release", "public" }
startproject "ludumdare56"

if (WINDOWS_SYSTEM_NAME == SYSTEM_NAME) then
	SCRIPT_EXTENSION = ".bat"
	architecture "x86_64"
end

--------------------------------------------------------------------------------- Common Project Settings (ALL projects)
location ("../build/" .. SYSTEM_NAME)
language ("C++")
cppdialect "C++20"
runtime "Release" -- 2024-08-05: Always use the static release /MT C runtime libs
warnings ("Extra")
flags { "FatalCompileWarnings" }
debugdir "../run"

defines {
	TB_PLATFORM_DEFINE,
	"tb_%{cfg.buildcfg}_build",
	"build_major=" .. BUILD_VERSION_MAJOR,
	"build_minor=" .. BUILD_VERSION_MINOR,
	"build_patch=" .. BUILD_VERSION_PATCH
}

includedirs {
	"../source/",
	"../external_libraries/includes/",
	TURTLE_BRAINS_DIRECTORY .. "source/",
	TURTLE_BRAINS_DIRECTORY .. "external_libraries/includes/",
	INTERNAL_COMBUSTION_DIRECTORY .. "source/",
	TRACK_BUILDER_DIRECTORY .. "includes/"
}

libdirs {
	"../external_libraries/libraries/" .. SYSTEM_NAME .. "/",
	-- 2024-01-04: Keeping around, this was used when injecting TurtleBrains/ICE for a nice workflow back when source
	--   control was SVN and allowed checking out/grabbing a specific sub-directory of the repository.
	--"../build/tb_external_libraries/libraries/" .. SYSTEM_NAME .. "/",
	TURTLE_BRAINS_DIRECTORY .. "libraries/" .. SYSTEM_NAME .. "/",
	TURTLE_BRAINS_DIRECTORY .. "external_libraries/libraries/" .. SYSTEM_NAME .. "/",
	INTERNAL_COMBUSTION_DIRECTORY .. "libraries/" .. SYSTEM_NAME .. "/",
	TRACK_BUILDER_DIRECTORY .. "libraries/" .. SYSTEM_NAME .. "/"
}

filter "configurations:debug"
	targetdir ("../build/" .. SYSTEM_NAME .. "/debug")
	objdir ("../build/" .. SYSTEM_NAME .. "/debug/objects")
	defines { "tb_with_internal_tests", "DEBUG" }
	symbols ("On")
filter "configurations:development"
	targetdir ("../build/" .. SYSTEM_NAME .. "/development")
	objdir ("../build/" .. SYSTEM_NAME .. "/development/objects")
	defines { "tb_without_splash_screen", "tb_with_debug_set", "tb_with_internal_tests", "NDEBUG" }
	symbols ("On")
	optimize ("On")
filter "configurations:release"
	targetdir ("../build/" .. SYSTEM_NAME .. "/release")
	objdir ("../build/" .. SYSTEM_NAME .. "/release/objects")
	defines { "tb_without_splash_screen", "tb_with_debug_set", "tb_with_internal_tests", "NDEBUG" }
	symbols ("On")
	optimize ("On")
filter "configurations:public"
	targetdir ("../build/" .. SYSTEM_NAME .. "/public")
	objdir ("../build/" .. SYSTEM_NAME .. "/public/objects")
	defines { "tb_without_debug_set", "NDEBUG" }
	symbols ("Off")
	optimize ("On")
filter {}

if (WINDOWS_SYSTEM_NAME == SYSTEM_NAME) then --------------------------------- Windows Platform Specifics (ALL projects)
	defines { "_CRT_SECURE_NO_WARNINGS", "_WINSOCK_DEPRECATED_NO_WARNINGS" }
	staticruntime "On"
	toolset "v143"
	characterset ("MBCS")
	buildoptions "/MP4"

	-- 2024-08-06: Removed this as I'm not sure it is strictly necessary for TrackBuilder, but if it is we should add
	--   the directory for all Windows projects to search out.
	-- libdirs {
	--  "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.14393.0/um/x86/", --for dxguid
	-- 	"C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x86/" --dxguid.lib
	-- }

elseif (MACOS_SYSTEM_NAME == SYSTEM_NAME) then --------------------------------- macOS Platform Specifics (ALL projects)
	externalincludedirs {
		"../external_libraries/includes/",
		TURTLE_BRAINS_DIRECTORY .. "source/",
		TURTLE_BRAINS_DIRECTORY .. "external_libraries/includes/",
		INTERNAL_COMBUSTION_DIRECTORY .. "source/",
		TRACK_BUILDER_DIRECTORY .. "includes/"
	}
elseif (LINUX_SYSTEM_NAME == SYSTEM_NAME) then --------------------------------- Linux Platform Specifics (ALL projects)
	includedirs {
		"/usr/includes/GL/"
	}
	libdirs {
		"../external_libraries/libraries/gmake/x64",
		"/opt/lib/"
	}
elseif (WEB_SYSTEM_NAME == SYSTEM_NAME) then ------------------------ Web (Emscripten) Platform Specifics (ALL projects)
	-- 2024-08-06: May need define tb_without_networking again, but waiting until we make a web_build.
	defines { "tb_without_legacy_gl", "tb_without_threading" }
	linkoptions "-stdlib=libc++"
end

------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------
--     SPECIFIC PROJECTS BELOW!   SPECIFIC PROJECTS BELOW!    SPECIFIC PROJECTS BELOW!   SPECIFIC PROJECTS BELOW!     --
------------------------------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------- ludumdare56
group "game"
project (PROJECT_NAME)
	kind ("WindowedApp")
	defines { "development_build", }
	dependson { "turtle_brains", "ice", "track_bundler" }
	debugargs { "--developer", "--other" }

	-- 2024-08-02: Removed TrackBuilder from dependencies to speed up the build process when possible. To avoid the trap
	--   of directly editing 'the real' TrackBuilder from project solution and not seeing edits occur when using the
	--   TrackBuilder application, it makes sense to keep the dependency when not using externals/track_builder.
	if not os.isdir("externals/track_builder") then
		dependson { "track_builder" }
	end

	files {
		"../source/**.cpp",
		"../source/**.hpp",
		"../source/**.c",
		"../source/**.h"
	}
	excludes { "../**/doxygen/**" }

	if (WINDOWS_SYSTEM_NAME == SYSTEM_NAME) then
		entrypoint "mainCRTStartup"
		--links { "OpenGL32", "OpenAL32", "glew32", "libcurl", "crypt32", "ws2_32", "zlib" }
	elseif (MACOS_SYSTEM_NAME == SYSTEM_NAME) then
		files {
			"../source/**.mm",
			"./macos_info.plist",
			--"./macos_app.entitlements"
		}
		links { "AppKit.framework", "IOKit.framework", "OpenGL.framework", "OpenAL.framework", "GLEW", "curl" }
		xcodebuildsettings {
			-- paths here are  relative to the generated XCode project file.
			["INFOPLIST_FILE"] = "../../build/macos_info.plist",
			--["CODE_SIGN_ENTITLEMENTS"] = ("../../build/macos_app.entitlements"),
		}
	elseif (LINUX_SYSTEM_NAME == SYSTEM_NAME) then
		linkoptions { "-Wl,--start-group" }
		links { "GL", "glew", "openal", "X11", "pthread", "curl", "tls", "ssl", "crypto" }
	end

	links { "turtle_brains_%{cfg.buildcfg}", "ice_%{cfg.buildcfg}", "track_bundler_%{cfg.buildcfg}" }

	if (WEB_SYSTEM_NAME ~= SYSTEM_NAME) then
		postbuildcommands { "../scripts/post_build" .. SCRIPT_EXTENSION .. " --build-config %{cfg.buildcfg} --platform " .. TB_PLATFORM_DEFINE .. " --name %{prj.name}" }
	end

------------------------------------------------------------------------------------------------- ludumdare56_server
project (PROJECT_NAME .. "_server")
	kind ("WindowedApp")
	dependson { "turtle_brains_headless", "ice_headless", "track_bundler_headless" }
	defines {
		"ludumdare56_headless_build",
		--"tb_without_input_devices", 2024-08-06: I don't think this does anything here? either turtle_brains_headless
		--   has or doesn't have input devices... right?
	}

	files {
		"../source/**.cpp",
		"../source/**.hpp",
		"../source/**.c",
		"../source/**.h"
	}
	excludes { "../**/doxygen/**", "../**/turtle_brains/graphics/**", "../**/turtle_brains/game/**", "../**/turtle_brains/audio/**",
		"../**/turtle_brains/tests/**", "../**/turtle_brains/express/**", "../**/turtle_brains/application/**",
		"../**/turtle_brains/**kit.hpp", "../**/ice/**", "../**source/game_client/**" }

	if (WINDOWS_SYSTEM_NAME == SYSTEM_NAME) then
		entrypoint "mainCRTStartup"
		--links { "libcurl", "crypt32", "ws2_32", "zlib" }
	elseif (MACOS_SYSTEM_NAME == SYSTEM_NAME) then
		files {
			"../source/**.mm",
			"./macos_info.plist",
			--"./macos_app.entitlements"
		}
		links { "AppKit.framework", "IOKit.framework", "OpenGL.framework", "OpenAL.framework", "GLEW", "curl" }
		xcodebuildsettings {
			-- More info: https://premake.github.io/docs/Embedding-Frameworks-in-Xcode/
			-- paths here are  relative to the generated XCode project file.
			["INFOPLIST_FILE"] = "../../build/macos_info.plist",
			--["CODE_SIGN_ENTITLEMENTS"] = ("../../build/macos_app.entitlements"),
		}
	elseif (LINUX_SYSTEM_NAME == SYSTEM_NAME) then
		linkoptions { "-Wl,--start-group" }
		links { "GL", "glew", "openal", "X11", "pthread", "curl", "tls", "ssl", "crypto" }
	end

	links { "turtle_brains_headless_%{cfg.buildcfg}", "ice_headless_%{cfg.buildcfg}", "track_bundler_headless_%{cfg.buildcfg}" }

	if (WEB_SYSTEM_NAME ~= SYSTEM_NAME) then
		postbuildcommands { "../scripts/post_build" .. SCRIPT_EXTENSION .. " --build-config %{cfg.buildcfg} --platform " .. TB_PLATFORM_DEFINE .. " --name %{prj.name}" }
	end

----------------------------------------------------------------------------- Visual Studio Solution or XCode Workspaces
if (WINDOWS_SYSTEM_NAME == SYSTEM_NAME or MACOS_SYSTEM_NAME == SYSTEM_NAME) then
	group "the_engine"
		externalproject "turtle_brains"
			location( TURTLE_BRAINS_DIRECTORY .. "build/" .. SYSTEM_NAME .. "/" )
			uuid "57940020-8E99-AEB6-271F-61E0F7F6B73B"
			kind "StaticLib"
			language "C++"

		externalproject "turtle_brains_headless"
			location( TURTLE_BRAINS_DIRECTORY .. "build/" .. SYSTEM_NAME .. "/" )
			uuid "57940020-8E99-AEB6-272F-61E0F7F6B73B"
			kind "StaticLib"
			language "C++"

		externalproject "ice"
			location( INTERNAL_COMBUSTION_DIRECTORY .. "build/" .. SYSTEM_NAME .. "/" )
			uuid "57940020-8E99-AEB6-271F-61E0F7F6B731"
			kind "StaticLib"
			language "C++"
			dependson { "turtle_brains" }

		externalproject "ice_headless"
			location( INTERNAL_COMBUSTION_DIRECTORY .. "build/" .. SYSTEM_NAME .. "/" )
			uuid "57940022-8E19-AEB6-271F-61E0F7F6B731"
			kind "StaticLib"
			language "C++"
			dependson { "turtle_brains_headless" }

		externalproject "track_builder"
			location( TRACK_BUILDER_DIRECTORY .. "build/" .. SYSTEM_NAME .. "/" )
			uuid "57940020-8E99-AEB6-271F-61E0F7F6B73D"
			kind "StaticLib"
			language "C++"
			dependson { "turtle_brains", "ice" }

		externalproject "track_bundler"
			location( TRACK_BUILDER_DIRECTORY .. "build/" .. SYSTEM_NAME .. "/" )
			uuid "57940020-8E99-AEB6-271F-61E0F7F6B73F"
			kind "StaticLib"
			language "C++"
			dependson { "turtle_brains", "ice" }

		externalproject "track_bundler_headless"
			location( TRACK_BUILDER_DIRECTORY .. "build/" .. SYSTEM_NAME .. "/" )
			uuid "57940021-8E99-AEB6-271F-61E0F7F6B73F"
			kind "StaticLib"
			language "C++"
			dependson { "turtle_brains_headless", "ice_headless" }
end
