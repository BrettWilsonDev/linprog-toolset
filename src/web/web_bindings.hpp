#ifdef EMSCRIPTEN

#include <emscripten/val.h>
#include <emscripten/bind.h>

#include <vector>
#include <iostream>
#include <string>
#include <sstream>

#include "../lpr_core/dual_simplex/dual_simplex.hpp"
#include "../lpr_core/branch_and_bound/branch_and_bound.hpp"
#include "../lpr_core/cutting_plane/cutting_plane.hpp"
#include "../lpr_core/duality/duality.hpp"
#include "../lpr_core/branch_and_bound_knapsack/branch_and_bound_knapsack.hpp"
#include "../lpr_core/machine_scheduling/machine_scheduling_penalty.hpp"
#include "../lpr_core/machine_scheduling/tardiness_scheduler.hpp"
#include "../lpr_core/hungarian_algorithm/hungarian_algorithm.hpp"
#include "../lpr_core/nearest_neighbor/nearest_neighbor_tsp.hpp"
#include "../lpr_core/cheapest_insertion/cheapest_insertion_tsp.hpp"
#include "../lpr_core/adding_acts_cons/adding_activities_and_constraints.hpp"
#include "../lpr_core/DEA/dea_solver.hpp"
#include "../lpr_core/goal_simplex/penalties_simplex/goal_penalties_simplex.hpp"
#include "../lpr_core/goal_simplex/preemptive_simplex/goal_preemptive_simplex.hpp"
#include "../lpr_core/two_phase_simplex/two_phase_simplex.hpp"
#include "../lpr_core/math_preliminaries/math_preliminaries.hpp"
#include "../lpr_core/sensitivity_analysis/sensitivity_analysis.hpp"
#include "../lpr_core/descent_algorithm/steepest_descent.hpp"
#include "../lpr_core/graphical_solver/graphical_solver.hpp"
#include "../lpr_core/goldenratio_search/golden_section_search.hpp"

struct LPRResult
{
    std::vector<std::vector<std::vector<double>>> tableaus;
    std::vector<int> pivotCols;
    std::vector<int> pivotRows;
    std::vector<std::string> headerRow;
    std::vector<int> phases;
    double optimalSolution;
    std::vector<double> changingVars;
};

template <typename SolverResult>
LPRResult toResult(const SolverResult &src)
{
    LPRResult dst;
    dst.tableaus = src.tableaus;
    dst.pivotCols = src.pivotCols;
    dst.pivotRows = src.pivotRows;
    dst.headerRow = src.headerRow;
    dst.phases = src.phases;
    dst.optimalSolution = src.optimalSolution;
    dst.changingVars = src.changingVars;
    return dst;
}

inline bool verbose = false;

inline void toggleVerbose()
{
    verbose = !verbose;
    std::cout << "Verbose mode is " << (verbose ? "ON" : "OFF") << std::endl;
}

// Input conversion helpers
std::vector<double> jsToVecDouble(const emscripten::val& jsArr)
{
    if (jsArr.isNull() || jsArr.isUndefined()) return {};
    unsigned len = jsArr["length"].as<unsigned>();
    std::vector<double> vec(len);
    for (unsigned i = 0; i < len; ++i) vec[i] = jsArr[i].as<double>();
    return vec;
}

std::vector<int> jsToVecInt(const emscripten::val& jsArr)
{
    if (jsArr.isNull() || jsArr.isUndefined()) return {};
    unsigned len = jsArr["length"].as<unsigned>();
    std::vector<int> vec(len);
    for (unsigned i = 0; i < len; ++i) vec[i] = jsArr[i].as<int>();
    return vec;
}

std::vector<std::string> jsToVecString(const emscripten::val& jsArr)
{
    if (jsArr.isNull() || jsArr.isUndefined()) return {};
    unsigned len = jsArr["length"].as<unsigned>();
    std::vector<std::string> vec(len);
    for (unsigned i = 0; i < len; ++i) vec[i] = jsArr[i].as<std::string>();
    return vec;
}

