#include "stat_reader.h"
#include "input_reader.h"
#include "transport_catalogue.h"
#include <iomanip>

namespace stat_read {

void LoadStat(std::istream& input, data_base::TransportCatalogue *tc_ptr) {
	size_t num_queries;
	input >> num_queries;
	std::string line;
	std::getline(input, line);
	while (num_queries > 0 && std::getline(input, line)) {
		input::detail::QueryHead query_head = input::detail::ParseHeader(line);
		if (query_head.type == input::detail::QueryType::Bus) {
			data_base::detail::BusInfo bus_info =
					std::move(tc_ptr->GetBusInfo(query_head.name));
			if (!bus_info.bus) {
				std::cout << "Bus "
						  << query_head.name
						  << ": not found" << std::endl;
				continue;
			}
			std::cout.precision(6);
			std::cout << "Bus "
					  << bus_info.bus->bus_name << ": "
					  << bus_info.count_stops  << " stops on route, "
					  << bus_info.unique_stops << " unique stops, "
					  << std::setprecision(6)
					  << bus_info.real_length << " route length, "
					  << std::setprecision(6)
					  << bus_info.curvature << " curvature"
					  << std:: endl;
		} else if (query_head.type == input::detail::QueryType::Stop) {
			data_base::detail::StopInfo stop_info =
					tc_ptr->GetStopInfo(query_head.name);
			if (!stop_info.stop) {
				std::cout << "Stop "
						  << query_head.name
						  << ": not found"
						  << std::endl;
				continue;
			} else if (stop_info.no_bus) {
				std::cout << "Stop "
						  << query_head.name
						  << ": no buses"
						  << std::endl;
				continue;
			} else {
				std::cout << "Stop "
						  << query_head.name << ": buses ";
				for (auto it = stop_info.buses_to_stop.begin();
					 it != stop_info.buses_to_stop.end(); ++it) {
					std::string space =
							std::distance(it, stop_info.buses_to_stop.end()) > 1 ? " " : "";
					std::cout << *it << space;
				} std::cout << std::endl;
			}
		}
		--num_queries;
	}
}
}
