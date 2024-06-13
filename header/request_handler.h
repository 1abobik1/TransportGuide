#pragma once

#include "filesystem"
#include "optional"

#include "json_reader.h"
#include "map_renderer.h"
#include "serialization.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace transport_catalogue {

    // ¬спомогательный класс - ‘асад реализующий основные запросы к системе
    class TransportCatalogueHandler final {
    public:

        using Route = transport_router::TransportRouter::TransportRoute;
        using RoutingSettings = transport_router::TransportRouter::RoutingSettings;

        // конструктор, принимает транспортный каталог по ссылке
        explicit TransportCatalogueHandler(TransportCatalogue& catalogue)
            : catalogue_(catalogue) {}

        // возвращает информацию о маршруе по его имени
        // если маршрута нет в каталоге - выбрасывает исключение std::out_of_range
        domain::RouteInfo GetRouteInfo(const std::string& route_name) const;

        // возвращает список автобусов, проход€щих через остановку
        // если остановки нет в каталоге - выбрасывает исключение std::out_of_range
        std::optional<std::reference_wrapper<const std::set<std::string_view>>>
            GetBusesOnStop(const std::string& stop_name) const;

        // возвращает сформированную "карту" маршрутов в формате svg-документа
        svg::Document RenderMap() const;

        // возвращает маршрут между двум€ остановками
        std::optional<Route> BuildRoute(const std::string& from, const std::string& to);

        // загружает все доступные данные из JSON
        void LoadDataFromJson(const json_reader::JsonIO& json);
        void LoadDataFronJson(const std::filesystem::path& file_path);

        // загружает запросы из Json и выводит ответы в поток out
        void LoadRequestsAndAnswer(const json_reader::JsonIO& json, std::ostream& out);

        // —ериализует доступные данные
        bool SerializeData();
        // ƒесериализует доступные данные
        bool DeserializeData();

        // ѕринудительно переинициализирует маршрутизатор
        // (необходимо в случае внесени€ изменений в каталог или настройки маршрутизации)
        bool ReInitRouter();

        // методы дл€ ручного выставлени€ настроек
        void SetRenderSettings(const renderer::RenderSettings& render_settings);
        void SetRoutingSettings(const RoutingSettings& routing_settings);
        void SetSerializeSettings(const serialize::Serializator::Settings& serialize_setings);

    private:
        // инициализирует маршрутизатор (если ещЄ не инициалирован)
        bool InitRouter();

        TransportCatalogue& catalogue_;
        std::unique_ptr<transport_router::TransportRouter> router_;

        std::optional<renderer::RenderSettings> render_settings_;
        std::optional<RoutingSettings> routing_settings_;
        std::optional<serialize::Serializator::Settings> serialize_settings_;

    };

} // namespace transport_catalogue