std::vector<std::vector<double>> jsToVecVecDouble(const emscripten::val& js2D)
{
    if (js2D.isNull() || js2D.isUndefined()) return {};
    unsigned rows = js2D["length"].as<unsigned>();
    std::vector<std::vector<double>> mat(rows);
    for (unsigned r = 0; r < rows; ++r)
    {
        mat[r] = jsToVecDouble(js2D[r]);
    }
    return mat;
}

std::vector<std::vector<std::string>> jsToVecVecString(const emscripten::val& js2D)
{
    if (js2D.isNull() || js2D.isUndefined()) return {};
    unsigned rows = js2D["length"].as<unsigned>();
    std::vector<std::vector<std::string>> mat(rows);
    for (unsigned r = 0; r < rows; ++r)
    {
        mat[r] = jsToVecString(js2D[r]);
    }
    return mat;
}

// Output conversion helpers
emscripten::val vecToJS(const std::vector<double>& vec)
{
    emscripten::val arr = emscripten::val::array();
    for (size_t i = 0; i < vec.size(); ++i) arr.set(i, vec[i]);
    return arr;
}

emscripten::val vecToJS(const std::vector<int>& vec)
{
    emscripten::val arr = emscripten::val::array();
    for (size_t i = 0; i < vec.size(); ++i) arr.set(i, vec[i]);
    return arr;
}

emscripten::val vecToJS(const std::vector<std::string>& vec)
{
    emscripten::val arr = emscripten::val::array();
    for (size_t i = 0; i < vec.size(); ++i) arr.set(i, vec[i]);
    return arr;
}

emscripten::val vecVecToJS(const std::vector<std::vector<double>>& mat)
{
    emscripten::val arr = emscripten::val::array();
    for (size_t r = 0; r < mat.size(); ++r)
    {
        arr.set(r, vecToJS(mat[r]));
    }
    return arr;
}

emscripten::val vecVecToJS(const std::vector<std::vector<std::string>>& mat)
{
    emscripten::val arr = emscripten::val::array();
    for (size_t r = 0; r < mat.size(); ++r)
    {
        arr.set(r, vecToJS(mat[r]));
    }
    return arr;
}

emscripten::val vecVecVecToJS(const std::vector<std::vector<std::vector<double>>>& tensor)
{
    emscripten::val arr = emscripten::val::array();
    for (size_t i = 0; i < tensor.size(); ++i)
    {
        arr.set(i, vecVecToJS(tensor[i]));
    }
    return arr;
}

emscripten::val simplexResultToJS(const LPRResult &res)
{
    emscripten::val jsRes = emscripten::val::object();
    jsRes.set("tableaus", vecVecVecToJS(res.tableaus));
    jsRes.set("changingVars", vecToJS(res.changingVars));
    jsRes.set("optimalSolution", res.optimalSolution);
    jsRes.set("pivotCols", vecToJS(res.pivotCols));
    jsRes.set("pivotRows", vecToJS(res.pivotRows));
    jsRes.set("headerRow", vecToJS(res.headerRow));
    jsRes.set("phases", vecToJS(res.phases));
    return jsRes;
}

emscripten::val resultToJS(const LPRResult &res)
{
    return simplexResultToJS(res);
}

std::tuple<std::vector<double>, std::vector<std::vector<double>>, bool> getInputSimplex(emscripten::val jsObjFunc, emscripten::val jsConstraints, std::string problemType)
{
    auto obj = jsToVecDouble(jsObjFunc);
    auto cons = jsToVecVecDouble(jsConstraints);
    bool isMin = (problemType == "Min");
    return {obj, cons, isMin};
}

