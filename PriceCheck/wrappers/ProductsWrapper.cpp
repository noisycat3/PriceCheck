#include "pch.h"
#include "wrappers/ProductsWrapper.h"

#include "WrapperUtil.h"

ProductsWrapper::ProductsWrapper(std::uintptr_t mem)
	: ObjectWrapper(mem)
{
}

ProductsWrapper::ProductsWrapper(const ProductsWrapper& other)
	: ObjectWrapper(other)
{
}

ProductsWrapper& ProductsWrapper::operator=(ProductsWrapper rhs)
{
	memory_address = rhs.memory_address;
	return *this;
}

ProductsWrapper::~ProductsWrapper() = default;

bool ProductsWrapper::IsNull() const
{
	return memory_address != 0;
}

ProductsWrapper::operator bool() const
{
	return !IsNull();
}

size_t ProductsWrapper::getInventoryScrollOffset()
{
	//const PointerPath pp = PointerPath(0x0237BBE0, { 0x120, 0x28, 0x10, 0x198, 0x6E8, 0x20, 0xEA8, 0x278, 0x284 });
	const PointerPath pp = PointerPath(0x0237BBE0, { 0x120, 0x28, 0xC8, 0x198, 0x6E8, 0x20, 0xEA8, 0x278, 0x284 });
	//const PointerPath pp = PointerPath(0x0237BBE0, { 0x30, 0x80, 0x20, 0x80, 0xC0, 0x170, 0x58, 0x10, 0x284 });
	LOG("SCROLL: {}, {}", pp.get(), *pp.get<int32_t>());
	return *pp.get<int32_t>();
}
