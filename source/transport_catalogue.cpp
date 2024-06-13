#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <unordered_set>

#include "../header/transport_catalogue.h"

using namespace std;

namespace transport_catalogue {

    void TransportCatalogue::AddStop(domain::Stop stop) noexcept {
        stops_.push_back(move(stop));
        stops_by_names_.insert({ stops_.back().name, &stops_.back() });
    }

    void TransportCatalogue::AddStop(const std::string& stop_name, geo::Coordinates coordinate) {
        domain::Stop stop;
        stop.name = stop_name;
        stop.coordinate = coordinate;
        AddStop(stop);
    }

    void TransportCatalogue::AddRoute(domain::Route route) noexcept {
        // добавл€ем маршрут в хранилище и в индекс
        routes_.push_back(move(route));
        string_view route_name = routes_.back().name;
        routes_by_names_.insert({ route_name, &routes_.back() });
        // добавл€ем информацию об автобусе в остановки по маршруту
        for (auto stop : routes_.back().stops) {
            buses_on_stops_[stop->name].insert(route_name);
        }
    }

    void TransportCatalogue::
        AddRoute(const string& route_name, domain::RouteType route_type, const vector<string>& stops) {
        // провер€ем что дл€ кольцевого маршрута перва€ и последн€€ остановки совпадают
        if (route_type == domain::RouteType::CIRCLE) {
            if (stops.front() != stops.back()) {
                throw std::invalid_argument("In circle route first and last stops must be equal!"s);
            }
        }
        // формируем маршрут с указател€ми на соответсвующие остановки из каталога
        domain::Route route;
        route.name = route_name;
        route.route_type = route_type;
        for (auto& stop_name : stops) {
            route.stops.push_back(FindStop(stop_name));
        }
        AddRoute(move(route));
    }

    void TransportCatalogue::SetDistance(const std::string& stop_from, const std::string& stop_to, int distance) {
        auto Stop_from = FindStop(stop_from);
        auto Stop_to = FindStop(stop_to);
        distances_[Stop_from->name][Stop_to->name] = distance;
    }

    const domain::Stop* TransportCatalogue::FindStop(const string& stop_name) const {
        if (stops_by_names_.count(stop_name) == 0) {
            throw std::out_of_range("Stop "s + stop_name + " does not exist in catalogue"s);
        }
        return stops_by_names_.at(stop_name);
    }

    const domain::Route* TransportCatalogue::FindRoute(const string& route_name) const {
        if (routes_by_names_.count(route_name) == 0) {
            throw std::out_of_range("Route "s + route_name + " does not exist in catalogue"s);
        }
        return routes_by_names_.at(route_name);
    }

    domain::RouteInfo TransportCatalogue::GetRouteInfo(const string& route_name) const {
        domain::RouteInfo result;
        auto route = FindRoute(route_name);
        result.name = route->name;
        result.route_type = route->route_type;
        result.num_of_stops = CalculateStops(route);
        result.num_of_unique_stops = CalculateUniqueStops(route);
        result.route_length = CalculateRealRouteLength(route);
        result.curvature = result.route_length / CalculateRouteLength(route);
        return result;
    }

    std::optional<std::reference_wrapper<const std::set<std::string_view>>>
        TransportCatalogue::GetBusesOnStop(const std::string& stop_name) const {
        auto found = buses_on_stops_.find(FindStop(stop_name)->name);
        if (found == buses_on_stops_.end()) {
            return std::nullopt;
        }
        else {
            return std::cref(found->second);
        }
    }

    int TransportCatalogue::GetForwardDistance(const std::string& stop_from,
        const std::string& stop_to) const {
        if (distances_.count(stop_from) == 0 ||
            distances_.at(stop_from).count(stop_to) == 0) {
            throw std::out_of_range("No information about distance from "s
                + stop_from + " to "s + stop_to);
        }
        return distances_.at(stop_from).at(stop_to);
    }

    int TransportCatalogue::GetDistance(const std::string& stop_from, const std::string& stop_to) const {
        int result = 0;
        try {
            result = GetForwardDistance(stop_from, stop_to);
        }
        catch (std::out_of_range&) {
            try {
                result = GetForwardDistance(stop_to, stop_from);
            }
            catch (std::out_of_range&) {
                throw std::out_of_range("No information about distance between stops "s
                    + stop_from + " and "s + stop_to);
            }
        }
        return result;
    }

    const std::unordered_map<string_view, const domain::Route*>
        & TransportCatalogue::GetRoutes() const {
        return routes_by_names_;
    }

    const std::unordered_map<string_view, const domain::Stop*>
        & TransportCatalogue::GetStops() const {
        return stops_by_names_;
    }

    const std::unordered_map<string_view, std::set<string_view>>
        & TransportCatalogue::GetBusesOnStops() const {
        return buses_on_stops_;
    }

    const std::unordered_map<string_view, std::unordered_map<string_view, int> >
        & TransportCatalogue::GetDistances() const {
        return distances_;
    }

    int TransportCatalogue::CalculateRealRouteLength(const domain::Route* route) const {
        int result = 0;
        if (route != nullptr) {
            // проходим по маршруту вперед
            for (auto iter1 = route->stops.begin(), iter2 = iter1 + 1;
                iter2 < route->stops.end();
                ++iter1, ++iter2) {
                result += GetDistance((*iter1)->name, (*iter2)->name);
            }
            // проходим по маршруту назад
            if (route->route_type == domain::RouteType::LINEAR) {
                for (auto iter1 = route->stops.rbegin(), iter2 = iter1 + 1;
                    iter2 < route->stops.rend();
                    ++iter1, ++iter2) {
                    result += GetDistance((*iter1)->name, (*iter2)->name);
                }
            }
        }
        return result;
    }

    int CalculateStops(const domain::Route* route) noexcept {
        int result = 0;
        if (route != nullptr) {
            result = static_cast<int>(route->stops.size());
            if (route->route_type == domain::RouteType::LINEAR) {
                result = result * 2 - 1;
            }
        }
        return result;
    }

    int CalculateUniqueStops(const domain::Route* route) noexcept {
        int result = 0;
        if (route != nullptr) {
            unordered_set<string_view> uniques;
            for (auto stop : route->stops) {
                uniques.insert(stop->name);
            }
            result = static_cast<int>(uniques.size());
        }
        return result;
    }

    double CalculateRouteLength(const domain::Route* route) noexcept {
        double result = 0.0;
        if (route != nullptr) {
            for (auto iter1 = route->stops.begin(), iter2 = iter1 + 1;
                iter2 < route->stops.end();
                ++iter1, ++iter2) {
                result += ComputeDistance((*iter1)->coordinate, (*iter2)->coordinate);
            }
            if (route->route_type == domain::RouteType::LINEAR) {
                result *= 2;
            }
        }
        return result;
    }

} // namespace transport_catalogue