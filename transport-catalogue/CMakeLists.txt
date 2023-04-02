cmake_minimum_required(VERSION 3.24)
project(TransportCatalogue)

set(CMAKE_CXX_STANDARD 17)

add_executable(TransportCatalogue
    main.cpp
    domain.h
    geo.h geo.cpp
    json.h json.cpp
    json_reader.h json_reader.cpp
    map_renderer.h map_renderer.cpp
    svg.h svg.cpp
    transport_catalogue.h transport_catalogue.cpp
    json_builder.h json_builder.cpp
    graph.h
    ranges.h
    router.h
    transport_router.cpp transport_router.h
)
