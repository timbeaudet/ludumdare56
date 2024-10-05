///
/// @file
/// @details Defines the CustomComponents for the LudumDare56 game.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_CustomComponents_hpp
#define LudumDare56_CustomComponents_hpp

#include <track_bundler/track_bundler.hpp>

namespace LudumDare56
{

	typedef TrackBundler::ComponentDefinitionKey ComponentDefinitionKey;

	namespace ComponentDefinition
	{
		extern const ComponentDefinitionKey kTrackInformationKey;
		extern const ComponentDefinitionKey kSpawnPointKey;

		extern const ComponentDefinitionKey kZoneFinishKey;
		extern const ComponentDefinitionKey kZoneForbiddenKey;

	};

}; /* namespace LudumDare56 */

#endif /* LudumDare56_CustomComponents_hpp */
