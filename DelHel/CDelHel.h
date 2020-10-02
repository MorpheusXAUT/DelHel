#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <future>
#include <fstream>
#include <filesystem>

#include "EuroScope/EuroScopePlugIn.h"
#include "semver/semver.hpp"
#include "nlohmann/json.hpp"

#include "constants.h"
#include "helpers.h"
#include "airport.h"
#include "validation_result.h"

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

private:
	bool debug;
	bool updateCheck;
	bool assignNap;
	std::future<std::string> latestVersion;
	std::map<std::string, airport> airports;

	void LoadSettings();
	void SaveSettings();
	void ReadAirportConfig();

	validation_result ValidateFlightPlan(const EuroScopePlugIn::CFlightPlan& fp);
	void ProcessFlightPlan(const EuroScopePlugIn::CFlightPlan& fp, bool nap);

	void LogMessage(std::string message);
	void LogMessage(std::string message, std::string handler);
	void LogDebugMessage(std::string message);
	void LogDebugMessage(std::string message, std::string type);

	void CheckForUpdate();
};

