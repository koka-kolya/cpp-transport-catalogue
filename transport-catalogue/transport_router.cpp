#include "transport_router.h"
#include <iostream>
#include <ostream>


namespace router {

void TransportRouter::SetWaitTime(double bus_wait_time) {
	bus_wait_time_ = bus_wait_time;
}

void TransportRouter::SetVelocity(double bus_velocity) {
	bus_velocity_ = bus_velocity;
}

double TransportRouter::GetDistanceWeightValue(double distance) {
	double distance_km = distance / kMetersInKm;
	return (distance_km / bus_velocity_) * kMinInHour;
}

void TransportRouter::FillGraphForPair (graph::DirectedWeightedGraph<double>& graph,
										double weight,
										size_t stop_from_id,
										size_t stop_to_id,
										size_t span_count,
										std::string_view bus_name) {
	graph::Edge<double> edge {stop_from_id, stop_to_id, weight, span_count, static_cast<std::string>(bus_name)};
	graph.AddEdge(std::move(edge));
}

void TransportRouter::FillGraphForForwardDirect(std::unique_ptr<data_base::TransportCatalogue>& tc,
												graph::DirectedWeightedGraph<double>& graph,
												const std::vector<domain::Stop*>& stops,
												std::string_view bus_name) {
	for (size_t i = 0; i < stops.size() - 1; ++i) {
		double weight = bus_wait_time_;
		size_t span_count = 1;
		for (size_t j = i + 1; j < stops.size(); ++j) {
			if (stops[i] != stops[j]) {
				weight += GetDistanceWeightValue(tc->GetDistanceForPairStops(stops[j - 1], stops [j]));
				FillGraphForPair(graph, weight, stops[i]->id, stops[j]->id, span_count, bus_name);
				++span_count;
			}
		}
	}
}

void TransportRouter::FillGraphForReverseDirect(std::unique_ptr<data_base::TransportCatalogue> &tc,
												graph::DirectedWeightedGraph<double> &graph,
												const std::vector<domain::Stop*>& stops,
												std::string_view bus_name) {
	for (size_t i = stops.size() - 1; i > 0; --i) {
		double weight = bus_wait_time_;
		size_t span_count = 1;
		for (size_t j = i; j > 0; --j) {
			if (stops[i] != stops[j - 1]) {
				weight += GetDistanceWeightValue(tc->GetDistanceForPairStops(stops[j], stops [j - 1]));
				FillGraphForPair(graph, weight, stops[i]->id, stops[j - 1]->id, span_count, bus_name);
				++span_count;
			}
		}
	}
}

void TransportRouter::FillGraph(std::unique_ptr<data_base::TransportCatalogue>& tc,
								graph::DirectedWeightedGraph<double>& graph) {
	// fill a graph for each pair stops from bus route
	// one stop - one pair of vertexes: (from, to)
	for (const auto& bus : tc->GetAllBuses()) {
		const std::vector<domain::Stop*>& route_stops = bus.route_;
		FillGraphForForwardDirect(tc, graph, route_stops, bus.bus_name);
		if (bus.route_type == domain::RouteType::Line) {
			FillGraphForReverseDirect(tc, graph, route_stops, bus.bus_name);
		}
	}
}

double TransportRouter::GetBusWaitTime() {
	return bus_wait_time_;
}
} // namespace tr_router

