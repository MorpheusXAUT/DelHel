#pragma once

#include "EuroScope/EuroScopePlugIn.h"

class RadarScreen : public EuroScopePlugIn::CRadarScreen
{
public:
	RadarScreen();
	virtual ~RadarScreen();

	inline void OnAsrContentToBeClosed() { delete this; }
};