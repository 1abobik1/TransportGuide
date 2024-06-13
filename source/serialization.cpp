#include <fstream>

#include "serialization.h"

namespace serialize {

    void Serializator::AddTransportCatalogue(const TransportCatalogue& catalogue) {
        SaveStops(catalogue);
        SaveRoutes(catalogue);
        SaveDistances(catalogue);
    }

    void Serializator::AddRenderSettings(const renderer::RenderSettings& settings) {
        SaveRenderSettings(settings);
    }

    void Serializator::AddTransportRouter(const transport_router::TransportRouter& router) {
        SaveTransportRouter(router);
        SaveTransportRouterSettings(router.GetSettings());
        SaveGraph(router.GetGraph());
        SaveRouter(router.GetRouter());
    }

    bool Serializator::Serialize() {
        std::ofstream ofs(settings_.path, std::ios::binary);
        if (!ofs.is_open()) {
            return false;
        }
        proto_catalogue_.SerializeToOstream(&ofs);
        Clear();
        return true;
    }

    bool Serializator::Deserialize(TransportCatalogue& catalogue,
        std::optional<renderer::RenderSettings>& settings,
        std::unique_ptr<TransportRouter>& router) {
        std::ifstream ifs(settings_.path, std::ios::binary);
        if (!ifs.is_open() || !proto_catalogue_.ParseFromIstream(&ifs)) {
            return false;
        }

        LoadStops(catalogue);
        LoadRoutes(catalogue);
        LoadDistances(catalogue);

        LoadRenderSettings(settings);

        LoadTransportRouter(catalogue, router);

        Clear();
        return true;
    }

    void Serializator::Clear() noexcept {
        proto_catalogue_.Clear();
        stop_name_by_id_.clear();
        stop_id_by_name_.clear();
        route_name_by_id_.clear();
        route_id_by_name_.clear();
    }

    void Serializator::SaveStops(const TransportCatalogue& catalogue) {
        auto& stops = catalogue.GetStops();
        uint32_t id = 0;
        for (auto [name, stop] : stops) {
            transport_catalogue_serialize::Stop p_stop;
            p_stop.set_id(id);
            p_stop.set_name(stop->name);
            *p_stop.mutable_coordinates() = MakeProtoCoordinates(stop->coordinate);
            stop_id_by_name_.insert({ name, id++ });
            *proto_catalogue_.mutable_catalogue()->add_stops() = std::move(p_stop);
        }
    }

    void Serializator::SaveRoutes(const TransportCatalogue& catalogue) {
        auto& routes = catalogue.GetRoutes();
        uint32_t id = 0;
        for (auto [name, route] : routes) {
            transport_catalogue_serialize::Route p_route;
            p_route.set_id(id);
            p_route.set_name(route->name);
            p_route.set_type(MakeProtoRouteType(route->route_type));
            SaveRouteStops(*route, p_route);
            route_id_by_name_.insert({ name, id++ });
            *proto_catalogue_.mutable_catalogue()->add_routes() = std::move(p_route);
        }
    }

    void Serializator::SaveRouteStops(const domain::Route& route,
        transport_catalogue_serialize::Route& p_route) {
        for (auto stop : route.stops) {
            p_route.add_stop_ids(stop_id_by_name_.at(stop->name));
        }
    }

    void Serializator::SaveDistances(const TransportCatalogue& catalogue) {
        auto& distances = catalogue.GetDistances();
        for (auto& [stop1, stops] : distances) {
            for (auto [stop2, distance] : stops) {
                transport_catalogue_serialize::Distance p_distance;
                p_distance.set_stop_id_from(stop_id_by_name_.at(stop1));
                p_distance.set_stop_id_to(stop_id_by_name_.at(stop2));
                p_distance.set_distance(distance);
                *proto_catalogue_.mutable_catalogue()->add_distances() = std::move(p_distance);
            }
        }
    }

