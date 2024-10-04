///
/// @file
/// @details A simple user interface graphic object for the user to control a sliding bar, like music volume slider.
///   This is NOT part of the TurtleBrains::ExpressKit interface in anyway, it is just a sprite object.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_SliderBarGraphic_hpp
#define LudumDare56_SliderBarGraphic_hpp

#include "../../game_client/user_interface/base_control_graphic.hpp"

#include <turtle_brains/graphics/tb_basic_shapes.hpp>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{

			class SliderBarGraphic : public BaseControlGraphic
			{
			public:
				explicit SliderBarGraphic(void);
				virtual ~SliderBarGraphic(void);

				float GetSliderPercentage(void) const;
				void SetSliderPercentage(const float percentage);

				virtual tbGraphics::PixelSpace GetPixelWidth(void) const override;
				virtual tbGraphics::PixelSpace GetPixelHeight(void) const override;

			protected:
				tbMath::Vector2 PositionFromTopLeft(const tbMath::Vector2& screenPoint) const;

				virtual void OnUpdate(const float deltaTime) override;
				virtual void OnRender(void) const override;

				tbGraphics::BoxShape mSliderTrackGraphic;
				tbGraphics::BoxShape mSliderHandleGraphic;

			private:
				float mSliderPercentage;
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_SliderBarGraphic_hpp */
