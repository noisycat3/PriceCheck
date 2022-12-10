#pragma once

#define CASE( str, value ) value ,
enum class ERocketMenu : uint8_t
{
	Unknown,
#include "MenuManagerMapping.inc"
	Count // Keep last
};
#undef CASE

// Utilities to convert to and from string
ERocketMenu getMenuFromStringName(const std::string& menuName);
const char* getMenuNameFromEnum(ERocketMenu menu);

/*
 * Interface for handling screen-specific operations
 */
struct IMenuScreenHandler
{
	// BOILERPLATE INTERFACE
	IMenuScreenHandler() = default;
	virtual ~IMenuScreenHandler() = default;

	IMenuScreenHandler(const IMenuScreenHandler&) = default;
	IMenuScreenHandler(IMenuScreenHandler&&) = default;
	IMenuScreenHandler& operator=(const IMenuScreenHandler&) = default;
	IMenuScreenHandler& operator=(IMenuScreenHandler&&) = default;
	// ~BOILERPLATE INTERFACE

	// Check if window should be visible in a given menu
	[[nodiscard]] virtual bool isVisibleIn(ERocketMenu menu) const = 0;

	// Get optional delay for enabling screen handler
	[[nodiscard]] virtual float getShowDelay() const { return 0.f; }

	// Called when handler is registered in menu manager
	virtual void onRegister(GameWrapper* gw) { }
	
	// Called when window becomes visible
	virtual void onEnable(GameWrapper* gw) { }

	// Called to render the window when enabled
	virtual void render(GameWrapper* gw) { }

	// Called when window is hidden
	virtual void onDisable(GameWrapper* gw) { }

	// Called when handler is removed from menu manager
	virtual void onUnregister(GameWrapper* gw) { }

	// Check if currently enabled
	[[nodiscard]] bool isEnabled() const { return _isEnabled; }

private:
	friend class MenuManager;
	bool _isEnabled = false;
};

/*
 * Class for handling menu navigation
 *		Contains callbacks for common operations with menu stack
 *		Can manage screen-specific windows
 */
class MenuManager
{
public:
	// Initializes and enables the manager
	void registerHooks(std::shared_ptr<GameWrapper> gameWrapper);
	void unregisterHooks();

	// Register handler
	void registerScreenHandler(IMenuScreenHandler& handler);
	void unregisterScreenHandler(IMenuScreenHandler& handler);

	// Call on plugin window render
	void renderHandlers() const;

	// Clean all handlers
	void resetHandlers();

	// Callbacks for menu changes
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

	void resetCallbacks();

private:
	std::shared_ptr<GameWrapper> gameWrapper;
	void handleMenuChanged(MenuStackWrapper menuStack);

	void popMenu();
	void pushMenu(ERocketMenu menu);

	// Current stack
	std::vector<ERocketMenu> stack;
	void debugMenuStack(const std::vector<std::string>& srcStack);

	// Registered handlers
	struct HandlerInstance
	{
		IMenuScreenHandler* h;
		size_t timerSequenceNumber;
	};
	std::vector<HandlerInstance> handlers;

	void notifyHandlersTopChanged(ERocketMenu top);
	void enableHandler(HandlerInstance& inst) const;
	void enableHandlerActual(HandlerInstance& inst) const;
	void disableHandler(HandlerInstance& inst) const;

	// Callbacks
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

