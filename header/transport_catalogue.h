#pragma once

#include <deque>
#include <functional>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "domain.h"

namespace transport_catalogue {

    // TransportCatalogue �������� ����� ������������� ��������
    class TransportCatalogue final {
        // ���������
        std::deque<domain::Stop> stops_;
        std::unordered_map<std::string_view, const domain::Stop*> stops_by_names_;
        // �������� �� ������ ���������
        std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stops_;
        // ��������
        std::deque<domain::Route> routes_;
        std::unordered_map<std::string_view, const domain::Route*> routes_by_names_;
        // ���������� ����� �����������
        std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> distances_;

    public:
        // ��������� ��������� � �������
        void AddStop(const std::string& stop_name, geo::Coordinates coordinate);
        // ��������� ������� �� ������ ��������� � ��������� ��� � �������.
        // ���� �����-�� ��������� �� ������ ��� � �������� - ����������� ����������
        void AddRoute(const std::string& route_name, domain::RouteType route_type, const std::vector<std::string>& stops);
        // ��������� � ������� ���������� � ���������� ����� ����� �����������
        // ���� �����-�� �� ��������� ��� � �������� - ����������� ����������
        void SetDistance(const std::string& stop_from, const std::string& stop_to, int distance);

        // ���������� ���������� � ������� �� ��� �����
        // ���� �������� ��� � �������� - ����������� ���������� std::out_of_range
        domain::RouteInfo GetRouteInfo(const std::string& route_name) const;

        // ���������� ������ ���������, ���������� ����� ���������
        // ���� ��������� ��� � �������� - ����������� ���������� std::out_of_range
        std::optional<std::reference_wrapper<const std::set<std::string_view>>>
            GetBusesOnStop(const std::string& stop_name) const;
        // ���������� ���������� ����� ����������� 1 � 2 - � ������, ���� ���� ��� - � �������� �����������
        // ���� ���������� � ���������� ��� � �������� - ����������� ����������
        int GetDistance(const std::string& stop_from, const std::string& stop_to) const;

        // ���������� ������ �� �������� � ��������
        const std::unordered_map<std::string_view, const domain::Route*>& GetRoutes() const;
        // ���������� ������ �� ��������� � ��������
        const std::unordered_map<std::string_view, const domain::Stop*>& GetStops() const;
        const std::unordered_map<std::string_view, std::set<std::string_view>>& GetBusesOnStops() const;
        // ���������� ������ �� ���������� ����� �����������
        const std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>>& GetDistances() const;


    private:
        // ��������� ��������� � �������
        void AddStop(domain::Stop stop) noexcept;
        // ��������� ������� � �������
        // !!��������� �� ��������� � ������������ �������� ������ ��������� �� ��������� � ���� �� ��������!!
        void AddRoute(domain::Route route) noexcept;
        // ���������� ��������� �� ��������� �� � �����
        // ���� ��������� ��� � �������� - ����������� ����������
        const domain::Stop* FindStop(const std::string& stop_name) const;
        // ���������� ��������� �� ������� �� ��� �����
        // ���� �������� ��� � �������� - ����������� ����������
        const domain::Route* FindRoute(const std::string& route_name) const;
        // ���������� ���������� �� ��������� 1 �� ��������� 2 � ������ �����������
        // ���� ���������� � ���������� ��� � �������� - ����������� ����������
        int GetForwardDistance(const std::string& stop_from, const std::string& stop_to) const;
        // ������� ����� ���������� �� ��������
        // ���� ��� ���������� � ���������� ����� �����-���� ����� �������� ��������� - �������� ����������
        int CalculateRealRouteLength(const domain::Route* route) const;
    };

    // ������� ���-�� ��������� �� ��������
    int CalculateStops(const domain::Route* route) noexcept;
    // ������� ���-�� ���������� ��������� �� ��������
    int CalculateUniqueStops(const domain::Route* route) noexcept;
    // ������� ���������� �� �������� �� ������ ����� ������������ ���������
    double CalculateRouteLength(const domain::Route* route) noexcept;

} // namespace transport_catalogue