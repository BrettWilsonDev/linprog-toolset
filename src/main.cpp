#ifdef EMSCRIPTEN
#include "web/web_bindings.hpp"
#endif

#include "./lpr_core/goldenratio_search/golden_section_search.hpp"


#include <iostream>

int main(int argc, char *argv[])
{
    GoldenSectionSearch gss = GoldenSectionSearch();
    gss.Test();

    std::cout << gss.getOutput();

    return 0;
}