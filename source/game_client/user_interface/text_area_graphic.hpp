///
/// @file
/// @details A multilined text area that is really not so smart, but should get a job done.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_TextBoxGraphic_hpp
#define LudumDare56_TextBoxGraphic_hpp

#include <turtle_brains/graphics/tb_text.hpp>
#include <turtle_brains/game/tb_game_timer.hpp>

#include <memory>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{

			class TextAreaGraphic : public tbGraphics::Graphic
			{
			public:
				explicit TextAreaGraphic(void);
				virtual ~TextAreaGraphic(void);

				void ClearText(void);
				void AddLineOfText(const tbGraphics::Text& text);

				virtual tbGraphics::PixelSpace GetPixelWidth(void) const { return mTextAreaWidth; }
				virtual tbGraphics::PixelSpace GetPixelHeight(void) const { return mTextAreaHeight; }

				void HideTextFor(const tbGame::GameTimer::Milliseconds& millseconds) { mHideTextTimer = millseconds; }
				void Simulate(void);

			protected:
				virtual void OnRender(void) const;

			private:
				//TODO: LudumDare56: Cleanup: We shouldn't need to use the unique_ptr here, but the tbGraphics::Text
				//  object does not support copy constructor/move symantics, perhaps it should even if it is a little
				//  costly? The idea was to prevent noobs/Tim from making silly mistakes that eat into performance. There
				//  is a refactoring that could happen in text implementation that would alleviate these concerns. 2022-07-10
				std::vector<std::unique_ptr<tbGraphics::Text>> mLinesOfText;
				tbGraphics::PixelSpace mTextAreaWidth;
				tbGraphics::PixelSpace mTextAreaHeight;
				tbGame::GameTimer mHideTextTimer;
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_TextBoxGraphic_hpp */
