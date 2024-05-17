#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace domain {

    // ��� ��������
    enum class RouteType {
        UNKNOWN,
        LINEAR,
        CIRCLE,
    };

    // ���������� � ��������
    struct RouteInfo {
        std::string name;
        RouteType route_type;
        int num_of_stops = 0;
        int num_of_unique_stops = 0;
        int route_length = 0;
        double curvature = 0.0;
    };

    // ��������� ������� �� ����� � ���������. �������, ��� ����� ���������.
    struct Stop {
        std::string name;
        geo::Coordinates coordinate;
        friend bool operator==(const Stop& lhs, const Stop& rhs);
    };

    // ������� ������� �� ����� (������ ��������), ���� � ������ ���������. �������, ��� ����� ���������.
    struct Route {
        std::string name;
        RouteType route_type = RouteType::UNKNOWN;
        std::vector<const Stop*> stops; // ��������� ������ ��������� �� ��������� ���������� � ���� �� ��������
        friend bool operator==(const Route& lhs, const Route& rhs);
    };

} // namespace domain