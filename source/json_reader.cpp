#include <sstream>
#include <stdexcept>
#include <string>


#include "../header/json_reader.h"

using namespace std::literals;

namespace json_reader {

    JsonIO::JsonIO(std::istream& data_in)
        : data_(json::Load(data_in)) {
    }

    bool JsonIO::LoadData(transport_catalogue::TransportCatalogue& catalogue) const {

        // Загружаем данные в каталог, если они есть
        if (data_.GetRoot().IsMap() && data_.GetRoot().AsMap().count("base_requests"s) > 0) {
            auto& base_requests = data_.GetRoot().AsMap().at("base_requests"s);
            // проверяем, что данные для загрузки хранятся в нужном формате
            if (base_requests.IsArray()) {
                LoadStops(base_requests.AsArray(), catalogue);
                LoadRoutes(base_requests.AsArray(), catalogue);
                LoadDistances(base_requests.AsArray(), catalogue);
                return true;
            }
        }
        return false;
    }

    std::optional<renderer::RenderSettings> JsonIO::LoadRenderSettings() const {
        // загружаем параметры рендеринга, если они есть
        if (data_.GetRoot().IsMap() && data_.GetRoot().AsMap().count("render_settings"s) > 0) {
            auto& render_settings = data_.GetRoot().AsMap().at("render_settings"s);
            if (render_settings.IsMap()) {
                return LoadSettings(render_settings.AsMap());
            }
        }
        return std::nullopt;
    }

    std::optional<serialize::Serializator::Settings> JsonIO::LoadSerializeSettings() const {
        // загружаем параметры сериализации, если они есть
        if (data_.GetRoot().IsMap() && data_.GetRoot().AsMap().count("serialization_settings"s) > 0) {
            auto& serialization_settngs = data_.GetRoot().AsMap().at("serialization_settings"s);
            if (serialization_settngs.IsMap() && serialization_settngs.AsMap().count("file"s) > 0) {
                serialize::Serializator::Settings result;
                result.path = serialization_settngs.AsMap().at("file"s).AsString();
                return result;
            }
        }
        return std::nullopt;
    }

    std::optional<transport_router::TransportRouter::RoutingSettings> JsonIO::LoadRoutingSettings() const {

        if (data_.GetRoot().IsMap() &&
            data_.GetRoot().AsMap().count("routing_settings"s) > 0 &&
            data_.GetRoot().AsMap().at("routing_settings"s).IsMap()) {
            auto& routing_settings = data_.GetRoot().AsMap().at("routing_settings"s).AsMap();
            if (routing_settings.count("bus_wait_time"s) && routing_settings.at("bus_wait_time"s).IsInt()
                &&
                routing_settings.count("bus_velocity"s) && routing_settings.at("bus_velocity"s).IsInt()) {
                transport_router::TransportRouter::RoutingSettings result;
                result.wait_time = routing_settings.at("bus_wait_time"s).AsInt();
                result.velocity = routing_settings.at("bus_velocity"s).AsDouble() * transport_router::KMH_TO_MMIN;
                return result;
            }
        }
        return std::nullopt;
    }

    void JsonIO::AnswerRequests(const transport_catalogue::TransportCatalogue& catalogue,
        const renderer::RenderSettings& render_settings,
        transport_router::TransportRouter& router,
        std::ostream& requests_out) const {

        // Загружаем запросы, если они есть и формируем ответы
        if (data_.GetRoot().IsMap() && data_.GetRoot().AsMap().count("stat_requests"s) > 0) {
            auto& requests = data_.GetRoot().AsMap().at("stat_requests"s);
            // проверяем, что запросы хранятся в нужном формате
            if (requests.IsArray()) {
                json::Array answers = LoadAnswers(requests.AsArray(), catalogue, render_settings, router);
                // выводим результат в поток
                json::Print(json::Document(json::Node{ answers }), requests_out);
            }
        }
    }

    void JsonIO::LoadStops(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue) {
        for (const auto& elem : data) {
            if (IsStop(elem)) {
                const auto& name = elem.AsMap().at("name"s).AsString();
                const auto lat = elem.AsMap().at("latitude"s).AsDouble();
                const auto lng = elem.AsMap().at("longitude"s).AsDouble();
                catalogue.AddStop(name, { lat, lng });
            }
        }
    }