    void Serializator::SaveRenderSettings(const renderer::RenderSettings& settings) {
        auto p_settings = proto_catalogue_.mutable_render_settings();

        *p_settings->mutable_size() = MakeProtoPoint(settings.size);

        p_settings->set_padding(settings.padding);

        p_settings->set_line_width(settings.line_width);
        p_settings->set_stop_radius(settings.stop_radius);

        p_settings->set_bus_label_font_size(settings.bus_label_font_size);
        *p_settings->mutable_bus_label_offset() = MakeProtoPoint(settings.bus_label_offset);

        p_settings->set_stop_label_font_size(settings.stop_label_font_size);
        *p_settings->mutable_stop_label_offset() = MakeProtoPoint(settings.stop_label_offset);

        *p_settings->mutable_underlayer_color() = MakeProtoColor(settings.underlayer_color);
        p_settings->set_underlayer_width(settings.underlayer_width);

        for (auto& color : settings.color_palette) {
            *p_settings->add_color_palette() = MakeProtoColor(color);
        }
    }

    void Serializator::SaveTransportRouter(const TransportRouter& router) {
        auto p_stops_by_id = proto_catalogue_.mutable_router()->mutable_stop_by_id();
        for (auto [name, id] : router.GetIdsByStopName()) {
            transport_router_serialize::StopById stop_by_id;
            stop_by_id.set_id(id);
            stop_by_id.set_stop_id(stop_id_by_name_.at(name));
            *p_stops_by_id->Add() = std::move(stop_by_id);
        }
    }

    void Serializator::SaveTransportRouterSettings(const TransportRouter::RoutingSettings& routing_settings) {
        auto p_settings = proto_catalogue_.mutable_router()->mutable_settings();

        p_settings->set_wait_time(routing_settings.wait_time);
        p_settings->set_velocity(routing_settings.velocity);
    }

    void Serializator::SaveGraph(const TransportRouter::Graph& graph) {
        auto p_graph = proto_catalogue_.mutable_router()->mutable_graph();

        for (auto& edge : graph.GetEdges()) {
            graph_serialize::Edge p_edge;
            p_edge.set_from(edge.from);
            p_edge.set_to(edge.to);
            *p_edge.mutable_weight() = MakeProtoWeight(edge.weight);
            *p_graph->add_edges() = std::move(p_edge);
        }

        for (auto& list : graph.GetIncidenceLists()) {
            auto p_list = p_graph->add_incidence_lists();
            for (auto id : list) {
                p_list->add_edge_id(id);
            }
        }

    }

    void Serializator::SaveRouter(const std::unique_ptr<TransportRouter::Router>& router) {
        auto p_router = proto_catalogue_.mutable_router()->mutable_router();

        for (const auto& data : router->GetRoutesInternalData()) {
            graph_serialize::RoutesInternalData p_data;
for (const auto& internal: data) {
    graph_serialize::OptionalRouteInternalData p_internal;
    if (internal.has_value()) {
        auto& value = internal.value();
        auto p_value = p_internal.mutable_route_internal_data();
        p_value->set_total_time(value.weight.total_time);
        if (value.prev_edge.has_value()) {
            p_value->set_prev_edge(value.prev_edge.value());
        }
    }
    *p_data.add_routes_internal_data() = std::move(p_internal);
}
                         *p_router->add_routes_internal_data() = std::move(p_data);
        }
    }

    void Serializator::LoadStops(TransportCatalogue& catalogue) {
        auto stops_count = proto_catalogue_.catalogue().stops_size();
        for (int i = 0; i < stops_count; ++i) {
            auto& p_stop = proto_catalogue_.catalogue().stops(i);
            catalogue.AddStop(p_stop.name(), MakeCoordinates(p_stop.coordinates()));
            stop_name_by_id_.insert({ p_stop.id(), p_stop.name() });
        }
    }

