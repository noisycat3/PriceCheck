#include "pch.h"
#include "APIClasses.h"

/*
* Many thanks to Martinn for guiding and helping 
* me with the Nlohmann JSON library
*/
#define J(var) j.at(#var).get_to(p.var);
#define J2(var, var2) j.at(#var).get_to(p.var2);
#define OBJ(var) j.get_to(p.var);

#define JOPT(var) if(j.find(#var) != j.end()) { j.at(#var).get_to(p.var); }
#define JOPT2(var, var2) if(j.find(#var) != j.end()) { j.at(#var).get_to(p.var2); }

ITEMPAINT ToItemPaint(const string& id)
{
  // Notes on unordered_map vs map:
  // For lookup tables/relative static maps use unordered -> faster
  // If need tons of insertions/deletions -> use map
  static std::unordered_map<string, ITEMPAINT> table
  {
    {"0", ITEMPAINT::DEFAULT},
    {"1", ITEMPAINT::CRIMSON},
    {"2", ITEMPAINT::LIME},
    {"3", ITEMPAINT::BLACK},
    {"4", ITEMPAINT::SKYBLUE},
    {"5", ITEMPAINT::COBALT},
    {"6", ITEMPAINT::BURNTSIENNA},
    {"7", ITEMPAINT::FORESTGREEN},
    {"8", ITEMPAINT::PURPLE},
    {"9", ITEMPAINT::PINK},
    {"a", ITEMPAINT::ORANGE},
    {"b", ITEMPAINT::GREY},
    {"c", ITEMPAINT::TITANIUMWHITE},
    {"d", ITEMPAINT::SAFFRON},
  };
  if (const auto it = table.find(id); it != table.end())
  {
    return it->second;
  }
  throw std::invalid_argument("Argument to paint mapping is unknown");
}
/// <summary>
/// Currently no need in production code.
/// String paints aren't used anywhere else but development logging.
/// </summary>
/// <param name="paint"></param>
/// <returns>Paint name as seen in Rocket League</returns>
string PaintToString(ITEMPAINT paint)
{
  switch (paint) 
  {
  case ITEMPAINT::DEFAULT:
    return "";
  case ITEMPAINT::CRIMSON:
    return "Crimson";
  case ITEMPAINT::LIME:
    return "Lime";
  case ITEMPAINT::BLACK:
    return "Black";
  case ITEMPAINT::SKYBLUE:
    return "Sky Blue";
  case ITEMPAINT::COBALT:
    return "Cobalt";
  case ITEMPAINT::BURNTSIENNA:
    return "Burnt Sienna";
  case ITEMPAINT::FORESTGREEN:
    return "Forest Green";
  case ITEMPAINT::PURPLE:
    return "Purple";
  case ITEMPAINT::PINK:
    return "Pink";
  case ITEMPAINT::ORANGE:
    return "Orange";
  case ITEMPAINT::GREY:
    return "Grey";
  case ITEMPAINT::TITANIUMWHITE:
    return "Titanium White";
  case ITEMPAINT::SAFFRON:
    return "Saffron";
  }
  return "???";
}

void from_json(const json j, PaintPrice& p)
{
  try 
  {
    auto prices = j.get<std::vector<int>>();
    if (prices.size() != 2)
    {
      LOG("PaintPrice seems invalid, should have 2 values");
      return;
    }
    p.min = prices[0];
    p.max = prices[1];
  }
  catch (std::exception& e)
  {
    LOG("Exeption in {}[line{}]: {}", __FUNCTION__, __LINE__, e.what());
  }
}

void from_json(const json j, Item& p)
{
  try 
  {
    auto tmp_data = j.get<std::map<string, PaintPrice>>();
    for (auto& [id, prices] : tmp_data)
    {
      auto item_paint = ToItemPaint(id);
      p.data[item_paint] = prices;
    }
  }
  catch (std::exception& e)
  {
    LOG("Exeption in {}[line{}]: {}", __FUNCTION__, __LINE__, e.what());
  }
}

void from_json(const json j, APIData& p)
{
  J2(error, isError);
  J2(time, last_refresh);
  J(items);
  J(prints);

  for (auto& [id, item] : p.items)
  {
    item.id = id;
    item.isError = p.isError;
    item.last_refresh = p.last_refresh;
  }

  for (auto& [id, bp] : p.prints)
  {
    bp.id = id;
    bp.isError = p.isError;
    bp.last_refresh = p.last_refresh;
  }
}

#undef J
#undef J2
#undef JOPT
#undef JOPT2
#undef OBJ