    void JsonIO::LoadRoutes(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue) {
        for (const auto& elem : data) {
            if (IsRoute(elem)) {
                const auto& name = elem.AsMap().at("name"s).AsString();
                const auto is_roundtrip = elem.AsMap().at("is_roundtrip"s).AsBool();
                domain::RouteType route_type;
                if (is_roundtrip) {
                    route_type = domain::RouteType::CIRCLE;
                }
                else {
                    route_type = domain::RouteType::LINEAR;
                }
                const auto stops = elem.AsMap().at("stops"s).AsArray();
                std::vector<std::string> stops_names;
                for (const auto& stop_name : stops) {
                    if (stop_name.IsString()) {
                        stops_names.push_back(stop_name.AsString());
                    }
                }
                catalogue.AddRoute(name, route_type, stops_names);
            }
        }
    }

    void JsonIO::LoadDistances(const json::Array& data,
        transport_catalogue::TransportCatalogue& catalogue) {
        for (const auto& elem : data) {
            if (IsStop(elem)) {
                const auto& name_from = elem.AsMap().at("name"s).AsString();
                const auto distances = elem.AsMap().at("road_distances"s).AsMap();
                for (const auto& [name_to, distance] : distances) {
                    if (distance.IsInt()) {
                        catalogue.SetDistance(name_from, name_to, distance.AsInt());
                    }
                }
            }
        }
    }

    renderer::RenderSettings JsonIO::LoadSettings(const json::Dict& data) const {
        renderer::RenderSettings result;

        // считываем все нужные параметры при их наличии
        if (data.count("width"s) != 0 && data.at("width"s).IsDouble()) {
            result.size.x = data.at("width"s).AsDouble();
        }
        if (data.count("height"s) != 0 && data.at("height"s).IsDouble()) {
            result.size.y = data.at("height"s).AsDouble();
        }
        if (data.count("padding"s) != 0 && data.at("padding"s).IsDouble()) {
            result.padding = data.at("padding"s).AsDouble();
        }
        if (data.count("line_width"s) != 0 && data.at("line_width"s).IsDouble()) {
            result.line_width = data.at("line_width"s).AsDouble();
        }
        if (data.count("stop_radius"s) != 0 && data.at("stop_radius"s).IsDouble()) {
            result.stop_radius = data.at("stop_radius"s).AsDouble();
        }
        if (data.count("bus_label_font_size"s) != 0 && data.at("bus_label_font_size"s).IsInt()) {
            result.bus_label_font_size = data.at("bus_label_font_size"s).AsInt();
        }
        if (data.count("bus_label_offset"s) != 0 && data.at("bus_label_offset"s).IsArray()) {
            result.bus_label_offset = ReadOffset(data.at("bus_label_offset"s).AsArray());
        }
        if (data.count("stop_label_font_size"s) != 0 && data.at("stop_label_font_size"s).IsInt()) {
            result.stop_label_font_size = data.at("stop_label_font_size"s).AsInt();
        }
        if (data.count("stop_label_offset"s) != 0 && data.at("stop_label_offset"s).IsArray()) {
            result.stop_label_offset = ReadOffset(data.at("stop_label_offset"s).AsArray());
        }
        if (data.count("underlayer_color"s) != 0) {
            result.underlayer_color = ReadColor(data.at("underlayer_color"s));
        }
        if (data.count("underlayer_width"s) != 0 && data.at("underlayer_width"s).IsDouble()) {
            result.underlayer_width = data.at("underlayer_width"s).AsDouble();
        }
        if (data.count("color_palette"s) != 0 && data.at("color_palette"s).IsArray()) {
            for (auto& color : data.at("color_palette"s).AsArray()) {
                result.color_palette.push_back(ReadColor(color));
            }
        }
        return result;
    }

    json::Array JsonIO::LoadAnswers(const json::Array& requests,
        const transport_catalogue::TransportCatalogue& catalogue,
        const renderer::RenderSettings& render_settings,
        transport_router::TransportRouter& router) const {
        json::Array result;
        for (const auto& request : requests) {
            if (IsRouteRequest(request)) {
                result.push_back(LoadRouteAnswer(request.AsMap(), catalogue));
            }
            else if (IsStopRequest(request)) {
                result.push_back(LoadStopAnswer(request.AsMap(), catalogue));
            }
            else if (IsMapRequest(request)) {
                result.push_back(LoadMapAnswer(request.AsMap(), catalogue, render_settings));
            }
            else if (IsRouteBuildRequest(request)) {
                result.push_back(LoadRouteBuildAnswer(request.AsMap(), catalogue, router));
            }
        }
        return result;
    }

