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

private:
	bool debug;
	bool updateCheck;
	bool assignNap;
	bool autoProcess;
	std::future<std::string> latestVersion;
	std::map<std::string, airport> airports;
	std::vector<std::string> processed;

	void LoadSettings();
	void SaveSettings();
	void ReadAirportConfig();

	validation ProcessFlightPlan(const EuroScopePlugIn::CFlightPlan& fp, bool nap, bool validateOnly = false);
	bool IsFlightPlanProcessed(const EuroScopePlugIn::CFlightPlan& fp);
	void AutoProcessFlightPlans();

	void LogMessage(std::string message);
	void LogMessage(std::string message, std::string handler);
	void LogDebugMessage(std::string message);
	void LogDebugMessage(std::string message, std::string type);

	void CheckForUpdate();
};

