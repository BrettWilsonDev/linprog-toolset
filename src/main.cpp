#ifdef EMSCRIPTEN
#include "web/web_bindings.hpp"
#endif

#include "./lpr_core/branch_and_bound/branch_and_bound.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    // std::vector<double> objFunc = {13, 8};
    // std::vector<std::vector<double>> constraints = {
    //     {1, 2, 10, 0},
    //     {5, 2, 20, 0},
    //     // {2, 1.5, 0.5, 8, 0}
    // };

    std::vector<double> objFunc = {2, 3, 3, 5, 2, 4};
    std::vector<std::vector<double>> constraints = {
        {11, 8, 6, 14, 10, 10, 40, 0},
        {1, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 1, 0, 0, 0, 0, 0, 1, 0},
        {0, 0, 1, 0, 0, 0, 0, 1, 0},
        {0, 0, 0, 1, 0, 0, 0, 1, 0},
        {0, 0, 0, 0, 1, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 1, 0, 1, 0},
    };

    BranchAndBound solver(false);
    solver.RunBranchAndBound(objFunc, constraints, false);

    return 0;
}