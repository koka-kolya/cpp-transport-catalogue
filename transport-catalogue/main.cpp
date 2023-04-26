#include "json_reader.h"
#include "serialization.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    std::ifstream input("input_make_base_1.json");

    data_base::TransportCatalogue data_base; // create empty data base
    router::TransportRouter transport_router; // create transport router
    serialization::Serialization serializator; // create serializator
    json_reader::JsonReader json_reader(data_base, transport_router, serializator); // initialize json_reader

    const std::string_view mode(argv[1]);
//    std::cerr << "Stop count before load json: " << data_base.GetStopCounts() << std::endl;

    if (mode == "make_base"sv) {
//        json_reader.LoadJsonAndSetDB(std::cin);	// load JSON requests and contain data base
        json_reader.LoadJsonAndSetDB(std::cin);	// load JSON requests and contain data base
//        std::cerr << "Stop count after load json: " << data_base.GetStopCounts() << std::endl;
    } else if (mode == "process_requests"sv) {
        json_reader.LoadRequestJSON(std::cin);
        json_reader.GetCompleteOutputJSON(std::cout); // get JSON info from data base to ostream
    } else {
        PrintUsage();
        return 1;
    }
}
