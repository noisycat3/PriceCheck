#include "pch.h"
#include "PriceCheck.h"
#include "TradeItem.h"
#include "bakkesmod/wrappers/items/TradeWrapper.h"
#include "bakkesmod/wrappers/items/ProductTradeInWrapper.h"
#include "bakkesmod/wrappers/items/ProductWrapper.h"
#include "defines.h"
#include "wrappers/ProductsWrapper.h"
#include "wrappers/WrapperUtil.h"

BAKKESMOD_PLUGIN(PriceCheck, "Check item prices.", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
std::shared_ptr<GameWrapper> _globalGameWrapper;
std::shared_ptr<SpecialEditionDatabaseWrapper> _globalSpecialEditionManager;
std::shared_ptr<PriceAPI> _globalPriceAPI;

void PriceCheck::registerCvars()
{
	/* ==================
	*		SET FILE CVARS
	* ===================
	*/
	try 
	{
		cvarManager->registerCvar(CVAR_PROVIDER, "1", "Select data provider", false)
			.bindTo(dataProvider);

		CVarWrapper forceShowVariable = cvarManager->registerCvar(CVAR_FORCE_SHOW, "0", "Show all UI elements", 
			false, true, 0,true, 1);
		forceShowVariable.bindTo(forceShow);
		forceShowVariable.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) 
		{
			if (newCvar.IsNull()) 
				return;
			newCvar.getBoolValue() ? StartRender() : StopRender();
		});

		/* DEBUG ITEM SERIES
		auto seriesInfo = cvarManager->registerCvar("pc_series", "1", "Check series items", true, true, 1);
		seriesInfo.addOnValueChanged(std::bind(&PriceCheck::checkSeriesItems, this, std::placeholders::_1, std::placeholders::_2));
		*/

		// Why the hell it won't just work with ""
		gameWrapper->LoadToastTexture("pricecheck_logo", std::string("./bakkesmod/data/assets/pricecheck_logo.tga"));
	}
	catch (std::exception& e)
	{
		LOG("Plugin failed to load! Please contact plugin developer");
		LOG("{}:{}", __FUNCTION__, e.what());
	}
}

