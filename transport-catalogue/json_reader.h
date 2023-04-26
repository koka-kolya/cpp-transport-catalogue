#pragma once
#include "json.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "router.h"
#include <memory>
#include <sstream>

namespace json_reader {
using namespace std::literals;

class JsonReader {
	using RequestsArr = const json::Array*;
	using RequestsDict = const json::Dict*;
	using Dictionaries = std::vector<const json::Dict*>;
    using DataBasePtr = std::unique_ptr<data_base::TransportCatalogue>;
	using TransportRouterPtr = std::unique_ptr<router::TransportRouter>;
    using SerializatorPtr = std::shared_ptr<serialization::Serialization>;

public:
	JsonReader();
    JsonReader(data_base::TransportCatalogue &db,
               router::TransportRouter &tr,
               serialization::Serialization &sr);

	void LoadJsonAndSetDB(std::istream& input);
    void LoadRequestJSON(std::istream& input);
    void LoadJSON(std::istream& input);
	void SetDB();

	void GetCompleteOutputJSON(std::ostream& out);

private:
    DataBasePtr db_;
//    data_base::TransportCatalogue db_;
	TransportRouterPtr tr_;
    SerializatorPtr sr_;

	json::Dict all_requests_;
	json::Array output_json;
	RequestsArr base_requests_;
	RequestsArr stat_requests_;
	RequestsDict render_settings_;
	RequestsDict routing_settings_;
	Dictionaries stops_to_db_;
	Dictionaries buses_to_db_;

	void MakeSVG(std::ostream& out) const;

	void SplitRequestByType();
	void SplitBaseRequestsByType();
	void SplitAndSetRoutingSettingsByType();
    void DeserializeRoutingSettingsAndSet();

	void AddStopsInfoToDB();
	void AddBusesInfoToDB();
	void SetDistancesInDB();

    void SerializeRenderSettings(const renderer::RenderSettings& rs);

	renderer::RenderSettings GetRenderSettings() const;
	std::deque<domain::Bus> GetSortedAllBusesFromDB() const;

	json::Node MakeStopInfoNode(json::Array::const_iterator it, int request_id);
	json::Node MakeBusInfoNode(json::Array::const_iterator it, int request_id);
	json::Node MakeRouteInfoNode(const graph::Router<double> &router,
								 json::Array::const_iterator it,
								 int request_id);
	json::Node MakeSVGNode(int request_id);
	json::Node MakeErrorMessage(const int request_id);
	json::Node MakeEmptyRouteInfoMessage(const int request_id);
	json::Node MakeOutputRouteInfoNode(const int request_id, const json::Array& arr, const double weight);
	json::Node MakeWaitNode(double wait_time, const std::string& stop_name);
	json::Node MakeTripNode(double weight, int span_count, const std::string& bus_name);
    void SerializeToFileProtoDB();
    void DeserializeAndSetDB();
    void WriteDistanceToProtoDB();
    void WriteRenderSettingsToProtoDB();
};
} // namespace json_reader
