///
/// @file
/// @details Contains the basic state of an object at or on the racetrack in LudumDare56.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_ObjectState_hpp
#define LudumDare56_ObjectState_hpp

#include <track_bundler/track_bundler.hpp>

#include <turtle_brains/core/tb_node.hpp>
#include <turtle_brains/core/tb_typed_range.hpp>
#include <turtle_brains/core/tb_defines.hpp>

#include <ice/physics/ice_physical_world.hpp>
#include <ice/physics/ice_rigid_body.hpp>

#include <memory>

namespace LudumDare56::GameState
{

	class ObjectState;
	class ComponentState;

	typedef std::unique_ptr<ComponentState> ComponentStatePtr;
	typedef std::unique_ptr<ObjectState> ObjectStatePtr;

	enum class ComponentIndexType : tbCore::uint16 { };
	typedef tbCore::TypedInteger<ComponentIndexType> ComponentIndex;


	class ComponentCreatorInterface : tbCore::Noncopyable
	{
	public:
		ComponentCreatorInterface(void);
		virtual ~ComponentCreatorInterface(void);

	protected:
		virtual ComponentStatePtr OnCreateComponent(ObjectState& object, const TrackBundler::Component& componentInformation) = 0;

		const TrackBundler::Component& GetComponent(const TrackBundler::NodeKey& nodeKey, const TrackBundler::ComponentKey& componentKey);
		const TrackBundler::Component& GetComponentByType(const TrackBundler::NodeKey& nodeKey, const TrackBundler::ComponentDefinitionKey& definitionKey);

	private:
		friend class ComponentState;
		const TrackBundler::ImprovedTrackBundle* mTrackBundle;
	};

	class ComponentState : tbCore::Noncopyable
	{
	public:
		virtual ~ComponentState(void);

		virtual void OnAwake(void) { }
		virtual void OnDestroy(void) { }

		virtual void OnActivate(void) { }
		virtual void OnDeactivate(void) { }

		virtual void OnSimulate(void) { }
		virtual void OnUpdate(const float /*deltaTime*/) { }
		virtual void OnRender(void) const { }

		static ComponentStatePtr CreateComponent(ObjectState& object, const TrackBundler::Component& componentInformation,
			const TrackBundler::ImprovedTrackBundle& trackBundle);

		inline const ObjectState& GetObject(void) const { return mObject; }
		bool IsActive(void) const;
		inline bool IsActiveSelf(void) const { return mIsActive; }
		void SetActive(const bool isActive);

		inline const TrackBundler::ComponentDefinitionKey& GetDefinitionKey(void) const { return mDefinitionKey; }


	protected:
		ComponentState(ObjectState& object);

		ObjectState& mObject;
		TrackBundler::ComponentDefinitionKey mDefinitionKey;
		bool mIsActive;
	};

	class ObjectState : public tbCore::Node
	{
	public:
		ObjectState(const TrackBundler::Node& objectNode);
		virtual ~ObjectState(void);

		virtual void OnAwake(void) { }
		virtual void OnDestroy(void) { }

		virtual void OnActivate(void);
		virtual void OnDeactivate(void);

		virtual void OnSimulate(void) { }
		virtual void OnUpdate(const float /*deltaTime*/) { }

		void AddComponent(ComponentStatePtr&& component);

		inline ComponentIndex GetNumberOfComponents(void) const { return tbCore::RangedCast<ComponentIndex::Integer>(mComponents.size()); }
		inline const ComponentState& GetComponent(const ComponentIndex componentIndex) const { return *mComponents[componentIndex]; }
		inline ComponentState& GetComponent(const ComponentIndex componentIndex) { return *mComponents[componentIndex]; }

		template<typename ComponentType> ComponentType* GetComponent(void);

		template<typename ComponentType> std::vector<ComponentType*> GetComponents(void);

		///
		/// @note Searches depth-first, returning the first component of matching type.
		///
		template<typename ComponentType> ComponentType* GetComponentInChildren(void);

