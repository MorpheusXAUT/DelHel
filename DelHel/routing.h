#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "route_entry.h"

struct routing {
	std::string adep;
	std::string adest;
	int maxlvl{};
	int minlvl{};
	std::vector<std::string> waypts;

	friend std::ostream& operator<<(std::ostream& os, const routing& rt);
};