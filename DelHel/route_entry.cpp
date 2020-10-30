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
