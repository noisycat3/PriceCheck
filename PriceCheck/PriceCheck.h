#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "version.h"

#include "classes/MenuManager.h"

#include "handlers/HandlerInventory.h"
#include "PersistentStorage/PersistentStorage.h"

constexpr auto plugin_version = 
	stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

struct GuiState
{
	bool showTrade = false;
	bool showTradeIn = false;
	bool showInventory = false;
};

constexpr const char* PLUGIN_NAME = "PriceCheckPlus";

class PriceCheck final
	: public BakkesMod::Plugin::BakkesModPlugin
	, public BakkesMod::Plugin::PluginWindow
	, public BakkesMod::Plugin::PluginSettingsWindow
{
	/* SET FILE */
	std::shared_ptr<bool> useAVG;
	std::shared_ptr<bool> forceShow;
	std::shared_ptr<std::string> dataProvider;

	GuiState guiState;

	// Storage
	std::unique_ptr<PersistentStorage> storage;

	// Handlers for custom display
	MenuManager menuMgr;
	HandlerInventory handlerInventory;

	/* ITEM DROPS */
	std::list<ProductInstanceID> itemDrops;

	/* INVETORY ITEM */
	bool showInventory = false;
	
	void registerHooks();

public:
	static void StartRender();
	static void StopRender();

	void onLoad() override;
	void onUnload() override;

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
	void checkSeriesItems(std::string cvarName, CVarWrapper newCvar);

	// Inherited via PluginWindow
	
	/* IMGUI STUFF */
	void DrawTradeWindow();
	void DrawTradeInWindow();

	// PluginWindow
	void Render() override;
	std::string GetMenuName() override { return PLUGIN_NAME; }
	std::string GetMenuTitle() override { return PLUGIN_NAME; }

	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;

	void OnOpen() override;
	void OnClose() override;
	// ~PluginWindow

	// PluginSettingsWindow
	std::string GetPluginName() override { return PLUGIN_NAME; }
	void RenderSettings() override;
	// ~PluginSettingsWindow

	// PluginWindow & PluginSettingsWindow
	void SetImGuiContext(uintptr_t ctx) override;
	// ~PluginWindow & PluginSettingsWindow
};