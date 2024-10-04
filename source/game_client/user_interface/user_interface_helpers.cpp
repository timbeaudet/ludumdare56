///
/// @file
/// @details Provide some interface helper functions for creating, well, interfaces!
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#include "user_interface_helpers.hpp"

//--------------------------------------------------------------------------------------------------------------------//

tbMath::Vector2 LudumDare56::GameClient::UserInterface::GetAnchorPositionOfScreen(const tbGraphics::AnchorLocation& anchor,
	const tbMath::Vector2& offset)
{
	switch (anchor)
	{
	case tbGraphics::kAnchorTopLeft:      return offset;
	case tbGraphics::kAnchorTopCenter:    return offset + tbMath::Vector2(0.5f * tbGraphics::ScreenWidth(), 0.0f * tbGraphics::ScreenHeight());
	case tbGraphics::kAnchorTopRight:     return offset + tbMath::Vector2(1.0f * tbGraphics::ScreenWidth(), 0.0f * tbGraphics::ScreenHeight());
	case tbGraphics::kAnchorCenterLeft:   return offset + tbMath::Vector2(0.0f * tbGraphics::ScreenWidth(), 0.5f * tbGraphics::ScreenHeight());
	case tbGraphics::kAnchorCenter:       return offset + tbMath::Vector2(0.5f * tbGraphics::ScreenWidth(), 0.5f * tbGraphics::ScreenHeight());
	case tbGraphics::kAnchorCenterRight:  return offset + tbMath::Vector2(1.0f * tbGraphics::ScreenWidth(), 0.5f * tbGraphics::ScreenHeight());
	case tbGraphics::kAnchorBottomLeft:   return offset + tbMath::Vector2(0.0f * tbGraphics::ScreenWidth(), 1.0f * tbGraphics::ScreenHeight());
	case tbGraphics::kAnchorBottomCenter: return offset + tbMath::Vector2(0.5f * tbGraphics::ScreenWidth(), 1.0f * tbGraphics::ScreenHeight());
	case tbGraphics::kAnchorBottomRight:  return offset + tbMath::Vector2(1.0f * tbGraphics::ScreenWidth(), 1.0f * tbGraphics::ScreenHeight());
	};

	return offset;
}

//--------------------------------------------------------------------------------------------------------------------//

tbMath::Vector2 LudumDare56::GameClient::UserInterface::GetAnchorPositionOfInterface(const tbGraphics::AnchorLocation& anchor,
	const float offsetX, const float offsetY)
{
	return GetAnchorPositionOfInterface(anchor, tbMath::Vector2(offsetX, offsetY));
}

//--------------------------------------------------------------------------------------------------------------------//

tbMath::Vector2 LudumDare56::GameClient::UserInterface::GetAnchorPositionOfInterface(const tbGraphics::AnchorLocation& anchor, const tbMath::Vector2& offset)
{
	const float interfaceWidth = tbMath::Minimum<float>(tbGraphics::ScreenWidth(), tbGraphics::ScreenHeight() * InterfaceAspectRatio());
	const float interfaceHeight = tbGraphics::ScreenHeight();
	const tbMath::Vector2 interfacePosition((tbGraphics::ScreenWidth() - interfaceWidth) / 2.0f, 0.0f);

	switch (anchor)
	{
	case tbGraphics::kAnchorTopLeft:      return interfacePosition + offset;
	case tbGraphics::kAnchorTopCenter:    return interfacePosition + offset + tbMath::Vector2(0.5f * interfaceWidth, 0.0f * interfaceHeight);
	case tbGraphics::kAnchorTopRight:     return interfacePosition + offset + tbMath::Vector2(1.0f * interfaceWidth, 0.0f * interfaceHeight);
	case tbGraphics::kAnchorCenterLeft:   return interfacePosition + offset + tbMath::Vector2(0.0f * interfaceWidth, 0.5f * interfaceHeight);
	case tbGraphics::kAnchorCenter:       return interfacePosition + offset + tbMath::Vector2(0.5f * interfaceWidth, 0.5f * interfaceHeight);
	case tbGraphics::kAnchorCenterRight:  return interfacePosition + offset + tbMath::Vector2(1.0f * interfaceWidth, 0.5f * interfaceHeight);
	case tbGraphics::kAnchorBottomLeft:   return interfacePosition + offset + tbMath::Vector2(0.0f * interfaceWidth, 1.0f * interfaceHeight);
	case tbGraphics::kAnchorBottomCenter: return interfacePosition + offset + tbMath::Vector2(0.5f * interfaceWidth, 1.0f * interfaceHeight);
	case tbGraphics::kAnchorBottomRight:  return interfacePosition + offset + tbMath::Vector2(1.0f * interfaceWidth, 1.0f * interfaceHeight);
	};

	return offset;
}