    void Serializator::LoadRoutes(TransportCatalogue& catalogue) {
        auto routes_count = proto_catalogue_.catalogue().routes_size();
        for (int i = 0; i < routes_count; ++i) {
            auto& p_route = proto_catalogue_.catalogue().routes(i);
            LoadRoute(catalogue, p_route);
            route_name_by_id_.insert({ p_route.id(), p_route.name() });
        }
    }

    void Serializator::LoadRoute(TransportCatalogue& catalogue,
        const transport_catalogue_serialize::Route& p_route) const {
        auto stops_count = p_route.stop_ids_size();
        std::vector<std::string> stops;
        stops.reserve(stops_count);
        for (int i = 0; i < stops_count; ++i) {
            auto stop_name = stop_name_by_id_.at(p_route.stop_ids(i));
            stops.push_back(std::string(stop_name));
        }
        catalogue.AddRoute(p_route.name(), MakeRouteType(p_route.type()), stops);
    }

    void Serializator::LoadDistances(TransportCatalogue& catalogue) const {
        auto distances_count = proto_catalogue_.catalogue().distances_size();
        for (int i = 0; i < distances_count; ++i) {
            auto& p_distance = proto_catalogue_.catalogue().distances(i);
            auto stop_from = std::string(stop_name_by_id_.at(p_distance.stop_id_from()));
            auto stop_to = std::string(stop_name_by_id_.at(p_distance.stop_id_to()));
            catalogue.SetDistance(stop_from, stop_to, p_distance.distance());
        }
    }

    void Serializator::LoadRenderSettings(std::optional<renderer::RenderSettings>& result_settings) const {

        // если данные о настройках не сериализованы - ничего не пишем
        if (!proto_catalogue_.has_render_settings()) {
            return;
        }

        auto& p_settings = proto_catalogue_.render_settings();

        renderer::RenderSettings settings;

        settings.size = MakePoint(p_settings.size());

        settings.padding = p_settings.padding();

        settings.line_width = p_settings.line_width();
        settings.stop_radius = p_settings.stop_radius();

        settings.bus_label_font_size = p_settings.bus_label_font_size();
        settings.bus_label_offset = MakePoint(p_settings.bus_label_offset());

        settings.stop_label_font_size = p_settings.stop_label_font_size();
        settings.stop_label_offset = MakePoint(p_settings.stop_label_offset());

        settings.underlayer_color = MakeColor(p_settings.underlayer_color());
        settings.underlayer_width = p_settings.underlayer_width();

        auto color_pallete_count = p_settings.color_palette_size();
        settings.color_palette.clear();
        settings.color_palette.reserve(color_pallete_count);
        for (int i = 0; i < color_pallete_count; ++i) {
            settings.color_palette.push_back(MakeColor(p_settings.color_palette(i)));
        }

        result_settings = settings;
    }

    void Serializator::LoadTransportRouter(const TransportCatalogue& catalogue,
        std::unique_ptr<TransportRouter>& transport_router) {

        // если данные о роутере не сериализованы, нчиего не пишем
        if (!proto_catalogue_.has_router()) {
            return;
        }

        // загружаем настройки
        TransportRouter::RoutingSettings routing_settings;
        LoadTransportRouterSettings(routing_settings);
        // создаём пустой транспортный маршрутизатор
        transport_router = std::make_unique<TransportRouter>(catalogue, routing_settings);

        // загружаем данные об используемых остановках
        auto& p_router = proto_catalogue_.router();
        auto stops_count = p_router.stop_by_id_size();
        for (auto i = 0; i < stops_count; ++i) {
            auto& p_stop_by_id = p_router.stop_by_id(i);
            auto stop = catalogue.GetStops().at(stop_name_by_id_.at(p_stop_by_id.stop_id()));
            transport_router->GetStopsById().insert({ p_stop_by_id.id(), stop });
            transport_router->GetIdsByStopName().insert({ stop->name, p_stop_by_id.id() });
        }

        // загружаем граф
        LoadGraph(catalogue, transport_router->GetGraph());
        // создаём роутер и загружаем внуттреннее состояние
        transport_router->GetRouter() =
            std::make_unique<TransportRouter::Router>(transport_router->GetGraph(), false);
        LoadRouter(catalogue, transport_router->GetRouter());
        // инициализируем маршрутизатор загруженными значениями
        transport_router->InternalInit();
    }

