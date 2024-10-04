///
/// @file
/// @details Provides base functionality for an element / control that the user can interact with.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_BaseControlGraphic_hpp
#define LudumDare56_BaseControlGraphic_hpp

#include <turtle_brains/graphics/tb_graphic.hpp>

#include <functional>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{

			class BaseControlGraphic : public tbGraphics::Graphic
			{
			public:
				explicit BaseControlGraphic(void);
				virtual ~BaseControlGraphic(void) = 0;

				///
				/// @details Sets a function to be called when the user interacts with the control in a meaningful manner.
				///   For a button/checkbox that might be when clicked, for a slider it might be when dragged, etc.
				///
				void SetCallback(std::function<void()> callbackFunction);

				///
				/// @details This annoying little function allows the SpriteButtonGraphic to be placed as a child of another
				///   Graphic, however it will start to fail as a grandchild... Good luck with that.
				///
				/// @note This would require TurtleBrains::Graphic::UnstableIsPointContained() to handle parents in some
				///   manner, which it doesn't quit have access.
				///
				void SetParentOffset(const tbMath::Vector2& parentOffset, const tbMath::Vector2& parentScale);

				inline bool IsEnabled(void) const { return mIsEnabled; }
				inline void SetEnabled(bool enabled) { mIsEnabled = enabled; }

				tbMath::Vector2 PointInParentSpace(const tbMath::Vector2& pointInGrandparentSpace) const;
				bool IsPointContained(const tbMath::Vector2& point) const;
				bool IsMouseContained(void) const;

			protected:
				std::function<void(void)> mCallback;

			private:
				tbMath::Vector2 mParentOffset;
				tbMath::Vector2 mParentScale;
				bool mIsEnabled;
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_BaseControlGraphic_hpp */
