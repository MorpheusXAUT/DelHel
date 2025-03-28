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
	std::map <std::string, config_sid> sids;
};