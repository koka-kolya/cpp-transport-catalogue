#include "serialization.h"

namespace serialization {

void Serialization::SetPathToDB(const Path &path) {
    path_ = path;
}

void Serialization::SetDistances(
    std::unique_ptr<data_base::TransportCatalogue>& tc
//    data_base::TransportCatalogue &tc
    ) {

    // Write distance to proto DB
    db_proto::Distance distance_proto;
//    for (const auto& distance : *tc.GetAllDistances()) {
    for (const auto& distance : *tc->GetAllDistances()) {
        distance_proto.set_from(distance.first.first->stop_name);
        distance_proto.set_to(distance.first.second->stop_name);
        distance_proto.set_length(distance.second);
        *db_proto_.mutable_db()->add_distances() = distance_proto;
    }
}

void Serialization::SerializeStop(const domain::Stop& stop) {

    db_proto::Stop stop_proto;
    stop_proto.set_id(stop.id);
    stop_proto.set_stop_name(stop.stop_name);
    stop_proto.mutable_coordinates()->set_lat(stop.coordinates.lat);
    stop_proto.mutable_coordinates()->set_lng(stop.coordinates.lng);

    *db_proto_.mutable_db()->add_stop() = stop_proto;
}

void Serialization::SerializeBus(const domain::Bus &bus) {

    // Write Buses to proto DB
    db_proto::Bus bus_proto;

    bus_proto.set_bus_name(bus.bus_name);

    if (bus.route_type == domain::RouteType::Ring) {
        bus_proto.set_route_type(db_proto::RouteType::Ring);
    } else {
        bus_proto.set_route_type(db_proto::RouteType::Line);
    }

    for (const auto& stop : bus.route_) {
        bus_proto.add_route(stop->stop_name);
    }
    *db_proto_.mutable_db()->add_bus() = bus_proto;
}

void Serialization::SerializeRenderSettings(const renderer::RenderSettings &rs) {

    db_proto_.mutable_rs()->set_width(rs.width);
    db_proto_.mutable_rs()->set_height(rs.height);
    db_proto_.mutable_rs()->set_padding(rs.padding);
    db_proto_.mutable_rs()->set_line_width(rs.line_width);
    db_proto_.mutable_rs()->set_stop_radius(rs.stop_radius);
    db_proto_.mutable_rs()->set_bus_label_font_size(rs.bus_label_font_size);

    db_proto_.mutable_rs()->mutable_bus_label_offset()->set_x(rs.bus_label_offset.x);
    db_proto_.mutable_rs()->mutable_bus_label_offset()->set_y(rs.bus_label_offset.y);

    db_proto_.mutable_rs()->set_stop_label_font_size(rs.stop_label_font_size);
    db_proto_.mutable_rs()->mutable_stop_label_offset()->set_x(rs.stop_label_offset.x);
    db_proto_.mutable_rs()->mutable_stop_label_offset()->set_y(rs.stop_label_offset.y);

    db_proto_.mutable_rs()->mutable_underlayer_color()->set_color_str(rs.underlayer_color);
    db_proto_.mutable_rs()->set_underlayer_width(rs.underlayer_width_new);

    for (const auto& color : rs.color_palette) {
        db_proto::Color color_proto;
        color_proto.set_color_str(color);
        *db_proto_.mutable_rs()->add_color_palette() = color_proto;
    }
}

void Serialization::DeserializeDB(
    std::unique_ptr<data_base::TransportCatalogue>& tc
//    data_base::TransportCatalogue &tc
    ) {

    std::ifstream input(path_, std::ios::binary);

    db_proto_.ParseFromIstream(&input);

    // Deserialize stops
    for (size_t i = 0; i < db_proto_.db().stop_size(); ++i) {
        domain::Stop stop;
        stop.coordinates.lat = db_proto_.db().stop(i).coordinates().lat();
        stop.coordinates.lng = db_proto_.db().stop(i).coordinates().lng();
        stop.stop_name = db_proto_.db().stop(i).stop_name();
//        stop.id = tc.GetStopCounts();
//        tc.AddStop(stop);
        stop.id = tc->GetStopCounts();
        tc->AddStop(stop);
    }

    // Deserialize distances
    for (size_t i = 0; i < db_proto_.db().distances_size(); ++i) {
        std::string_view stop_from = db_proto_.db().distances(i).from();
        std::string_view stop_to = db_proto_.db().distances(i).to();
        double length = db_proto_.db().distances(i).length();
//        tc.SetDistances(stop_from, stop_to, length);
        tc->SetDistances(stop_from, stop_to, length);
    }

    // Deserialize buses
    for (size_t i = 0; i < db_proto_.db().bus_size(); ++i) {
        domain::Bus bus;
        bus.bus_name = db_proto_.db().bus(i).bus_name();
        bus.route_type = db_proto_.db().bus(i).route_type() == db_proto::RouteType::Line
                             ? domain::RouteType::Line : domain::RouteType::Ring;
//        std::cout << (bus.route_type == domain::RouteType::Line ? "Line" : "Ring") << std::endl;
        for (const auto& stop_name : db_proto_.db().bus(i).route()) {
//            std::cout << stop_name << std::endl;
            bus.route_.push_back(tc->FindStop(stop_name));
//bus.route_.push_back(tc.FindStop(stop_name));
        }
        tc->AddBus(std::move(bus));
//        tc.AddBus(std::move(bus));
//        std::cout << "================" << std::endl;
    }
    tc->SetBusesInfo();
//    tc.SetBusesInfo();

}

void Serialization::DeserializeRenderSettings(renderer::MapRenderer &mr) {
    std::ifstream input(path_, std::ios::binary);

    db_proto_.ParseFromIstream(&input);

    renderer::RenderSettings rs;
    rs.width = db_proto_.rs().width();
    rs.height = db_proto_.rs().height();
    rs.padding = db_proto_.rs().padding();
    rs.line_width = db_proto_.rs().line_width();
    rs.stop_radius = db_proto_.rs().stop_radius();

    rs.bus_label_font_size = db_proto_.rs().bus_label_font_size();

    rs.bus_label_offset.x = db_proto_.rs().bus_label_offset().x();
    rs.bus_label_offset.y = db_proto_.rs().bus_label_offset().y();

    rs.stop_label_font_size = db_proto_.rs().stop_label_font_size();
    rs.stop_label_offset.x = db_proto_.rs().stop_label_offset().x();
    rs.stop_label_offset.y = db_proto_.rs().stop_label_offset().y();

    rs.underlayer_color = db_proto_.rs().underlayer_color().color_str();
    rs.underlayer_width_new = db_proto_.rs().underlayer_width();
    for (const auto& color : db_proto_.rs().color_palette()) {
        rs.color_palette.push_back(color.color_str());
    }
    mr.SetRenderSettings(rs);
}

void Serialization::SerializeWaitTime(const double bus_wait_time) {
    db_proto_.mutable_route_settings()->set_bus_wait_time(bus_wait_time);
}

void Serialization::SerializeVelocity(const double bus_velocity) {
    db_proto_.mutable_route_settings()->set_bus_velocity(bus_velocity);
}

void Serialization::DeserializeRouteSettings(std::unique_ptr<router::TransportRouter>& tr) {
    std::ifstream input(path_, std::ios::binary);
    db_proto_.ParseFromIstream(&input);

    tr->SetVelocity(db_proto_.route_settings().bus_velocity());
    tr->SetWaitTime(db_proto_.route_settings().bus_wait_time());
}

void Serialization::SerializeToFile()
{
    std::ofstream out(path_, std::ios::binary);
    db_proto_.SerializeToOstream(&out);
}

} // namespace serialization
