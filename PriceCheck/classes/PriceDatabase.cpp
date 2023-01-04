#include <pch.h>
#include "ThirdParty/HTTPRequest.hpp"

#include "classes/PriceDatabase.h"
#include <json.hpp>

const ApiProductPriceData& SafePricePointer::getPriceData()
{
	const PriceDatabase& pdb = PriceDatabase::getInstance();

	if (pdb.pricePtrValiditySequenceNumber != cachedSequenceNumber)
	{
		cachedData = internalResolveCache();
		cachedSequenceNumber = pdb.pricePtrValiditySequenceNumber;
	}

	return *cachedData;
}

ApiProductPriceData* SafePricePointer::internalResolveCache() const
{
	PriceDatabase& pdb = PriceDatabase::getInstance();

	{
		std::lock_guard lkDb(pdb.lockDb);

		const auto existingData = pdb.priceMap.find(productId);
		if (existingData != pdb.priceMap.end())
			return pdb.prices.data() + existingData->second;
	}

	return pdb.requestLoad(productId);
}

PriceDatabase& PriceDatabase::getInstance()
{
	static PriceDatabase db;
	return db;
}

void PriceDatabase::startApiThread()
{
	apiThreadRun.store(true);
	apiThread = std::thread(&PriceDatabase::apiThreadMainProc, this);
}

void PriceDatabase::killApiThread()
{
	apiThreadRun.store(false);
	lockRequestsChanged.notify_all();
	apiThread.join();
}

void PriceDatabase::updateTimestamp(size_t newTimestamp)
{
	newTimestamp = (newTimestamp / 3600) * 3600;
	if (newTimestamp == timestamp)
		return;

	std::lock_guard lkDb(lockDb);

}

void PriceDatabase::apiThreadMainProc()
{
	constexpr size_t MAX_ITEMS_PER_REQUEST = 8;
	constexpr size_t MAX_REQUEST_LEN = 512;

	struct Request { PriceDataIdx idx; ApiProductId id; };
	std::vector<Request> requestedIds;			// Ids that user asked for
	std::vector<PriceDataIdx> resolvedPrices;	// Indices that were

	const std::unique_ptr<char[]> requestBuffer = std::make_unique<char[]>(MAX_REQUEST_LEN);
	requestedIds.reserve(MAX_ITEMS_PER_REQUEST);

	LOG("STARTED API THREAD");

	while (apiThreadRun.load())
	{
		requestedIds.clear();
		resolvedPrices.clear();

		// Locking the requests
		{
			// Wait for updates in the requests
			std::unique_lock lk(lockRequests);
			lockRequestsChanged.wait(lk, [&] {
				return !apiThreadRun.load() || !priceRequests.empty();
			});

			// Leave now requested
			if (!apiThreadRun.load())
				break;

			// Process the requests, store that in own memory
			std::lock_guard lkDb(lockDb);
			for (PriceDataIdx requestIdx : priceRequests)
			{
				if (requestedIds.size() >= MAX_ITEMS_PER_REQUEST)
					break;

				if (prices[requestIdx].resolved)
				{
					resolvedPrices.push_back(requestIdx);
					continue;
				}

				requestedIds.push_back({ requestIdx, prices[requestIdx].id });
			}
		}

		// Prepare request string
		char* buf = requestBuffer.get();
		for (size_t i = 0; i < requestedIds.size(); ++i)
		{
			const ApiProductId id = requestedIds[i].id;
			buf = fmt::format_to_n(buf, MAX_REQUEST_LEN - (buf - requestBuffer.get()) - 1,
				"{}{}.{}.{}", (i > 0) ? "," : "", id.productId, id.paintId, id.isBlueprint).out;
		}
		(*buf) = 0;

		//std::string requestParams = std::string{ requestBuffer.get(), buf };
		std::string requestParams = fmt::format("http://localhost/?l={}", std::string{ requestBuffer.get(), buf });
		LOG("SENT REQUEST: {}", requestParams);

		// Process the request
		http::Request request{ requestParams };
		const http::Response response = request.send("GET");
		std::string responseStr = std::string{ response.body.begin(), response.body.end() };

		if (response.status.code == http::Status::Ok)
		{
			LOG("GOT RESPONSE: {}", responseStr);
			nlohmann::json json = nlohmann::json::parse(responseStr);

			// Lock the requests
			std::unique_lock lk(lockRequests);

			// Lock db again to process changes
			{
				std::lock_guard lkDb(lockDb);

				for (auto v : json)
				{
					ApiProductId id;
					id.productId = v["i"].get<int32_t>();
					id.paintId = v["p"].get<int32_t>();
					id.isBlueprint = v["b"].get<int32_t>();

					auto toResolve = priceMap.find(id);
					if (toResolve == priceMap.end())
						continue;

					ApiProductPriceData& pd = prices[toResolve->second];
					pd.resolved = true;
					for (int i = 0; i < 2; ++i)
					{
						pd.price[i] = v["pri"][i].get<double>();
						pd.vol[i] = v["vol"][i].get<int32_t>();
					}
					resolvedPrices.push_back(toResolve->second);
				}
			}

			// After unlocking db, remove pending requests while still locked
			for (PriceDataIdx idx : resolvedPrices)
				priceRequests.erase(idx);
		}
		else
		{
			LOG("BAD RESPONSE: {}", responseStr);
		}
	}
}

size_t PriceDatabase::newPriceData()
{
	std::lock_guard lkDb(lockDb);

	const size_t index = prices.size();
	prices.push_back({});

	pricePtrValiditySequenceNumber++;
	return index;
}

ApiProductPriceData* PriceDatabase::requestLoad(const ApiProductId& inId)
{
	std::unique_lock lk(lockRequests);
	std::lock_guard lkDb(lockDb);

	const size_t newIndex = newPriceData();
	ApiProductPriceData* data = prices.data() + newIndex;
	data->id = inId;
	data->resolved = false;

	priceMap[inId] = newIndex;

	priceRequests.insert(newIndex);
	lockRequestsChanged.notify_all();

	return data;
}