    json::Dict JsonIO::LoadRouteAnswer(const json::Dict& request,
        const transport_catalogue::TransportCatalogue& catalogue) {

        int id = request.at("id"s).AsInt();
        const auto& name = request.at("name"s).AsString();
        try {
            auto answer = catalogue.GetRouteInfo(name);
            // если маршрут существует - возвращаем данные о нём
            return json::Builder{}.StartDict().
                Key("request_id").Value(id).
                Key("curvature").Value(answer.curvature).
                Key("route_length").Value(answer.route_length).
                Key("stop_count").Value(answer.num_of_stops).
                Key("unique_stop_count").Value(answer.num_of_unique_stops).
                EndDict().Build().AsMap();
        }
        catch (std::out_of_range&) {
            // если маршрута нет - возвращаем сообщение с ошибкой
            return ErrorMessage(id);
        }
    }

    json::Dict JsonIO::LoadStopAnswer(const json::Dict& request,
        const transport_catalogue::TransportCatalogue& catalogue) {

        int id = request.at("id"s).AsInt();
        const auto& name = request.at("name"s).AsString();
        try {
            auto answer = catalogue.GetBusesOnStop(name);
            //  если остановка существует возвращаем список автобусов через неё проходящих
            json::Array buses;
            if (answer) {
                for (auto name : answer.value().get()) {
                    buses.push_back(std::string(name));
                }
            }
            return json::Builder{}.StartDict().
                Key("request_id"s).Value(id).
                Key("buses"s).Value(buses).
                EndDict().Build().AsMap();
        }
        catch (std::out_of_range&) {
            // если остановки нет - возвращаем сообщение с ошибкой
            return ErrorMessage(id);
        }
    }

    json::Dict JsonIO::LoadMapAnswer(const json::Dict& request,
        const transport_catalogue::TransportCatalogue& catalogue,
        const renderer::RenderSettings& render_settings) {

        int id = request.at("id"s).AsInt();
        // формируем карту и выводим её в виде строки
        std::ostringstream out;
        renderer::MapRenderer renderer;
        renderer.SetSettings(render_settings);
        renderer.RenderMap(catalogue).Render(out);
        return json::Builder{}.StartDict().
            Key("request_id"s).Value(id).
            Key("map"s).Value(out.str()).
            EndDict().Build().AsMap();
    }

    json::Dict JsonIO::LoadRouteBuildAnswer(const json::Dict& request,
        const transport_catalogue::TransportCatalogue& catalogue,
        transport_router::TransportRouter& router) const {
        int id = request.at("id"s).AsInt();
        const auto& from = request.at("from"s).AsString();
        const auto& to = request.at("to"s).AsString();

        auto route = router.BuildRoute(from, to);
        if (!route.has_value()) {
            return ErrorMessage(id);
        }

        double total_time = 0;
        int wait_time = router.GetSettings().wait_time;
        json::Array items;
        for (const auto& edge : route.value()) {
            total_time += edge.total_time;
            json::Dict wait_elem = json::Builder{}.StartDict().
                Key("type"s).Value("Wait"s).
                Key("stop_name"s).Value(std::string(edge.stop_from)).
                Key("time"s).Value(wait_time).
                EndDict().Build().AsMap();
            json::Dict ride_elem = json::Builder{}.StartDict().
                Key("type"s).Value("Bus"s).
                Key("bus"s).Value(std::string(edge.bus_name)).
                Key("span_count"s).Value(edge.span_count).
                Key("time"s).Value(edge.total_time - wait_time).
                EndDict().Build().AsMap();
            items.push_back(wait_elem);
            items.push_back(ride_elem);
        }
        return json::Builder{}.StartDict().
            Key("request_id"s).Value(id).
            Key("total_time"s).Value(total_time).
            Key("items"s).Value(items).
            EndDict().Build().AsMap();
    }

    json::Dict JsonIO::ErrorMessage(int id) {
        return json::Builder{}.StartDict().
            Key("request_id"s).Value(id).
            Key("error_message"s).Value("not found"s).
            EndDict().Build().AsMap();
    }

