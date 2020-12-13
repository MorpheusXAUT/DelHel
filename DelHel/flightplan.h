#pragma once

#include <string>
#include <vector>
#include <regex>
#include <iostream>

#include "EuroScope/EuroScopePlugIn.h"

#include "route_entry.h"

class flightplan
{
public:
	std::string callsign;
	std::vector<route_entry> route;
	double direction;
	double distance;

	flightplan(std::string callsign, const EuroScopePlugIn::CFlightPlanExtractedRoute& route, std::string rawRoute);
	flightplan(const char* callsign, const EuroScopePlugIn::CFlightPlanExtractedRoute& route, const char* rawRoute);
	
	void ParseRoute(const EuroScopePlugIn::CFlightPlanExtractedRoute& route, std::string rawRoute);

	friend std::ostream& operator<<(std::ostream& os, const flightplan& fp);
};

