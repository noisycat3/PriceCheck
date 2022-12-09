#pragma once
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "zlib.lib")

#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_ZLIB_SUPPORT
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <TlHelp32.h>
uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);

#pragma warning(push)
#pragma warning(disable: 4267)
#include <httplib.h>
#pragma warning(pop)

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "api/PriceAPI.h"

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>

#include "imgui/imgui.h"

#include "fmt/core.h"
#include "fmt/ranges.h"

extern std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
extern std::shared_ptr<GameWrapper> _globalGameWrapper;
extern std::shared_ptr<PriceAPI> _globalPriceAPI;
extern std::shared_ptr<SpecialEditionDatabaseWrapper> _globalSpecialEditionManager;

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

template<typename S, typename... Args>
void LOG(const S& format_str, Args&&... args)
{
	_globalCvarManager->log(fmt::format(format_str, std::forward<Args>(args)...));
}

template <typename T, typename std::enable_if<std::is_base_of<ObjectWrapper, T>::value>::type*>
void GameWrapper::HookEventWithCallerPost(std::string eventName,
  std::function<void(T caller, void* params, std::string eventName)> callback)
{
  auto wrapped_callback = [callback](ActorWrapper caller, void* params, std::string eventName)
  {
    callback(T(caller.memory_address), params, eventName);
  };
  HookEventWithCaller<ActorWrapper>(eventName, wrapped_callback);
}

bool operator==(const ProductInstanceID& A, const ProductInstanceID& B);
inline bool operator!=(const ProductInstanceID& A, const ProductInstanceID& B) { return !(A == B); }

template <> struct fmt::formatter<ProductInstanceID> {
    auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
        throw format_error("invalid format");
    }

    template <typename FormatContext>
    auto format(const ProductInstanceID& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        return fmt::format_to(ctx.out(), "{}-{}", p.upper_bits, p.lower_bits);
    }
};