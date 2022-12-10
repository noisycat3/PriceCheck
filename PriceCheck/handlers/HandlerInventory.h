#pragma once

#include "classes/MenuManager.h"
#include <bakkesmod/wrappers/GameWrapper.h>

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

private:
	int32_t* ptrScroll = nullptr;
};