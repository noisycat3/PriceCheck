#pragma once

#include <cstdint>
#include <unordered_set>

struct ApiProductId
{
	int32_t productId			= 0;
	int32_t paintId		: 31	= 0;
	int32_t isBlueprint	: 1		= 0;

	bool operator==(const ApiProductId&) const = default;
	bool operator!=(const ApiProductId&) const = default;
};

template<>
struct std::hash<ApiProductId>
{
	size_t operator()(const ApiProductId& x) const noexcept
	{
		// We can fit all data without collision in size_t as we are targeting x64
		static_assert(sizeof(ApiProductId) == sizeof(size_t), "Hash assumes we are same size as size_t!");
		return *reinterpret_cast<const size_t*>(&x);
	}
};

struct ApiProductPriceData
{
	ApiProductId id;
	bool resolved;

	double price[2];
	int32_t vol[2];
};

class PriceDatabase;
struct SafePricePointer
{
	SafePricePointer(const ApiProductId& inProductId)
		: productId(inProductId), cachedData(nullptr) { }

	[[nodiscard]] const ApiProductId& getProductId() const { return productId; }
	[[nodiscard]] const ApiProductPriceData& getPriceData();

private:
	ApiProductId productId;

	size_t cachedSequenceNumber = 0;
	ApiProductPriceData* cachedData;

	[[nodiscard]] ApiProductPriceData* internalResolveCache() const;
};

class PriceDatabase
{
public:
	friend struct SafePricePointer;
	static PriceDatabase& getInstance();

	void startApiThread();
	void killApiThread();

	void updateTimestamp(size_t newTimestamp);

private:
	using PriceDataIdx = size_t;

	std::thread apiThread;
	std::mutex lockRequests;
	std::condition_variable lockRequestsChanged;

	std::recursive_mutex lockDb;

	std::atomic_bool apiThreadRun;
	void apiThreadMainProc();

	size_t timestamp = 0;
	size_t pricePtrValiditySequenceNumber = 1;
	std::vector<ApiProductPriceData> prices;

	std::unordered_set<PriceDataIdx> priceRequests;
	std::unordered_map<ApiProductId, PriceDataIdx> priceMap;

	size_t newPriceData();
	ApiProductPriceData* requestLoad(const ApiProductId& inId);
};
