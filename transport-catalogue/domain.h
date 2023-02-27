#pragma once
#include "geo.h"

#include <string>
#include <vector>

namespace domain {

struct Stop {
	std::string stop_name;
	geo::Coordinates coordinates;
};

enum class RouteType {
	Ring,
	Line
};

struct Bus {
	std::string bus_name;
	std::vector<Stop*> route_;
	RouteType route_type;
};

struct BusInfo {
	const Bus* bus;
	int unique_stops = 0;
	int count_stops = 0;
	double geo_length = 0;
	double real_length = 0;
	double curvature = 0;
	std::string route_type;
};

struct StopInfo {
	Stop* stop;
	std::vector<std::string_view> buses_to_stop;
	bool no_bus;
};

struct Distance {
	std::pair<const domain::Stop*, const domain::Stop*> from_to;
	size_t value = 0;
};
}
