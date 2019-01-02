#include "osrm/match_parameters.hpp"
#include "osrm/nearest_parameters.hpp"
#include "osrm/route_parameters.hpp"
#include "osrm/table_parameters.hpp"
#include "osrm/trip_parameters.hpp"

#include "osrm/coordinate.hpp"
#include "osrm/engine_config.hpp"
#include "osrm/json_container.hpp"

#include "osrm/osrm.hpp"
#include "osrm/status.hpp"

#include <chrono>
#include <exception>
#include <iostream>
#include <random>
#include <string>
#include <utility>

#include <cstdlib>

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " data.osrm\n";
        return EXIT_FAILURE;
    }

    using namespace osrm;

    // Configure based on a .osrm base path, and no datasets in shared mem from osrm-datastore
    EngineConfig config;

    config.storage_config = {argv[1]};
    config.use_shared_memory = false;

    // We support two routing speed up techniques:
    // - Contraction Hierarchies (CH): requires extract+contract pre-processing
    // - Multi-Level Dijkstra (MLD): requires extract+partition+customize pre-processing
    //
    config.algorithm = EngineConfig::Algorithm::CH;
    // config.algorithm = EngineConfig::Algorithm::MLD;

    // Routing machine with several services (such as Route, Table, Nearest, Trip, Match)
    const OSRM osrm{config};
    TableParameters params;

    auto generator = std::default_random_engine{}; // NOTE: not random!
    // Random square in London:
    auto latitude_distribution = std::uniform_real_distribution<double>{51.5062628, 51.5293873};
    auto longitude_distribution = std::uniform_real_distribution<double>{-0.124899, -0.0996648};

    auto num_sources = 3000;
    auto num_targets = 3000;
    for (auto i = 0; i < num_sources + num_targets; i++)
    {
        auto latitude = latitude_distribution(generator);
        auto longitude = longitude_distribution(generator);

        params.coordinates.push_back({
            util::FloatLongitude{longitude},
            util::FloatLatitude{latitude},
        });
    }

    // Route in monaco
    for (auto i = 0; i < num_sources; i++)
    {
        params.sources.push_back(i);
    }
    for (auto i = 0; i < num_targets; i++)
    {
        params.destinations.push_back(num_sources + i);
    }

    json::Object result;

    auto start = std::chrono::steady_clock::now();
    auto status = osrm.Table(params, result);
    auto end = std::chrono::steady_clock::now();
    std::cout << "Execution duration: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n";

    if (status == Status::Ok)
    {
        return EXIT_SUCCESS;
    }
    else if (status == Status::Error)
    {
        const auto code = result.values["code"].get<json::String>().value;
        const auto message = result.values["message"].get<json::String>().value;

        std::cout << "Code: " << code << "\n";
        std::cout << "Message: " << code << "\n";
        return EXIT_FAILURE;
    }
}
