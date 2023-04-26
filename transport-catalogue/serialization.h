#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_catalogue.pb.h"
#include "map_renderer.pb.h"
#include "svg.pb.h"
#include "transport_router.h"

#include <filesystem>
#include <fstream>

namespace serialization {

class Serialization {
    using DataBasePtr = std::shared_ptr<data_base::TransportCatalogue>;
    using Path = std::filesystem::path;
public:

    void SetPathToDB(const Path& path);

    void SetDistances(
        std::unique_ptr<data_base::TransportCatalogue>& tc
//        data_base::TransportCatalogue& tc
        );
    void SerializeStop(const domain::Stop& stop);
    void SerializeBus(const domain::Bus& bus);

    void SerializeRenderSettings(const renderer::RenderSettings& render_settings);
    void DeserializeRenderSettings(renderer::MapRenderer& mr);

    void SerializeWaitTime(const double bus_wait_time);
    void SerializeVelocity(const double bus_velocity);
    void DeserializeRouteSettings(std::unique_ptr<router::TransportRouter>& tr);

    void DeserializeDB(
        std::unique_ptr<data_base::TransportCatalogue>& tc
//        data_base::TransportCatalogue& tc
        );

    void SerializeToFile();

private:
//    DataBasePtr db_;
//    data_base::TransportCatalogue& db_;
    Path path_;
    db_proto::TC db_proto_;
};
} // namespace serialization