		///
		/// @note Dives in depth-first, returning all components in all children with the matching type.
		///
		template<typename ComponentType> std::vector<ComponentType*> GetComponentsInChildren(void);

		typedef tbCore::OtherTypedRange<ComponentIndex, const ComponentState&> ConstComponentContainerAccessor;
		typedef tbCore::OtherTypedRange<ComponentIndex, ComponentState&> ComponentContainerAccessor;
		inline ConstComponentContainerAccessor AllComponents(void) const {
			return ConstComponentContainerAccessor(std::bind(&ObjectState::GetNumberOfComponents, this),
				std::bind(static_cast<const ComponentState & (ObjectState::*)(const ComponentIndex) const>(&ObjectState::GetComponent), this, std::placeholders::_1));
		}

		inline ComponentContainerAccessor AllComponents(void) {
			return ComponentContainerAccessor(std::bind(&ObjectState::GetNumberOfComponents, this),
				std::bind(static_cast<ComponentState & (ObjectState::*)(const ComponentIndex)>(&ObjectState::GetComponent), this, std::placeholders::_1));
		}

	private:
		virtual void OnRender(void) const { }

		std::vector<ComponentStatePtr> mComponents;
	};

};	//namespace LudumDare56::GameState


//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------------------------------------------------------------------------//

template<typename ComponentType> ComponentType* LudumDare56::GameState::ObjectState::GetComponent(void)
{
	for (ComponentStatePtr& component : mComponents)
	{
		ComponentType* typedComponent = dynamic_cast<ComponentType>(component.get());
		if (nullptr != typedComponent)
		{
			return typedComponent;
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------//

template<typename ComponentType> std::vector<ComponentType*> LudumDare56::GameState::ObjectState::GetComponents(void)
{
	std::vector<ComponentType*> matchingComponents;
	for (ComponentStatePtr& component : mComponents)
	{
		ComponentType* typedComponent = dynamic_cast<ComponentType*>(component.get());
		if (nullptr != typedComponent)
		{
			matchingComponents.push_back(typedComponent);
		}
	}

	return matchingComponents;
}

//--------------------------------------------------------------------------------------------------------------------//

template<typename ComponentType> ComponentType* LudumDare56::GameState::ObjectState::GetComponentInChildren(void)
{
	for (ComponentStatePtr& component : mComponents)
	{
		ComponentType* typedComponent = dynamic_cast<ComponentType*>(component.get());
		if (nullptr != typedComponent)
		{
			return typedComponent;
		}
	}

	for (tbCore::Node& child : AllChildren())
	{
		if (false == child.IsActive())
		{
			continue;
		}

		ObjectState* childState = dynamic_cast<ObjectState*>(&child);
		tb_error_if(nullptr == childState, "Error: How did a non ObjectState get added to the root/node hierarchy?");
		ComponentType* childComponent = childState->GetComponentInChildren<ComponentType>();
		if (nullptr != childComponent)
		{
			return childComponent;
		}
	}

	return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------//

template<typename ComponentType> std::vector<ComponentType*> LudumDare56::GameState::ObjectState::GetComponentsInChildren(void)
{
	std::vector<ComponentType*> matchingComponents = GetComponents<ComponentType>();

	for (tbCore::Node& child : AllChildren())
	{
		if (false == child.IsActive())
		{
			continue;
		}

		ObjectState* childState = dynamic_cast<ObjectState*>(&child);
		tb_error_if(nullptr == childState, "Error: How did a non ObjectState get added to the root/node hierarchy?");
		std::vector<ComponentType*> childComponents = childState->GetComponentsInChildren<ComponentType>();
		matchingComponents.insert(matchingComponents.end(), childComponents.begin(), childComponents.end());
	}

	return matchingComponents;
}

//--------------------------------------------------------------------------------------------------------------------//

#endif /* LudumDare56_ObjectState_hpp */
