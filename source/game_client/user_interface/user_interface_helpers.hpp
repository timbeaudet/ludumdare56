///
/// @file
/// @details Provide some interface helper functions for creating, well, interfaces!
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_UserInterfaceHelpers_hpp
#define LudumDare56_UserInterfaceHelpers_hpp

#include <turtle_brains/express/graphics/tbx_shadowed_text.hpp>
#include <turtle_brains/graphics/tb_basic_shapes.hpp>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{
			///
			/// @details Finds the location of anchor point to the screen/window and adds the offset provided. It is
			///   unlikely this will be used much, prefer GetAnchorPositionOfInterface() for interface elements.
			///
			/// @note It is extremely likely you'll want to use GetAnchorPositionOfInterface() rather than of screen.
			///
			tbMath::Vector2 GetAnchorPositionOfScreen(const tbGraphics::AnchorLocation& anchor, const tbMath::Vector2& offset = tbMath::Vector2::Zero());

			///
			/// @details Finds the location of anchor point in screen space but using some magic interface bounds based
			///   on some user settings then adds the offset to it.
			///
			tbMath::Vector2 GetAnchorPositionOfInterface(const tbGraphics::AnchorLocation& anchor, const float offsetX, const float offsetY);
			tbMath::Vector2 GetAnchorPositionOfInterface(const tbGraphics::AnchorLocation& anchor, const tbMath::Vector2& offset = tbMath::Vector2::Zero());

			///
			/// @details Finds the location of an anchor point relative to another graphic object, returning the screen
			///   space position with the offset added.
			///
			/// @note This does not handle parented graphics as the graphic has no idea who the parents might be or where
			///   they are located.
			///
			tbMath::Vector2 GetAnchorPositionOf(const tbGraphics::Graphic& graphic, const tbGraphics::AnchorLocation& anchor,
				const float offsetX, const float offsetY);
			tbMath::Vector2 GetAnchorPositionOf(const tbGraphics::Graphic& graphic, const tbGraphics::AnchorLocation& anchor,
				const tbMath::Vector2& offset = tbMath::Vector2::Zero());

			///
			/// @details Returns the aspect ratio of the user interface based on user configurations.
			///
			float InterfaceAspectRatio(void);

			///
			/// @details Returns the scale of the interface which all graphic items should be scaled with.
			///
			float InterfaceScale(void);

			///
			/// @note It is extremely likely you'll want to use InterfaceScale() rather than horizontal, vertical or mixed.
			///
			float HorizontalScale(void);

			///
			/// @note It is extremely likely you'll want to use InterfaceScale() rather than horizontal, vertical or mixed.
			///
			float VerticalScale(void);

			///
			/// @param mixedScale is 0.0f for full horizontal scale, 1.0f for full vertical scale and in between to mix.
			///
			/// @note It is extremely likely you'll want to use InterfaceScale() rather than horizontal, vertical or mixed.
			///
			float MixedScale(const float mixedScale);

			float TargetWidth(void);
			float TargetHeight(void);


		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_UserInterfaceHelpers_hpp */
