///
/// @file
/// @details The EventBroadcaster sends an Event to all the EventListeners that are attached.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef TyreBytes_EventSystem_hpp
#define TyreBytes_EventSystem_hpp

#include <turtle_brains/core/tb_types.hpp>
#include <turtle_brains/core/tb_noncopyable.hpp>
#include <turtle_brains/core/tb_error.hpp>

#include <list>

namespace TyreBytes
{
	namespace Core
	{

		class Event
		{
		public:
			Event(tbCore::uint32 id);

			/// @note The events are requiring dynamic_cast<> and RTTI to work properly with the As() and TryAs() methods
			///   which require the virtual destructor to create a v-table. RTTI requires polymorphic information to
			///   get the type dynamically.
			virtual ~Event(void);

			tbCore::uint32 GetID(void) const;

			template <typename Type> const Type& As(void) const
			{
				const Type* castedObject = dynamic_cast<const Type*>(this);
				tb_error_if(nullptr == castedObject, "Error: Expected an object of this Type.");
				return *castedObject;
			}

			template <typename Type> const Type* TryAs(void) const
			{
				return dynamic_cast<const Type*>(this);
			}

		private:
			const tbCore::uint32 mID;
		};

		class EventListener
		{
		public:
			EventListener(void);
			virtual ~EventListener(void);

		protected:
			virtual void OnHandleEvent(const Event& event) = 0;

		private:
			friend class EventBroadcaster;
			void HandleEvent(const Event& event);
		};

		class EventBroadcaster : public tbCore::Noncopyable
		{
		public:
			EventBroadcaster(void);
			virtual ~EventBroadcaster(void);

			void AddEventListener(EventListener& eventListener);
			void RemoveEventListener(EventListener& eventListener);

			void SendEvent(const Event& event);

		private:
			std::list<EventListener*> mListeners;
		};

	};	//namespace Core
};	//namespace TyreBytes

#endif /* TyreBytes_EventSystem_hpp */
