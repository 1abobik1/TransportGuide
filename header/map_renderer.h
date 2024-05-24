#pragma once

#include <cmath>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>

#include "domain.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace renderer {

    // Парметры рендеринга
    struct RenderSettings {
        svg::Point size;

        double padding = 0.0;

        double line_width = 0.0;
        double stop_radius = 0;

        int bus_label_font_size = 0;
        svg::Point bus_label_offset;

        int stop_label_font_size = 0;
        svg::Point stop_label_offset;

        svg::Color underlayer_color;
        double underlayer_width = 0.0;

        std::vector<svg::Color> color_palette;
    };

    // класс для отрисовки маршрутов в формате svg
    class MapRenderer final {
    public:
        using Routes = std::map<std::string_view, const domain::Route*>;
        using Stops = std::map<std::string_view, const domain::Stop*>;
        using BusesOnStops = std::unordered_map<std::string_view, std::set<std::string_view>>;

        void SetSettings(const RenderSettings& settings);

        svg::Document RenderMap(const transport_catalogue::TransportCatalogue& catalogue);

    private:
        void RenderLines(svg::Document& doc, const Routes& routes) const;
        void RenderRouteNames(svg::Document& doc, const Routes& routes) const;
        void RenderStops(svg::Document& doc, const Stops& stops, const BusesOnStops& buses_on_stops) const;
        void RenderStopNames(svg::Document& doc, const Stops& stops, const BusesOnStops& buses_on_stops) const;

        // возвращает пару - минимальная и максимальная координаты прямоугольника,
        // в который вписаны все остановки на маршрутах
        std::pair<geo::Coordinates, geo::Coordinates>
            ComputeFieldSize(const transport_catalogue::TransportCatalogue& catalogue) const;

        // пересчитывает широту и долготу в координаты для рисования на карте
        svg::Point GetRelativePoint(geo::Coordinates coordinate) const;

        RenderSettings settings_;
        std::pair<geo::Coordinates, geo::Coordinates> field_size_;

    };

} // namespace renderer

