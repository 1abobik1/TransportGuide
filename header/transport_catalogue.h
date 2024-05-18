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

    // TransportCatalogue основной класс транспортного каталога
    class TransportCatalogue final {
        // остановки
        std::deque<domain::Stop> stops_;
        std::unordered_map<std::string_view, const domain::Stop*> stops_by_names_;
        // автобусы на каждой остановке
        std::unordered_map<std::string_view, std::set<std::string_view>> buses_on_stops_;
        // маршруты
        std::deque<domain::Route> routes_;
        std::unordered_map<std::string_view, const domain::Route*> routes_by_names_;
        // расстояния между остановками
        std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> distances_;

    public:
        // добавляет остановку в каталог
        void AddStop(const std::string& stop_name, geo::Coordinates coordinate);
        // формирует маршрут из списка остановок и добавляет его в каталог.
        // если какой-то остановки из списка нет в каталоге - выбрасывает исключение
        void AddRoute(const std::string& route_name, domain::RouteType route_type, const std::vector<std::string>& stops);
        // добавляет в каталог информацию о расстоянии между двумя остановками
        // если какой-то из остановок нет в каталоге - выбрасывает исключение
        void SetDistance(const std::string& stop_from, const std::string& stop_to, int distance);

        // возвращает информацию о маршруе по его имени
        // если маршрута нет в каталоге - выбрасывает исключение std::out_of_range
        domain::RouteInfo GetRouteInfo(const std::string& route_name) const;

        // возвращает список автобусов, проходящих через остановку
        // если остановки нет в каталоге - выбрасывает исключение std::out_of_range
        std::optional<std::reference_wrapper<const std::set<std::string_view>>>
            GetBusesOnStop(const std::string& stop_name) const;
        // возвращает расстояние между остановками 1 и 2 - в прямом, либо если нет - в обратном направлении
        // если информации о расстоянии нет в каталоге - выбрасывает исключение
        int GetDistance(const std::string& stop_from, const std::string& stop_to) const;

        // возвращает ссылку на маршруты в каталоге
        const std::unordered_map<std::string_view, const domain::Route*>& GetRoutes() const;
        // возвращает ссылку на остановки в каталоге
        const std::unordered_map<std::string_view, const domain::Stop*>& GetStops() const;
        const std::unordered_map<std::string_view, std::set<std::string_view>>& GetBusesOnStops() const;
        // возвращает ссылку на расстояния между остановками
        const std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>>& GetDistances() const;


    private:
        // добавляет остановку в каталог
        void AddStop(domain::Stop stop) noexcept;
        // добавляет маршрут в каталог
        // !!Указатели на остановки в передаваемом маршруте должны указывать на остановки в этом же каталоге!!
        void AddRoute(domain::Route route) noexcept;
        // возвращает указатель на остановку по её имени
        // если остановки нет в каталоге - выбрасывает исключение
        const domain::Stop* FindStop(const std::string& stop_name) const;
        // возвращает указатель на маршрут по его имени
        // если маршрута нет в каталоге - выбрасывает исключение
        const domain::Route* FindRoute(const std::string& route_name) const;
        // возвращает расстояние от остановки 1 до остановки 2 в прямом направлении
        // если информации о расстоянии нет в каталоге - выбрасывает исключение
        int GetForwardDistance(const std::string& stop_from, const std::string& stop_to) const;
        // считает общее расстояние по маршруту
        // если нет информации о расстоянии между какой-либо парой соседних остановок - выбросит исключение
        int CalculateRealRouteLength(const domain::Route* route) const;
    };

    // считает кол-во остановок по маршруту
    int CalculateStops(const domain::Route* route) noexcept;
    // считает кол-во уникальных остановок по маршруту
    int CalculateUniqueStops(const domain::Route* route) noexcept;
    // считает расстояние по маршруту по прямой между координатами остановок
    double CalculateRouteLength(const domain::Route* route) noexcept;

} // namespace transport_catalogue