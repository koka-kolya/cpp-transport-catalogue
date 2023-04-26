#pragma once
#include "domain.h"
#include <algorithm>
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

class StopHasher {
public:
	using Pair_Stops = std::pair<const domain::Stop*, const domain::Stop*>;
	std::size_t operator()(Pair_Stops pointers) const noexcept {
		std::size_t hs1 = hash_pointer_(pointers.first);
		std::size_t hs2 = hash_pointer_(pointers.second);
		return hs1 * 37 + hs2 * 37 * 37;
	}
	std::size_t operator()(const domain::Stop* stop) const noexcept {
		return hash_pointer_(stop) * 37;
	}

private:
	std::hash<const void*> hash_pointer_;
};
} // namespace detail

class TransportCatalogue {
	struct StopCount {
		size_t stop_count = 0;
		size_t unique_stops = 0;
	};
public:
	using AllStops = std::deque<domain::Stop>;
	using AllBuses = std::deque<domain::Bus>;
	using AllBusesInfo = std::unordered_map<std::string_view, domain::BusInfo>;
	using FromTo = std::pair<const domain::Stop*, const domain::Stop*>;
	using Buses = std::unordered_set<domain::Bus*>;
	using Distances = std::unordered_map<FromTo, double, detail::StopHasher>;

	TransportCatalogue();

	void AddStop(const domain::Stop& stop);
	void AddBus(const domain::Bus& bus);

	domain::Stop* FindStop(std::string_view stop_name) const;
	domain::Stop* FindStopById(size_t id);
	domain::Bus* FindBus(std::string_view bus_name) const;

	domain::BusInfo GetBusInfo(std::string_view bus_name) const;
	domain::StopInfo GetStopInfo(std::string_view stop_name) const;

	void SetDistances (std::string_view from, std::string_view to, double dist);
	void SetBusesInfo();

	AllBusesInfo GetAllBusesInfo() const;
	AllBuses GetAllBuses() const;
	AllStops GetAllStops() const;

	size_t GetStopCounts() const;
	size_t GetBusCounts() const;

	const Distances* GetAllDistances();
	double GetDistanceForPairStops(const domain::Stop *from, const domain::Stop *to) const;

private:
	AllStops stops_ {};
	AllBuses buses_ {};
	std::unordered_map<std::string_view, domain::Stop*> stopname_to_stop_ {};
	std::unordered_map<std::string_view, domain::Bus*> busname_to_bus_ {};
	std::unordered_map<domain::Stop*, Buses> stop_to_buses_ {};
	Distances distances_ {};
	AllBusesInfo buses_info_ {};

	double GetDistance (const domain::Stop* stop_from, const domain::Stop* stop_to) const;
	size_t GetDistanceFromTo(const domain::Stop* from,
							 const domain::Stop* to) const;
	StopCount GetStopCount (const domain::Bus* bus) const;
	double GetRealRouteLength (const domain::Bus* bus) const;
	double GetGeoRouteLength (const domain::Bus* bus) const;
	void MakeBusInfo(const domain::Bus* bus);
};
}  // namespace data_base
