#include "json_reader.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "svg.h"

#include <cstdlib>

namespace json_reader {

JsonReader::JsonReader(data_base::TransportCatalogue& db)
	: db_(std::move(std::make_unique<data_base::TransportCatalogue>(db))) {
}

void JsonReader::LoadJsonAndSetDB(std::istream& input) {
	LoadJSON(input);
	SetDB();
}

void JsonReader::LoadJSON(std::istream& input) {
	all_requests_ = std::move(json::Load(input).GetRoot().AsDict());
	SplitRequestByType();
	SplitBaseRequestsByType();
}

void JsonReader::SetDB() {
	AddStopsInfoToDB();
	SetDistancesInDB();
	AddBusesInfoToDB();
}

void JsonReader::GetCompleteOutputJSON(std::ostream& out) {
	json::Array output;
	for (auto it = stat_requests_->begin(); it != stat_requests_->end(); ++it) {
		int request_id = it->AsDict().at("id"s).AsInt();
		if (it->AsDict().at("type"s) == "Stop"s) {
			if (db_->FindStop(it->AsDict().at("name"s).AsString()) == nullptr) {
				output.push_back(GetErrorMessage(request_id));
				continue;
			}
			output.push_back(MakeStopInfoNode(it, request_id));
		} else if (it->AsDict().at("type"s) == "Map"s) {
			output.push_back(MakeSVGNode(request_id));
		} else {
			if (db_->FindBus(it->AsDict().at("name"s).AsString()) == nullptr) {
				output.push_back(GetErrorMessage(request_id));
				continue;
			}
			output.push_back(MakeBusInfoNode(it, request_id));
		}
	}
	json::Document out_doc(std::move(output));
	Print(std::move(out_doc), out);
}

void JsonReader::MakeSVG(std::ostream& out) const {
	renderer::MapRenderer map_renderer(std::move(GetSortedAllBusesFromDB()), GetRenderSettings());
	map_renderer.SetRenderer();
	map_renderer.OutputRenderedMap(out);
}

void JsonReader::SplitRequestByType() {
	base_requests_ = &all_requests_.at("base_requests"s).AsArray();
	stat_requests_ = &all_requests_.at("stat_requests"s).AsArray();
	render_settings_ = &all_requests_.at("render_settings"s).AsDict();
}

void JsonReader::SplitBaseRequestsByType() {
	for (auto it = base_requests_->begin(); it != base_requests_->end(); ++it) {
		if (it->IsNull()) { break; }
		if (it->AsDict().at("type"s) == "Stop"s) {
			stops_to_db_.emplace_back(&(it->AsDict()));
		} else {
			buses_to_db_.emplace_back(&(it->AsDict()));
		}
	}
}

void JsonReader::SetDistancesInDB() {
	for (const auto& stop : stops_to_db_) {
		for (const auto& [stop_to, dist] : stop->at("road_distances"s).AsDict()) {
			db_->SetDistances(stop->at("name"s).AsString(), stop_to, dist.AsDouble());
		}
	}
}

void JsonReader::AddStopsInfoToDB() {
	for (const auto& stop : stops_to_db_) {
		domain::Stop stop_from;
		stop_from.stop_name = stop->at("name"s).AsString();
		stop_from.coordinates.lat = stop->at("latitude"s).AsDouble();
		stop_from.coordinates.lng = stop->at("longitude"s).AsDouble();
		db_->AddStop(std::move(stop_from));
	}
}

void JsonReader::AddBusesInfoToDB() {
	for (const auto& bus : buses_to_db_) {
		domain::Bus bus_output;
		bus_output.bus_name = bus->at("name"s).AsString();
		bus_output.route_type = bus->at("is_roundtrip"s).AsBool() ?
					domain::RouteType::Ring : domain::RouteType::Line;
		for (const auto& stop_name : bus->at("stops"s).AsArray()) {
			bus_output.route_.push_back(db_->FindStop(stop_name.AsString()));
		}
		db_->AddBus(std::move(bus_output));
	}
	db_->SetBusesInfo();
}

json::Node JsonReader::MakeSVGNode(int request_id) {
	std::ostringstream os;
	MakeSVG(os);
	return json::Node (json::Builder{}
						.StartDict()
							.Key("map"s).Value(std::move(os.str()))
							.Key("request_id"s).Value(request_id)
						.EndDict()
						.Build());
}

json::Node JsonReader::MakeStopInfoNode(json::Array::const_iterator it, int request_id) {
	domain::StopInfo stop_info = db_->GetStopInfo(it->AsDict().at("name"s).AsString());
	std::vector<json::Node> buses;
	for (auto it = stop_info.buses_to_stop.begin(); it != stop_info.buses_to_stop.end(); ++it) {
		buses.emplace_back(json::Node{static_cast<std::string>(*it)});
	}
	return json::Node (json::Builder{}
						.StartDict()
							.Key("buses"s).Value(std::move(buses))
							.Key("request_id"s).Value(request_id)
						.EndDict()
						.Build());
}

json::Node JsonReader::MakeBusInfoNode(json::Array::const_iterator it, int request_id) {
	domain::BusInfo bus_info = db_->GetBusInfo(it->AsDict().at("name"s).AsString());
	return json::Node(json::Builder{}
				.StartDict()
					.Key("curvature"s).Value(bus_info.curvature)
					.Key("request_id"s).Value(request_id)
					.Key("route_length"s).Value(bus_info.real_length)
					.Key("stop_count"s).Value(bus_info.count_stops)
					.Key("unique_stop_count"s).Value(bus_info.unique_stops)
				.EndDict()
				.Build());
}

json::Node JsonReader::GetErrorMessage(const int request_id) {
	return json::Node (json::Builder{}
				.StartDict()
					.Key("request_id"s).Value(request_id)
					.Key("error_message"s).Value("not found"s)
				.EndDict()
				.Build());
}

svg::Color MakeRGBaString(json::Node color) {
	std::string tmp = "rgb"s;
	if (color.AsArray().size() == 4) {
		tmp.push_back('a');
	}
	tmp.push_back('(');
	bool is_first = true;
	for (const auto& val : color.AsArray()) {
		if (!is_first) {
			tmp.push_back(',');
		}
		std::ostringstream os;
		os << val.AsDouble();
		tmp += os.str();
		is_first = false;
	}
	tmp.push_back(')');
	return tmp;
}

renderer::RenderSettings JsonReader::GetRenderSettings () const {
	renderer::RenderSettings rs {};
	for (auto it = render_settings_->begin(); it != render_settings_->end(); ++it) {
		if (it->first == "bus_label_font_size"s) {
			rs.bus_label_font_size = it->second.AsDouble();
			continue;
		} else if (it->first == "bus_label_offset"s) {
			rs.bus_label_offset.x = it->second.AsArray()[0].AsDouble();
			rs.bus_label_offset.y = it->second.AsArray()[1].AsDouble();
			continue;
		} else if (it->first == "color_palette"s) {
			for (const auto& color : it->second.AsArray()) {
				if (color.IsString()) {
					rs.color_palette.push_back(std::move(color.AsString()));
				} else {
					rs.color_palette.push_back(std::move(MakeRGBaString(color)));
				}
			}
			continue;
		} else if (it->first == "height"s) {
			rs.height = it->second.AsDouble();
			continue;
		} else if (it->first == "line_width"s) {
			rs.line_width = it->second.AsDouble();
			continue;
		} else if (it->first == "padding"s) {
			rs.padding = it->second.AsDouble();
			continue;
		} else if (it->first == "stop_label_font_size"s) {
			rs.stop_label_font_size = it->second.AsDouble();
			continue;
		} else if (it->first == "stop_label_offset"s) {
				rs.stop_label_offset.x = it->second.AsArray()[0].AsDouble();
				rs.stop_label_offset.y = it->second.AsArray()[1].AsDouble();
			continue;
		} else if (it->first == "stop_radius"s) {
			rs.stop_radius = it->second.AsDouble();
			continue;
		} else if (it->first == "underlayer_color"s) {
			if(it->second.IsString()) {
				rs.underlayer_color = std::move(it->second.AsString());
			} else {
				rs.underlayer_color = std::move(MakeRGBaString(it->second));
			}
			continue;
		} else if (it->first == "underlayer_width"s) {
			rs.uderlayer_width = it->second.AsDouble();
			continue;
		} else if (it->first == "width"s) {
			rs.width = it->second.AsDouble();
			continue;
		}
	}
	return rs;
}

std::deque<domain::Bus> JsonReader::GetSortedAllBusesFromDB() const {
	std::deque<domain::Bus> all_buses = std::move(db_->GetAllBuses());
	std::sort(std::move(all_buses.begin()), std::move(all_buses.end()),
			  [] (const domain::Bus& lhs, const domain::Bus& rhs) {
		return lhs.bus_name < rhs.bus_name;
	});
	return all_buses;
}

} // namespace json_reader
