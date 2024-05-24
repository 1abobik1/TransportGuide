#include "../header/map_renderer.h"

using namespace std::literals;

namespace renderer {

    namespace {

        inline const double EPSILON = 1e-6;
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

    } // namespace

    void MapRenderer::SetSettings(const RenderSettings& settings) {
        settings_ = settings;
    }

    svg::Document MapRenderer::RenderMap(const transport_catalogue::TransportCatalogue& catalogue) {

        field_size_ = ComputeFieldSize(catalogue);

        const auto& routes = catalogue.GetRoutes();
        const auto& stops = catalogue.GetStops();

        // перекладываем в map, дл€ упор€дочивани€ по имени
        Routes sorted_routes;
        Stops sorted_stops;
        for (auto& route : routes) {
            sorted_routes.insert(route);
        }
        for (auto& stop : stops) {
            sorted_stops.insert(stop);
        }

        const auto& buses_on_stops = catalogue.GetBusesOnStops();

        svg::Document doc;
        RenderLines(doc, sorted_routes);
        RenderRouteNames(doc, sorted_routes);
        RenderStops(doc, sorted_stops, buses_on_stops);
        RenderStopNames(doc, sorted_stops, buses_on_stops);
        return doc;
    }

    void MapRenderer::RenderLines(svg::Document& doc, const Routes& routes) const {
        auto max_color_count = settings_.color_palette.size();
        size_t color_index = 0;
        for (const auto& route : routes) {
            // работает только не с пустыми маршрутами
            if (route.second->stops.size() > 0) {
                // задаЄм параметры рисовани€ линии
                svg::Polyline line;
                line.SetStrokeColor(settings_.color_palette.at(color_index % max_color_count)).
                    SetFillColor(svg::NoneColor).SetStrokeWidth(settings_.line_width).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                // проходим по маршруту, добавл€€ точки от первой остановки до последней
                for (auto iter = route.second->stops.begin(); iter < route.second->stops.end(); ++iter) {
                    line.AddPoint(GetRelativePoint((*iter)->coordinate));
                }
                // проходим по маршруту назад если он не кольцевой
                if (route.second->route_type == domain::RouteType::LINEAR) {
                    for (auto iter = std::next(route.second->stops.rbegin()); iter < route.second->stops.rend(); ++iter) {
                        line.AddPoint(GetRelativePoint((*iter)->coordinate));
                    }
                }
                doc.Add(line);
                ++color_index;
            }
        }
    }

