#pragma once

#include <string>
#include <map>

struct sidinfo {
	std::string rwy;
	std::string dep;
	std::string nap;
	int prio{};
};

struct sid {
	std::string wp;
	int cfl{};
	std::map<std::string, sidinfo> rwys;
};