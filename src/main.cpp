#ifdef EMSCRIPTEN
#include "web/web_bindings.hpp"
#endif

#include "./lpr_core/nearest_neighbor/nearest_neighbor_tsp.hpp"

#include <iostream>

int main(int argc, char *argv[])
{

    std::vector<std::vector<double>> distanceMatrix = {
        {0, 520, 980, 450, 633},  // City 1
        {520, 0, 204, 888, 557},  // City 2
        {980, 204, 0, 446, 1020}, // City 3
        {450, 888, 446, 0, 249},  // City 4
        {633, 557, 1020, 249, 0}  // City 5
    };

    NearestNeighbour solver(true);

    solver.runNearestNeighbour(distanceMatrix, 1);

    return 0;
}