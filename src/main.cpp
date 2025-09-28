#ifdef EMSCRIPTEN
#include "web/web_bindings.hpp"
#endif

#include "./lpr_core/branch_and_bound_knapsack/branch_and_bound_knapsack.hpp"


#include <iostream>

int main(int argc, char *argv[])
{
    std::vector<double> objFunc = {300, 840, 160, 520};
    std::vector<std::vector<double>> constraints = {{7, 15, 3, 13, 23, 0},
                                                    // {4, 2, 1.5, 20, 0},
                                                    // {2, 1.5, 0.5, 8, 0}
                                                };

    KnapSack knapsackSolver(true);
    knapsackSolver.RunBranchAndBoundKnapSack(objFunc, constraints);

    return 0;
}