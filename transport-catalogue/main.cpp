#include "json_reader.h"

#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>
#include <fstream>

using namespace std::literals;

int main() {
	data_base::TransportCatalogue data_base; // create empty data base
	router::TransportRouter transport_router; // creat transport router
	json_reader::JsonReader json_reader(data_base, transport_router); // initialize json_reader
	json_reader.LoadJsonAndSetDB(std::cin);	// load JSON requests and contain data base
	json_reader.GetCompleteOutputJSON(std::cout); // get JSON info from data base to ostream
}
