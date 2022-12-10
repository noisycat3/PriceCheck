#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "version.h"

#include "classes/TradeIn.h"
#include "classes/PlayerTrade.h"
#include "classes/ItemSeries.h"
#include "classes/MenuManager.h"
#include "gui/Fonts.h"
#include "handlers/HandlerInventory.h"
#include "wrappers/ProductsWrapper.h"

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

struct GuiState
{
	bool showTrade = false;
	bool showTradeIn = false;
	bool showInventory = false;
};

class PriceCheck : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
private:
	// Params to keep track of actions
	//std::shared_ptr<bool> gettingNewItems;

	/* SET FILE */
	std::shared_ptr<bool> useAVG;
	std::shared_ptr<bool> forceShow;
	std::shared_ptr<string> dataProvider;

	GuiState guiState;

	// Handlers for custom display
	MenuManager menuMgr;
	HandlerInventory handlerInventory;

	/* TRADE */
	bool showTrade = false;
	PlayerTrade playerTrade = PlayerTrade();

	/* TRADE-IN */
	bool showTradeIn = false;
	TradeIn tradeIn = TradeIn();
	ItemSeriesDatabaseWrapper itemSeriesDatabaseWrapper = ItemSeriesDatabaseWrapper();

	/* ITEM DROPS */
	std::list<ProductInstanceID> itemDrops;

	/* INVETORY ITEM */
	bool showInventory = false;

	void registerCvars();
	void registerHooks();


public:
	static void StartRender();
	static void StopRender();

	void onLoad() override;
	void onUnload() override;

	std::shared_ptr<PriceAPI> api;

	/* INVENTORY FUNCTIONS */
	void inventoryScrolled(ProductsWrapper caller);

	/* TRADE FUNCTIONS */
	void tradeStart(TradeWrapper trade);
	void tradeEnd(TradeWrapper trade);
	void checkPrices(TradeWrapper trade);

	/* ITEM DROP FUNCTIONS */
	void getNewOnlineItem(ActorWrapper wrap, void* p);
	void showNewOnlineItem(ActorWrapper wrap, int count);
	void itemsEnded(ActorWrapper wrap);

	/* TRADE-IN FUNCTIONS */
	void checkPrices(ProductTradeInWrapper wrap);
	void tradeInEnded(ProductTradeInWrapper wrap);
	void checkSeriesItems(string cvarName, CVarWrapper newCvar);

	// Inherited via PluginWindow
	
	/* IMGUI STUFF */
	static const std::string s_wndName;

	bool isWindowOpen_ = false;
	Fonts fonts = Fonts();

	virtual void DrawTradeWindow();
	virtual void DrawTradeInWindow();

	void Render() override;
	std::string GetMenuName() override { return s_wndName; }
	std::string GetMenuTitle() override { return s_wndName; }

	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;

	void OnOpen() override;
	void OnClose() override;
};