///
/// @file
/// @details Provide constant values for the game interface stuffs. This started with Turbo Boom! and will be useful
///   in many projects to come.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_UserInterfaceConstants_hpp
#define LudumDare56_UserInterfaceConstants_hpp

#include <turtle_brains/math/tb_vector.hpp>
#include <turtle_brains/graphics/tb_color.hpp>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{
			namespace Padding
			{
				typedef float PixelValue;

				static const PixelValue ScreenEdge = PixelValue(40);
			};

			namespace Color
			{
				static const tbGraphics::Color White(0xFFF8F8F8);

				static const tbGraphics::Color TyreBytesBlue(0xFF2E9FFF);
				static const tbGraphics::Color TyreBytesPink(0xFFFF2E9F);
				static const tbGraphics::Color TyreBytesOrange(0xFFFF9F2E);

				static const tbGraphics::Color DarkBackdrop(0x80323232);

				static const tbGraphics::Color ControlActive(0xFFFF7675); //This added delayed flash state on some buttons, so kept same as Hovered.
				static const tbGraphics::Color ControlHovered(0xFFFF7675);
				static const tbGraphics::Color ControlEnabled(0xFFD63031);
				static const tbGraphics::Color ControlDisabled(0xFF746564); //Was once; 0xFFb4b4b4

				static const tbGraphics::Color Negative(0xFFFF7675);

				static const tbGraphics::Color PenaltyText(Negative);
				static const tbGraphics::Color DidNotFinishText(Negative);
				static const tbGraphics::Color DriverOnTrack(0xFF2E9FFF);
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_UserInterfaceConstants_hpp */
