#include "stat_reader.h"
#include "input_reader.h"
#include <algorithm>
#include <iomanip>

namespace input {
namespace detail {
void PrintStr(std::string_view str) {
	std::cout << str << std::endl;
}

std::string_view RemovePrefixSpaces (std::string_view word) {
	if (word.empty()) return "";
	word.remove_prefix(word.find_first_not_of(' '));
	return word;
}

std::string_view RemoveSuffixSpaces (std::string_view word) {
	if (word.empty()) return "";
	while (word[word.size() - 1] == ' ') {
		word.remove_suffix(1);
	}
	return word;
}

std::string_view CropWord (std::string_view word) {
	word = RemovePrefixSpaces(word);
	word = RemoveSuffixSpaces(word);
	return word;
}

std::string_view CropName (std::string_view head, size_t pos) {
	head.remove_prefix(pos);
	head = CropWord(head);
	return head;
}

QueryHead ParseHeader (std::string_view head) {
	using namespace std;
	if (head.empty()) return {};

	QueryHead output;
	bool is_stop = head[0] == 'S';
	if (is_stop) {
		output.name = std::move(CropName(head, 5));
		output.type = QueryType::Stop;
	} else {
		output.name = std::move(CropName(head, 4));
		output.type = QueryType::Bus;
	}
	return output;
}

Query ParseLine (std::string_view line) {
	Query output;
	size_t pos = line.find_first_of(':');
	if (pos == line.npos) { return {}; }
	output.query_head = ParseHeader(line.substr(0, pos));
	if (output.query_head.type == QueryType::Bus) {
		output.query_data = std::string(line.substr(pos + 1, line.size()));
		return output;
	} else {
		if (GetMarksCount(line, ',') == 1) {
			output.query_data = std::string(line.substr(pos + 1, line.size()));
			return output;
		} else {
			auto splitted_line =
					SplitExtraStopQueryString(line.substr(pos + 1, line.size()));
			output.query_data = std::string(splitted_line.first);
			output.query_extra = std::string(splitted_line.second);
			output.is_extra = true;
		}
	}
	return output;
}

geo::Coordinates GetCoordinates(std::string_view str) {
	if (str.empty()) return {};
	geo::Coordinates output;
	output.lat = std::stod(std::string(str.substr(0, str.find(','))));
	output.lng = std::stod(std::string(str.substr(str.find(',') + 1, str.size())));
	return output;
}

size_t GetMarksCount (std::string_view data_str, const char mark) {
	return std::count(data_str.begin(), data_str.end(), mark);
}

std::pair<std::string_view, std::string_view>
SplitExtraStopQueryString (std::string_view str) {
	if (str.empty()) return {};
	std::pair<std::string_view, std::string_view> output;
	size_t pos1 = str.find_first_of(',');
	size_t pos2 = (str.substr(pos1 + 1, str.size())).find(',');
	output.first = str.substr(0, pos1 + pos2 + 1);
	output.second = str.substr(pos1 + pos2 + 2, str.size());
	return output;
}
} // end input::details namespace

void ParseAndExportStop (std::string_view name,
						 std::string_view data_str,
						 data_base::TransportCatalogue* tc_ptr) {
	data_base::detail::Stop stop_;
	stop_.stop_name	= std::move(std::string(name));
	stop_.coordinates = detail::GetCoordinates(data_str);
	tc_ptr->AddStop(stop_);
}

void ParseAdnExportStopExtra (std::string_view stop_name,
							  std::string_view data_str,
							  data_base::TransportCatalogue* tc_ptr) {

	data_base::detail::Stop* stop_from = tc_ptr->FindStop(stop_name);
	size_t pos = data_str.find_first_of('m');
	while (pos != data_str.npos) {
		double dist = std::stod(std::string(detail::CropWord(data_str.substr(0, pos))));
		pos = data_str.find_first_of('o');
		data_str.remove_prefix(pos + 1);
		pos = data_str.find(',');
		data_base::detail::Stop* stop_to =
				tc_ptr->FindStop(detail::CropWord(data_str.substr(0, pos)));
		if (pos == data_str.npos && !data_str.empty()) {
			data_str.remove_prefix(data_str.size());
		}
		data_str.remove_prefix(pos + 1);
		pos = data_str.find('m');
		data_base::detail::Distance output;
		output.value = dist;
		output.from_to.first = stop_from;
		output.from_to.second = stop_to;
		tc_ptr->SetDistances(output);
	}
}

void ParseAndExportBus (std::string_view name,
						std::string_view data_str,
						data_base::TransportCatalogue* tc_ptr) {
	data_base::detail::Bus bus_;
	bus_.bus_name = std::move(std::string(name));
	bool is_ring_route = std::any_of(data_str.rbegin(), data_str.rend(),
									 [] (const char c) { return c == '>';} );
	char mark = is_ring_route ? '>' : '-' ;
	bus_.route_type = is_ring_route ?
				data_base::detail::RouteType::Ring :
				data_base::detail::RouteType::Line;
	size_t pos = data_str.find(mark);
	while (pos != data_str.npos) {
		bus_.route_.push_back(tc_ptr->FindStop(
								  (detail::CropWord(data_str.substr(0, pos)))));
		data_str = data_str.substr(pos + 1, data_str.size());
		pos = data_str.find(mark);
		if (pos == data_str.npos && !data_str.empty()) { // последнее название в строке
			bus_.route_.push_back(tc_ptr->FindStop((detail::CropWord(data_str))));
		}
	}
	tc_ptr->AddBus(bus_);
}

void ParseAndExportData (const detail::Query& query,
						 data_base::TransportCatalogue* tc_ptr) {
	if (query.query_head.type == detail::QueryType::Stop) {
		ParseAndExportStop(query.query_head.name, query.query_data, tc_ptr);
	} else {
		ParseAndExportBus(query.query_head.name, query.query_data, tc_ptr);
	}
}

void PushQueries (const std::vector<detail::Query>& query_queie,
				  data_base::TransportCatalogue* tc_ptr) {
	for (const detail::Query& query : query_queie) {
		ParseAndExportData(query, tc_ptr);
	}
}

void PushQueries (const std::vector<detail::QueryExtra>& query_queie_extra,
				  data_base::TransportCatalogue* tc_ptr) {
	for (const detail::QueryExtra& qe : query_queie_extra) {
		ParseAdnExportStopExtra(qe.stop_name, qe.query_extra, tc_ptr);
	}
}

void Load (std::istream &input, data_base::TransportCatalogue& tc) {
	std::string line;
	size_t num_queries;
	input >> num_queries;
	data_base::TransportCatalogue* tc_ptr = &tc;
	std::getline(input, line);
	std::vector<detail::Query> query_queue;
	std::vector<detail::QueryExtra> query_queue_extra;
	while (num_queries > 0 && std::getline(input, line)) {
		detail::Query query = detail::ParseLine(line);
		if (query.is_extra) {
			query_queue_extra.push_back({query.query_head.name, query.query_extra});
		}
		query_queue.push_back(query);
		--num_queries;
	}
	std::sort(std::move(query_queue.begin()), std::move(query_queue.end()),
			  [] (const detail::Query& q1, const detail::Query& q2) {
		return q1.query_head.type == detail::QueryType::Stop &&
			   q2.query_head.type == detail::QueryType::Bus;
	});

	PushQueries(query_queue, tc_ptr);
	PushQueries(query_queue_extra, tc_ptr);
	stat_read::LoadStat(input, tc_ptr);
}
}
