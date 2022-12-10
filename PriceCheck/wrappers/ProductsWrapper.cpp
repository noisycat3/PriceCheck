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

int32_t* ProductsWrapper::getInventoryScrollOffsetPtr()
{
	MAKE_VPP(scrollOffset, 
		VPP_CASE(221202.34185.407707, 
			PointerPath(),
			PointerPath(0x0237BBE8, { 0x120, 0x28, 0x10, 0x198, 0x6E8, 0x20, 0xED0, 0x278, 0x284 })
		)
	);

	return scrollOffset.get<int32_t>();
}
