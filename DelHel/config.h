#pragma once

#include <string>
#include <map>

struct config_sid
{
	std::string wp;
	std::map <std::string, int> rwyPrio;
};

struct config
{
	std::string def;
	std::map <std::string, config_sid> sids;
};