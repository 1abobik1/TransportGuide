#include <fstream>
#include <memory>

#include "../header/request_handler.h"

using namespace std::literals;

namespace transport_catalogue {

    domain::RouteInfo TransportCatalogueHandler::GetRouteInfo(const std::string& route_name) const {
        return catalogue_.GetRouteInfo(route_name);
    }

    std::optional<std::reference_wrapper<const std::set<std::string_view>>>
        TransportCatalogueHandler::GetBusesOnStop(const std::string& stop_name) const {
        return catalogue_.GetBusesOnStop(stop_name);
    }

    svg::Document TransportCatalogueHandler::RenderMap() const {
        if (render_settings_) {
            renderer::MapRenderer renderer;
            renderer.SetSettings(render_settings_.value());
            return renderer.RenderMap(catalogue_);
        }
        else {
            std::cerr << "Can't find rendering settings"s << std::endl;
            return {};
        }
    }

    bool TransportCatalogueHandler::InitRouter() {
        if (!router_) {
            return ReInitRouter();
        }
        return true;
    }

    std::optional<TransportCatalogueHandler::Route>
        TransportCatalogueHandler::BuildRoute(const std::string& from, const std::string& to) {
        if (!InitRouter()) {
            std::cerr << "Can't init Transport Router"s << std::endl;
            return std::nullopt;
        }
        else {
            return router_->BuildRoute(from, to);
        }
    }

    void TransportCatalogueHandler::LoadDataFromJson(const json_reader::JsonIO& json) {
        json.LoadData(catalogue_);
        render_settings_ = json.LoadRenderSettings();
        serialize_settings_ = json.LoadSerializeSettings();
        routing_settings_ = json.LoadRoutingSettings();
    }

    void TransportCatalogueHandler::LoadDataFronJson(const std::filesystem::__cxx11::path& file_path) {
        std::ifstream in(file_path);
        if (in.is_open()) {
            LoadDataFromJson(json_reader::JsonIO(in));
        }
        else {
            std::cerr << "Error opening file : "s + file_path.filename().string() << std::endl;
        }
    }

    void TransportCatalogueHandler::LoadRequestsAndAnswer(const json_reader::JsonIO& json, std::ostream& out) {
        if (!InitRouter()) {
            std::cerr << "Can't init Transport Router"s << std::endl;
            return;
        }
        json.AnswerRequests(catalogue_, render_settings_.value_or(renderer::RenderSettings{}), *router_, out);
    }

    bool TransportCatalogueHandler::SerializeData() {
        if (!serialize_settings_) {
            std::cerr << "Can't find Serialize Settings : "s << std::endl;
            return false;
        }
        serialize::Serializator serializator(serialize_settings_.value());
        serializator.AddTransportCatalogue(catalogue_);
        if (render_settings_) {
            serializator.AddRenderSettings(render_settings_.value());
        }
        if (routing_settings_) {
            InitRouter();
            router_->InitRouter();
            serializator.AddTransportRouter(*router_.get());
        }
        return serializator.Serialize();
    }

    bool TransportCatalogueHandler::DeserializeData() {
        if (!serialize_settings_) {
            std::cerr << "Can't find Serialize Settings : "s << std::endl;
            return false;
        }
        serialize::Serializator serializator(serialize_settings_.value());
        if (serializator.Deserialize(catalogue_, render_settings_, router_)) {
            if (router_) {
                routing_settings_ = router_->GetSettings();
            }
            return true;
        }
        return false;
    }

    bool TransportCatalogueHandler::ReInitRouter() {
        if (routing_settings_) {
            router_ = std::make_unique<transport_router::TransportRouter>(catalogue_, routing_settings_.value());
            return true;
        }
        else {
            std::cerr << "Can't find routing settings"s << std::endl;
            return false;
        }
    }

    void TransportCatalogueHandler::SetRenderSettings(const renderer::RenderSettings& render_settings) {
        render_settings_ = render_settings;
    }

    void TransportCatalogueHandler::SetRoutingSettings(const RoutingSettings& routing_settings) {
        routing_settings_ = routing_settings;
    }

    void TransportCatalogueHandler::SetSerializeSettings(const serialize::Serializator::Settings& serialize_setings) {
        serialize_settings_ = serialize_setings;
    }

} // namespace transport_catalogue