// The bound run functions
emscripten::val runDualSimplex(emscripten::val jsObjFunc, emscripten::val jsConstraints, std::string problemType)
{
    auto input = getInputSimplex(jsObjFunc, jsConstraints, problemType);

    DualSimplex dualSimplex(verbose);
    dualSimplex.DoDualSimplex(std::get<0>(input), std::get<1>(input), std::get<2>(input));

    auto solverRes = dualSimplex.GetResult();
    LPRResult res = toResult(solverRes);
    return resultToJS(res);
}

emscripten::val runTwoPhaseSimplex(emscripten::val jsObjFunc, emscripten::val jsConstraints, std::string problemType)
{
    auto input = getInputSimplex(jsObjFunc, jsConstraints, problemType);

    TwoPhaseSimplex twoPhaseSimplex(verbose);
    twoPhaseSimplex.DoTwoPhase(std::get<0>(input), std::get<1>(input), std::get<2>(input));

    auto solverRes = twoPhaseSimplex.GetResult();
    LPRResult res = toResult(solverRes);

    auto jsRes = resultToJS(res);
    jsRes.set("wString", emscripten::val(twoPhaseSimplex.GetWString()));
    return jsRes;
}

emscripten::val runGraphicalSolver(emscripten::val jsObjFunc,
                                   emscripten::val jsConstraints,
                                   std::string problemType)
{
    auto input = getInputSimplex(jsObjFunc, jsConstraints, problemType);

    GraphicalSolver solver(verbose);

    auto [objFunc, constraints, isMin] = input;

    // Step 1: feasible region & intersections
    auto [feasiblePts, lineSegPts, interPts] = solver.getSortedPoint2ds(constraints);

    // Step 2: compute optimal solution
    auto [optVal, optPt] = solver.solveGraphical(objFunc, feasiblePts, isMin);

    // Step 3: build graph data
    GraphData data = solver.drawGraph(feasiblePts, lineSegPts, interPts, optPt, optVal);

    emscripten::val jsRes = emscripten::val::object();

    // scalar result
    jsRes.set("optimalSolution", data.optimalValue);

    emscripten::val vars = emscripten::val::array();
    vars.set(0, data.optimalPoint2d.x);
    vars.set(1, data.optimalPoint2d.y);
    jsRes.set("changingVars", vars);

    // feasible region points
    emscripten::val feasible = emscripten::val::array();
    for (size_t i = 0; i < data.feasiblePoint2ds.size(); ++i)
    {
        emscripten::val p = emscripten::val::object();
        p.set("x", data.feasiblePoint2ds[i].x);
        p.set("y", data.feasiblePoint2ds[i].y);
        feasible.set(i, p);
    }
    jsRes.set("feasiblePoints", feasible);

    // convex hull (polygon boundary)
    emscripten::val hull = emscripten::val::array();
    for (size_t i = 0; i < data.feasibleHull.size(); ++i)
    {
        emscripten::val p = emscripten::val::object();
        p.set("x", data.feasibleHull[i].x);
        p.set("y", data.feasibleHull[i].y);
        hull.set(i, p);
    }
    jsRes.set("feasibleHull", hull);

    return jsRes;
}

emscripten::val runGoalPenaltiesSimplex(
    emscripten::val jsGoalConstraints,
    emscripten::val jsConstraints,
    emscripten::val jsPenalties,
    emscripten::val jsOrderOverride = emscripten::val::array())
{
    auto goals = jsToVecVecDouble(jsGoalConstraints);
    auto constraints = jsToVecVecDouble(jsConstraints);
    auto penalties = jsToVecDouble(jsPenalties);
    auto orderOverride = jsToVecInt(jsOrderOverride);

    // Solve
    GoalPenaltiesSimplex solver(verbose);
    solver.doPenalties(goals, constraints, penalties, orderOverride);

    // Build JS object using getters
    emscripten::val jsResult = emscripten::val::object();

    // Tableaus
    jsResult.set("tableaus", vecVecVecToJS(solver.getTableaus()));

    // GoalMetStrings
    jsResult.set("goalMetStrings", vecVecToJS(solver.getGoalMetStrings()));

    // opTable
    jsResult.set("opTable", solver.getOpTable());

    // penaltiesTotals
    jsResult.set("penaltiesTotals", vecToJS(solver.getPenaltiesTotals()));

    // GuiHeaderRow
    jsResult.set("headerRow", vecToJS(solver.getGuiHeaderRow()));

    // GuiPivotCols
    jsResult.set("pivotCols", vecToJS(solver.getGuiPivotCols()));

    // GuiPivotRows
    jsResult.set("pivotRows", vecToJS(solver.getGuiPivotRows()));

    return jsResult;
}

