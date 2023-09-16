#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <future>
#include <fstream>
#include <filesystem>
#include <algorithm>

#include "EuroScope/EuroScopePlugIn.h"
#include "semver/semver.hpp"
#include "nlohmann/json.hpp"

#include "constants.h"
#include "helpers.h"
#include "airport.h"
#include "validation.h"
#include "flightplan.h"
#include "sid.h"
#include "RadarScreen.h"

using json = nlohmann::json;
using namespace std::chrono_literals;

class CDelHel : public EuroScopePlugIn::CPlugIn
{
public:
	CDelHel();
	virtual ~CDelHel();

	bool OnCompileCommand(const char* sCommandLine);
	void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData, char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
	void OnTimer(int Counter);
	void OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan FlightPlan);
	void OnAirportRunwayActivityChanged();
	EuroScopePlugIn::CRadarScreen* OnRadarScreenCreated(const char* sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved, bool CanBeCreated);

private:
	bool debug;
	bool updateCheck;
	bool assignNap;
	bool autoProcess;
	bool warnRFLBelowCFL;
	bool logMinMaxRFL;
	bool checkMinMaxRFL;
	bool flashOnMessage;
	bool squawkAssignmentCCAMS;
	std::future<std::string> latestVersion;
	std::map<std::string, airport> airports;
	std::vector<std::string> processed;
	RadarScreen* radarScreen;

	void LoadSettings();
	void SaveSettings();
	void ReadRoutingConfig();
	void ReadAirportConfig();
	void UpdateActiveAirports();

	validation ProcessFlightPlan(EuroScopePlugIn::CFlightPlan& fp, bool nap, bool validateOnly = false);
	bool CDelHel::CheckFlightPlanProcessed(const EuroScopePlugIn::CFlightPlan& fp);
	bool IsFlightPlanProcessed(const EuroScopePlugIn::CFlightPlan& fp);
	void AutoProcessFlightPlans();

	void LogMessage(std::string message);
	void LogMessage(std::string message, std::string handler);
	void LogDebugMessage(std::string message);
	void LogDebugMessage(std::string message, std::string type);

	void CheckForUpdate();
};

