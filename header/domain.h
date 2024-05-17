#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace domain {

    // тип маршрута
    enum class RouteType {
        UNKNOWN,
        LINEAR,
        CIRCLE,
    };

    // информация о маршруте
    struct RouteInfo {
        std::string name;
        RouteType route_type;
        int num_of_stops = 0;
        int num_of_unique_stops = 0;
        int route_length = 0;
        double curvature = 0.0;
    };

    // остановка состоит из имени и координат. Считаем, что имена уникальны.
    struct Stop {
        std::string name;
        geo::Coordinates coordinate;
        friend bool operator==(const Stop& lhs, const Stop& rhs);
    };

    // Маршрут состоит из имени (номера автобуса), типа и списка остановок. Считаем, что имена уникальны.
    struct Route {
        std::string name;
        RouteType route_type = RouteType::UNKNOWN;
        std::vector<const Stop*> stops; // указатели должны указывать на остановки хранящиеся в этом же каталоге
        friend bool operator==(const Route& lhs, const Route& rhs);
    };

} // namespace domain