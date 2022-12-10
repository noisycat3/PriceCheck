#include "pch.h"
#include "ItemSeries.h"

string ItemSeriesDatabaseWrapper::ToSeriesString(const int& id)
{
  // Thanks to ItsBranK for providing data
  static std::unordered_map<int, string> series
  {
    {    1, "No" }, // OG items w/o series
    {    2, "Champions 1" },
    {    3, "Champions 2" },
    {    6, "Champions 3" },
    {    7, "Champions 4" },
    {    8, "Player's Choice" },
    {   10, "Turbo" },
    {   11, "Nitro" },
    {   13, "Overdrive" },
    {   18, "Accelerator" },
    {   19, "Velocity" },
    {   20, "Haunted Hallows" },
    {   21, "Secret Santa" },
    {   26, "Victory" },
    {   27, "Spring Fever" },
    {   29, "Triumph" },
    {   33, "Impact" },
    {   34, "RL Beach Blast" },
    {   47, "Golden Egg '18" },
    {   48, "Zephyr" },
    {  190, "Elevation" },
    {  191, "Golden Pumpkin '18" },
    {  207, "Golden Gift '18" },
    {  299, "Ferocity" },
    {  300, "Golden Lantern '19" },
    {  403, "Totally Awesome" },
    {  443, "Golden Egg '19" },
    {  534, "Vindicator" },
    {  541, "Golden Pumpkin '19" },
    {  542, "Golden Gift '19" },
    {  635, "Bonus Gift" },
    {  636, "Revival" },
    {  737, "Ignition" },
    {  855, "Accolade" },
    {  900, "Momentum" },
    {  902, "Golden Egg '20" },
    {  935, "Kismet" },
    { 1032, "Season 1" },
    { 1033, "Golden Pumpkin '20" },
    { 1147, "Accolade II"}, // Tournament item w/o series
    { 1154, "Golden Gift '20" },
    { 1180, "Season 2" },
    { 1181, "Golden Lantern '21" },
    { 1204, "Accolade III" },
  };
  if (const auto it = series.find(id); it != series.end())
  {
    return it->second + " Series";
  }
  return "Unknown series";
}

