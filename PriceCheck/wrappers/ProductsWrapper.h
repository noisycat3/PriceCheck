#pragma once

class ProductsWrapper : public ObjectWrapper {
public:
	CONSTRUCTORS(ProductsWrapper)

	//BEGIN SELF IMPLEMENTED
	_NODISCARD bool IsNull() const;
	explicit operator bool() const;
	//END SELF IMPLEMENTED

	[[nodiscard]] static int32_t* getInventoryScrollOffsetPtr();
};