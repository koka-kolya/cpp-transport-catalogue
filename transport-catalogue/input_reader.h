#pragma once
#include <iostream>
#include "transport_catalogue.h"

namespace input {
namespace detail {

enum class QueryType {
	Stop,
	Bus
};

struct QueryHead {
	QueryType type;
	std::string name;
};

struct Query {
	QueryHead query_head;
	std::string query_data;
	bool is_extra = false;
	std::string query_extra = "";
};

struct QueryExtra {
	std::string stop_name;
	std::string query_extra;
};

std::string_view RemovePrefixSpaces (std::string_view word);
std::string_view RemoveSuffixSpaces (std::string_view word);
std::string_view CropWord (std::string_view word);
std::string_view CropName (std::string_view head, std::string_view query_type);
QueryHead ParseHeader (std::string_view head);
Query ParseLine (std::string_view line);
geo::Coordinates GetCoordinates(std::string_view str);
size_t GetMarksCount (std::string_view data_str, const char mark);
std::pair<std::string_view, std::string_view>
SplitExtraStopQueryString (std::string_view str);
} // end input::detail namespace

void ParseAndExportStopMain (const detail::Query& query,
							 data_base::TransportCatalogue* tc_ptr);
void ParseAndExportBus (std::string_view name,
						std::string_view data_str,
						data_base::TransportCatalogue* tc_ptr);
void ParseAdnExportStopExtra (std::string_view data_str,
							  data_base::TransportCatalogue* tc_ptr);
void ParseAndExportData (const detail::Query& query,
						 data_base::TransportCatalogue* tc_ptr);
void PushQueries (const std::vector<detail::Query>& query_queie,
				  data_base::TransportCatalogue* tc_ptr);
void PushQueries (const std::vector<std::string>& query_queie_extra,
				  data_base::TransportCatalogue* tc_ptr);
void Load(std::istream &input, data_base::TransportCatalogue& catalogue);
}