// Raw API data from ItsBranK, then used parser
// https://github.com/matias-kovero/RLParseRawAPIData
/*
* Special series:
*  931: Black Market Drop         (Capsule)
*  932: Exotic Drop               (Capsule)
*  933: Import Drop               (Capsule)
*  934: Rare Drop                 (Capsule)
*  935: Uncommon Drop             (Capsule)
*  936: Very Rare Drop            (Capsule)
* 1205: Season 3 Prospects Cup    (Tournament)
* 1206: Season 3 Challengers Cup  (Tournament)
* 1207: Season 3 All-Stars Cup    (Tournament)
*/
std::vector<int> ItemSeriesDatabaseWrapper::SeriesToItems(const int& id)
{
  static std::unordered_map<int, std::vector<int>> seriesItems
  {
    { 2, { 1048, 1102, 1130, 1453, 1456, 1047, 1096, 1131, 1423, 1018, 1158, 1295, 517, 820, 1435, 1436, 1437, 1438 } },
    { 3, { 1059, 1087, 1442, 1455, 1458, 1079, 1093, 1062, 1092, 1159, 1300, 1449, 731, 818, 1435, 1436, 1437, 1438 } },
    { 6, { 1501, 1520, 1527, 1532, 1534, 1502, 1052, 1104, 1416, 1522, 1542, 1551, 819, 822, 1435, 1436, 1437, 1438 } },
    { 7, { 1544, 1545, 1546, 1562, 1593, 1561, 1565, 1099, 1443, 1564, 1568, 1584, 1580, 1581, 1435, 1577, 1578 } },
    { 8, { 1059, 1955, 1980, 2551, 2693, 1423, 1951, 1975, 2819, 1856, 2837, 2853, 1580, 1661, 2614, 2854, 2966, 3239, 3453 } },
    { 10, { 1633, 1636, 1637, 1649, 1779, 1550, 1656, 1716, 271, 1066, 1552, 1624, 1625, 1627, 1577, 1578, 1679, 1684 } },
    { 11, { 1769, 1770, 1773, 1774, 1775, 1056, 1694, 1767, 1691, 1736, 1784, 1826, 1661, 1667, 1579, 1679, 1684, 1888 } },
    { 13, { 1917, 1953, 1955, 1978, 1681, 1898, 1899, 1900, 1945, 1638, 1919, 1932, 1655, 1929, 1904, 1905, 1907, 1908 } },
    { 18, { 1980, 2012, 2024, 2336, 2498, 1974, 1975, 2047, 1856, 2054, 2254, 2392, 1604, 2059, 1905, 1908, 2027, 2329 } },
    { 19, { 1963, 2021, 2025, 2363, 1950, 2275, 2359, 1883, 1943, 2383, 2394, 825, 1931, 1907, 2027, 2044, 2349 } },
    { 20, { 2321, 2322, 2323, 2325, 2575, 1875, 2328, 2488, 2023, 2355, 2401, 1655, 2854, 2966, 3239, 3453 } },
    { 20, { 2321, 2322, 2323, 2325, 2575, 1875, 2328, 2488, 2023, 2355, 2401, 1655, 2854, 2966, 3239, 3453 } },
    { 21, { 2489, 2495, 2496, 2500, 2579, 2400, 2441, 2478, 2442, 2481, 2482, 2532, 2854, 2966, 3239, 3453 } },
    { 21, { 2489, 2495, 2496, 2500, 2579, 2400, 2441, 2478, 2442, 2481, 2482, 2532, 2854, 2966, 3239, 3453 } },
    { 26, { 2540, 2547, 2548, 2693, 2588, 1976, 2695, 2070, 2645, 2654, 2732, 2614, 2729, 2501, 2694, 2702, 2736 } },
    { 27, { 2320, 2459, 2550, 2680, 2419, 2060, 2722, 2696, 2711, 2712, 2700, 2703, 2854, 2966, 3239, 3453 } },
    { 27, { 2320, 2459, 2550, 2680, 2419, 2060, 2722, 2696, 2711, 2712, 2700, 2703, 2854, 2966, 3239, 3453 } },
    { 29, { 2340, 2460, 2535, 2581, 2629, 2512, 1060, 2767, 2850, 1894, 2778, 2837, 2758, 2827, 2694, 2702, 2817, 2854 } },
    { 33, { 2339, 2461, 2544, 2672, 2533, 2555, 1981, 2819, 2853, 2899, 2916, 2935, 2971, 2817, 2915, 2966, 2967 } },
    { 34, { 2458, 2596, 2627, 2337, 2474, 1777, 2880, 2513, 2904, 2956, 2918, 2922, 2854, 2966, 3239, 3453 } },
    { 34, { 2458, 2596, 2627, 2337, 2474, 1777, 2880, 2513, 2904, 2956, 2918, 2922, 2854, 2966, 3239, 3453 } },
    { 47, { 5367, 5369, 5366, 5365, 5364 } },
    { 48, { 2345, 2346, 2551, 2679, 2407, 1123, 3072, 3220, 3001, 3031, 3114, 2972, 3079, 2702, 2966, 3071, 3239 } },
    { 190, { 2565, 2948, 3144, 2411, 2917, 2409, 1951, 3342, 3313, 3331, 3451, 3459, 3337, 3370, 3071, 3239, 3453, 3460 } },
    { 191, { 1456, 1458, 1532, 1633, 1636, 1637, 1649, 1769, 1770, 1773, 1774, 1775, 1779, 1056, 1131, 1550, 1656, 1694, 1716, 1767, 271, 1018, 1066, 1416, 1522, 1552, 1568, 1624, 1691, 1736, 1784, 1826, 1580, 1625, 1627, 1661, 1667, 1435, 1577, 1578, 1579, 1679, 1684, 1888 } },
    { 207, { 5367, 5369, 5366, 5365, 5364 } },
    { 299, { 2592, 2963, 3852, 3890, 2514, 3747, 2784, 3338, 3426, 3808, 3847, 3012, 3314, 3453, 3460, 3800, 3854 } },
    { 300, { 5367, 5369, 5366, 5365, 5364 } },
    { 403, { 3378, 3916, 3922, 4136, 4240, 3887, 4127, 4226, 4132, 4143, 4284, 3310, 4202, 3763, 3854, 4133, 4179 } },
    { 443, { 5367, 5369, 5366, 5365, 5364 } },
    { 534, { 3825, 4159, 4249, 4369, 2423, 3878, 4297, 4462, 4268, 4282, 4335, 4173, 4384, 3763, 3800, 4118, 4244 } },
    { 541, { 5367, 5369, 5366, 5365, 5364 } },
    { 542, { 5367, 5369, 5366, 5365, 5364 } },
    { 635, { 5367, 5369, 5366, 5365, 5364 } },
    { 636, { 1059, 1456, 1527, 1898, 2359, 4646, 1092, 1522, 1584, 4717, 731, 1580, 1667, 1684, 2702, 2736, 2817 } },
    { 737, { 3521, 4584, 4732, 4958, 4183, 4208, 4382, 4957, 4241, 4698, 3311, 4386, 4711, 4434, 4524 } },
    { 855, { 3406, 3609, 4016, 4195, 4832, 1768, 3344, 4488, 5293, 3987, 5033, 5165, 5219, 5296, 4336, 4414, 5023, 5176, 5183, 5191, 4221, 5036, 5131 } },
    { 900, { 4622, 4992, 5078, 5306, 5309, 4640, 4726, 5105, 5079, 1132, 5121, 5179, 3997, 4781, 4847, 5184, 5178, 5127, 4549, 4989 } },
    { 902, { 5367, 5369, 5366, 5365, 5364 } },
    { 931, { 5364 } },
    { 932, { 5365, 5364 } },
    { 933, { 5366, 5365, 5364 } },
    { 934, { 5367, 5369, 5366 } },
    { 935, { 5368, 5367, 5369 } },
    { 936, { 5369, 5366, 5365 } },
    { 1032, { 4063, 4493, 4302, 4057, 5613, 5565, 4997, 5337, 5192, 4406, 5069, 5167, 5341, 5266, 5345, 3582, 5460, 5141, 4305, 4870, 4625, 5378 } },
    { 1033, { 5367, 5369, 5366, 5365, 5364 } },
    { 1154, { 5367, 5369, 5366, 5365, 5364 } },
    { 1180, { 5810, 5816, 5815, 5637, 5416, 5827, 5749, 5148, 5114, 5814, 5628, 5348, 4998, 5821, 3702, 5846, 5746, 5674, 5751, 5744, 4552, 5287 } },
    { 1181, { 5367, 5369, 5366, 5365, 5364 } },
    { 1204, { 5826, 4869, 5935, 5851, 5589, 5908, 5930, 5418, 5755, 5173, 4703, 5994, 5550, 5898, 5996, 3701, 4029, 3632, 3826, 5729 } },
    { 1205, { 5826, 4869, 5935, 5851, 5589, 5908, 5930, 5418, 5755, 5173, 4703, 5994, 5550, 5898, 5996, 3701, 4029, 3632, 3826, 5729 } },
    { 1206, { 5826, 4869, 5935, 5851, 5589, 5908, 5930, 5418, 5755, 5173, 4703, 5994 } },
    { 1207, { 5826, 4869, 5935, 5851, 5589, 5908, 5930, 5418, 5755, 5173, 4703 } }
  };
  if (const auto it = seriesItems.find(id); it != seriesItems.end())
  {
    return it->second;
  }
  // Just return empty if can't find
  return std::vector<int>{};
}

