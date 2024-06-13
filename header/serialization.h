#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "transport_catalogue.pb.h"

namespace serialize {

    class Serializator final {
    public:
        using ProtoTransportCatalogue = transport_catalogue_serialize::TransportCatalogue;
        using TransportCatalogue = transport_catalogue::TransportCatalogue;

        using ProtoTransportRouter = transport_router_serialize::TransportRouter;
        using TransportRouter = transport_router::TransportRouter;

        struct Settings {
            std::filesystem::path path;
        };

        Serializator(const Settings& settings) : settings_(settings) {};
        void ResetSettings(const Settings& settings);

        // ƒобавл€ет данные транспортного каталога дл€ сериализации
        void AddTransportCatalogue(const TransportCatalogue& catalogue);
        // ƒобавл€ет настройки рендеринга дл€ сериализации
        void AddRenderSettings(const renderer::RenderSettings& settings);
        // ƒобавл€ет данные маршрутизатора дл€ сериализации
        void AddTransportRouter(const TransportRouter& router);

        // сохран€ет данные транспортного каталога в бинарном виде в соответсвии с настройками
        bool Serialize();

        // загружает данные в транспортный каталог из файла в соответствии с настройками
        bool Deserialize(TransportCatalogue& catalogue,
            std::optional<renderer::RenderSettings>& settings,
            std::unique_ptr<TransportRouter>& router_);
    private:
        void Clear() noexcept;

        void SaveStops(const TransportCatalogue& catalogue);
        void LoadStops(TransportCatalogue& catalogue);

        void SaveRoutes(const TransportCatalogue& catalogue);
        void LoadRoutes(TransportCatalogue& catalogue);

        void SaveRouteStops(const domain::Route& route, transport_catalogue_serialize::Route& p_route);
        void LoadRoute(TransportCatalogue& catalogue, const transport_catalogue_serialize::Route& p_route) const;

        void SaveDistances(const TransportCatalogue& catalogue);
        void LoadDistances(TransportCatalogue& catalogue) const;

        void SaveRenderSettings(const renderer::RenderSettings& settings);
        void LoadRenderSettings(std::optional<renderer::RenderSettings>& settings) const;

        void SaveTransportRouter(const TransportRouter& router);
        void LoadTransportRouter(const TransportCatalogue& catalogue,
            std::unique_ptr<TransportRouter>& transport_router);

        void SaveTransportRouterSettings(const TransportRouter::RoutingSettings& routing_settings);
        void LoadTransportRouterSettings(TransportRouter::RoutingSettings& routing_settings) const;

        void SaveGraph(const TransportRouter::Graph& graph);
        void LoadGraph(const TransportCatalogue& catalogue, TransportRouter::Graph& graph);

        void SaveRouter(const std::unique_ptr<TransportRouter::Router>& router);
        void LoadRouter(const TransportCatalogue& catalogue, std::unique_ptr<TransportRouter::Router>& router);

        static transport_catalogue_serialize::Coordinates MakeProtoCoordinates(const geo::Coordinates& coordinates);
        static geo::Coordinates MakeCoordinates(const transport_catalogue_serialize::Coordinates& p_coordinates);

        static transport_catalogue_serialize::RouteType MakeProtoRouteType(domain::RouteType route_type);
        static domain::RouteType MakeRouteType(transport_catalogue_serialize::RouteType p_route_type);

        static svg_serialize::Point MakeProtoPoint(const svg::Point& point);
        static svg::Point MakePoint(const svg_serialize::Point& p_point);

        static svg_serialize::Color MakeProtoColor(const svg::Color& color);
        static svg::Color MakeColor(const svg_serialize::Color& p_color);

        graph_serialize::RouteWeight MakeProtoWeight(const transport_router::RouteWeight& weight) const;
        transport_router::RouteWeight MakeWeight(const TransportCatalogue& catalogue,
            const graph_serialize::RouteWeight& p_weight) const;

        Settings settings_;

        ProtoTransportCatalogue proto_catalogue_;
        std::unordered_map<int, std::string_view> stop_name_by_id_;
        std::unordered_map<std::string_view, int> stop_id_by_name_;
        std::unordered_map<int, std::string_view> route_name_by_id_;
        std::unordered_map<std::string_view, int> route_id_by_name_;
    };


} // namespace serialize