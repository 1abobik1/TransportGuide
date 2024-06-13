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

    //  ласс реализующий потоковый ввод/вывод данных транспортного каталога в формате JSON
    class JsonIO final {
    public:

        // ѕри создании считывает все данные из входного потока
        JsonIO(std::istream& data_in);

        // «агружает данные об остановках и маршрутах в TransportCatalogue
        bool LoadData(transport_catalogue::TransportCatalogue& catalogue) const;
        // «агружает и возвращает настройки рендеринга
        std::optional<renderer::RenderSettings> LoadRenderSettings() const;
        // «агружает и возвращает настройки сериализации
        std::optional<serialize::Serializator::Settings> LoadSerializeSettings() const;
        // «агружает и возвращает настройки маршрутизации
        std::optional<transport_router::TransportRouter::RoutingSettings> LoadRoutingSettings() const;

        // ќтрабатывает запросы и записывает ответы в выходной поток
        void AnswerRequests(const transport_catalogue::TransportCatalogue& catalogue,
            const renderer::RenderSettings& render_settings,
            transport_router::TransportRouter& router,
            std::ostream& requests_out) const;

    private:
        // формирует настройки рендеринга
        renderer::RenderSettings LoadSettings(const json::Dict& data) const;

        // формирует и возвращает ответы на запросы
        json::Array LoadAnswers(const json::Array& requests,
            const transport_catalogue::TransportCatalogue& catalogue,
            const renderer::RenderSettings& render_settings,
            transport_router::TransportRouter& router) const;

        // загрузка данных из json в каталог
        static void LoadStops(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue);
        static void LoadRoutes(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue);
        static void LoadDistances(const json::Array& data, transport_catalogue::TransportCatalogue& catalogue);

        // возвращает ответ на запрос инфромации о маршруте
        static json::Dict LoadRouteAnswer(const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue);
        // возвращает ответ на запрос инфромации об остановке
        static json::Dict LoadStopAnswer(const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue);
        // возвращает ответ на запрос построени€ карты маршрутов
        static json::Dict LoadMapAnswer(const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue,
            const renderer::RenderSettings& render_settings);
        // возвращает ответ на запрос построени€ маршрута
        json::Dict LoadRouteBuildAnswer(const json::Dict& request,
            const transport_catalogue::TransportCatalogue& catalogue,
            transport_router::TransportRouter& router) const;
        // возвращает сообщение с ошибкой о запросе с некорректным именем автобуса или маршрута
        static json::Dict ErrorMessage(int id);
        // провер€ет, что внутри ноды записаны валидные данные остановки
        static bool IsStop(const json::Node& node);
        // провер€ет, что внутри ноды записаны валидные данные маршрута
        static bool IsRoute(const json::Node& node);
        // провер€ет, что внутри ноды записан валидный запрос маршрута
        static bool IsRouteRequest(const json::Node& node);
        // провер€ет, что внутри ноды записан валидный запрос остановки
        static bool IsStopRequest(const json::Node& node);
        // провер€ет, что внутри ноды записан валидный запрос карты маршрутов
        static bool IsMapRequest(const json::Node& node);
        // провер€ет, что внутри ноды записан валидный запрос посторени€ маршрута
        static bool IsRouteBuildRequest(const json::Node& node);

        // считывает значение цвета из ноды
        static svg::Color ReadColor(const json::Node& node);
        // считывает пару значений (offset) из ноды
        static svg::Point ReadOffset(const json::Array& node);

        json::Document data_;
    };

} // namespace json_reader