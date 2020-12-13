#pragma once

#include <string>
#include <vector>
#include <iostream>

class route_entry
{
public:
	std::string name;
	std::vector<route_entry> waypoints;
	bool airway;
	double direction;
	double distance;
	int rfl;
	int speed;

	route_entry();
	route_entry(std::string name);
	route_entry(std::string name, double direction, double distance);
	route_entry(std::string name, int rfl, int speed);

	friend std::ostream& operator<<(std::ostream& os, const route_entry& re);
};

