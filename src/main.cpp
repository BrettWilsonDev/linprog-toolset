#ifdef EMSCRIPTEN
#include "web/web_bindings.hpp"
#endif

// #include "lpr_core/DEA/dea_solver.hpp"
#include "./lpr_core/adding_acts_cons/adding_activities_and_constraints.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    std::vector<double> objFunc = {60, 30, 20};
    std::vector<std::vector<double>> constraints = {{8, 6, 1, 48, 0},
                                                    {4, 2, 1.5, 20, 0},
                                                    {2, 1.5, 0.5, 8, 0}};

    std::vector<std::vector<double>> addedCon = {
        {0, 0, 1, 5, 0},
    };

    AddingActivitiesAndConstraints solver(true);
    // solver.DoPreliminaries(objFunc, constraints, false);
    // solver.DoAddActivity({1, 2, 4, 5});
    solver.DoAddActivity(objFunc, constraints, false, {20, -1, -1, -1});
    // solver.DoAddConstraint(objFunc, constraints, false, addedCon);

    return 0;
}