emscripten::val runGoalPreemptiveSimplex(
    emscripten::val jsGoalConstraints,
    emscripten::val jsConstraints,
    emscripten::val jsOrderOverride = emscripten::val::array())
{
    auto goals = jsToVecVecDouble(jsGoalConstraints);
    auto constraints = jsToVecVecDouble(jsConstraints);
    auto orderOverride = jsToVecInt(jsOrderOverride);

    // Solve
    GoalPreemptiveSimplex solver(verbose);
    solver.doPreemptive(goals, constraints, orderOverride);

    // Build JS object using getters
    emscripten::val jsResult = emscripten::val::object();

    // Tableaus
    jsResult.set("tableaus", vecVecVecToJS(solver.getTableaus()));

    // GoalMetStrings
    jsResult.set("goalMetStrings", vecVecToJS(solver.getGoalMetStrings()));

    // opTable
    jsResult.set("opTable", solver.getOpTable());

    // GuiHeaderRow
    jsResult.set("headerRow", vecToJS(solver.getGuiHeaderRow()));

    // GuiPivotCols
    jsResult.set("pivotCols", vecToJS(solver.getGuiPivotCols()));

    // GuiPivotRows
    jsResult.set("pivotRows", vecToJS(solver.getGuiPivotRows()));

    return jsResult;
}

emscripten::val runMathPreliminaries(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin, emscripten::val jsCurrentDeltaSelection, emscripten::val jsAbsRule, emscripten::val jsOptTabLockState, emscripten::val jsSolveDelta, emscripten::val jsValue)
{
    auto objFuncDouble = jsToVecDouble(objFunc);
    auto constraintsDouble = jsToVecVecDouble(jsConstraints);

    // Scalars with defaults
    bool isMin = jsIsMin.as<bool>();
    std::string currentDeltaSelection = jsCurrentDeltaSelection.as<std::string>();
    bool absRule = jsAbsRule.as<bool>();
    bool optTabLockState = jsOptTabLockState.as<bool>();
    bool solveDelta = jsSolveDelta.as<bool>();
    double doubleValue = jsValue.as<double>();

    MathPreliminaries mathPreliminaries(verbose);
    mathPreliminaries.DoMathPreliminaries(objFuncDouble, constraintsDouble, isMin, currentDeltaSelection, absRule, optTabLockState, solveDelta, doubleValue);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("cbv", vecToJS(mathPreliminaries.getMatrixCbv()));
    jsResult.set("B", vecVecToJS(mathPreliminaries.getMatrixB()));
    jsResult.set("BInv", vecVecToJS(mathPreliminaries.getMatrixBNegOne()));
    jsResult.set("cbvBInv", vecToJS(mathPreliminaries.getMatrixCbvNegOne()));

    jsResult.set("initialTable", vecVecToJS(mathPreliminaries.GetInitialTable()));
    jsResult.set("changingTable", vecVecToJS(mathPreliminaries.GetChangingTable()));
    jsResult.set("optimalTable", vecVecToJS(mathPreliminaries.GetOptimalTable()));

    jsResult.set("headerRow", vecToJS(mathPreliminaries.getHeaderRow()));

    jsResult.set("shouldReOptimize", mathPreliminaries.getShouldReOptimize());
    jsResult.set("pivotCols", vecToJS(mathPreliminaries.getPivotCols()));
    jsResult.set("pivotRows", vecToJS(mathPreliminaries.getPivotRows()));
    jsResult.set("reOptTableaus", vecVecVecToJS(mathPreliminaries.getReOptTableaus()));

    return jsResult;
}

