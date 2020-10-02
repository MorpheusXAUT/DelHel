#pragma once

#include <string>
#include <map>

#include "sid.h"

struct airport {
	std::string icao;
	std::map<std::string, sid> sids;
};