    void Serializator::LoadTransportRouterSettings(TransportRouter::RoutingSettings& routing_settings) const {
        auto& p_settings = proto_catalogue_.router().settings();

        routing_settings.wait_time = p_settings.wait_time();
        routing_settings.velocity = p_settings.velocity();
    }

    void Serializator::LoadGraph(const TransportCatalogue& catalogue, TransportRouter::Graph& graph) {
        auto& p_graph = proto_catalogue_.router().graph();
        auto edge_count = p_graph.edges_size();

        for (auto i = 0; i < edge_count; ++i) {
            graph::Edge<transport_router::RouteWeight> edge;
            auto& p_edge = p_graph.edges(i);
            edge.from = p_edge.from();
            edge.to = p_edge.to();
            edge.weight = MakeWeight(catalogue, p_edge.weight());
            graph.GetEdges().push_back(std::move(edge));
        }

        auto incidence_lists_count = p_graph.incidence_lists_size();
        for (auto i = 0; i < incidence_lists_count; ++i) {
            TransportRouter::Graph::IncidenceList list;
            auto& p_list = p_graph.incidence_lists(i);
            auto list_count = p_list.edge_id_size();
            for (auto j = 0; j < list_count; ++j) {
                list.push_back(p_list.edge_id(j));
            }
            graph.GetIncidenceLists().push_back(list);
        }
    }

    void Serializator::LoadRouter(const TransportCatalogue& catalogue,
        std::unique_ptr<TransportRouter::Router>& router) {
        auto& p_router = proto_catalogue_.router().router();
        auto& routes_internal_data = router->GetRoutesInternalData();

        auto routes_internal_data_count = p_router.routes_internal_data_size();

        for (int i = 0; i < routes_internal_data_count; ++i) {
            auto& p_internal_data = p_router.routes_internal_data(i);
            auto internal_data_count = p_internal_data.routes_internal_data_size();
            for (int j = 0; j < internal_data_count; ++j) {
                auto& p_optional_data = p_internal_data.routes_internal_data(j);
                if (p_optional_data.optional_route_internal_data_case() ==
                    graph_serialize::OptionalRouteInternalData::kRouteInternalData) {
                    TransportRouter::Router::RouteInternalData data;
                    auto& p_data = p_optional_data.route_internal_data();
                    data.weight.total_time = p_data.total_time();
                    if (p_data.optional_prev_edge_case() ==
                        graph_serialize::RouteInternalData::kPrevEdge) {
                        data.prev_edge = p_data.prev_edge();
                    }
                    else {
                        data.prev_edge = std::nullopt;
                    }
                    routes_internal_data[i][j] = std::move(data);
                }
                else {
                    routes_internal_data[i][j] = std::nullopt;
                }
            }
        }
    }

    transport_catalogue_serialize::Coordinates
        Serializator::MakeProtoCoordinates(const geo::Coordinates& coordinates) {
        transport_catalogue_serialize::Coordinates p_coordinates;
        p_coordinates.set_lat(coordinates.lat);
        p_coordinates.set_lng(coordinates.lng);
        return p_coordinates;
    }

    geo::Coordinates
        Serializator::MakeCoordinates(const transport_catalogue_serialize::Coordinates& p_coordinates) {
        geo::Coordinates coordinates;
        coordinates.lat = p_coordinates.lat();
        coordinates.lng = p_coordinates.lng();
        return coordinates;
    }

