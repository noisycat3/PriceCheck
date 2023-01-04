#include "pch.h"
#include "HandlerInventory.h"

#include "gui/Fonts.h"

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

void HandlerInventory::prepareIndexTable(ArrayWrapper<int32_t> hashArray)
{
	// Rebuild the index table
	hashIdsSorted.clear();
	hashIdsSorted.reserve(hashArray.Count());

	const int32_t count = hashArray.Count();
	for (int32_t i = 0; i < count; ++i)
		hashIdsSorted.push_back(hashArray.Get(i));
}

void HandlerInventory::resolveIndexTable(const aiw::GfxProductsWrapper& gfxProducts)
{
	// Resolve indices with LoadingProducts
	std::unordered_map<int32_t, uintptr_t> hashIdToWrapper;

	for (int32_t i = 0; i < gfxProducts.GetNumLoadingProducts(); ++i)
	{
		aiw::LoadingProductWrapper product = gfxProducts.GetLoadingProduct(i);
		hashIdToWrapper.insert(std::make_pair(product.GetProductHashId(), product.memory_address));
	}

	productList.clear();
	productList.reserve(hashIdsSorted.size());

	for (const int32_t& hashId : hashIdsSorted)
	{
		auto found = hashIdToWrapper.find(hashId);
		if (found == hashIdToWrapper.end())
			continue;

		aiw::LoadingProductWrapper loadingProduct { found->second };
		OnlineProductWrapper onlineProduct = loadingProduct.GetOnlineProduct();

		ApiProductId id;
		id.productId = loadingProduct.GetProductId();
		if (!onlineProduct.IsNull())
		{
			for (ProductAttributeWrapper attribute : onlineProduct.GetAttributes())
			{
				switch (crc32(attribute.GetAttributeType()))
				{
				case crc32("ProductAttribute_Painted_TA"):
					id.paintId = ProductAttribute_PaintedWrapper(attribute.memory_address).GetPaintID();
					break;
				case crc32("ProductAttribute_SpecialEdition_TA"):
					//id.specialId = ProductAttribute_SpecialEditionWrapper(attribute.memory_address).GetEditionID();
					//break;
				case crc32("ProductAttribute_Certified_TA"):
					//id.certId = ProductAttribute_CertifiedWrapper(attribute.memory_address).GetStatId();
					break;
				case crc32("ProductAttribute_Quality_TA"):
				{
					// Safe to ignore, it means nothing I guess?
					uint8_t quality = ProductAttribute_QualityWrapper(attribute.memory_address).GetQuality();
					LOG("On `{}` quality {}", id.productId, quality);
					break;
				}
				default:
					LOG("Unknown attribute `{}`, ignoring", attribute.GetAttributeType().c_str());
					continue;
				}
			}
		}

		productList.emplace_back(ProductEntry{ loadingProduct, onlineProduct, SafePricePointer(id) });
	}
}

void HandlerInventory::onEnable(GameWrapper* gw)
{
	LOG("WINDOW ENABLE");
	
	gameSize = gw->GetScreenSize();
	uiScale = gw->GetDisplayScale();
}

void HandlerInventory::render(GameWrapper* gw)
{
	//LOG("WINDOW DRAW {}", *ptrScroll);

	ImGui::PushFont(USE_FONT("main"));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.25f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {1.0, 1.0});
	// IM_COL32(5, 10, 19, 255)
	// IM_COL32(21, 50, 85, 255)
	// IM_COL32(41, 68, 118, 255)

	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));

	//ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(36, 140, 110, 90));
	//ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(99, 121, 157, 80));

	const float sizeBase = min((float)gameSize.X * 0.47f, (float)gameSize.Y * 0.73f) * uiScale;

	const ImVec2 itemListPos = { sizeBase * 0.0325f, sizeBase * 0.065f };
	const ImVec2 itemListSize = { sizeBase * 1.255f, sizeBase * 1.0f };

	const ImVec2 itemSlotPanelPos = { itemListPos.x + sizeBase * 0.01521f, itemListPos.y + sizeBase * 0.203422f };
	const ImVec2 itemSlotPanelSize = { sizeBase * 1.2015209f, sizeBase * 0.78447f };
	
	const ImVec2 itemSlotAllottedSize = { itemSlotPanelSize.x / 7.f, itemSlotPanelSize.y / 4.f };
	const ImVec2 itemSlotSize = { sizeBase * 0.1653992f, sizeBase * 0.18821293f };

	constexpr float windowMargin = 5.f;

	ImGui::SetNextWindowPos({ itemSlotPanelPos.x - windowMargin, itemSlotPanelPos.y - windowMargin });
	ImGui::SetNextWindowSize({ itemSlotPanelSize.x + windowMargin * 2, itemSlotPanelSize.y + windowMargin * 2});

	constexpr ImGuiWindowFlags wndFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMouseInputs;

	if (ImGui::Begin("pricecheck_inventory", nullptr, wndFlags))
	{
		ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 0, 0, 120));
		//ImGui::PushStyleColor(ImGuiCol_ChildBg, 0xFFFFFFFF);
		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 200));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);

		ImGui::SetWindowFontScale(sizeBase / 600.f);
		for (uint8_t y = 0; y < 4; ++y)
		{
			for (uint8_t x = 0; x < 7; ++x)
			{
				const uint32_t idx = (y * 7) + x;

				const ImVec2 nextPos = {
					static_cast<float>(x) * itemSlotAllottedSize.x + windowMargin,
					static_cast<float>(y) * itemSlotAllottedSize.y + windowMargin
				};
				const ImVec2 nextSize = { itemSlotSize.x/* * 0.55f*/, 2.f * ImGui::GetTextLineHeight() + 2.f };

				if (productList.size() <= idx)
					continue;

				ImGui::SetCursorPos(nextPos);

				if (ImGui::BeginChild(fmt::format("wnd-{}", idx).c_str(), nextSize, false, wndFlags))
				{
					ProductEntry& p = productList[idx];

					const ApiProductPriceData& data = p.price.getPriceData();

					for (int i = 0; i < 2; i++)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, (i == 0) ? IM_COL32(255, 80, 80, 255) : IM_COL32(80, 255, 80, 255));

						ImGui::PushFont(USE_FONT("main-b"));
						ImGui::Text((i == 0) ? "B:" : "S:");
						ImGui::PopFont();

						ImGui::SameLine();
						if (data.resolved)
						{
							if (data.price[i] > 2000.0)
								ImGui::Text("%.2lfk (%d)", data.price[i] / 1000.0, data.vol[i]);
							else
								ImGui::Text("%.0lf (%d)", data.price[i], data.vol[i]);
						}
						else
							ImGui::Text("??");

						ImGui::PopStyleColor(1);
					}
				}
				ImGui::EndChild();

			}
		}

		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(1);
	}
	ImGui::End();


	ImGui::PopFont();
	ImGui::PopStyleVar(4);
	ImGui::PopStyleColor(2);
}

void HandlerInventory::onDisable(GameWrapper* gw)
{
	LOG("WINDOW DISABLE");

	gameSize = { 0, 0 };
	uiScale = 1.0f;
}
