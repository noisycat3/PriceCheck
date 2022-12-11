#include "pch.h"

namespace test_ns
{
	uintptr_t InternalGetModuleBase()
	{
		static uintptr_t s_moduleBase = (GetModuleBaseAddress(GetCurrentProcessId(), L"RocketLeague.exe"));
		return s_moduleBase;
	}

	// Single pointer path
	struct PointerPath
	{
		PointerPath() = default;
		PointerPath(ptrdiff_t inModuleOffset, std::initializer_list<uint16_t> inOffsets)
			: moduleOffset(inModuleOffset), offsets(inOffsets)
		{ }

		[[nodiscard]] void* get() const
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

		template <typename T>
		[[nodiscard]] T* get() const { return static_cast<T*>(get()); }

	private:
		ptrdiff_t moduleOffset = 0;
		std::vector<uint16_t> offsets;
	};

}

uint32_t* testPointerPath()
{
	// Pointer path to scroll value while in inventory menu
	static const test_ns::PointerPath path =
		test_ns::PointerPath(0x0237BBE8, { 0x120, 0x28, 0x10, 0x198, 0x6E8, 0x20, 0xED0, 0x278, 0x284 });

	return path.get<uint32_t>();
}