    transport_catalogue_serialize::RouteType
        Serializator::MakeProtoRouteType(domain::RouteType route_type) {
        using ProtoRouteType = transport_catalogue_serialize::RouteType;
        ProtoRouteType type;
        switch (route_type) {
        case domain::RouteType::LINEAR:
            type = ProtoRouteType::LINEAR;
            break;
        case domain::RouteType::CIRCLE:
            type = ProtoRouteType::CIRCLE;
            break;
        default:
            type = ProtoRouteType::UNKNOWN;
            break;
        }
        return type;
    }

    domain::RouteType
        Serializator::MakeRouteType(transport_catalogue_serialize::RouteType p_route_type) {
        using ProtoRouteType = transport_catalogue_serialize::RouteType;
        domain::RouteType type;
        switch (p_route_type) {
        case ProtoRouteType::LINEAR:
            type = domain::RouteType::LINEAR;
            break;
        case ProtoRouteType::CIRCLE:
            type = domain::RouteType::CIRCLE;
            break;
        default:
            type = domain::RouteType::UNKNOWN;
            break;
        }
        return type;
    }

    svg_serialize::Point
        Serializator::MakeProtoPoint(const svg::Point& point) {
        svg_serialize::Point result;
        result.set_x(point.x);
        result.set_y(point.y);
        return result;
    }

    svg::Point
        Serializator::MakePoint(const svg_serialize::Point& p_point) {
        svg::Point result;
        result.x = p_point.x();
        result.y = p_point.y();
        return result;
    }

    svg_serialize::Color
        Serializator::MakeProtoColor(const svg::Color& color) {
        svg_serialize::Color result;
        if (std::holds_alternative<std::string>(color)) {
            result.set_string_color(std::get<std::string>(color));
        }
        else if (std::holds_alternative<svg::Rgb>(color)) {
            auto& rgb = std::get<svg::Rgb>(color);
            auto p_color = result.mutable_rgb_color();
            p_color->set_r(rgb.red);
            p_color->set_g(rgb.green);
            p_color->set_b(rgb.blue);
        }
        else if (std::holds_alternative<svg::Rgba>(color)) {
            auto& rgba = std::get<svg::Rgba>(color);
            auto p_color = result.mutable_rgba_color();
            p_color->set_r(rgba.red);
            p_color->set_g(rgba.green);
            p_color->set_b(rgba.blue);
            p_color->set_o(rgba.opacity);
        }
        return result;
    }

    svg::Color
        Serializator::MakeColor(const svg_serialize::Color& p_color) {
        svg::Color color;
        switch (p_color.color_case()) {
        case svg_serialize::Color::kStringColor:
            color = p_color.string_color();
            break;
        case svg_serialize::Color::kRgbColor:
        {
            auto& p_rgb = p_color.rgb_color();
            color = svg::Rgb(p_rgb.r(), p_rgb.g(), p_rgb.b());
        }
        break;
        case svg_serialize::Color::kRgbaColor:
        {
            auto& p_rgba = p_color.rgba_color();
            color = svg::Rgba(p_rgba.r(), p_rgba.g(), p_rgba.b(), p_rgba.o());
        }
        break;
        default:
            color = svg::NoneColor;
            break;
        }
        return color;
    }

    graph_serialize::RouteWeight
        Serializator::MakeProtoWeight(const transport_router::RouteWeight& weight) const {
        graph_serialize::RouteWeight p_weight;
        p_weight.set_bus_id(route_id_by_name_.at(weight.bus_name));
        p_weight.set_span_count(weight.span_count);
        p_weight.set_total_time(weight.total_time);
        return p_weight;
    }

    transport_router::RouteWeight
        Serializator::MakeWeight(const TransportCatalogue& catalogue,
            const graph_serialize::RouteWeight& p_weight) const {
        transport_router::RouteWeight weight;

        auto route_name = catalogue.GetRoutes().at(route_name_by_id_.at(p_weight.bus_id()));
        weight.bus_name = route_name->name;
        weight.span_count = p_weight.span_count();
        weight.total_time = p_weight.total_time();
        return weight;
    }

} // namespace serialize