emscripten::val runSensitivityAnalysis(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin, emscripten::val jsCurrentDeltaSelection)
{
    auto objFuncDouble = jsToVecDouble(objFunc);
    auto constraintsDouble = jsToVecVecDouble(jsConstraints);

    bool isMin = jsIsMin.as<bool>();
    std::string currentDeltaSelection = jsCurrentDeltaSelection.as<std::string>();

    SensitivityAnalysis sensitivityAnalysis(verbose);
    sensitivityAnalysis.doSensitivityAnalysisDouble(objFuncDouble, constraintsDouble, isMin, currentDeltaSelection);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("outputString", emscripten::val(sensitivityAnalysis.getOutputString()));

    return jsResult;
}

emscripten::val runDuality(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin)
{
    auto objFuncDouble = jsToVecDouble(objFunc);
    auto constraintsDouble = jsToVecVecDouble(jsConstraints);

    bool isMin = jsIsMin.as<bool>();

    Duality duality(verbose);

    duality.RunDuality(objFuncDouble, constraintsDouble, isMin);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("outputString", emscripten::val(duality.getOutput()));

    return jsResult;
}

emscripten::val runDEA(emscripten::val jsLpInputs, emscripten::val jsLpOutputs, emscripten::val jsIsMin)
{
    auto LpInputs = jsToVecVecDouble(jsLpInputs);
    auto LpOutputs = jsToVecVecDouble(jsLpOutputs);

    bool isMin = jsIsMin.as<bool>();

    DEASolver dea(verbose);

    if (isMin)
    {
        dea.setProblemType("Min");
    }
    else
    {
        dea.setProblemType("Max");
    }

    dea.doDEA(LpInputs, LpOutputs, isMin);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("outputString", emscripten::val(dea.getOutput()));

    return jsResult;
}

emscripten::val runAddActivity(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin, emscripten::val jsNewActivity)
{
    auto objFuncDouble = jsToVecDouble(objFunc);
    auto constraintsDouble = jsToVecVecDouble(jsConstraints);
    auto newActivity = jsToVecDouble(jsNewActivity);

    bool isMin = jsIsMin.as<bool>();

    AddingActivitiesAndConstraints addingActivitiesAndConstraints(verbose);

    addingActivitiesAndConstraints.DoAddActivity(objFuncDouble, constraintsDouble, isMin, newActivity);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("actCol", vecToJS(addingActivitiesAndConstraints.getNewActivitysCol()));
    jsResult.set("OptTabs", vecVecVecToJS(addingActivitiesAndConstraints.getSolvedTabs()));
    jsResult.set("newOptTabs", vecVecVecToJS(addingActivitiesAndConstraints.getNewOptTabs()));
    jsResult.set("pivotRows", vecToJS(addingActivitiesAndConstraints.getPivotRows()));
    jsResult.set("pivotCols", vecToJS(addingActivitiesAndConstraints.getPivotCols()));

    return jsResult;
}

emscripten::val runAddConstraints(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin, emscripten::val jsNewConstraints)
{
    auto objFuncDouble = jsToVecDouble(objFunc);
    auto constraintsDouble = jsToVecVecDouble(jsConstraints);
    auto newConstraints = jsToVecVecDouble(jsNewConstraints);

    bool isMin = jsIsMin.as<bool>();

    AddingActivitiesAndConstraints addingActivitiesAndConstraints(verbose);

    addingActivitiesAndConstraints.DoAddConstraint(objFuncDouble, constraintsDouble, isMin, newConstraints);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("OptTabs", vecVecVecToJS(addingActivitiesAndConstraints.getSolvedTabs()));
    jsResult.set("newOptTabs", vecVecVecToJS(addingActivitiesAndConstraints.getNewOptTabs()));
    jsResult.set("pivotRows", vecToJS(addingActivitiesAndConstraints.getPivotRows()));
    jsResult.set("pivotCols", vecToJS(addingActivitiesAndConstraints.getPivotCols()));

    return jsResult;
}