    bool JsonIO::IsStop(const json::Node& node) {
        if (!node.IsMap()) {
            return false;
        }
        const auto& stop = node.AsMap();
        if (stop.count("type"s) == 0 || stop.at("type"s) != "Stop"s) {
            return false;
        }
        if (stop.count("name"s) == 0 || !(stop.at("name"s).IsString())) {
            return false;
        }
        if (stop.count("latitude"s) == 0 || !(stop.at("latitude"s).IsDouble())) {
            return false;
        }
        if (stop.count("longitude"s) == 0 || !(stop.at("longitude"s).IsDouble())) {
            return false;
        }
        if (stop.count("road_distances"s) == 0 || (stop.at("longitude"s).IsMap())) {
            return false;
        }
        return true;
    }

    bool JsonIO::IsRoute(const json::Node& node) {
        if (!node.IsMap()) {
            return false;
        }
        const auto& bus = node.AsMap();
        if (bus.count("type"s) == 0 || bus.at("type"s) != "Bus"s) {
            return false;
        }
        if (bus.count("name"s) == 0 || !(bus.at("name"s).IsString())) {
            return false;
        }
        if (bus.count("is_roundtrip"s) == 0 || !(bus.at("is_roundtrip"s).IsBool())) {
            return false;
        }
        if (bus.count("stops"s) == 0 || !(bus.at("stops"s).IsArray())) {
            return false;
        }
        return true;
    }

    bool JsonIO::IsRouteRequest(const json::Node& node) {
        if (!node.IsMap()) {
            return false;
        }
        const auto& request = node.AsMap();
        if (request.count("type"s) == 0 || request.at("type"s) != "Bus"s) {
            return false;
        }
        if (request.count("id"s) == 0 || !(request.at("id"s).IsInt())) {
            return false;
        }
        if (request.count("name"s) == 0 || !(request.at("name"s).IsString())) {
            return false;
        }
        return true;
    }

    bool JsonIO::IsStopRequest(const json::Node& node) {
        if (!node.IsMap()) {
            return false;
        }
        const auto& request = node.AsMap();
        if (request.count("type"s) == 0 || request.at("type"s) != "Stop"s) {
            return false;
        }
        if (request.count("id"s) == 0 || !(request.at("id"s).IsInt())) {
            return false;
        }
        if (request.count("name"s) == 0 || !(request.at("name"s).IsString())) {
            return false;
        }
        return true;
    }

    bool JsonIO::IsMapRequest(const json::Node& node) {
        if (!node.IsMap()) {
            return false;
        }
        const auto& request = node.AsMap();
        if (request.count("type"s) == 0 || request.at("type"s) != "Map"s) {
            return false;
        }
        if (request.count("id"s) == 0 || !(request.at("id"s).IsInt())) {
            return false;
        }
        return true;
    }

    bool JsonIO::IsRouteBuildRequest(const json::Node& node) {
        if (!node.IsMap()) {
            return false;
        }
        const auto& request = node.AsMap();
        if (request.count("type"s) == 0 || request.at("type"s) != "Route"s) {
            return false;
        }
        if (request.count("id"s) == 0 || !(request.at("id"s).IsInt())) {
            return false;
        }
        if (request.count("from") == 0 || !request.at("from").IsString()) {
            return false;
        }
        if (request.count("to") == 0 || !request.at("to").IsString()) {
            return false;
        }
        return true;
    }

    svg::Color JsonIO::ReadColor(const json::Node& color) {
        if (color.IsString()) {
            return color.AsString();
        }
        else if (color.IsArray() && color.AsArray().size() == 3) {
            auto result_color = svg::Rgb(static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
                static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
                static_cast<uint8_t>(color.AsArray().at(2).AsInt()));
            return result_color;
        }
        else if (color.IsArray() && color.AsArray().size() == 4) {
            auto result_color = svg::Rgba(static_cast<uint8_t>(color.AsArray().at(0).AsInt()),
                static_cast<uint8_t>(color.AsArray().at(1).AsInt()),
                static_cast<uint8_t>(color.AsArray().at(2).AsInt()),
                color.AsArray().at(3).AsDouble());
            return result_color;
        }
        else {
            return svg::NoneColor;
        }
    }

    svg::Point JsonIO::ReadOffset(const json::Array& offset) {
        svg::Point result;
        if (offset.size() > 1) {
            if (offset.at(0).IsDouble()) {
                result.x = offset.at(0).AsDouble();
            }
            if (offset.at(1).IsDouble()) {
                result.y = offset.at(1).AsDouble();
            }
        }
        return result;
    }

} // namespace json_reader