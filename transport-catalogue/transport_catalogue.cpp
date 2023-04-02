#include "transport_catalogue.h"

namespace data_base {

TransportCatalogue::TransportCatalogue()
{
}

void TransportCatalogue::AddStop(const domain::Stop& stop) {
	if (stopname_to_stop_.count(stop.stop_name) != 0) return;
	stops_.emplace_back(stop);
	stopname_to_stop_.insert({stops_.back().stop_name, &stops_.back()});
}

void TransportCatalogue::AddBus(const domain::Bus& bus) {
	if (busname_to_bus_.count(bus.bus_name) != 0) return;
	buses_.emplace_back(bus);
	busname_to_bus_.emplace(buses_.back().bus_name, &buses_.back());
	for (domain::Stop* stop : bus.route_) {
		stop_to_buses_[stop].insert(&buses_.back());
	}
}

domain::Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
	if (stopname_to_stop_.count(stop_name) == 0) {
		return nullptr;
	}
	return stopname_to_stop_.find(stop_name)->second;
}

domain::Stop *TransportCatalogue::FindStopById(size_t id)
{
	return FindStop(stops_[id].stop_name);
}

domain::Bus* TransportCatalogue::FindBus(std::string_view bus_name) const {
	if (busname_to_bus_.count(bus_name) == 0) {
		return nullptr;
	}
	return busname_to_bus_.at(bus_name);
}

domain::BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) const {
	if (buses_info_.count(bus_name) == 0) {
		domain::BusInfo output {};
		output.bus = nullptr;
		return output;
	}
	return buses_info_.at(bus_name);
}

domain::StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) const  {
	domain::StopInfo output;
	domain::Stop* stop = FindStop(stop_name);
	if (stop == nullptr) {
		output.stop = nullptr;
		return output;
	}
	if (stop_to_buses_.count(stop) == 0) {
		output.stop = stop;
		output.no_bus = true;
		return output;
	}
	for (const domain::Bus* bus : stop_to_buses_.at(stop)) {
		output.buses_to_stop.push_back(bus->bus_name);
	}
	std::sort(output.buses_to_stop.begin(), output.buses_to_stop.end());
	output.stop = stop;
	output.no_bus = false;
	return output;
}

void data_base::TransportCatalogue::SetDistances (domain::Distance &distance) {
	if (distances_.count({distance.from_to.second, distance.from_to.first}) != 0) {
		if (distances_.at({distance.from_to.second, distance.from_to.first}) == distance.value) {
			return;
		}
	}
	distances_.insert({distance.from_to, distance.value});
}

void data_base::TransportCatalogue::SetDistances (std::string_view from, std::string_view to, double dist) {
	domain::Stop* stop_from = FindStop(from);
	domain::Stop* stop_to = FindStop(to);
	if (distances_.count({stop_to, stop_from}) != 0) {
		if (distances_.at({stop_to, stop_from}) == dist) {
			return;
		}
	}
	distances_.insert({{stop_from, stop_to}, dist});
}

void TransportCatalogue::SetBusesInfo() {
	for(const auto& [_, bus] : busname_to_bus_) {
		if (!bus->route_.empty()) {
			MakeBusInfo(bus);
		}
	}
}

TransportCatalogue::AllBusesInfo TransportCatalogue::GetAllBusesInfo() const {
	return buses_info_;
}

TransportCatalogue::AllBuses TransportCatalogue::GetAllBuses() const {
	return buses_;
}
TransportCatalogue::AllStops TransportCatalogue::GetAllStops() const {
	return stops_;
}

size_t TransportCatalogue::GetStopCounts() const {
	return stops_.size();
}

size_t TransportCatalogue::GetBusCounts() const {
	return buses_.size();
}

double TransportCatalogue::GetDistance(const domain::Stop* stop_from,
												 const domain::Stop* stop_to) const {

	double dist_val = GetDistanceFromTo(stop_from, stop_to);
	return dist_val > 0 ? dist_val : GetDistanceFromTo(stop_to, stop_from);;
}

const TransportCatalogue::Distances* TransportCatalogue::GetAllDistances() {
	Distances* dist_ptr = &distances_;
	return dist_ptr;
}

double TransportCatalogue::GetDistanceForPairStops(const domain::Stop *from,
												   const domain::Stop *to) const {
	auto it_dist = distances_.find({from , to});
	if (it_dist == distances_.end()) {
		it_dist = distances_.find({to, from});
	}
	return it_dist->second;
}

size_t TransportCatalogue::GetDistanceFromTo(const domain::Stop* from,
											 const domain::Stop* to) const {
	if (distances_.count({from, to}) > 0) {
		return distances_.at({from, to});
	}
	return 0u;
}

TransportCatalogue::StopCount
TransportCatalogue::GetStopCount (const domain::Bus* bus) const{
	StopCount output {};
	bool is_ring_route = bus->route_type == domain::RouteType::Ring;
	output.stop_count = is_ring_route ?
				bus->route_.size() : bus->route_.size() * 2 - 1;
	std::unordered_set<const domain::Stop*> unique_stops;
	unique_stops.insert(bus->route_.begin(), bus->route_.end());
	output.unique_stops = unique_stops.size();
	return output;
}

double TransportCatalogue::GetRealRouteLength (const domain::Bus* bus) const {
	double length = 0;
	for (long unsigned int i = 1; i < bus->route_.size(); ++i) {
		length += GetDistance(bus->route_[i - 1], bus->route_[i]);
	}
	if (bus->route_type == domain::RouteType::Ring) {
		return length;
	} else {
		for (int i = bus->route_.size(); i > 1; --i) {
			length += GetDistance(bus->route_[i - 1], bus->route_[i - 2]);
		}
		double check_last_stop =
				GetDistance(bus->route_[bus->route_.size() - 1],
							bus->route_[bus->route_.size() - 1]);
		if (check_last_stop > 0) {
			length += check_last_stop;
		}
	}
	return length;
}



double TransportCatalogue::GetGeoRouteLength (const domain::Bus* bus) const {
	double length = 0;
	for (size_t i = 1; i < bus->route_.size(); ++i) {
		length += (ComputeDistance(bus->route_[i - 1]->coordinates,
				   bus->route_[i]->coordinates));
	}
	if (bus->route_type == domain::RouteType::Line) {
		length *= 2.;
	}
	return length;
}

void TransportCatalogue::MakeBusInfo(const domain::Bus* bus) {
	domain::BusInfo bus_info {};
	bus_info.bus = bus;
	StopCount stop_count = GetStopCount(bus);
	bus_info.count_stops = stop_count.stop_count;
	bus_info.unique_stops = stop_count.unique_stops;
	if (bus_info.count_stops > 1) {
		bus_info.geo_length = GetGeoRouteLength(bus);
		bus_info.real_length = GetRealRouteLength(bus);
		bus_info.curvature = bus_info.real_length / bus_info.geo_length;
	} else {
		bus_info.geo_length = 0;
		bus_info.real_length = GetRealRouteLength(bus);
	}
	buses_info_.insert({bus->bus_name, std::move(bus_info)});
}
}
