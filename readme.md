
# Trophy Brawlers

This is a template project for any future "multiplayer ready" racing game using TurtleBrains and ICE.

Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved

## To Build This Project

To get this project building you will need premake5 and it will need to be added to PATH.

This project depends on [TurtleBrains](http://tyrebytes.com:3000/TyreBytes/turtle_brains), [Internal Combusion Engine](http://tyrebytes.com:3000/TyreBytes/ice) and [Track Builder](http://tyrebytes.com:3000/TyreBytes/track_builder) among perhaps dependencies included in the project repository. For setup it is recommended to keep the project and all engine dependencies in a single folder. _Note: This doesn't mean put engine dependencies in the Trophy Brawlers project folder, but rather a structure like:_

	tyre_bytes/
		turtle_brains/
		ice/
		track_builder/
		ludumdare56/

This structure should have things pretty much "JUST WORKING" otherwise you'll need to modify the `ludumdare56/build/ludumdare56.lua` premake file to point to the correct locations in your specific setup.

	git clone timgitea@tyrebytes.com:TyreBytes/turtle_brains.git
	git clone timgitea@tyrebytes.com:TyreBytes/ice.git
	git clone timgitea@tyrebytes.com:TyreBytes/track_builder.git
	git clone timgitea@tyrebytes.com:TyreBytes/soapbox_racer.git ludumdare56

### Windows
1. From `ludumdare56/build` directory run `make_project.bat` to create a Visual Studio 2022 project inside `build/windows`
2. Open Visual Studio 2022 and build the project.

### macOS
Note: Unlike Windows, building the engine dependencies is a manual step until I solve the Premake dependencies issue with XCode in a similar fashion to Visual Studio solutions having multiple projects and setting build order based on `depends { }`. This means any time the engine is updated/modified you'll need to build each Turtle Brains, Internal Combusion Engine and Track Builder (in that order) before building LudumDare56. Also not sure yet if I could create XCode builds from script to just have `make_turtle_brains.sh` work like it does on Linux.
1. for each (first Turtle Brains, then Internal Combusion Engine, then Track Builder)
  1. Run the `/build/make_project.sh` to create the XCode project inside `build/macos`
  2. Then Open the XCode project and build it for debug, release and public, 🤞 everything goes to plan.
2. From `ludumdare56/build` directory run `make_project.sh` to create an XCode project inside `build/macos`
3. Open the XCode project and build the project.

### Linux
Note: Unlike Windows, building the engine dependencies is a manual step until I solve the Premake dependencies issue with GCC in a similar fashion to Visual Studio solutions having multiple projects and setting build order based on `depends { }`.  This means any time the engine is updated/modified you'll need to build each Turtle Brains, Internal Combusion Engine and Track Builder (in that order) before building LudumDare56.
1. for each (first Turtle Brains, then Internal Combusion Engine, then Track Builder)
2. Run `turtle_brains/build/make_turtle_brains.sh` _(will build debug, release and public)_
3. Run `ice/build/make_ice.sh` _(will build debug, release and public)_
4. Run `track_builder/build/make_project.sh --linux --debug`
5. Run `track_builder/build/make_project.sh --linux --release`
6. Run `track_builder/build/make_project.sh --linux --public`

1. From the `stream_helper/build` directory run `make_project.sh --linux`

Credits
open_art_grass.png by "LuminousDragonGames"