emscripten::val runKnapsack(emscripten::val objFunc, emscripten::val jsConstraints)
{
    auto objFuncDouble = jsToVecDouble(objFunc);
    auto constraintsDouble = jsToVecVecDouble(jsConstraints);

    KnapSack knapsack(verbose);

    knapsack.RunBranchAndBoundKnapSack(objFuncDouble, constraintsDouble);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("json", emscripten::val(knapsack.getJSON()));
    jsResult.set("ranking", emscripten::val(knapsack.getRanking()));
    jsResult.set("solution", emscripten::val(knapsack.getFinalSolution()));

    return jsResult;
}

emscripten::val runBranchAndBound(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin)
{
    auto objFuncDouble = jsToVecDouble(objFunc);
    auto constraintsDouble = jsToVecVecDouble(jsConstraints);

    bool isMin = jsIsMin.as<bool>();

    BranchAndBound branchAndBound(verbose);

    branchAndBound.RunBranchAndBound(objFuncDouble, constraintsDouble, isMin);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("json", emscripten::val(branchAndBound.getJSON()));
    jsResult.set("solution", emscripten::val(branchAndBound.getSolutionStr()));

    return jsResult;
}

emscripten::val runCuttingPlane(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin)
{
    auto objFuncDouble = jsToVecDouble(objFunc);
    auto constraintsDouble = jsToVecVecDouble(jsConstraints);

    bool isMin = jsIsMin.as<bool>();

    CuttingPlane cuttingPlane(verbose);

    cuttingPlane.RunCuttingPlane(objFuncDouble, constraintsDouble, isMin);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("solution", emscripten::val(cuttingPlane.getCollectedOutput()));

    return jsResult;
}

emscripten::val runCheapestInsertion(emscripten::val jsDistanceMatrix, emscripten::val jsStartCity)
{
    auto distanceMatrix = jsToVecVecDouble(jsDistanceMatrix);

    int startCity = jsStartCity.as<int>();

    CheapestInsertion cheapestInsertion(verbose);

    cheapestInsertion.runCheapestInsertion(distanceMatrix, startCity);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("solution", emscripten::val(cheapestInsertion.getCollectedOutput()));

    return jsResult;
}

emscripten::val runNearestNeighbor(emscripten::val jsDistanceMatrix, emscripten::val jsStartCity)
{
    auto distanceMatrix = jsToVecVecDouble(jsDistanceMatrix);

    int startCity = jsStartCity.as<int>();

    NearestNeighbour nearestNeighbour(verbose);

    nearestNeighbour.runNearestNeighbour(distanceMatrix, startCity);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("solution", emscripten::val(nearestNeighbour.getCollectedOutput()));

    return jsResult;
}

emscripten::val runHungarianAlgorithm(emscripten::val jsCostMatrixWithBlanks, emscripten::val jsMaximize, emscripten::val jsBlankValue, emscripten::val jsHasBlankValue)
{
    auto costMatrixWithBlanks = jsToVecVecDouble(jsCostMatrixWithBlanks);

    bool maximize = jsMaximize.as<bool>();
    double blankValue = jsBlankValue.as<double>();
    bool hasBlankValue = jsHasBlankValue.as<bool>();

    Hungarian hungarian(verbose);

    hungarian.runHungarian(costMatrixWithBlanks, maximize, blankValue, hasBlankValue);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("solution", emscripten::val(hungarian.getCollectedOutput()));

    return jsResult;
}

