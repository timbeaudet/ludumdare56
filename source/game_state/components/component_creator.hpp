///
/// @file
/// @details Implements the ComponentCreatorInterface to create the components for LudumDare56 state/logic.
///
/// <!-- Copyright (c) 2024 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_ComponentCreator_hpp
#define LudumDare56_ComponentCreator_hpp

#include "../../ludumdare56.hpp"
#include "../../game_state/object_state.hpp"

namespace LudumDare56::GameState
{

	class ComponentCreator : public LudumDare56::GameState::ComponentCreatorInterface
	{
	public:
		virtual ~ComponentCreator(void);
		virtual ComponentStatePtr OnCreateComponent(ObjectState& object, const TrackBundler::Component& componentInformation) override;
	};

};	//namespace LudumDare56::GameState

#endif /* LudumDare56_ComponentCreator_hpp */
