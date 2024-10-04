///
/// @file
/// @details Creates a tabbed display area for user interface controls to exist and swap between tabs.
///
/// <!-- Copyright (c) 2023 Tyre Bytes LLC - All Rights Reserved -->
///------------------------------------------------------------------------------------------------------------------///

#ifndef LudumDare56_TabbedDisplayGraphic_hpp
#define LudumDare56_TabbedDisplayGraphic_hpp

#include "../../game_client/user_interface/base_control_graphic.hpp"
#include "../../game_client/user_interface/sprite_button_graphic.hpp"

#include <turtle_brains/graphics/tb_text.hpp>
#include <turtle_brains/graphics/tb_graphic_list.hpp>
#include <turtle_brains/express/graphics/tbx_nine_slice.hpp>

#include <vector>
#include <functional>
#include <memory>

namespace LudumDare56
{
	namespace GameClient
	{
		namespace UserInterface
		{

			class TabbedButtonGraphic : public SpriteButtonGraphic
			{
			public:
				TabbedButtonGraphic(const tbCore::tbString& tabName);
				virtual ~TabbedButtonGraphic(void);

				void SetOpened(bool opened);

				virtual void OnUpdate(const float deltaTime) override;
				virtual void OnRender(void) const override;

			private:
				tbxGraphics::NineSlice mOutlineGraphic;
				bool mIsOpened;
			};

			class TabbedDisplayGraphic : public BaseControlGraphic
			{
			public:
				explicit TabbedDisplayGraphic(void);
				virtual ~TabbedDisplayGraphic(void);

				void AddTab(const tbCore::tbString& tabName);
				void AddControlLine(BaseControlGraphic* graphicLeft);
				void AddControlLine(BaseControlGraphic* graphicLeft, BaseControlGraphic* graphicRight);

				virtual tbGraphics::PixelSpace GetPixelWidth(void) const override;
				virtual tbGraphics::PixelSpace GetPixelHeight(void) const override;

			protected:
				virtual void OnUpdate(const float deltaTime) override;
				virtual void OnRender(void) const override;

			private:
				tbxGraphics::NineSlice mBackdropGraphic;
				tbxGraphics::NineSlice mOutlineGraphic;

				struct Tab
				{
					Tab(const tbCore::tbString& name) :
						mTabButton(name),
						mControls(),
						mGraphics()
					{
					}

					TabbedButtonGraphic mTabButton;
					std::vector<BaseControlGraphic*> mControls;
					tbGraphics::GraphicList mGraphics;
				};

				typedef std::unique_ptr<Tab> TabPtr;
				std::vector<TabPtr> mTabs;

				size_t mSelectedTabIndex;
				int mLineCount;
			};

		};	//namespace UserInterface

		namespace ui = UserInterface;
	};	//namespace GameClient
};	//namespace LudumDare56

#endif /* LudumDare56_TabbedDisplayGraphic_hpp */