void PriceCheck::registerHooks()
{
	gameWrapper->HookEventWithCallerPost<ProductsWrapper>(HOOK_INV_SCROLL, 
		[this](ProductsWrapper caller, void* params, std::string eventName) { inventoryScrolled(caller); });


	gameWrapper->HookEventWithCallerPost<TradeWrapper>(HOOK_TRADE_START,
		[this](TradeWrapper caller, void* params, std::string eventName) { tradeStart(caller); });

	gameWrapper->HookEventWithCallerPost<TradeWrapper>(HOOK_TRADE_END,
		[this](TradeWrapper caller, void* params, std::string eventName) { tradeEnd(caller); });

	gameWrapper->HookEventWithCallerPost<TradeWrapper>(HOOK_TRADE_FORCE_END,
		[this](TradeWrapper caller, void* params, std::string eventName) { tradeEnd(caller); });

	gameWrapper->HookEventWithCallerPost<TradeWrapper>(HOOK_TRADE_CHANGE,
		[this](TradeWrapper caller, void* params, std::string eventName) { checkPrices(caller); });
	
	gameWrapper->HookEventWithCallerPost<ActorWrapper>(HOOK_NEW_ITEM,
		[this](ActorWrapper caller, void* params, std::string eventName) { 
			getNewOnlineItem(caller, params); 
		});

	gameWrapper->HookEventWithCallerPost<ActorWrapper>(HOOK_SHOW_NEW_ITEM,
		[this](ActorWrapper caller, void* params, std::string eventName) {
			showNewOnlineItem(caller, 1);
		});

	gameWrapper->HookEventWithCallerPost<ActorWrapper>(HOOK_PENDING_DROP,
		[this](ActorWrapper caller, void* params, std::string eventName) {
			if (!params) return;
			auto t_params = (DropParams*)params;
			int pendingAmount = t_params->ReturnValue;
			if (pendingAmount > 0 )
			{
				showNewOnlineItem(caller, pendingAmount);
			}
		});

	gameWrapper->HookEventWithCallerPost<ActorWrapper>(HOOK_DROPS_ENDED,
		[this](ActorWrapper caller, void* params, std::string eventName) { itemsEnded(caller); });

	gameWrapper->HookEventWithCallerPost<ProductTradeInWrapper>(HOOK_TRADE_IN_UPDATE,
		[this](ProductTradeInWrapper caller, void* params, std::string eventName) { checkPrices(caller); });

	gameWrapper->HookEventWithCallerPost<ProductTradeInWrapper>(HOOK_TRADE_IN_CLOSE,
		[this](ProductTradeInWrapper caller, void* params, std::string eventName) { tradeInEnded(caller); });

	gameWrapper->HookEventWithCallerPost<ProductTradeInWrapper>(HOOK_TRADE_IN_END,
		[this](ProductTradeInWrapper caller, void* params, std::string eventName) { tradeInEnded(caller); });

	/* THIS IS NOT WORKING YET - unable to check if blueprint or item */
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>(HOOK_INV_OPEN,
	//	[this](ActorWrapper caller, void* params, std::string eventName)
	//	{
	//		showInventory = true;
	//	});

	//gameWrapper->HookEventWithCallerPost<ActorWrapper>(HOOK_INV_CLOSE,
	//	[this](ActorWrapper caller, void* params, std::string eventName)
	//	{
	//		showInventory = false;
	//		hoverItem.Clear();
	//	});

	//gameWrapper->HookEventWithCallerPost<ActorWrapper>(HOOK_INV_HOVER,
	//	[this](ActorWrapper caller, void* params, std::string eventName) 
	//	{ 
	//		hoverItem.OnHover();
	//	});

	//gameWrapper->HookEventWithCallerPost<OnlineProductWrapper>(HOOK_INVENTORY_ITEM,
	//	[this](OnlineProductWrapper caller, void* params, std::string eventName) { showInvetoryItem(caller); });
}

const std::string PriceCheck::s_wndName = "PriceCheck";

void PriceCheck::StartRender()
{
	_globalCvarManager->executeCommand("openmenu " + s_wndName);
}

void PriceCheck::StopRender()
{
	_globalCvarManager->executeCommand("closemenu " + s_wndName);
}

void PriceCheck::onLoad()
{
	try 
	{
		_globalCvarManager = cvarManager;
		_globalGameWrapper = gameWrapper;

		// Set global version for pointer paths
		VersionedPointerPath::setVersion(gameWrapper->GetPsyBuildID(),
			gameWrapper->IsUsingSteamVersion() ? HostDependentPointerPath::STEAM : HostDependentPointerPath::EPIC);

		/* ITEM API */
		api = std::make_shared<PriceAPI>(cvarManager, gameWrapper);
		_globalPriceAPI = api;
		api->LoadData();

		/* SET FILE PARAMS */
		dataProvider = std::make_shared<string>();
		useAVG = std::make_shared<bool>(false);
		forceShow = std::make_shared<bool>(false);

		/* MISC STUFF */
		registerHooks();
		registerCvars();

		menuMgr.registerHooks(gameWrapper);
		menuMgr.addOnTopChanged([this](ERocketMenu m)
		{
				LOG("Top menu changed: {}", getMenuNameFromEnum(m));
		});

		menuMgr.addOnUnknownMenu([this](const std::string& menu)
		{
			LOG("UNKNOWN MENU: `{}`", menu.c_str());
		});

		menuMgr.registerScreenHandler(handlerInventory);

		/* FOR TRADEITEMS */ // Is this NONO? check TradeItem.cpp -> updateItemInfo()
		SpecialEditionDatabaseWrapper sedb = gameWrapper->GetItemsWrapper().GetSpecialEditionDB();
		_globalSpecialEditionManager = std::make_shared<SpecialEditionDatabaseWrapper>(sedb);
	}
	catch (std::exception& e)
	{
		LOG("Plugin failed to load! Please contact plugin developer");
		LOG("{}:{}", __FUNCTION__, e.what());
	}
}

void PriceCheck::onUnload()
{
	StopRender();

	menuMgr.unregisterHooks();
	menuMgr.resetHandlers();
	menuMgr.resetCallbacks();
}

void PriceCheck::inventoryScrolled(ProductsWrapper caller)
{
	//const PointerPath pp = PointerPath(0x0237BBE0, { 0x120, 0x28, 0x10, 0x198, 0x6E8, 0x20, 0xEA8, 0x278, 0x284 });
	//const PointerPath pp = PointerPath(0x0237BBE0, { 0x120, 0x28, 0xC8, 0x198, 0x6E8, 0x20, 0xEA8, 0x278, 0x284 });
	//const PointerPath pp = PointerPath(0x0237BBE0, { 0x30, 0x80, 0x20, 0x80, 0xC0, 0x170, 0x58, 0x10, 0x284 });
	//LOG("SCROLL: {}\nsteam: {}, bakkes: {}, psy: {}", caller.getInventoryScrollOffset(), 
	//	gameWrapper->GetSteamVersion(), gameWrapper->GetBakkesModVersion(), gameWrapper->GetPsyBuildID().c_str());
}

void PriceCheck::tradeStart(TradeWrapper trade)
{
	api->Refresh();

	if (trade.IsNull())
	{
		LOG("{}: Trade is null", __FUNCTION__);
		return;
	}
	StartRender();
	guiState.showTrade = true;
	// Should be empty but double checking
	playerTrade.Clear();
}

void PriceCheck::tradeEnd(TradeWrapper trade)
{
	if (trade.IsNull())
	{
		LOG("{}: Trade is null", __FUNCTION__);
		return;
	}
	StopRender();
	guiState.showTrade = false;
	playerTrade.Clear();
}

void PriceCheck::checkPrices(TradeWrapper trade)
{
	if (trade.IsNull())
	{
		LOG("{}: Trade is null", __FUNCTION__);
		return;
	}
	playerTrade.CheckTrade(trade);
}

void PriceCheck::getNewOnlineItem(ActorWrapper wrap, void* params)
{
	api->Refresh();

	if (!params)
	{
		LOG("{}: Params are null", __FUNCTION__);
		return;
	}
	auto t_params = (HandleNewOnlineItemParam*)params;
	OnlineProductWrapper online_product{ t_params->online_product_ptr };
	if (!online_product) 
	{
		LOG("{}: online_product is null", __FUNCTION__);
		return;
	}
	// Big amount of debuggin' on item drops. Order of drops still unknown.
	auto pr = online_product.GetProduct();
	LOG("ID: {}, Series: {}, Quality: {}, Container: {}, Blueprint: {}, UnlockMethod: {}, Slot: {}, SortLabel: {}", pr.GetID(), online_product.GetSeriesID(), online_product.GetQuality(), pr.IsContainer(), online_product.IsBlueprint(), pr.GetUnlockMethod(), pr.GetSlot().GetSlotIndex(), pr.GetSortLabel().ToString());
	// ID: 1111, SortLabel: Octane: Toon Sketch, UnlockMethod: 1, IsContainer: false, UnlockMthd: 1, DisplayLabelSlot: Animated Decal
	/* GOT 2 DROPS: [3036, 5587] -> [3036, 5587] | push_front is not right */
	// ID: 3036, Series: 1315, Quality: 8, Container: false, Blueprint: false, UnlockMethod: 1, Slot: 21, SortLabel: Title
	// ID: 5587, Series: 1282, Quality: 8, Container: false, Blueprint: false, UnlockMethod: 1, Slot: 14, SortLabel: Pixel Pointer
	// GOT 1 DROP: [5610] -> [5610] | it was an blueprint -> Season 1 Series
	// ID: 5610, Series: 4, Quality: 3, Container: true, Blueprint: true, UnlockMethod: 1, Slot: 24, SortLabel: Season 1
	// GOT 1 DROP: [5592] -> [5592] | Quality = Limited
	// ID: 5592, Series: 1283, Quality: 8, Container: false, Blueprint: false, UnlockMethod: 1, Slot: 2, SortLabel: Imptekk
	// GOT 3 DROPS: [5144,5301,5931] -> [5931, ????, 5144] | Got only 2 drops to show. Array also had 4 items? Video: itemdrop0305
	// ID: 5144, Series: 4, Quality: 4, Container: true, Blueprint: true, UnlockMethod: 1, Slot: 24, SortLabel: Momentum
	// ID: 5301, Series: 1119, Quality: 2, Container: true, Blueprint: false, UnlockMethod: 1, Slot: 11, SortLabel: Rare Drop
	// ID: 5931, Series: 1284, Quality: 8, Container: false, Blueprint: false, UnlockMethod: 1, Slot: 1, SortLabel: Tyranno: Reviver
	// 
	// Drops: 4, count: 3
	// Vector length: 4
	// Got toast from Item Drop: Tyranno: Reviver
	// Drops: 3, count: 2
	// Vector length: 3
	// Got toast from Item Drop: Rare Drop
	// Drops: 2, count: 2
	// Got toast from Item Drop: HNY Blueprint
	// 
	// GOT DROPS 1: [5610] -> [5610] | Item Drop: Fennec: Distortion Blueprint
	// ID: 5610, Series: 4, Quality: 3, Container: true, Blueprint: true, UnlockMethod: 1, Slot: 24, SortLabel: Season 1
	// GOT DROPS 1: [4737] -> [4737] | Item Drop: Ribbon Candy
	// ID: 4737, Series: 1285, Quality: 8, Container: false, Blueprint: false, UnlockMethod: 1, Slot: 7, SortLabel: Ribbon Candy
	// GOT DROPS 1: [5301] -> [5301] | Item Drop: Ribbon Candy
	// ID: 5301, Series: 1119, Quality: 2, Container: true, Blueprint: false, UnlockMethod: 1, Slot: 11, SortLabel: Rare Drop
	// 

	// ID: 5303, Series : 1120, Quality : 3, Container : true, Blueprint : false, UnlockMethod : 1, Slot : 11, SortLabel : Very Rare Drop
	// Should add : 17939505570, itemDrops : {17939505570, 17938362897}
	// ID : 5340, Series : 1294, Quality : 8, Container : false, Blueprint : false, UnlockMethod : 1, Slot : 1, SortLabel : Retrogression
	// Should add : 17939505578, itemDrops : {17939505578, 17939505570, 17938362897}
	// New Item: Retrogression
	// New Item: Very Rare Drop
	// New Item: Peregrine TT: Crisis Blueprint
	// 
	// GOT 2 DROPS: [5303, 5979] -> [5979, 5303] | Notifications were right
	// ID: 5303, Series: 1120, Quality: 3, Container: true, Blueprint: false, UnlockMethod: 1, Slot: 11, SortLabel: Very Rare Drop
	// Should add: 18050945811, itemDrops: {18050945811}
	// ID: 5979, Series: 1299, Quality: 8, Container: false, Blueprint: false, UnlockMethod: 1, Slot: 0, SortLabel: Tyranno GXT
	// Should add: 18050945812, itemDrops: {18050945812, 18050945811}
	// 
	// Maybe its highest quality, then smallest ID
	// Seems its using LIFO when displaying over 1 drops
	// Could we use map -> <ID, InstanceID>
	const ProductInstanceID PID = online_product.GetInstanceIDV2();
	itemDrops.push_front(PID);
	LOG("Should add: {}-{}, itemDrops: {}", PID.lower_bits, PID.upper_bits, itemDrops);
}

void PriceCheck::showNewOnlineItem(ActorWrapper wrap, int count)
{
	// LOG("Drops: {}, caller: {}", itemDrops.size(), count);

	if (!itemDrops.empty() && itemDrops.size() >= static_cast<size_t>(count))
	{
		// LOG("Drops amount: {}", itemDrops.size());
		ProductInstanceID id = itemDrops.front();
		TradeItem i = gameWrapper->GetItemsWrapper().GetOnlineProduct(id.lower_bits);
		if (!i) 
		{
			LOG("{}: Item is null", __FUNCTION__);
			return;
		}

		PaintPrice price = i.GetPrice();
		string paint = i.GetPaint();

		// LOG("IsContainer: {}", i.GetProduct().IsContainer());
		// LOG("AssetPath: {}", i.GetProduct().GetThumbnailAssetPath().ToString());
		// AssetPath: Wheel_Hanzawa_T.Wheel_Hanzawa_T
		gameWrapper->Toast(
			"New Item",
			i.GetLongLabel().ToString() + "\n" +				// Name
			(paint != "" ? "(" + paint + ")\n" : "") +	// Paint
			std::to_string(price.min) + " - " + std::to_string(price.max), // Price min - max
			"pricecheck_logo", 4.5f, ToastType_Info);

		itemDrops.pop_front();
	}
	else {
		// LOG("{}: Drops seem empty", __FUNCTION__);
	}
}

void PriceCheck::itemsEnded(ActorWrapper wrap) 
{
	itemDrops.clear();
}

void PriceCheck::checkPrices(ProductTradeInWrapper wrap)
{
	if (wrap.IsNull())
	{
		LOG("{}: Trade In is null", __FUNCTION__);
		return;
	}
	ArrayWrapper<OnlineProductWrapper> items = wrap.GetProducts();
	if (items.IsNull()) 
	{
		guiState.showTradeIn = false;
		StopRender();
		return;
	}
	guiState.showTradeIn = items.Count() > 0 ? true : false;
	if (guiState.showTradeIn) StartRender();
	// As we will loop old items as well, clear the existing list.
	tradeIn.Clear();

	for (TradeItem i : items) 
		tradeIn.AddItem(i);
}

void PriceCheck::tradeInEnded(ProductTradeInWrapper wrap)
{
	StopRender();
	//showTradeIn = false;
	guiState.showTradeIn = false;
	tradeIn.Clear();
}

/* USED TO DEBUG ITEM SERIES
void PriceCheck::checkSeriesItems(string cvarName, CVarWrapper newCvar)
{
	if (newCvar.IsNull() || cvarName.empty()) return;
	int seriesId = newCvar.getIntValue();
	auto name = itemSeriesDatabaseWrapper.ToSeriesString(seriesId);
	auto items = itemSeriesDatabaseWrapper.SeriesToItems(seriesId);
	LOG("{} has {} items.", name, items.size());
	ItemsWrapper iw = gameWrapper->GetItemsWrapper();

	for (const auto& item : items)
	{
		auto p = iw.GetProduct(item);
		auto i = api->FindItem(item).data[ITEMPAINT::DEFAULT];
		LOG("{} | {} [{}-{}]", itemSeriesDatabaseWrapper.QualityToString(p.GetQuality()), p.GetLongLabel().ToString(), i.min, i.max);
	}
}
*/