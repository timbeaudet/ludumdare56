///
/// @file
/// @details A simple entity/object to automatically hide the mouse when not used.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_MouseHidingEntity_hpp
#define LudumDare56_MouseHidingEntity_hpp

#include <turtle_brains/game/tb_entity.hpp>

namespace LudumDare56
{
	namespace GameClient
	{

		class MouseHidingEntity : public tbGame::Entity
		{
		public:
			MouseHidingEntity(void);
			virtual ~MouseHidingEntity(void);

		protected:
			virtual void OnAdd(void) override;
			virtual void OnRemove(void) override;
			virtual void OnUpdate(const float deltaTime) override;

		private:
			float mAutoHideTimer;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_MouseHidingEntity_hpp */
