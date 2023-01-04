#include "pch.h"
#include "PriceCheck.h"
#include "bakkesmod/wrappers/items/TradeWrapper.h"
#include "bakkesmod/wrappers/items/ProductTradeInWrapper.h"
#include "bakkesmod/wrappers/items/ProductWrapper.h"
#include "defines.h"

#include "AdvancedInventoryWrappers.h"
#include "gui/Fonts.h"

BAKKESMOD_PLUGIN(PriceCheck, "Check item prices.", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
std::shared_ptr<GameWrapper> _globalGameWrapper;
std::shared_ptr<SpecialEditionDatabaseWrapper> _globalSpecialEditionManager;

void PriceCheck::StartRender()
{
	_globalCvarManager->executeCommand(fmt::format("openmenu {}", PLUGIN_NAME), false);
}

void PriceCheck::StopRender()
{
	_globalCvarManager->executeCommand(fmt::format("closemenu {}", PLUGIN_NAME), false);
}

struct HandleNewOnlineItemParam
{
	void* no_touch;
	uintptr_t online_product_ptr;
};

struct DropParams
{
	unsigned char padding[0x8];
	int ReturnValue;
};


void PriceCheck::registerHooks()
{
	/*gameWrapper->HookEventWithCallerPost<TradeWrapper>(HOOK_TRADE_START,
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
		[this](ProductTradeInWrapper caller, void* params, std::string eventName) { tradeInEnded(caller); });*/

	gameWrapper->HookEventWithCaller<aiw::GfxProductsWrapper>(HOOK_INV_UPDATE,
		[this](const aiw::GfxProductsWrapper&, void* params, const std::string&) {

			const aiw::GfxProducts_SetViewProductsParamsWrapper paramsWrapper(params);
			handlerInventory.prepareIndexTable(paramsWrapper.GetHashIDs());
		});

	gameWrapper->HookEventWithCallerPost<aiw::GfxProductsWrapper>(HOOK_INV_UPDATE,
		[this](const aiw::GfxProductsWrapper& caller, void*, const std::string&) {
			handlerInventory.resolveIndexTable(caller);
		});
}

void PriceCheck::onLoad()
{
	try 
	{
		_globalCvarManager = cvarManager;
		_globalGameWrapper = gameWrapper;

		// Set global version for pointer paths
		const std::string buildId = gameWrapper->GetPsyBuildID();
		const bool isSteam = gameWrapper->IsUsingSteamVersion();
		LOG("Detected version: {}/{}", isSteam ? "steam" : "epic", buildId.c_str());

		// Kickoff api thread
		PriceDatabase::getInstance().startApiThread();

		// Initialize persistent storage
		storage = std::make_unique<PersistentStorage>(this, "pricecheck", true, true);
		//storage->RegisterPersistentCvar()

		/* SET FILE PARAMS */
		dataProvider = std::make_shared<std::string>();
		useAVG = std::make_shared<bool>(false);
		forceShow = std::make_shared<bool>(false);

		/* MISC STUFF */
		gameWrapper->LoadToastTexture("pricecheck_logo", std::string("./bakkesmod/data/assets/pricecheck_logo.tga"));

		registerHooks();
		menuMgr.registerHooks(gameWrapper);
		menuMgr.addOnTopChanged([this](ERocketMenu m)
		{
			LOG("Top menu changed: {}", getMenuNameFromEnum(m));
		});

		menuMgr.addOnUnknownMenu([this](const std::string& menu)
		{
			LOG("UNKNOWN MENU: `{}`", menu.c_str());
		});

		// Handler for inventory screen
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

	// Stop api thread
	PriceDatabase::getInstance().killApiThread();

	menuMgr.unregisterHooks();
	menuMgr.resetHandlers();
	menuMgr.resetCallbacks();
}

void PriceCheck::tradeStart(TradeWrapper trade)
{
	if (trade.IsNull())
	{
		LOG("{}: Trade is null", __FUNCTION__);
		return;
	}
	StartRender();
	guiState.showTrade = true;
	// Should be empty but double checking
	//playerTrade.Clear();
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
	//playerTrade.Clear();
}

void PriceCheck::checkPrices(TradeWrapper trade)
{
	if (trade.IsNull())
	{
		LOG("{}: Trade is null", __FUNCTION__);
		return;
	}
	//playerTrade.CheckTrade(trade);
}

void PriceCheck::getNewOnlineItem(ActorWrapper wrap, void* params)
{
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
		//TradeItem i = gameWrapper->GetItemsWrapper().GetOnlineProduct(id.lower_bits);
		//if (!i) 
		//{
		//	LOG("{}: Item is null", __FUNCTION__);
		//	return;
		//}

		//PaintPrice price = i.GetPrice();
		//std::string paint = "";// i.GetPaint();

		//// LOG("IsContainer: {}", i.GetProduct().IsContainer());
		//// LOG("AssetPath: {}", i.GetProduct().GetThumbnailAssetPath().ToString());
		//// AssetPath: Wheel_Hanzawa_T.Wheel_Hanzawa_T
		//gameWrapper->Toast(
		//	"New Item",
		//	i.GetLongLabel().ToString() + "\n" +				// Name
		//	(paint != "" ? "(" + paint + ")\n" : "") +	// Paint
		//	std::to_string(price.min) + " - " + std::to_string(price.max), // Price min - max
		//	"pricecheck_logo", 4.5f, ToastType_Info);

		//itemDrops.pop_front();
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
	//tradeIn.Clear();

	//for (TradeItem i : items) 
	//	tradeIn.AddItem(i);
}

void PriceCheck::tradeInEnded(ProductTradeInWrapper wrap)
{
	StopRender();
	//showTradeIn = false;
	guiState.showTradeIn = false;
	//tradeIn.Clear();
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

void PriceCheck::RenderSettings()
{
	const ImVec4 linkColor = { 20, 20, 160, 255 };
	const ImVec4 linkHoverColor = { 50, 50, 180, 255 };

	// Title
	ImGui::PushFont(USE_FONT("title"));
	ImGui::TextUnformatted("Price Check Plus");
	ImGui::PopFont();

	// Repository link
	ImGui::Hyperlink("https://github.com/noisycat3/PriceCheck", linkColor, linkHoverColor);

	// End description
	ImGui::Separator();

	//storage->RegisterPersistentCvar()
	//ImGui::SliderFloat("Font scale: ", )

	if (ImGui::Button("Save user settings", { 150.f, 0.f }))
		storage->WritePersistentStorage();

	// End config
	ImGui::Separator();

	// Footer
	ImGui::TextUnformatted("Created by noisycat, based on PriceCheck: ");
	ImGui::SameLine(0, 0);
	ImGui::Hyperlink("https://github.com/matias-kovero/PriceCheck", linkColor, linkHoverColor);
}

void PriceCheck::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
	Fonts::Initialize(gameWrapper);

	Fonts::LoadFont({ "main", "Roboto-Regular.ttf", 14 });
	Fonts::LoadFont({ "main-b", "Roboto-Bold.ttf", 14 });
	Fonts::LoadFont({ "main-i", "Roboto-Italic.ttf", 14 });

	Fonts::LoadFont({ "title", "Roboto-Bold.ttf", 24 });
}
