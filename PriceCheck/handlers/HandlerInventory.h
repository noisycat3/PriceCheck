#pragma once

#include <aiw/GfxProductsWrapper.h>

#include "classes/MenuManager.h"
#include <bakkesmod/wrappers/GameWrapper.h>
#include <IMGUI/imgui.h>

#include "classes/PriceDatabase.h"

class HandlerInventory : public IMenuScreenHandler
{
public:
	[[nodiscard]] bool isVisibleIn(ERocketMenu menu) const override;
	[[nodiscard]] float getShowDelay() const override;

	void onRegister(GameWrapper* gw) override;
	void onEnable(GameWrapper* gw) override;
	void render(GameWrapper* gw) override;
	void onDisable(GameWrapper* gw) override;
	void onUnregister(GameWrapper* gw) override;

	void prepareIndexTable(ArrayWrapper<int32_t> hashArray);
	void resolveIndexTable(const aiw::GfxProductsWrapper& gfxProducts);

private:
	Vector2 gameSize = { 0, 0 };
	float uiScale = 1.0f;

	// Receive what we currently see in order from SetViewProducts parameter
	std::vector<int32_t> hashIdsSorted;

	// Produce sorted list of products
	struct ProductEntry
	{
		aiw::LoadingProductWrapper lp;
		OnlineProductWrapper op;
		SafePricePointer price;
	};
	std::vector<ProductEntry> productList;
};
