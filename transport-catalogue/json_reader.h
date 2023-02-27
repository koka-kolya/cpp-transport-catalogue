#pragma once
#include "json.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include <memory>

namespace json_reader {
using namespace std::literals;

class JsonReader {
	using RequestsArr = const json::Array*;
	using RequestsDict = const json::Dict*;
	using Dictionaries = std::vector<const json::Dict*>;
	using DataBasePtr = std::unique_ptr<data_base::TransportCatalogue>;
public:
	JsonReader();
	JsonReader(data_base::TransportCatalogue& db);

	void LoadJsonAndSetDB(std::istream& input);
	void LoadJSON(std::istream& input);
	void SetDB();
	void GetCompleteOutputJSON(std::ostream& out);

	renderer::RenderSettings GetRenderSettings() const;
	std::deque<domain::Bus> GetSortedAllBusesFromDB() const;

private:
	DataBasePtr db_;
	json::Dict all_requests_;
	RequestsArr base_requests_;
	RequestsArr stat_requests_;
	RequestsDict render_settings_;
	Dictionaries stops_to_db_;
	Dictionaries buses_to_db_;

	void MakeSVG(std::ostream& out) const;
	void SplitRequestByType();
	void SplitBaseRequestsByType();
	void AddStopsInfoToDB();
	void AddBusesInfoToDB();
	void SetDistancesInDB();
	json::Node MakeStopInfoNode(json::Array::const_iterator it, int request_id);
	json::Node MakeBusInfoNode(json::Array::const_iterator it, int request_id);
	json::Node MakeSVGNode(int request_id);
	json::Node GetErrorMessage(const int request_id);

};
} // namespace json_reader
