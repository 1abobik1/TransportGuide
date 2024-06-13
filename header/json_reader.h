#pragma once

#include <iostream>
#include <memory>
#include <optional>

#include "json_builder.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace json_reader {

    // ����� ����������� ��������� ����/����� ������ ������������� �������� � ������� JSON
    class JsonIO final {
    public:

        // ��� �������� ��������� ��� ������ �� �������� ������
        JsonIO(std::istream& data_in);

        // ��������� ������ �� ���������� � ��������� � TransportCatalogue
        bool LoadData(transport_catalogue::TransportCatalogue& catalogue) const;
        // ��������� � ���������� ��������� ����������
        std::optional<renderer::RenderSettings> LoadRenderSettings() const;
        // ��������� � ���������� ��������� ������������
        std::optional<serialize::Serializator::Settings> LoadSerializeSettings() const;
        // ��������� � ���������� ��������� �������������
        std::optional<transport_router::TransportRouter::RoutingSettings> LoadRoutingSettings() const;

        // ������������ ������� � ���������� ������ � �������� �����
        void AnswerRequests(const transport_catalogue::TransportCatalogue& catalogue,
            const renderer::RenderSettings& render_settings,
            transport_router::TransportRouter& router,
            std::ostream& requests_out) const;

    private:
        // ��������� ��������� ����������
        renderer::RenderSettings LoadSettings(const json::Dict& data) const;

        // ��������� � ���������� ������ �� �������
        json::Array LoadAnswers(const json::Array& requests,
            const transport_catalogue::TransportCatalogue& catalogue,
            const renderer::RenderSettings& render_settings,
            transport_router::TransportRouter& router) const;

        // �������� ������ �� json � �������
        static void LoadStops(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue);
        static void LoadRoutes(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue);
        static void LoadDistances(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue);

        // ���������� ����� �� ������ ���������� � ��������
        static json::Dict LoadRouteAnswer(const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue);
        // ���������� ����� �� ������ ���������� �� ���������
        static json::Dict LoadStopAnswer(const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue);
        // ���������� ����� �� ������ ���������� ����� ���������
        static json::Dict LoadMapAnswer(const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue,
            const renderer::RenderSettings& render_settings);
        // ���������� ����� �� ������ ���������� ��������
        json::Dict LoadRouteBuildAnswer(const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue,
            transport_router::TransportRouter& router) const;
        // ���������� ��������� � ������� � ������� � ������������ ������ �������� ��� ��������
        static json::Dict ErrorMessage(int id);
        // ���������, ��� ������ ���� �������� �������� ������ ���������
        static bool IsStop(const json::Node& node);
        // ���������, ��� ������ ���� �������� �������� ������ ��������
        static bool IsRoute(const json::Node& node);
        // ���������, ��� ������ ���� ������� �������� ������ ��������
        static bool IsRouteRequest(const json::Node& node);
        // ���������, ��� ������ ���� ������� �������� ������ ���������
        static bool IsStopRequest(const json::Node& node);
        // ���������, ��� ������ ���� ������� �������� ������ ����� ���������
        static bool IsMapRequest(const json::Node& node);
        // ���������, ��� ������ ���� ������� �������� ������ ���������� ��������
        static bool IsRouteBuildRequest(const json::Node& node);

        // ��������� �������� ����� �� ����
        static svg::Color ReadColor(const json::Node& node);
        // ��������� ���� �������� (offset) �� ����
        static svg::Point ReadOffset(const json::Array& node);

        json::Document data_;
    };

} // namespace json_reader