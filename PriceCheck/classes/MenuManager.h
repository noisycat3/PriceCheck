#pragma once

enum class ERocketMenu : uint8_t
{
	Unknown,
	MainMenu,
		PlayMain,
			PlayCasual,
			PlayComp,
		GarageMain,
			GarageInventory,
		
	GameSearching,

	Count // Keep last
};

// Utilities to convert to and from string
ERocketMenu getMenuFromStringName(const std::string& menuName);
const char* getMenuNameFromEnum(ERocketMenu menu);

class MenuManager
{
public:
	// Use this function to initialize menu manager
	void registerHooks(std::shared_ptr<GameWrapper> gameWrapper);
	void unregisterHooks(std::shared_ptr<GameWrapper> gameWrapper);

	using MenuCallbackFunc = std::function<void(ERocketMenu)>;
	using MenuUnknownCallbackFunc = std::function<void(const std::string&)>;

#define EVENT_LIST( type, ev ) \
	uint64_t addOn##ev(type fn) { return addCallbackToList(on##ev, fn); } \
	void removeOn##ev(uint64_t id) { removeCallbackFromList(on##ev, id); }

#define EVENT_MAP( type, ev ) \
	uint64_t addOn##ev(ERocketMenu menu, type fn) { return addCallbackToMap(on##ev, menu, fn); } \
	void removeOn##ev(ERocketMenu menu, uint64_t id) { removeCallbackFromMap(on##ev, menu, id); }

	EVENT_LIST(MenuCallbackFunc, Push)
	EVENT_LIST(MenuCallbackFunc, Pop)
	EVENT_LIST(MenuCallbackFunc, TopChanged)

	EVENT_MAP(MenuCallbackFunc, Show)
	EVENT_MAP(MenuCallbackFunc, Hide)

	EVENT_LIST(MenuUnknownCallbackFunc, UnknownMenu)

#undef EVENT_LIST
#undef EVENT_MAP

private:
	void handleMenuChanged(MenuStackWrapper menuStack);
	void popMenu();
	void pushMenu(ERocketMenu menu);

	std::vector<ERocketMenu> stack;

	// Handle callbacks
	template <typename FNC> struct MenuCallback { uint64_t id; FNC f; };
	template <typename FNC> using MenuCallbackList = std::vector<MenuCallback<FNC>>;
	template <typename FNC> using MenuCallbackMap = MenuCallbackList<FNC>[static_cast<size_t>(ERocketMenu::Count)];

	static uint64_t id_counter;

	MenuCallbackList<MenuCallbackFunc> onPush;
	MenuCallbackList<MenuCallbackFunc> onPop;
	MenuCallbackList<MenuCallbackFunc> onTopChanged;

	MenuCallbackMap<MenuCallbackFunc> onShow;
	MenuCallbackMap<MenuCallbackFunc> onHide;

	MenuCallbackList<MenuUnknownCallbackFunc> onUnknownMenu;

	template <typename FNC>
	static uint64_t addCallbackToList(MenuCallbackList<FNC>& list, FNC func)
	{
		const uint64_t id = id_counter++;
		list.push_back({ id, func });
		return id;
	}

	template <typename FNC, typename... ARGS>
	static void executeCallbackList(const MenuCallbackList<FNC>& list, ARGS... args)
	{
		for (const auto& c : list)
			c.f(std::forward<ARGS...>(args...));
	}

	template <typename FNC>
	static void removeCallbackFromList(MenuCallbackList<FNC>& list, uint64_t id)
	{
		for (size_t idx = 0; idx < list.size(); ++idx)
		{
			if (list[idx].id != id)
				continue;

			std::iter_swap(list.begin() + idx, list.end() - 1);
			list.pop_back();
			break;
		}
	}

	template <typename FNC>
	static uint64_t addCallbackToMap(MenuCallbackMap<FNC>& map, ERocketMenu menu, FNC func)
	{
		return addCallbackToList(map[static_cast<size_t>(menu)], func);
	}

	template <typename FNC, typename... ARGS>
	static void executeCallbackMap(const MenuCallbackMap<FNC>& map, ERocketMenu menu, ARGS... args)
	{
		executeCallbackList(map[static_cast<size_t>(menu)], std::forward<ARGS...>(args...));
	}

	template <typename FNC>
	static void removeCallbackFromMap(MenuCallbackMap<FNC>& map, ERocketMenu menu, uint64_t id)
	{
		removeCallbackFromList(map[static_cast<size_t>(menu)], id);
	}
};