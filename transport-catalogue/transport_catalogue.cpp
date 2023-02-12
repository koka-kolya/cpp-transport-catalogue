#include "transport_catalogue.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <unordered_set>

namespace data_base {

TransportCatalogue::TransportCatalogue()
{
}

void TransportCatalogue::AddStop(const detail::Stop& stop) {
	if (stopname_to_stop_.count(stop.stop_name) != 0) return;
	stops_.emplace_back(std::move(stop));
	stopname_to_stop_.insert({stops_.back().stop_name, &stops_.back()});
}

void TransportCatalogue::AddBus(const detail::Bus& bus) {
	if (busname_to_bus_.count(bus.bus_name) != 0) return;
	buses_.emplace_back(std::move(bus));
	busname_to_bus_.emplace(buses_.back().bus_name, &buses_.back());
	for (detail::Stop* stop : bus.route_) {
		stop_to_buses_[stop].insert(&buses_.back());
	}
}

detail::Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
	if (stopname_to_stop_.count(stop_name) == 0) {
		return nullptr;
	}
	return stopname_to_stop_.at(stop_name);
}

detail::Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
	if (busname_to_bus_.count(bus_name) == 0) {
		return nullptr;
	}
	return busname_to_bus_.at(bus_name);
}

detail::BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
	detail::BusInfo output {};
	if (busname_to_bus_.count(bus_name) == 0) {
		output.bus = nullptr;
		return output;
	}
	detail::Bus* bus = FindBus(bus_name);
	output.bus = bus;
	StopCount stop_count = GetStopCount(bus);
	output.count_stops = stop_count.stop_count;
	output.unique_stops = stop_count.unique_stops;
	if (output.count_stops > 1) {
		output.geo_length = GetGeoRouteLength(bus);
		output.real_length = GetRealRouteLength(bus);
		output.curvature = output.real_length / output.geo_length;
	} else {
		output.geo_length = 0;
		output.real_length = GetRealRouteLength(bus);
	}
	return output;
}

detail::StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const  {
	detail::StopInfo output;
	detail::Stop* stop = FindStop(stop_name);
	if (stop == nullptr) {
		output.stop = nullptr;
		return output;
	}
	if (stop_to_buses_.count(stop) == 0) {
		output.stop = stop;
		output.no_bus = true;
		return output;
	}
	for (const detail::Bus* bus : stop_to_buses_.at(stop)) {
		output.buses_to_stop.push_back(bus->bus_name);
	}
	std::sort(output.buses_to_stop.begin(), output.buses_to_stop.end());
	output.stop = stop;
	output.no_bus = false;
	return output;
}

TransportCatalogue::StopCount
TransportCatalogue::GetStopCount (const detail::Bus* bus) const{
	StopCount output {};
	bool is_ring_route = bus->route_type == detail::RouteType::Ring;
	output.stop_count = is_ring_route ?
				bus->route_.size() : bus->route_.size() * 2 - 1;
	std::unordered_set<const detail::Stop*> unique_stops;
	unique_stops.insert(bus->route_.begin(), bus->route_.end());
	output.unique_stops = unique_stops.size();
	return output;
}

double TransportCatalogue::GetGeoRouteLength (const detail::Bus* bus) const {
	double length = 0;
	for (size_t i = 1; i < bus->route_.size(); ++i) {
		length += (ComputeDistance(bus->route_[i - 1]->coordinates,
				   bus->route_[i]->coordinates));
	}
	if (bus->route_type == detail::RouteType::Line) {
		length *= 2;
	}
	return length;
}

void data_base::TransportCatalogue::SetDistances (detail::Distance& distance) {
	if (distances_.count({distance.from_to.second, distance.from_to.first}) != 0) {
		if (distances_.at({distance.from_to.second, distance.from_to.first}) == distance.value) {
			return;
		}
	}
	distances_.insert({distance.from_to, distance.value});
}

size_t TransportCatalogue::GetDistanceFromTo(const detail::Stop* from,
											 const detail::Stop* to) const {
	if (distances_.count({from, to}) > 0) {
		return distances_.at({from, to});
	} else {
		return 0u;
	}
}

detail::Distance
TransportCatalogue::GetDistance(const detail::Stop *stop_from,
								const detail::Stop* stop_to) const {
	detail::Distance dist;
	dist.from_to = {stop_from, stop_to};
	size_t dist_val = GetDistanceFromTo(stop_from, stop_to);
	dist.value = dist_val > 0 ? dist_val : GetDistanceFromTo(stop_to, stop_from);
	return dist;
}

double TransportCatalogue::GetRealRouteLength (const detail::Bus* bus) const {
	double length = 0;
	for (int i = 1; i < bus->route_.size(); ++i) {
		length += GetDistance(bus->route_[i - 1], bus->route_[i]).value;
	}
	if (bus->route_type == detail::RouteType::Ring) {
		return length;
	} else {
		for (int i = bus->route_.size(); i > 1; --i) {
			length += GetDistance(bus->route_[i - 1], bus->route_[i - 2]).value;
		}
		double check_last_stop =
				GetDistance(bus->route_[bus->route_.size() - 1],
							bus->route_[bus->route_.size() - 1]).value;
		if (check_last_stop > 0) {
			length += check_last_stop;
		}
	}
	return length;
}
}
