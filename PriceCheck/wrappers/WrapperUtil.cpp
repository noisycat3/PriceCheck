#include "pch.h"
#include "wrappers/WrapperUtil.h"

uintptr_t InternalGetModuleBase()
{
	static uintptr_t s_moduleBase = (GetModuleBaseAddress(GetCurrentProcessId(), L"RocketLeague.exe"));
	return s_moduleBase;
}

void* PointerPath::get() const
{
	void* const moduleBase = reinterpret_cast<void*>(InternalGetModuleBase());  // NOLINT(performance-no-int-to-ptr)
	if (moduleBase == nullptr || moduleOffset == 0)
		return nullptr;

	using PtrType = uint8_t*;
	PtrType ptr = static_cast<PtrType>(moduleBase) + moduleOffset;
	for (const uint16_t off : offsets)
		ptr = *reinterpret_cast<PtrType*>(ptr) + off;
	return ptr;
}


std::string VersionedPointerPath::s_version;
uint32_t VersionedPointerPath::s_versionCrc;
HostDependentPointerPath::EHost VersionedPointerPath::s_hostEnv;