#ifdef EMSCRIPTEN
#include "web/web_bindings.hpp"
#endif

#include "./lpr_core/hungarian_algorithm/hungarian_algorithm.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    const double blank = -999;
    std::vector<std::vector<double>> costMatrixWithBlanks = {
        {22, 18, 30, 18},
        {18, blank, 27, 22},
        {26, 20, 28, 28},
        {16, 22, blank, 14},
        {21, blank, 25, 28}};

    Hungarian solver(true);

    solver.runHungarian(costMatrixWithBlanks, false, -999, true);

    return 0;
}