emscripten::val runMachineSchedulingTardinessScheduler(emscripten::val jsJobData)
{
    auto jobData = jsToVecVecDouble(jsJobData);

    MachineSchedulingTardiness solver(verbose);

    solver.runTardinessScheduler(jobData);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("json", emscripten::val(solver.getJSON()));

    return jsResult;
}

emscripten::val runMachineSchedulingPenaltyScheduler(emscripten::val jsJobData, emscripten::val jsPenalty)
{
    auto jobData = jsToVecVecDouble(jsJobData);
    auto penalty = jsToVecDouble(jsPenalty);

    MachineSchedulingPenalty solver(verbose);

    solver.runPenaltyScheduler(jobData, penalty);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("json", emscripten::val(solver.getJSON()));

    return jsResult;
}

emscripten::val runGoldenSectionSearch(emscripten::val jsFunction, emscripten::val jsXLower, emscripten::val jsXUpper, emscripten::val jsTol, emscripten::val jsIsMin, emscripten::val jsMaxIter)
{
    std::string function = jsFunction.as<std::string>();
    double xLower = jsXLower.as<double>();
    double xUpper = jsXUpper.as<double>();
    double tol = jsTol.as<double>();
    bool isMin = jsIsMin.as<bool>();
    int maxIter = jsMaxIter.as<int>();

    GoldenSectionSearch solver(verbose);

    solver.DoGoldenRatioSearch(function, xLower, xUpper, tol, isMin, maxIter);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("outputString", emscripten::val(solver.getOutput()));

    return jsResult;
}

emscripten::val runSteepestDescent(emscripten::val jsFunction, emscripten::val jsVars, emscripten::val jsPoints, emscripten::val jsIsMin)
{
    std::string function = jsFunction.as<std::string>();
    auto vars = jsToVecString(jsVars);
    auto points = jsToVecDouble(jsPoints);
    bool isMin = jsIsMin.as<bool>();

    SteepestDescent solver(verbose);
    solver.DoSteepestDescent(function, vars, points, isMin);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("outputString", emscripten::val(solver.getOutput()));

    return jsResult;
}

// Bindings
EMSCRIPTEN_BINDINGS(simplex_module)
{
    emscripten::function("toggleVerbose", &toggleVerbose);
    emscripten::function("runDualSimplex", &runDualSimplex);
    emscripten::function("runTwoPhaseSimplex", &runTwoPhaseSimplex);
    emscripten::function("runGraphicalSolver", &runGraphicalSolver);
    emscripten::function("runGoalPenaltiesSimplex", &runGoalPenaltiesSimplex);
    emscripten::function("runGoalPreemptiveSimplex", &runGoalPreemptiveSimplex);
    emscripten::function("runMathPreliminaries", &runMathPreliminaries);
    emscripten::function("runSensitivityAnalysis", &runSensitivityAnalysis);
    emscripten::function("runDuality", &runDuality);
    emscripten::function("runDEA", &runDEA);
    emscripten::function("runAddActivity", &runAddActivity);
    emscripten::function("runAddConstraints", &runAddConstraints);
    emscripten::function("runKnapsack", &runKnapsack);
    emscripten::function("runBranchAndBound", &runBranchAndBound);
    emscripten::function("runCuttingPlane", &runCuttingPlane);
    emscripten::function("runCheapestInsertion", &runCheapestInsertion);
    emscripten::function("runNearestNeighbor", &runNearestNeighbor);
    emscripten::function("runHungarianAlgorithm", &runHungarianAlgorithm);
    emscripten::function("runMachineSchedulingTardinessScheduler", &runMachineSchedulingTardinessScheduler);
    emscripten::function("runMachineSchedulingPenaltyScheduler", &runMachineSchedulingPenaltyScheduler);
    emscripten::function("runGoldenSectionSearch", &runGoldenSectionSearch);
    emscripten::function("runSteepestDescent", &runSteepestDescent);
}

#endif