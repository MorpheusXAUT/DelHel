#pragma once

#include <string>
#include <map>

struct rwy_config_sid
{
	std::string wp;
	std::map <std::string, int> rwyPrio;
};

struct rwy_config
{
	std::string def;
	std::map <std::string, rwy_config_sid> sids;
};