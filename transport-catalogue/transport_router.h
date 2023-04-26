#pragma once

#include "router.h"
#include "transport_catalogue.h"
namespace router {

class TransportRouter {
public:
	TransportRouter() = default;

	void SetWaitTime(const double bus_wait_time);
	void SetVelocity(const double bus_velocity);

    void FillGraph(std::unique_ptr<data_base::TransportCatalogue>& tc,
                   graph::DirectedWeightedGraph<double>& graph);

	double GetBusWaitTime();

private:
	double bus_wait_time_;
	double bus_velocity_;
	graph::Router<double>* graph_;
	static constexpr double kMetersInKm = 1000;
	static constexpr double kMinInHour = 60;

	double GetDistanceWeightValue(double distance);

	void FillGraphForPair (graph::DirectedWeightedGraph<double>& graph,
						   double weight,
						   size_t i,
						   size_t j,
						   size_t span_count,
						   std::string_view bus_num);

    void FillGraphForForwardDirect(std::unique_ptr<data_base::TransportCatalogue>& tc,
								   graph::DirectedWeightedGraph<double> &graph,
								   const std::vector<domain::Stop*>& stops,
								   std::string_view bus_name);

    void FillGraphForReverseDirect(std::unique_ptr<data_base::TransportCatalogue>& tc,
								   graph::DirectedWeightedGraph<double> &graph,
								   const std::vector<domain::Stop*>& stops,
								   std::string_view bus_name);
};

} // namespace router
