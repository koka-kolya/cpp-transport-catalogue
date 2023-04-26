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
    void SetPathToProtoDB(const Path& path);
    
    void WriteDistancesToProtoDB(std::unique_ptr<data_base::TransportCatalogue>& tc);
    void WriteStopToProtoDB(const domain::Stop& stop);
    void WriteBusToProtoDB(const domain::Bus& bus);
    
    void WriteRenderSettingsToProtoDB(const renderer::RenderSettings& render_settings);
    void DeserializeRenderSettingsAndSetToMapRenderer(renderer::MapRenderer& mr);

    void SerializeWaitTime(const double bus_wait_time);
    void SerializeVelocity(const double bus_velocity);
    void DeserializeRouteSettings(std::unique_ptr<router::TransportRouter>& tr);

    void DeserializeAndSetDB(std::unique_ptr<data_base::TransportCatalogue>& tc);

    void SerializeToFile();

private:
    Path path_;
    db_proto::TC db_proto_;
private:
    void DeserializeAndSetStopsToDB(std::unique_ptr<data_base::TransportCatalogue>& tc);
    void DeserializeAndSetDistancesToDB(std::unique_ptr<data_base::TransportCatalogue>& tc);
    void DeserializeAndSetBusesToDB(std::unique_ptr<data_base::TransportCatalogue>& tc);
};
} // namespace serialization
