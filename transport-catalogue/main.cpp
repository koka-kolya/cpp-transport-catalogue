#include "json_reader.h"

#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>
#include <fstream>

using namespace std::literals;

int main() {
	data_base::TransportCatalogue db; // create empty data base
	json_reader::JsonReader json_reader(db); // initialize json_reader
	json_reader.LoadJsonAndSetDB(std::cin);	// load JSON requests and contain data base
	json_reader.GetCompleteOutputJSON(std::cout); // get JSON info from data base to ostream
}
