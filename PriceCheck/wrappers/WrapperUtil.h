#pragma once
#include <vector>

struct PointerPath
{
	PointerPath() = default;
	PointerPath(ptrdiff_t inModuleOffset, std::initializer_list<uint16_t> inOffsets)
		: moduleOffset(inModuleOffset), offsets(inOffsets)
	{ }

	[[nodiscard]] void* get() const;

	template <typename T>
	[[nodiscard]] T* get() const { return static_cast<T*>(get()); }

private:
	ptrdiff_t moduleOffset = 0;
	std::vector<uint16_t> offsets;
};




