#include "pch.h"
#include "HandlerInventory.h"

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
}

void HandlerInventory::render(GameWrapper* gw)
{
	LOG("WINDOW DRAW {}", *ptrScroll);
}

void HandlerInventory::onDisable(GameWrapper* gw)
{
	LOG("WINDOW DISABLE");

	ptrScroll = nullptr;
}
