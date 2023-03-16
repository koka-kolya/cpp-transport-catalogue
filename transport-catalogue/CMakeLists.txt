cmake_minimum_required(VERSION 3.24)
project(TransportCatalogue)

set(CMAKE_CXX_STANDARD 17)

add_executable(TransportCatalogue
    main.cpp
    domain.h domain.cpp
    geo.h geo.cpp
    json_builder.h json_builder.cpp
    json_reader.h json_reader.cpp
    json.h json.cpp
    map_renderer.h map_renderer.cpp
    request_handler.h request_handler.cpp
    svg.h svg.cpp
    transport_catalogue.h transport_catalogue.cpp
)
