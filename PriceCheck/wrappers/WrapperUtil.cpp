#include "pch.h"
#include "wrappers/WrapperUtil.h"

uintptr_t InternalGetModuleBase()
{
	static uintptr_t s_moduleBase = (GetModuleBaseAddress(GetCurrentProcessId(), L"RocketLeague.exe"));
	return s_moduleBase;
}

// Define to 1 to enable safe pointer path traversal. Might have performance impact
#define SAFE_POINTER_PATH_RESOLVE 1

template <typename PTRTYPE>
PTRTYPE* resolvePtr(PTRTYPE* ptr)
{
#if SAFE_POINTER_PATH_RESOLVE
	const HANDLE hProc = GetCurrentProcess();
	if (ReadProcessMemory(hProc, ptr, &ptr, sizeof(ptr), NULL))
		return ptr;
	return nullptr;
#else
	return *reinterpret_cast<PTRTYPE*>(ptr);
#endif
}

void* PointerPath::get() const
{
	void* const moduleBase = reinterpret_cast<void*>(InternalGetModuleBase());  // NOLINT(performance-no-int-to-ptr)
	if (moduleBase == nullptr || moduleOffset == 0)
		return nullptr;

	using PtrType = uint8_t*;
	PtrType ptr = static_cast<PtrType>(moduleBase) + moduleOffset;
	for (const uint16_t off : offsets)
	{
		ptr = resolvePtr(ptr);
		if (ptr == nullptr)
			break;
		ptr = ptr + off;
	}
	return ptr;
}

void PointerPath::getAll(std::vector<void*>& out) const
{
	void* const moduleBase = reinterpret_cast<void*>(InternalGetModuleBase());  // NOLINT(performance-no-int-to-ptr)
	if (moduleBase == nullptr || moduleOffset == 0)
		return;

	using PtrType = uint8_t*;
	PtrType ptr = static_cast<PtrType>(moduleBase) + moduleOffset;
	for (const uint16_t off : offsets)
	{
		ptr = resolvePtr(ptr);
		if (ptr == nullptr)
			break;
		out.push_back(ptr);
		ptr = ptr + off;
	}
	out.push_back(ptr);
}

std::string VersionedPointerPath::s_version;
uint32_t VersionedPointerPath::s_versionCrc;
HostDependentPointerPath::EHost VersionedPointerPath::s_hostEnv;