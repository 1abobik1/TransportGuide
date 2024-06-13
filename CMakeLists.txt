cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

set (proto
    "proto/graph.proto"
    "proto/transport_catalogue.proto"
    "proto/transport_router.proto"
    "proto/map_renderer.proto"
    "proto/svg.proto")

set (sources
    "main.cpp"
    "src/domain.cpp"
    "src/geo.cpp"
    "src/json.cpp"
    "src/json_builder.cpp"
    "src/json_reader.cpp"
    "src/map_renderer.cpp"
    "src/request_handler.cpp"
    "src/serialization.cpp"
    "src/svg.cpp"
    "src/transport_catalogue.cpp"
    "src/transport_router.cpp"
    )

set (headers
    "include/domain.h"
    "include/geo.h"
    "include/graph.h"
    "include/json.h"
    "include/json_builder.h"
    "include/json_reader.h"
    "include/map_renderer.h"
    "include/ranges.h"
    "include/request_handler.h"
    "include/router.h"
    "include/serialization.h"
    "include/svg.h"
    "include/transport_catalogue.h"
    "include/transport_router.h"
   )

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto})

add_executable(transport_catalogue ${sources} ${headers} ${proto} ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(transport_catalogue PRIVATE "include")

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY_RELEASE}>" Threads::Threads)

if (MSVC)
    target_compile_options(transport_catalogue PRIVATE /W3 /WX)
else ()
    target_compile_options(transport_catalogue PRIVATE
        -Werror
        -Wall
        )
endif ()