//--------------------------------------------------------------------------------------------------------------------//

tbMath::Vector2 LudumDare56::GameClient::UserInterface::GetAnchorPositionOf(const tbGraphics::Graphic& graphic,
	const tbGraphics::AnchorLocation& anchor, const float offsetX, const float offsetY)
{
	return GetAnchorPositionOf(graphic, anchor, tbMath::Vector2(offsetX, offsetY));
}

//--------------------------------------------------------------------------------------------------------------------//

tbMath::Vector2 LudumDare56::GameClient::UserInterface::GetAnchorPositionOf(const tbGraphics::Graphic& graphic,
	const tbGraphics::AnchorLocation& anchor, const tbMath::Vector2& offset)
{
	const tbMath::Vector2 scaledOrigin(graphic.GetOrigin().x * graphic.GetScale().x, graphic.GetOrigin().y * graphic.GetScale().y);
	const tbMath::Vector2 topLeft = graphic.GetPosition() - scaledOrigin;
	const float graphicWidth(graphic.GetScaledWidth());
	const float graphicHeight(graphic.GetScaledHeight());

	switch (anchor)
	{
	case tbGraphics::kAnchorTopLeft:      return offset + topLeft;
	case tbGraphics::kAnchorTopCenter:    return offset + topLeft + tbMath::Vector2(0.5f * graphicWidth, 0.0f * graphicHeight);
	case tbGraphics::kAnchorTopRight:     return offset + topLeft + tbMath::Vector2(1.0f * graphicWidth, 0.0f * graphicHeight);
	case tbGraphics::kAnchorCenterLeft:   return offset + topLeft + tbMath::Vector2(0.0f * graphicWidth, 0.5f * graphicHeight);
	case tbGraphics::kAnchorCenter:       return offset + topLeft + tbMath::Vector2(0.5f * graphicWidth, 0.5f * graphicHeight);
	case tbGraphics::kAnchorCenterRight:  return offset + topLeft + tbMath::Vector2(1.0f * graphicWidth, 0.5f * graphicHeight);
	case tbGraphics::kAnchorBottomLeft:   return offset + topLeft + tbMath::Vector2(0.0f * graphicWidth, 1.0f * graphicHeight);
	case tbGraphics::kAnchorBottomCenter: return offset + topLeft + tbMath::Vector2(0.5f * graphicWidth, 1.0f * graphicHeight);
	case tbGraphics::kAnchorBottomRight:  return offset + topLeft + tbMath::Vector2(1.0f * graphicWidth, 1.0f * graphicHeight);
	};

	return offset;
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameClient::UserInterface::InterfaceAspectRatio(void)
{
	//return UserSettings::GetSettings().mInterfaceAspectRatio;
	return TargetWidth() / TargetHeight();
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameClient::UserInterface::InterfaceScale(void)
{
	return VerticalScale();
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameClient::UserInterface::HorizontalScale(void)
{
	return static_cast<float>(tbGraphics::ScreenWidth()) / 1920.0f;
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameClient::UserInterface::VerticalScale(void)
{
	return static_cast<float>(tbGraphics::ScreenHeight()) / 1080.0f;
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameClient::UserInterface::MixedScale(const float mixedScale)
{
	return (((1.0f - mixedScale) * HorizontalScale()) + (mixedScale * VerticalScale()));
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameClient::UserInterface::TargetWidth(void)
{
	return tbGraphics::ScreenWidth();
}

//--------------------------------------------------------------------------------------------------------------------//

float LudumDare56::GameClient::UserInterface::TargetHeight(void)
{
	return tbGraphics::ScreenHeight();
}

//--------------------------------------------------------------------------------------------------------------------//
