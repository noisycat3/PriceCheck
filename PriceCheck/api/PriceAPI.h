#include <nlohmann/json.hpp>
#include "APIClasses.h"

class PriceAPI
{
public:
	PriceAPI(std::shared_ptr<CVarManagerWrapper> _cvar, std::shared_ptr<GameWrapper> _gw);
	std::string lastUpdated;

	void LoadData();

	void Refresh();

	Item FindItem(std::string id);
	Item FindItem(int id);

	Item FindBlueprint(std::string id);
	Item FindBlueprint(int id);

private:
	std::map<std::string, Item> _blueprintData;
	std::map<std::string, Item> _priceData;

	std::shared_ptr<CVarManagerWrapper> _cvar;
	std::shared_ptr<GameWrapper> _gw;
	httplib::SSLClient cli;

	intmax_t _last_refresh = 0;
	bool _fetching = false;
	int _error_count = 0;

	void SetUpParser();

	bool CheckTimeStamp(intmax_t last_update);

	// When initial data is loaded
	void OnLoadData(APIData data);

	void OnResponseError();
};
