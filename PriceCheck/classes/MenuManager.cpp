#include "pch.h"

#include <utility>
#include "classes/MenuManager.h"

#include "PriceCheck.h"

#define HOOK_MENU_CHANGED "Function TAGame.GFxData_MenuStack_TA.EventTopMenuChanged"

ERocketMenu getMenuFromStringName(const std::string& menuName)
{
	const size_t hash = crc32(menuName);

#define CASE( str, result ) case crc32( str ): return ERocketMenu::##result;
    switch (hash)
	{
		#include "MenuManagerMapping.inc"
		default: return ERocketMenu::Unknown;
	}
#undef CASE
}

const char* getMenuNameFromEnum(ERocketMenu menu)
{
#define CASE( str, value ) case ERocketMenu::##value: return str;
    switch (menu)
    {
		#include "MenuManagerMapping.inc"
	    default: return "unknown";
    }
#undef CASE
}

size_t MenuManager::id_counter = 0;

void MenuManager::registerHooks(std::shared_ptr<GameWrapper> inGameWrapper)
{
    gameWrapper = std::move(inGameWrapper);

	gameWrapper->HookEventWithCallerPost<MenuStackWrapper>(HOOK_MENU_CHANGED,
		[this](const MenuStackWrapper& caller, void*, std::string) { handleMenuChanged(caller); });
}

void MenuManager::unregisterHooks()
{
    gameWrapper->UnhookEventPost(HOOK_MENU_CHANGED);

    gameWrapper.reset();
}

void MenuManager::registerScreenHandler(IMenuScreenHandler& handler)
{
    handler.onRegister(gameWrapper.get());
    handlers.emplace_back(HandlerInstance { &handler, 0 });
}

void MenuManager::unregisterScreenHandler(IMenuScreenHandler& handler)
{
    for (size_t idx = 0; idx < handlers.size(); ++idx)
    {
        if (handlers[idx].h != &handler)
            continue;

        if (handler._isEnabled)
            disableHandler(handlers[idx]);

        handler.onUnregister(gameWrapper.get());
        handlers.erase(handlers.begin() + idx);
        break;
    }
}

void MenuManager::renderHandlers() const
{
    for (const HandlerInstance& inst : handlers)
    {
        if (inst.h->_isEnabled)
            inst.h->render(gameWrapper.get());
    }
}

void MenuManager::resetHandlers()
{
    for (HandlerInstance& inst : handlers)
    {
        if (inst.h->_isEnabled)
            disableHandler(inst);

        inst.h->onUnregister(gameWrapper.get());
    }

    handlers.clear();
}

void MenuManager::resetCallbacks()
{
    onPush.clear();
    onPop.clear();
    onTopChanged.clear();

    for (size_t e = 0; e < static_cast<size_t>(ERocketMenu::Count); ++e)
    {
        onShow[e].clear();
        onHide[e].clear();
    }

    onUnknownMenu.clear();
}

void MenuManager::handleMenuChanged(MenuStackWrapper menuStack)
{
    const std::vector<std::string> srcStack = menuStack.GetMenuStack();

    // Remove what we had to remove
    while (stack.size() > srcStack.size())
        popMenu();

    // Pop all different menus, converting in flight
    ERocketMenu convertedTemp[16] = {};
    while (!stack.empty())
    {
        const size_t idx = stack.size() - 1;
        const ERocketMenu existing = 
            convertedTemp[idx] = getMenuFromStringName(srcStack[idx]);

        if (existing == ERocketMenu::Unknown)
            executeCallbackList(onUnknownMenu, srcStack[idx]);

        if (existing == stack[idx])
            break;

        popMenu();
    }

    // Add missing
    while (stack.size() < srcStack.size())
    {
        const size_t idx = stack.size();
        const ERocketMenu existing = (convertedTemp[idx] != ERocketMenu::Unknown) ? 
            convertedTemp[idx] : getMenuFromStringName(srcStack[idx]);
        if (existing == ERocketMenu::Unknown)
            executeCallbackList(onUnknownMenu, srcStack[idx]);

        pushMenu(existing);
    }
}

void MenuManager::popMenu()
{
    const ERocketMenu pop = stack.back();
    stack.pop_back();
    const ERocketMenu top = stack.back();

    executeCallbackList(onPop, pop);
    executeCallbackMap(onHide, pop, top);
    executeCallbackList(onTopChanged, stack.back());

    notifyHandlersTopChanged(stack.back());
}

void MenuManager::pushMenu(ERocketMenu menu)
{
    const ERocketMenu top = stack.empty() ? ERocketMenu::Count : stack.back();
    stack.push_back(menu);
    const ERocketMenu push = stack.back();

    executeCallbackList(onPush, push);
    executeCallbackMap(onShow, push, top);
    executeCallbackList(onTopChanged, stack.back());

	notifyHandlersTopChanged(stack.back());
}

void MenuManager::debugMenuStack(const std::vector<std::string>& srcStack)
{
    std::string out = "source stack: ";
    for (const std::string& st : srcStack)
    {
        out += st + ", ";
    }

    out += "\n";
    for (ERocketMenu menu : stack)
    {
        out += getMenuNameFromEnum(menu) + std::string(", ");
    }

    LOG("{}", out.c_str());
}

void MenuManager::notifyHandlersTopChanged(ERocketMenu top)
{
    for (HandlerInstance& inst : handlers)
    {
        if (inst.h->_isEnabled && !inst.h->isVisibleIn(top))
            disableHandler(inst);
        if (!inst.h->_isEnabled && inst.h->isVisibleIn(top))
            enableHandler(inst);
    }
}

void MenuManager::enableHandler(HandlerInstance& inst) const
{
    if (inst.h->_isEnabled)
        return;

    const float delay = inst.h->getShowDelay();
    if (delay > 0.f)
    {
        const size_t timerSequenceNumber = ++inst.timerSequenceNumber;
        gameWrapper->SetTimeout([this, timerSequenceNumber, &inst](GameWrapper*)
        {
            if (inst.timerSequenceNumber == timerSequenceNumber)
                enableHandlerActual(inst);
        }, delay);
    }
    else
    {
        enableHandlerActual(inst);
    }
}

void MenuManager::enableHandlerActual(HandlerInstance& inst) const
{
    PriceCheck::StartRender();

    inst.h->_isEnabled = true;
    inst.h->onEnable(gameWrapper.get());
}

void MenuManager::disableHandler(HandlerInstance& inst) const
{
    inst.timerSequenceNumber++; // Reset enable timer, if any
    inst.h->onDisable(gameWrapper.get());
    inst.h->_isEnabled = false;
}
