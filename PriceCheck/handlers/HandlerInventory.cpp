#include "pch.h"
#include "HandlerInventory.h"

#include "gui/Fonts.h"
#include "gui/GUITools.h"
#include "wrappers/ProductsWrapper.h"

bool HandlerInventory::isVisibleIn(ERocketMenu menu) const
{
	return (menu == ERocketMenu::GarageInventory);
}

float HandlerInventory::getShowDelay() const
{
	return 0.5f;
}

void HandlerInventory::onRegister(GameWrapper* gw)
{
	LOG("WINDOW REGISTER");
}

void HandlerInventory::onUnregister(GameWrapper* gw)
{
	LOG("WINDOW UNREGISTER");
}

void HandlerInventory::onEnable(GameWrapper* gw)
{
	LOG("WINDOW ENABLE");

	ptrScroll = ProductsWrapper::getInventoryScrollOffsetPtr();
	gameSize = gw->GetScreenSize();
	uiScale = gw->GetDisplayScale();

	fontTitle = Fonts::getInstance().GetFont("RLHeadI");
	fontText = Fonts::getInstance().GetFont("default");
}

void HandlerInventory::render(GameWrapper* gw)
{
	LOG("WINDOW DRAW {}", *ptrScroll);

	ImGui::PushFont(fontText);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.25f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	// IM_COL32(5, 10, 19, 255)
	// IM_COL32(21, 50, 85, 255)
	// IM_COL32(41, 68, 118, 255)
	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(36, 62, 110, 80));
	ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(99, 121, 157, 80));

	const float sizeBase = min((float)gameSize.X * 0.47f, (float)gameSize.Y * 0.73f) * uiScale;

	const ImVec2 itemListPos = { sizeBase * 0.0325f, sizeBase * 0.065f };
	const ImVec2 itemListSize = { sizeBase * 1.255f, sizeBase * 1.0f };

	const ImVec2 itemSlotPanelPos = { itemListPos.x + sizeBase * 0.01521f, itemListPos.y + sizeBase * 0.203422f };
	const ImVec2 itemSlotPanelSize = { sizeBase * 1.2015209f, sizeBase * 0.77947f };
	
	const ImVec2 itemSlotAllottedSize = { itemSlotPanelSize.x / 7.f, itemSlotPanelSize.y / 4.f };
	const ImVec2 itemSlotSize = { sizeBase * 0.1653992f, sizeBase * 0.18821293f };

	for (uint8_t y = 0; y < 4; ++y)
	{
		for (uint8_t x = 0; x < 7; ++x)
		{
			const uint32_t idx = (y * 7) + x;

			const ImVec2 nextPos = {
				itemSlotPanelPos.x + static_cast<float>(x) * itemSlotAllottedSize.x,
				itemSlotPanelPos.y + static_cast<float>(y) * itemSlotAllottedSize.y
			};
			const ImVec2 nextSize = itemSlotSize;

			ImGui::SetNextWindowPos(nextPos);
			ImGui::SetNextWindowSize(nextSize);
			
			if (ImGui::Begin(fmt::format("itemslot_{}", idx).c_str(), nullptr,
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
			{
				const uint32_t itemSlot = idx + *ptrScroll;

				ImGui::Text("price %u", itemSlot);
			}

			//GUITools::BoxShadow(ImGui::GetWindowPos(), ImGui::GetWindowSize(), 1.1f, true);

			ImGui::End();
		}
	}


	ImGui::PopFont();
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(2);
}

void HandlerInventory::onDisable(GameWrapper* gw)
{
	LOG("WINDOW DISABLE");

	ptrScroll = nullptr;
	gameSize = { 0, 0 };
	uiScale = 1.0f;

	fontTitle = nullptr;
	fontText = nullptr;
}
