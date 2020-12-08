#include "pch.h"

#include "route_entry.h"

route_entry::route_entry() : name(""), airway(false), direction(0.0), distance(0.0), rfl(0), speed(0)
{
}

route_entry::route_entry(std::string name) : name(name), airway(false), direction(0.0), distance(0.0), rfl(0), speed(0)
{
}

route_entry::route_entry(std::string name, double direction, double distance) : name(name), airway(false), direction(direction), distance(distance), rfl(0), speed(0)
{
}

route_entry::route_entry(std::string name, int rfl, int speed) : name(name), airway(false), direction(0.0), distance(0.0), rfl(rfl), speed(speed)
{
}

std::ostream& operator<<(std::ostream& os, const route_entry& re)
{
	os << re.name;

	if (re.airway) {
		os << "[aw]";
	}

	if (re.waypoints.size() > 0) {
		os << "(";
		for (int i = 0; i < re.waypoints.size(); i++) {
			os << re.waypoints[i];
			if (i < re.waypoints.size() - 1) {
				os << ",";
			}
		}
		os << ")";
	}

	return os;
}
