///
/// @file
/// @details A simple graphic to give the drivers some fans to watch them.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_SpectatorGraphic_hpp
#define LudumDare56_SpectatorGraphic_hpp

#include "../../ludumdare56.hpp"

#include <ice/graphics/ice_graphic.hpp>

namespace LudumDare56
{

	namespace GameClient
	{

		class SpectatorGraphic
		{
		public:
			static void SpawnSpectatorsAt(const Matrix4& bleacherToWorld);
			static void ClearAllSpectators(void);
			static void UpdateAllSpectators(const float deltaTime);

			explicit SpectatorGraphic(const Matrix4& spectatorToWorld);
			virtual ~SpectatorGraphic(void);

		private:
			iceGraphics::Graphic mSpectatorGraphic;
			tbMath::Vector3 mOriginalPosition;
			float mBobTimer;
		};

	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_SpectatorGraphic_hpp */
