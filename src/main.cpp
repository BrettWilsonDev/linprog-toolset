#ifdef EMSCRIPTEN
#include "web/web_bindings.hpp"
#endif

#include "./lpr_core/cutting_plane/cutting_plane.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    std::vector<double> objFunc = {13, 8};
    std::vector<std::vector<double>> constraints = {
        {1, 2, 10, 0},
        {5, 2, 20, 0},
    };

    CuttingPlane solver(false);

    solver.RunCuttingPlane(objFunc, constraints, false);

    return 0;
}