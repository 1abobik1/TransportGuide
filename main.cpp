#include <cassert>
#include <fstream>
#include <iostream>
#include <string_view>

#include "header/request_handler.h"

using namespace std;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    transport_catalogue::TransportCatalogue catalogue;
    transport_catalogue::TransportCatalogueHandler catalogue_handler(catalogue);

    if (mode == "make_base"sv) {
        ifstream in("make_base.json"s);
        json_reader::JsonIO json(in);

        catalogue_handler.LoadDataFromJson(json);
        catalogue_handler.SerializeData();

    }
    else if (mode == "process_requests"sv) {
        ifstream in("process_requests.json"s);
        json_reader::JsonIO json(in);

        catalogue_handler.LoadDataFromJson(json);
        catalogue_handler.DeserializeData();

        ofstream out("result.json"s);
        catalogue_handler.LoadRequestsAndAnswer(json, out);

    }
    else {
        PrintUsage();
        return 1;
    }
}