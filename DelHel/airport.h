#pragma once

#include <string>
#include <map>
#include <set>
#include <regex>

#include "sid.h"

struct airport {
	std::string icao;
	std::map<std::string, sid> sids;
	std::set<std::string> rwys;
	std::regex rwy_regex;
};