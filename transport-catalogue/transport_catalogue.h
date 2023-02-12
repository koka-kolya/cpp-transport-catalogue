#pragma once
#include "geo.h"
#include <deque>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace data_base {
namespace detail {

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
	Bus* bus;
	int unique_stops = 0;
	int count_stops = 0;
	double geo_length = 0;
	double real_length = 0;
	double curvature = 0;
};

struct StopInfo {
	Stop* stop;
	std::vector<std::string_view> buses_to_stop;
	bool no_bus;
};

struct Distance {
	std::pair<const Stop*, const Stop*> from_to;
	size_t value = 0;
};

class StopHasher {
public:
	using Pair_Stops = std::pair<const Stop*, const Stop*>;
	std::size_t operator()(Pair_Stops pointers) const noexcept {
		std::size_t hs1 = hash_pointer_(pointers.first);
		std::size_t hs2 = hash_pointer_(pointers.second);
		return hs1 * 37 + hs2 * 37 * 37;
	}
	std::size_t operator()(const Stop* stop) const noexcept {
		return hash_pointer_(stop) * 37;
	}

private:
	std::hash<const void*> hash_pointer_;
};
} // end data_base::details namespace

class TransportCatalogue {


	struct StopCount {
		size_t stop_count = 0;
		size_t unique_stops = 0;
	};
public:
	TransportCatalogue();
	void PrintAllStops();
	void AddStop(const detail::Stop& stop);
	void AddBus(const detail::Bus& bus);
	detail::Stop* FindStop(std::string_view stop_name) const;
	detail::Bus* FindBus(std::string_view bus_name) const;
	detail::BusInfo GetBusInfo(std::string_view bus_name) const;
	detail::StopInfo GetStopInfo(std::string_view stop_name) const;
	void SetDistances (detail::Distance& distance);
	detail::Distance GetDistance (const detail::Stop* stop_from,
								  const detail::Stop* stop_to) const;
	size_t GetDistanceFromTo(const detail::Stop* from,
							 const detail::Stop* to) const;

private:
	using All_Stops = std::deque<detail::Stop>;
	using All_Buses = std::deque<detail::Bus>;
	using Pair_Stops = std::pair<const detail::Stop*, const detail::Stop*>;
	using Buses = std::unordered_set<detail::Bus*>;
	All_Stops stops_ {};
	All_Buses buses_ {};
	std::unordered_map<std::string_view, detail::Stop*> stopname_to_stop_ {};
	std::unordered_map<std::string_view, detail::Bus*> busname_to_bus_ {};
	std::unordered_map<detail::Stop*, Buses> stop_to_buses_ {};
	std::unordered_map<Pair_Stops, int, detail::StopHasher> distances_ {};

	StopCount GetStopCount (const detail::Bus* bus) const;
	double GetRealRouteLength (const detail::Bus* bus) const;
	double GetGeoRouteLength (const detail::Bus* bus) const;

};
}
