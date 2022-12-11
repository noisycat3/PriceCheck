#pragma once
#include "bakkesmod/wrappers/items/OnlineProductWrapper.h"

struct Info
{
	int id = 0;
	ITEMPAINT paint = ITEMPAINT::DEFAULT;
  int quality = 0;
};

class TradeItem : public OnlineProductWrapper
{
public:
  TradeItem(const OnlineProductWrapper& i) : OnlineProductWrapper(i) { m_info = updateItemInfo(); }

  PaintPrice GetPrice();
  std::string GetPaint();
  COLOR GetPaintColor();
  COLOR GetQualityColor();
  std::string GetSeries();

private:
  Info m_info = Info();
  Info updateItemInfo();
  std::string ToSeriesString(const int& id);
  std::vector<int> SeriesToItems(const int& id);
};