    void MapRenderer::RenderRouteNames(svg::Document& doc, const Routes& routes) const {
        auto max_color_count = settings_.color_palette.size();
        size_t color_index = 0;
        for (const auto& route : routes) {
            // работает только не с пустыми маршрутами
            if (route.second->stops.size() > 0) {
                // задаем общие параметры отрисовки текста и подложки
                svg::Text text, underlayer_text;
                text.SetData(std::string(route.first)).
                    SetPosition(GetRelativePoint(route.second->stops.front()->coordinate)).
                    SetOffset(settings_.bus_label_offset).
                    SetFontSize(static_cast<std::uint32_t>(settings_.bus_label_font_size)).
                    SetFontFamily("Verdana"s).SetFontWeight("bold");
                underlayer_text = text;
                // добавл€ем индивидуальные дл€ текста и подложки параметры
                text.SetFillColor(settings_.color_palette.at(color_index % max_color_count));
                underlayer_text.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).
                    SetStrokeWidth(settings_.underlayer_width).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                // отрисовываем название маршрута у первой остановки
                doc.Add(underlayer_text);
                doc.Add(text);
                // если маршрут не кольцевой и перва€ остановка не совпадает с последней
                // то отрисовываем название маршрута у последней остановки
                if (route.second->route_type == domain::RouteType::LINEAR &&
                    route.second->stops.back() != route.second->stops.front()) {
                    text.SetPosition(GetRelativePoint(route.second->stops.back()->coordinate));
                    underlayer_text.SetPosition(GetRelativePoint(route.second->stops.back()->coordinate));
                    doc.Add(underlayer_text);
                    doc.Add(text);
                }
                ++color_index;
            }
        }
    }

    void MapRenderer::RenderStops(svg::Document& doc, const Stops& stops, const BusesOnStops& buses_on_stops) const {
        for (const auto& stop : stops) {
            // проходим по всем остановкам, которые вход€т в какой либо маршрут
            if (buses_on_stops.count(stop.first) != 0) {
                // отрисовываем значок остановки
                svg::Circle circle;
                circle.SetCenter(GetRelativePoint(stop.second->coordinate)).
                    SetRadius(settings_.stop_radius).SetFillColor("white"s);
                doc.Add(circle);
            }
        }
    }

    void MapRenderer::RenderStopNames(svg::Document& doc, const Stops& stops, const BusesOnStops& buses_on_stops) const {
        for (const auto& stop : stops) {
            // проходим по всем остановкам, которые вход€т в какой либо маршрут
            if (buses_on_stops.count(stop.first) != 0) {
                // формируем текст и подложку
                svg::Text text, underlayer_text;
                text.SetData(std::string(stop.first)).SetPosition(GetRelativePoint(stop.second->coordinate)).
                    SetOffset(settings_.stop_label_offset).
                    SetFontSize(static_cast<std::uint32_t>(settings_.stop_label_font_size)).
                    SetFontFamily("Verdana");
                underlayer_text = text;
                // добавл€ем индивидуальные дл€ текста и подложки параметры
                text.SetFillColor("black");
                underlayer_text.SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).
                    SetStrokeWidth(settings_.underlayer_width).
                    SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                // отрисовываем подложку и текст
                doc.Add(underlayer_text);
                doc.Add(text);
            }
        }
    }

    svg::Point MapRenderer::GetRelativePoint(geo::Coordinates coordinate) const {
        double zoom_coef;

        double field_width = field_size_.second.lng - field_size_.first.lng;
        double field_height = field_size_.second.lat - field_size_.first.lat;

        if (IsZero(field_width) && IsZero(field_height)) {
            zoom_coef = 0;
        }
        else if (IsZero(field_width)) {
            zoom_coef = (settings_.size.y - 2 * settings_.padding) / field_height;
        }
        else if (IsZero(field_height)) {
            zoom_coef = (settings_.size.x - 2 * settings_.padding) / field_width;
        }
        else {
            zoom_coef = std::min((settings_.size.y - 2 * settings_.padding) / field_height,
                (settings_.size.x - 2 * settings_.padding) / field_width);
        }

        svg::Point result;
        result.x = (coordinate.lng - field_size_.first.lng) * zoom_coef + settings_.padding;
        result.y = (field_size_.second.lat - coordinate.lat) * zoom_coef + settings_.padding;
        return result;
    }

    std::pair<geo::Coordinates, geo::Coordinates>
        MapRenderer::ComputeFieldSize(const transport_catalogue::TransportCatalogue& catalogue) const {
        geo::Coordinates min{ 90.0, 180.0 };
        geo::Coordinates max{ -90.0, -180.0 };
        for (const auto& stop : catalogue.GetStops()) {
            if (catalogue.GetBusesOnStop(std::string(stop.first))) {
                const auto& coordinates = stop.second->coordinate;
                if (coordinates.lat < min.lat) {
                    min.lat = coordinates.lat;
                }
                if (coordinates.lat > max.lat) {
                    max.lat = coordinates.lat;
                }
                if (coordinates.lng < min.lng) {
                    min.lng = coordinates.lng;
                }
                if (coordinates.lng > max.lng) {
                    max.lng = coordinates.lng;
                }
            }
        }
        return std::pair<geo::Coordinates, geo::Coordinates>{min, max};
    }

} // namespace renderer