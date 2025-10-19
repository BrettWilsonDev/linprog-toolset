#ifdef EMSCRIPTEN
#include "web/web_bindings.hpp"
#endif

#include "./lpr_core/machine_scheduling/tardiness_scheduler.hpp"
#include "./lpr_core/machine_scheduling/machine_scheduling_penalty.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    std::vector<std::vector<double>> jobData = {
        {1, 6, 8},  // Job 1: processing time 6, due date 8
        {2, 4, 4},  // Job 2: processing time 4, due date 4
        {3, 5, 12}, // Job 3: processing time 5, due date 12
        {4, 8, 16}  // Job 4: processing time 8, due date 16
    };

    // MachineSchedulingTardiness solver(true);

    // solver.runTardinessScheduler(jobData);

    // Tardiness solver(true);

    // solver.runTardinessScheduler(jobData);

    MachineSchedulingPenalty solver(true);

    solver.runPenaltyScheduler(jobData, {5, 6, 5, 6});

    return 0;
}