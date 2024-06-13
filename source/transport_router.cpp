#include "../header/transport_router.h"

namespace transport_router {

    TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue& catalogue,
        const RoutingSettings& settings)
        : catalogue_(catalogue), settings_(settings) {
    }

    void TransportRouter::InitRouter() {
        // если роутер ещё не был инициализирован - делаем это
        if (!is_initialized_) {
            graph::DirectedWeightedGraph<RouteWeight>graph(CountStops());
            graph_ = std::move(graph);
            // записываем маршруты в граф
            BuildEdges();
            // строим маршрутизатор
            router_ = std::make_unique<graph::Router<RouteWeight>>(graph_);
            is_initialized_ = true;
        }
    }

    std::optional<TransportRouter::TransportRoute>
        TransportRouter::BuildRoute(const std::string& from, const std::string& to) {
        // если начальная и конечная остановка одинаковые - возвращаем пустой результат
        if (from == to) {
            return TransportRoute{};
        }
        InitRouter();
        auto from_id = id_by_stop_name_.at(from);
        auto to_id = id_by_stop_name_.at(to);
        auto route = router_->BuildRoute(from_id, to_id);
        if (!route) {
            return std::nullopt;
        }

        TransportRoute result;
        // проходим по всем ребрам маршрута
        for (auto edge_id : route->edges) {
            const auto& edge = graph_.GetEdge(edge_id);
            RouterEdge route_edge;
            route_edge.bus_name = edge.weight.bus_name;
            route_edge.stop_from = stops_by_id_.at(edge.from)->name;
            route_edge.stop_to = stops_by_id_.at(edge.to)->name;
            route_edge.span_count = edge.weight.span_count;
            route_edge.total_time = edge.weight.total_time;
            result.push_back(route_edge);
        }
        return result;
    }

    const TransportRouter::RoutingSettings& TransportRouter::GetSettings() const {
        return settings_;
    }

    TransportRouter::RoutingSettings& TransportRouter::GetSettings() {
        return settings_;
    }

    void TransportRouter::InternalInit() {
        is_initialized_ = true;
    }

    TransportRouter::Graph& TransportRouter::GetGraph() {
        return graph_;
    }
    const TransportRouter::Graph& TransportRouter::GetGraph() const {
        return graph_;
    }

    std::unique_ptr<TransportRouter::Router>& TransportRouter::GetRouter() {
        return router_;
    }
    const std::unique_ptr<TransportRouter::Router>& TransportRouter::GetRouter() const {
        return router_;
    }

    TransportRouter::StopsById& TransportRouter::GetStopsById() {
        return stops_by_id_;
    }
    const TransportRouter::StopsById& TransportRouter::GetStopsById() const {
        return stops_by_id_;
    }

    TransportRouter::IdsByStopName& TransportRouter::GetIdsByStopName() {
        return id_by_stop_name_;
    }
    const TransportRouter::IdsByStopName& TransportRouter::GetIdsByStopName() const {
        return id_by_stop_name_;
    }

    void TransportRouter::BuildEdges() {
        // проходим по всем маршрутам
        for (const auto& [route_name, route] : catalogue_.GetRoutes()) {
            int stops_count = static_cast<int>(route->stops.size());
            // перебираем все пары остановок на маршруте и строим ребра
            for (int i = 0; i < stops_count - 1; ++i) {
                // общее время движения по ребру с учетом ожидания автобуса в минутах
                double route_time = settings_.wait_time;
                double route_time_back = settings_.wait_time;
                for (int j = i + 1; j < stops_count; ++j) {
                    graph::Edge<RouteWeight> edge = MakeEdge(route, i, j);
                    route_time += ComputeRouteTime(route, j - 1, j);
                    edge.weight.total_time = route_time;
                    graph_.AddEdge(edge);

                    // если маршрут линейный, строим ребра так же для обратного направления
                    if (route->route_type == domain::RouteType::LINEAR) {
                        int i_back = stops_count - 1 - i;
                        int j_back = stops_count - 1 - j;
                        graph::Edge<RouteWeight> edge = MakeEdge(route, i_back, j_back);
                        route_time_back += ComputeRouteTime(route, j_back + 1, j_back);
                        edge.weight.total_time = route_time_back;
                        graph_.AddEdge(edge);
                    }
                }
            }
        }
    }

    size_t TransportRouter::CountStops() {
        // нумеруем остановки
        size_t stops_counter = 0;
        const auto& stops = catalogue_.GetStops();
        id_by_stop_name_.reserve(stops.size());
        stops_by_id_.reserve(stops.size());
        for (auto stop : stops) {
            id_by_stop_name_.insert({ stop.first, stops_counter });
            stops_by_id_.insert({ stops_counter++, stop.second });
        }
        return stops_counter;
    }

    graph::Edge<RouteWeight> TransportRouter::MakeEdge(const domain::Route* route,
        int stop_from_index, int stop_to_index) {

        graph::Edge<RouteWeight> edge;
        edge.from = id_by_stop_name_.at(route->stops.at(static_cast<size_t>(stop_from_index))->name);
        edge.to = id_by_stop_name_.at(route->stops.at(static_cast<size_t>(stop_to_index))->name);
        edge.weight.bus_name = route->name;
        edge.weight.span_count = static_cast<int>(stop_to_index - stop_from_index);
        return edge;
    }

    double TransportRouter::ComputeRouteTime(const domain::Route* route, int stop_from_index, int stop_to_index) {
        auto split_distance =
            catalogue_.GetDistance(route->stops.at(static_cast<size_t>(stop_from_index))->name,
                route->stops.at(static_cast<size_t>(stop_to_index))->name);
        return split_distance / settings_.velocity;
    }

    bool operator<(const RouteWeight& left, const RouteWeight& right) {
        return left.total_time < right.total_time;
    }

    RouteWeight operator+(const RouteWeight& left, const RouteWeight& right) {
        RouteWeight result;
        result.total_time = left.total_time + right.total_time;
        return result;
    }

    bool operator>(const RouteWeight& left, const RouteWeight& right) {
        return left.total_time > right.total_time;
    }

} // namespace transport_router
