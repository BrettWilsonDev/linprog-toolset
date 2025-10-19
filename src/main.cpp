#ifdef EMSCRIPTEN
#include "web/web_bindings.hpp"
#endif

#include "./lpr_core/descent_algorithm/steepest_descent.hpp"


#include <iostream>

int main(int argc, char *argv[])
{
    SteepestDescent steepestDescent;
    steepestDescent.test();

    std::cout << steepestDescent.getOutput();

    return 0;
}