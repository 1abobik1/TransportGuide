#pragma once

#include "filesystem"
#include "optional"

#include "json_reader.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace transport_catalogue {

    // ��������������� ����� - ����� ����������� �������� ������� � �������
    class TransportCatalogueHandler final {
    public:

        using Route = transport_router::TransportRouter::TransportRoute;
        using RoutingSettings = transport_router::TransportRouter::RoutingSettings;

        // �����������, ��������� ������������ ������� �� ������
        explicit TransportCatalogueHandler(TransportCatalogue& catalogue)
            : catalogue_(catalogue) {}

        // ���������� ���������� � ������� �� ��� �����
        // ���� �������� ��� � �������� - ����������� ���������� std::out_of_range
        domain::RouteInfo GetRouteInfo(const std::string& route_name) const;

        // ���������� ������ ���������, ���������� ����� ���������
        // ���� ��������� ��� � �������� - ����������� ���������� std::out_of_range
        std::optional<std::reference_wrapper<const std::set<std::string_view>>>
            GetBusesOnStop(const std::string& stop_name) const;

        // ���������� �������������� "�����" ��������� � ������� svg-���������
        svg::Document RenderMap() const;

        // ���������� ������� ����� ����� �����������
        std::optional<Route> BuildRoute(const std::string& from, const std::string& to);

        // ��������� ��� ��������� ������ �� JSON
        void LoadDataFromJson(const json_reader::JsonIO& json);
        void LoadDataFronJson(const std::filesystem::path& file_path);

        // ��������� ������� �� Json � ������� ������ � ����� out
        void LoadRequestsAndAnswer(const json_reader::JsonIO& json, std::ostream& out);

        // ����������� ��������� ������
        bool SerializeData();
        // ������������� ��������� ������
        bool DeserializeData();

        // ������������� ������������������ �������������
        // (���������� � ������ �������� ��������� � ������� ��� ��������� �������������)
        bool ReInitRouter();

        // ������ ��� ������� ����������� ��������
        void SetRenderSettings(const renderer::RenderSettings& render_settings);
        void SetRoutingSettings(const RoutingSettings& routing_settings);
        void SetSerializeSettings(const serialize::Serializator::Settings& serialize_setings);

    private:
        // �������������� ������������� (���� ��� �� �������������)
        bool InitRouter();

        TransportCatalogue& catalogue_;
        std::unique_ptr<transport_router::TransportRouter> router_;

        std::optional<renderer::RenderSettings> render_settings_;
        std::optional<RoutingSettings> routing_settings_;
        std::optional<serialize::Serializator::Settings> serialize_settings_;

    };

} // namespace transport_catalogue