string ItemSeriesDatabaseWrapper::QualityToString(const int& quality)
{
  switch (quality)
  {
  case 0:
    return "Unknown";
  case 1:
    return "Uncommon"; // Uncommon
  case 2:
    return "Rare";
  case 3:
    return "Very Rare";
  case 4:
    return "Import";
  case 5:
    return "Exotic";
  case 6:
    return "Black Market";
  case 7:
    return "Unkown";
  case 8:
    return "Limited";
  }
  return "???";
}

ItemSeriesDatabaseWrapper::Series ItemSeriesDatabaseWrapper::GetSeriesItems(const int& id)
{
  auto it = cache.find(id);
  if (it != cache.end())
  {
    // Stuff was in cache!
    return it->second;
  }
  else
  {
    // Not found in cache, get items from PriceAPI
    // Warning!! We are forcing data to cache slot id
    cache[id] = FillSeries(id);
    return cache[id];
  }
}

ItemSeriesDatabaseWrapper::Series ItemSeriesDatabaseWrapper::FillSeries(const int& id)
{
  Series sd = Series();
  sd.id = id;
  sd.name = ToSeriesString(id);

  auto seriesItems = SeriesToItems(id);
  auto pw = _globalGameWrapper->GetItemsWrapper();

  for (const auto& itemId : seriesItems)
  {
    // This could be also blueprints!!
    auto p = pw.GetProduct(itemId);
    auto price = _globalPriceAPI->FindItem(itemId).data;
    // string, int, std::map<ITEMPAINT, PaintPrice>
    sd.items.push_back({
      p.GetLongLabel().ToString(),
      p.GetQuality(),
      price
      });
  }
  LOG("Found: {}({}) with {} items.", sd.name, sd.id, seriesItems.size());
  return sd;
}