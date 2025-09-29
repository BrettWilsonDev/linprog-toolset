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
#include "../lpr_core/nearest_neighbour/nearest_neighbour_tsp.hpp"
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

// Helper: convert 3D std::vector to JS array of arrays of arrays
emscripten::val tableausToJS(const std::vector<std::vector<std::vector<double>>> &tableaus)
{
    emscripten::val jsTableaus = emscripten::val::array();
    for (size_t i = 0; i < tableaus.size(); ++i)
    {
        emscripten::val jsTableau = emscripten::val::array();
        for (size_t r = 0; r < tableaus[i].size(); ++r)
        {
            emscripten::val jsRow = emscripten::val::array();
            for (size_t c = 0; c < tableaus[i][r].size(); ++c)
            {
                jsRow.set(c, tableaus[i][r][c]);
            }
            jsTableau.set(r, jsRow);
        }
        jsTableaus.set(i, jsTableau);
    }
    return jsTableaus;
}

emscripten::val simplexResultToJS(const LPRResult &res)
{
    emscripten::val jsRes = emscripten::val::object();

    jsRes.set("tableaus", tableausToJS(res.tableaus));

    emscripten::val jsChanging = emscripten::val::array();
    for (size_t i = 0; i < res.changingVars.size(); ++i)
        jsChanging.set(i, res.changingVars[i]);
    jsRes.set("changingVars", jsChanging);

    jsRes.set("optimalSolution", res.optimalSolution);

    emscripten::val jsPivotCols = emscripten::val::array();
    for (size_t i = 0; i < res.pivotCols.size(); ++i)
        jsPivotCols.set(i, res.pivotCols[i]);
    jsRes.set("pivotCols", jsPivotCols);

    emscripten::val jsPivotRows = emscripten::val::array();
    for (size_t i = 0; i < res.pivotRows.size(); ++i)
        jsPivotRows.set(i, res.pivotRows[i]);
    jsRes.set("pivotRows", jsPivotRows);

    emscripten::val jsHeader = emscripten::val::array();
    for (size_t i = 0; i < res.headerRow.size(); ++i)
        jsHeader.set(i, res.headerRow[i]);
    jsRes.set("headerRow", jsHeader);

    emscripten::val jsPhases = emscripten::val::array();
    for (size_t i = 0; i < res.phases.size(); ++i)
        jsPhases.set(i, res.phases[i]);
    jsRes.set("phases", jsPhases);

    return jsRes;
}

emscripten::val resultToJS(const LPRResult &res)
{
    auto jsRes = simplexResultToJS(res);

    return jsRes;
}

std::tuple<std::vector<double>, std::vector<std::vector<double>>, bool> getInputSimplex(emscripten::val jsObjFunc, emscripten::val jsConstraints, std::string problemType)
{
    std::vector<double> obj;
    unsigned len = jsObjFunc["length"].as<unsigned>();
    for (unsigned i = 0; i < len; ++i)
        obj.push_back(jsObjFunc[i].as<double>());

    std::vector<std::vector<double>> cons;
    unsigned outerLen = jsConstraints["length"].as<unsigned>();
    for (unsigned i = 0; i < outerLen; ++i)
    {
        emscripten::val inner = jsConstraints[i];
        unsigned innerLen = inner["length"].as<unsigned>();
        std::vector<double> row;
        for (unsigned j = 0; j < innerLen; ++j)
            row.push_back(inner[j].as<double>());
        cons.push_back(row);
    }

    bool isMin = (problemType == "Min");

    return std::make_tuple(obj, cons, isMin);
}

// The bound run functions
emscripten::val runDualSimplex(emscripten::val jsObjFunc, emscripten::val jsConstraints, std::string problemType)
{
    auto input = getInputSimplex(jsObjFunc, jsConstraints, problemType);

    DualSimplex dualSimplex(verbose);
    auto blank = dualSimplex.DoDualSimplex(std::get<0>(input), std::get<1>(input), std::get<2>(input));

    auto solverRes = dualSimplex.GetResult();
    LPRResult res = toResult(solverRes);
    // return simplexResultToJS(res);

    auto jsRes = resultToJS(res);
    return jsRes;
}

emscripten::val runTwoPhaseSimplex(emscripten::val jsObjFunc, emscripten::val jsConstraints, std::string problemType)
{
    auto input = getInputSimplex(jsObjFunc, jsConstraints, problemType);

    TwoPhaseSimplex twoPhaseSimplex(verbose);
    auto blank = twoPhaseSimplex.DoTwoPhase(std::get<0>(input), std::get<1>(input), std::get<2>(input));

    auto solverRes = twoPhaseSimplex.GetResult();
    LPRResult res = toResult(solverRes);

    // return simplexResultToJS(res);

    auto jsRes = resultToJS(res);

    // jsRes.set("wStr", twoPhaseSimplex.GetWString());
    jsRes.set("wString", emscripten::val(twoPhaseSimplex.GetWString().c_str()));

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
    // Convert JS arrays to C++ vectors
    std::vector<std::vector<double>> goals;
    for (unsigned i = 0; i < jsGoalConstraints["length"].as<unsigned>(); ++i)
    {
        std::vector<double> row;
        emscripten::val jsRow = jsGoalConstraints[i];
        for (unsigned j = 0; j < jsRow["length"].as<unsigned>(); ++j)
        {
            row.push_back(jsRow[j].as<double>());
        }
        goals.push_back(row);
    }

    std::vector<std::vector<double>> constraints;
    for (unsigned i = 0; i < jsConstraints["length"].as<unsigned>(); ++i)
    {
        std::vector<double> row;
        emscripten::val jsRow = jsConstraints[i];
        for (unsigned j = 0; j < jsRow["length"].as<unsigned>(); ++j)
        {
            row.push_back(jsRow[j].as<double>());
        }
        constraints.push_back(row);
    }

    std::vector<double> penalties;
    for (unsigned i = 0; i < jsPenalties["length"].as<unsigned>(); ++i)
    {
        penalties.push_back(jsPenalties[i].as<double>());
    }

    std::vector<int> orderOverride;
    if (!jsOrderOverride.isUndefined() && jsOrderOverride["length"].as<unsigned>() > 0)
    {
        for (unsigned i = 0; i < jsOrderOverride["length"].as<unsigned>(); ++i)
        {
            orderOverride.push_back(jsOrderOverride[i].as<int>());
        }
    }

    // Solve
    GoalPenaltiesSimplex solver(verbose);
    auto blank = solver.doPenalties(goals, constraints, penalties, orderOverride);

    // Build JS object using getters
    emscripten::val jsResult = emscripten::val::object();

    // Tableaus
    emscripten::val jsTableaus = emscripten::val::array();
    const auto &tableaus = solver.getTableaus();
    for (size_t t = 0; t < tableaus.size(); ++t)
    {
        emscripten::val tbl = emscripten::val::array();
        for (size_t r = 0; r < tableaus[t].size(); ++r)
        {
            emscripten::val row = emscripten::val::array();
            for (size_t c = 0; c < tableaus[t][r].size(); ++c)
            {
                row.set(c, tableaus[t][r][c]);
            }
            tbl.set(r, row);
        }
        jsTableaus.set(t, tbl);
    }
    jsResult.set("tableaus", jsTableaus);

    // GoalMetStrings
    emscripten::val jsGoalMetStrings = emscripten::val::array();
    const auto &gms = solver.getGoalMetStrings();
    for (size_t i = 0; i < gms.size(); ++i)
    {
        emscripten::val row = emscripten::val::array();
        for (size_t j = 0; j < gms[i].size(); ++j)
        {
            row.set(j, gms[i][j]);
        }
        jsGoalMetStrings.set(i, row);
    }
    jsResult.set("goalMetStrings", jsGoalMetStrings);

    // opTable
    jsResult.set("opTable", solver.getOpTable());

    // penaltiesTotals
    emscripten::val jsPenaltiesTotals = emscripten::val::array();
    const auto &pt = solver.getPenaltiesTotals();
    for (size_t i = 0; i < pt.size(); ++i)
    {
        jsPenaltiesTotals.set(i, pt[i]);
    }
    jsResult.set("penaltiesTotals", jsPenaltiesTotals);

    // GuiHeaderRow
    emscripten::val jsGuiHeaderRow = emscripten::val::array();
    const auto &headerRow = solver.getGuiHeaderRow();
    for (size_t i = 0; i < headerRow.size(); ++i)
    {
        jsGuiHeaderRow.set(i, headerRow[i]);
    }
    jsResult.set("headerRow", jsGuiHeaderRow);

    // GuiPivotCols
    emscripten::val jsGuiPivotCols = emscripten::val::array();
    const auto &pivotCols = solver.getGuiPivotCols();
    for (size_t i = 0; i < pivotCols.size(); ++i)
    {
        jsGuiPivotCols.set(i, pivotCols[i]);
    }
    jsResult.set("pivotCols", jsGuiPivotCols);

    // GuiPivotRows
    emscripten::val jsGuiPivotRows = emscripten::val::array();
    const auto &pivotRows = solver.getGuiPivotRows();
    for (size_t i = 0; i < pivotRows.size(); ++i)
    {
        jsGuiPivotRows.set(i, pivotRows[i]);
    }
    jsResult.set("pivotRows", jsGuiPivotRows);

    return jsResult;
}

emscripten::val runGoalPreemptiveSimplex(
    emscripten::val jsGoalConstraints,
    emscripten::val jsConstraints,
    emscripten::val jsOrderOverride = emscripten::val::array())
{
    // Convert JS arrays to C++ vectors
    std::vector<std::vector<double>> goals;
    for (unsigned i = 0; i < jsGoalConstraints["length"].as<unsigned>(); ++i)
    {
        std::vector<double> row;
        emscripten::val jsRow = jsGoalConstraints[i];
        for (unsigned j = 0; j < jsRow["length"].as<unsigned>(); ++j)
        {
            row.push_back(jsRow[j].as<double>());
        }
        goals.push_back(row);
    }

    std::vector<std::vector<double>> constraints;
    for (unsigned i = 0; i < jsConstraints["length"].as<unsigned>(); ++i)
    {
        std::vector<double> row;
        emscripten::val jsRow = jsConstraints[i];
        for (unsigned j = 0; j < jsRow["length"].as<unsigned>(); ++j)
        {
            row.push_back(jsRow[j].as<double>());
        }
        constraints.push_back(row);
    }

    std::vector<int> orderOverride;
    if (!jsOrderOverride.isUndefined() && jsOrderOverride["length"].as<unsigned>() > 0)
    {
        for (unsigned i = 0; i < jsOrderOverride["length"].as<unsigned>(); ++i)
        {
            orderOverride.push_back(jsOrderOverride[i].as<int>());
        }
    }

    // Solve
    GoalPreemptiveSimplex solver(verbose);
    auto blank = solver.doPreemptive(goals, constraints, orderOverride);

    // Build JS object using getters
    emscripten::val jsResult = emscripten::val::object();

    // Tableaus
    emscripten::val jsTableaus = emscripten::val::array();
    const auto &tableaus = solver.getTableaus();
    for (size_t t = 0; t < tableaus.size(); ++t)
    {
        emscripten::val tbl = emscripten::val::array();
        for (size_t r = 0; r < tableaus[t].size(); ++r)
        {
            emscripten::val row = emscripten::val::array();
            for (size_t c = 0; c < tableaus[t][r].size(); ++c)
            {
                row.set(c, tableaus[t][r][c]);
            }
            tbl.set(r, row);
        }
        jsTableaus.set(t, tbl);
    }
    jsResult.set("tableaus", jsTableaus);

    // GoalMetStrings
    emscripten::val jsGoalMetStrings = emscripten::val::array();
    const auto &gms = solver.getGoalMetStrings();
    for (size_t i = 0; i < gms.size(); ++i)
    {
        emscripten::val row = emscripten::val::array();
        for (size_t j = 0; j < gms[i].size(); ++j)
        {
            row.set(j, gms[i][j]);
        }
        jsGoalMetStrings.set(i, row);
    }
    jsResult.set("goalMetStrings", jsGoalMetStrings);

    // opTable
    jsResult.set("opTable", solver.getOpTable());

    // GuiHeaderRow
    emscripten::val jsGuiHeaderRow = emscripten::val::array();
    const auto &headerRow = solver.getGuiHeaderRow();
    for (size_t i = 0; i < headerRow.size(); ++i)
    {
        jsGuiHeaderRow.set(i, headerRow[i]);
    }
    jsResult.set("headerRow", jsGuiHeaderRow);

    // GuiPivotCols
    emscripten::val jsGuiPivotCols = emscripten::val::array();
    const auto &pivotCols = solver.getGuiPivotCols();
    for (size_t i = 0; i < pivotCols.size(); ++i)
    {
        jsGuiPivotCols.set(i, pivotCols[i]);
    }
    jsResult.set("pivotCols", jsGuiPivotCols);

    // GuiPivotRows
    emscripten::val jsGuiPivotRows = emscripten::val::array();
    const auto &pivotRows = solver.getGuiPivotRows();
    for (size_t i = 0; i < pivotRows.size(); ++i)
    {
        jsGuiPivotRows.set(i, pivotRows[i]);
    }
    jsResult.set("pivotRows", jsGuiPivotRows);

    return jsResult;
}

emscripten::val runMathPreliminaries(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin, emscripten::val jsCurrentDeltaSelection, emscripten::val jsAbsRule, emscripten::val jsOptTabLockState, emscripten::val jsSolveDelta, emscripten::val jsValue)
{
    // Convert 1D array -> std::vector<double>
    std::vector<double> objFuncDouble;
    if (!objFunc.isUndefined() && !objFunc.isNull())
    {
        unsigned len = objFunc["length"].as<unsigned>();
        objFuncDouble.reserve(len);
        for (unsigned i = 0; i < len; ++i)
        {
            objFuncDouble.push_back(objFunc[i].as<double>());
        }
    }

    // Convert 2D array -> std::vector<std::vector<double>>
    std::vector<std::vector<double>> constraintsDouble;
    if (!jsConstraints.isUndefined() && !jsConstraints.isNull())
    {
        unsigned rows = jsConstraints["length"].as<unsigned>();
        constraintsDouble.reserve(rows);
        for (unsigned r = 0; r < rows; ++r)
        {
            emscripten::val row = jsConstraints[r];
            unsigned cols = row["length"].as<unsigned>();
            std::vector<double> rowVec;
            rowVec.reserve(cols);
            for (unsigned c = 0; c < cols; ++c)
            {
                rowVec.push_back(row[c].as<double>());
            }
            constraintsDouble.push_back(rowVec);
        }
    }

    // Scalars with defaults
    bool isMin = (!jsIsMin.isUndefined() && !jsIsMin.isNull()) ? jsIsMin.as<bool>() : false;
    std::string currentDeltaSelection =
        (!jsCurrentDeltaSelection.isUndefined() && !jsCurrentDeltaSelection.isNull())
            ? jsCurrentDeltaSelection.as<std::string>()
            : "dStore0";
    bool absRule = (!jsAbsRule.isUndefined() && !jsAbsRule.isNull()) ? jsAbsRule.as<bool>() : false;
    bool optTabLockState = (!jsOptTabLockState.isUndefined() && !jsOptTabLockState.isNull()) ? jsOptTabLockState.as<bool>() : false;
    bool solveDelta = (!jsSolveDelta.isUndefined() && !jsSolveDelta.isNull()) ? jsSolveDelta.as<bool>() : false;

    double doubleValue = jsValue.as<double>();

    MathPreliminaries mathPreliminaries(verbose);
    mathPreliminaries.DoMathPreliminaries(objFuncDouble, constraintsDouble, isMin, currentDeltaSelection, absRule, optTabLockState, solveDelta, doubleValue);

    emscripten::val jsResult = emscripten::val::object();

    // Helper: convert vector<string> -> JS Array
    auto vecStrToJS = [](const std::vector<std::string> &vec)
    {
        emscripten::val arr = emscripten::val::array();
        for (size_t i = 0; i < vec.size(); ++i)
        {
            arr.set(i, vec[i]);
        }
        return arr;
    };

    // Helper: convert vector<vector<string>> -> JS 2D Array
    auto vecVecStrToJS = [&](const std::vector<std::vector<std::string>> &mat)
    {
        emscripten::val arr = emscripten::val::array();
        for (size_t r = 0; r < mat.size(); ++r)
        {
            arr.set(r, vecStrToJS(mat[r]));
        }
        return arr;
    };

    // Helper: vector<int> -> JS Array
    auto vecIntToJS = [](const std::vector<int> &vec)
    {
        emscripten::val arr = emscripten::val::array();
        for (size_t i = 0; i < vec.size(); ++i)
        {
            arr.set(i, vec[i]);
        }
        return arr;
    };

    // Helper: vector<vector<vector<double>>> -> JS 3D Array
    auto vec3DDoubleToJS = [](const std::vector<std::vector<std::vector<double>>> &tensor)
    {
        emscripten::val arr = emscripten::val::array();
        for (size_t i = 0; i < tensor.size(); ++i)
        {
            emscripten::val mat = emscripten::val::array();
            for (size_t j = 0; j < tensor[i].size(); ++j)
            {
                emscripten::val row = emscripten::val::array();
                for (size_t k = 0; k < tensor[i][j].size(); ++k)
                {
                    row.set(k, tensor[i][j][k]);
                }
                mat.set(j, row);
            }
            arr.set(i, mat);
        }
        return arr;
    };

    jsResult.set("cbv", vecStrToJS(mathPreliminaries.getMatrixCbv()));           // 1D
    jsResult.set("B", vecVecStrToJS(mathPreliminaries.getMatrixB()));            // 2D
    jsResult.set("BInv", vecVecStrToJS(mathPreliminaries.getMatrixBNegOne()));   // 2D
    jsResult.set("cbvBInv", vecStrToJS(mathPreliminaries.getMatrixCbvNegOne())); // 1D

    jsResult.set("initialTable", vecVecStrToJS(mathPreliminaries.GetInitialTable()));
    jsResult.set("changingTable", vecVecStrToJS(mathPreliminaries.GetChangingTable()));
    jsResult.set("optimalTable", vecVecStrToJS(mathPreliminaries.GetOptimalTable()));

    jsResult.set("headerRow", vecStrToJS(mathPreliminaries.getHeaderRow()));

    jsResult.set("shouldReOptimize", mathPreliminaries.getShouldReOptimize());
    jsResult.set("pivotCols", vecIntToJS(mathPreliminaries.getPivotCols()));
    jsResult.set("pivotRows", vecIntToJS(mathPreliminaries.getPivotRows()));
    jsResult.set("reOptTableaus", vec3DDoubleToJS(mathPreliminaries.getReOptTableaus()));

    return jsResult;
}

emscripten::val runSensitivityAnalysis(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin, emscripten::val jsCurrentDeltaSelection)
{
    // Convert 1D array -> std::vector<double>
    std::vector<double> objFuncDouble;
    if (!objFunc.isUndefined() && !objFunc.isNull())
    {
        unsigned len = objFunc["length"].as<unsigned>();
        objFuncDouble.reserve(len);
        for (unsigned i = 0; i < len; ++i)
        {
            objFuncDouble.push_back(objFunc[i].as<double>());
        }
    }

    // Convert 2D array -> std::vector<std::vector<double>>
    std::vector<std::vector<double>> constraintsDouble;
    if (!jsConstraints.isUndefined() && !jsConstraints.isNull())
    {
        unsigned rows = jsConstraints["length"].as<unsigned>();
        constraintsDouble.reserve(rows);
        for (unsigned r = 0; r < rows; ++r)
        {
            emscripten::val row = jsConstraints[r];
            unsigned cols = row["length"].as<unsigned>();
            std::vector<double> rowVec;
            rowVec.reserve(cols);
            for (unsigned c = 0; c < cols; ++c)
            {
                rowVec.push_back(row[c].as<double>());
            }
            constraintsDouble.push_back(rowVec);
        }
    }

    // Scalars with defaults
    bool isMin = (!jsIsMin.isUndefined() && !jsIsMin.isNull()) ? jsIsMin.as<bool>() : false;
    std::string currentDeltaSelection =
        (!jsCurrentDeltaSelection.isUndefined() && !jsCurrentDeltaSelection.isNull())
            ? jsCurrentDeltaSelection.as<std::string>()
            : "dStore0";

    SensitivityAnalysis sensitivityAnalysis(verbose);
    sensitivityAnalysis.doSensitivityAnalysisDouble(objFuncDouble, constraintsDouble, isMin, currentDeltaSelection);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("outputString", emscripten::val(sensitivityAnalysis.getOutputString()));

    return jsResult;
}

emscripten::val runDuality(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin)
{
    // Convert 1D array -> std::vector<double>
    std::vector<double> objFuncDouble;
    if (!objFunc.isUndefined() && !objFunc.isNull())
    {
        unsigned len = objFunc["length"].as<unsigned>();
        objFuncDouble.reserve(len);
        for (unsigned i = 0; i < len; ++i)
        {
            objFuncDouble.push_back(objFunc[i].as<double>());
        }
    }

    // Convert 2D array -> std::vector<std::vector<double>>
    std::vector<std::vector<double>> constraintsDouble;
    if (!jsConstraints.isUndefined() && !jsConstraints.isNull())
    {
        unsigned rows = jsConstraints["length"].as<unsigned>();
        constraintsDouble.reserve(rows);
        for (unsigned r = 0; r < rows; ++r)
        {
            emscripten::val row = jsConstraints[r];
            unsigned cols = row["length"].as<unsigned>();
            std::vector<double> rowVec;
            rowVec.reserve(cols);
            for (unsigned c = 0; c < cols; ++c)
            {
                rowVec.push_back(row[c].as<double>());
            }
            constraintsDouble.push_back(rowVec);
        }
    }

    bool isMin = (!jsIsMin.isUndefined() && !jsIsMin.isNull()) ? jsIsMin.as<bool>() : false;

    Duality duality(verbose);

    duality.RunDuality(objFuncDouble, constraintsDouble, isMin);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("outputString", emscripten::val(duality.getOutput()));

    return jsResult;
}

emscripten::val runDEA(emscripten::val jsLpInputs, emscripten::val jsLpOutputs, emscripten::val jsIsMin)
{
    std::vector<std::vector<double>> LpInputs;
    if (!jsLpInputs.isUndefined() && !jsLpInputs.isNull())
    {
        unsigned rows = jsLpInputs["length"].as<unsigned>();
        LpInputs.reserve(rows);
        for (unsigned r = 0; r < rows; ++r)
        {
            emscripten::val row = jsLpInputs[r];
            unsigned cols = row["length"].as<unsigned>();
            std::vector<double> rowVec;
            rowVec.reserve(cols);
            for (unsigned c = 0; c < cols; ++c)
            {
                rowVec.push_back(row[c].as<double>());
            }
            LpInputs.push_back(rowVec);
        }
    }

    std::vector<std::vector<double>> LpOutputs;
    if (!jsLpOutputs.isUndefined() && !jsLpOutputs.isNull())
    {
        unsigned rows = jsLpOutputs["length"].as<unsigned>();
        LpOutputs.reserve(rows);
        for (unsigned r = 0; r < rows; ++r)
        {
            emscripten::val row = jsLpOutputs[r];
            unsigned cols = row["length"].as<unsigned>();
            std::vector<double> rowVec;
            rowVec.reserve(cols);
            for (unsigned c = 0; c < cols; ++c)
            {
                rowVec.push_back(row[c].as<double>());
            }
            LpOutputs.push_back(rowVec);
        }
    }

    bool isMin = (!jsIsMin.isUndefined() && !jsIsMin.isNull()) ? jsIsMin.as<bool>() : false;

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
    // Convert 1D array -> std::vector<double>
    std::vector<double> objFuncDouble;
    if (!objFunc.isUndefined() && !objFunc.isNull())
    {
        unsigned len = objFunc["length"].as<unsigned>();
        objFuncDouble.reserve(len);
        for (unsigned i = 0; i < len; ++i)
        {
            objFuncDouble.push_back(objFunc[i].as<double>());
        }
    }

    // Convert 2D array -> std::vector<std::vector<double>>
    std::vector<std::vector<double>> constraintsDouble;
    if (!jsConstraints.isUndefined() && !jsConstraints.isNull())
    {
        unsigned rows = jsConstraints["length"].as<unsigned>();
        constraintsDouble.reserve(rows);
        for (unsigned r = 0; r < rows; ++r)
        {
            emscripten::val row = jsConstraints[r];
            unsigned cols = row["length"].as<unsigned>();
            std::vector<double> rowVec;
            rowVec.reserve(cols);
            for (unsigned c = 0; c < cols; ++c)
            {
                rowVec.push_back(row[c].as<double>());
            }
            constraintsDouble.push_back(rowVec);
        }
    }

    std::vector<double> newActivity;
    if (!jsNewActivity.isUndefined() && !jsNewActivity.isNull())
    {
        unsigned len = jsNewActivity["length"].as<unsigned>();
        newActivity.reserve(len);
        for (unsigned i = 0; i < len; ++i)
        {
            newActivity.push_back(jsNewActivity[i].as<double>());
        }
    }

    bool isMin = (!jsIsMin.isUndefined() && !jsIsMin.isNull()) ? jsIsMin.as<bool>() : false;

    AddingActivitiesAndConstraints addingActivitiesAndConstraints(verbose);

    addingActivitiesAndConstraints.DoAddActivity(objFuncDouble, constraintsDouble, isMin, newActivity);

    emscripten::val jsResult = emscripten::val::object();

    auto vec3DDoubleToJS = [](const std::vector<std::vector<std::vector<double>>> &tensor)
    {
        emscripten::val arr = emscripten::val::array();
        for (size_t i = 0; i < tensor.size(); ++i)
        {
            emscripten::val mat = emscripten::val::array();
            for (size_t j = 0; j < tensor[i].size(); ++j)
            {
                emscripten::val row = emscripten::val::array();
                for (size_t k = 0; k < tensor[i][j].size(); ++k)
                {
                    row.set(k, tensor[i][j][k]);
                }
                mat.set(j, row);
            }
            arr.set(i, mat);
        }
        return arr;
    };

    auto vecDoubleToJS = [](const std::vector<double> &vec)
    {
        emscripten::val arr = emscripten::val::array();
        for (size_t i = 0; i < vec.size(); ++i)
        {
            arr.set(i, vec[i]);
        }
        return arr;
    };

    auto vecIntToJS = [](const std::vector<int> &vec)
    {
        emscripten::val arr = emscripten::val::array();
        for (size_t i = 0; i < vec.size(); ++i)
        {
            arr.set(i, vec[i]);
        }
        return arr;
    };

    jsResult.set("actCol", vecDoubleToJS(addingActivitiesAndConstraints.getNewActivitysCol()));
    jsResult.set("OptTabs", vec3DDoubleToJS(addingActivitiesAndConstraints.getSolvedTabs()));
    jsResult.set("newOptTabs", vec3DDoubleToJS(addingActivitiesAndConstraints.getNewOptTabs()));
    jsResult.set("pivotRows", vecIntToJS(addingActivitiesAndConstraints.getPivotRows()));
    jsResult.set("pivotCols", vecIntToJS(addingActivitiesAndConstraints.getPivotCols()));

    // jsResult.set("OptTabs", vec3DDoubleToJS(std::vector<std::vector<std::vector<double>>>{addingActivitiesAndConstraints.getNewActivitysCol().begin(), addingActivitiesAndConstraints.getNewActivitysCol().end()}));
    // jsResult.set("newOptTabs", vec3DDoubleToJS(std::vector<std::vector<std::vector<double>>>{addingActivitiesAndConstraints.getNewActivitysCol().begin(), addingActivitiesAndConstraints.getNewActivitysCol().end()}));

    return jsResult;
}

emscripten::val runAddConstraints(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin, emscripten::val jsNewConstraints)
{
    // Convert 1D array -> std::vector<double>
    std::vector<double> objFuncDouble;
    if (!objFunc.isUndefined() && !objFunc.isNull())
    {
        unsigned len = objFunc["length"].as<unsigned>();
        objFuncDouble.reserve(len);
        for (unsigned i = 0; i < len; ++i)
        {
            objFuncDouble.push_back(objFunc[i].as<double>());
        }
    }

    // Convert 2D array -> std::vector<std::vector<double>>
    std::vector<std::vector<double>> constraintsDouble;
    if (!jsConstraints.isUndefined() && !jsConstraints.isNull())
    {
        unsigned rows = jsConstraints["length"].as<unsigned>();
        constraintsDouble.reserve(rows);
        for (unsigned r = 0; r < rows; ++r)
        {
            emscripten::val row = jsConstraints[r];
            unsigned cols = row["length"].as<unsigned>();
            std::vector<double> rowVec;
            rowVec.reserve(cols);
            for (unsigned c = 0; c < cols; ++c)
            {
                rowVec.push_back(row[c].as<double>());
            }
            constraintsDouble.push_back(rowVec);
        }
    }

    std::vector<std::vector<double>> newConstraints;
    if (!jsNewConstraints.isUndefined() && !jsNewConstraints.isNull())
    {
        unsigned rows = jsNewConstraints["length"].as<unsigned>();
        newConstraints.reserve(rows);
        for (unsigned r = 0; r < rows; ++r)
        {
            emscripten::val row = jsNewConstraints[r];
            unsigned cols = row["length"].as<unsigned>();
            std::vector<double> rowVec;
            rowVec.reserve(cols);
            for (unsigned c = 0; c < cols; ++c)
            {
                rowVec.push_back(row[c].as<double>());
            }
            newConstraints.push_back(rowVec);
        }
    }

    bool isMin = (!jsIsMin.isUndefined() && !jsIsMin.isNull()) ? jsIsMin.as<bool>() : false;

    AddingActivitiesAndConstraints addingActivitiesAndConstraints(verbose);

    addingActivitiesAndConstraints.DoAddConstraint(objFuncDouble, constraintsDouble, isMin, newConstraints);

    emscripten::val jsResult = emscripten::val::object();

    auto vec3DDoubleToJS = [](const std::vector<std::vector<std::vector<double>>> &tensor)
    {
        emscripten::val arr = emscripten::val::array();
        for (size_t i = 0; i < tensor.size(); ++i)
        {
            emscripten::val mat = emscripten::val::array();
            for (size_t j = 0; j < tensor[i].size(); ++j)
            {
                emscripten::val row = emscripten::val::array();
                for (size_t k = 0; k < tensor[i][j].size(); ++k)
                {
                    row.set(k, tensor[i][j][k]);
                }
                mat.set(j, row);
            }
            arr.set(i, mat);
        }
        return arr;
    };

    auto vecIntToJS = [](const std::vector<int> &vec)
    {
        emscripten::val arr = emscripten::val::array();
        for (size_t i = 0; i < vec.size(); ++i)
        {
            arr.set(i, vec[i]);
        }
        return arr;
    };

    // jsResult.set("OptTabs", vec3DDoubleToJS(addingActivitiesAndConstraints.getNewActivitysCol()));
    // jsResult.set("newOptTabs", vec3DDoubleToJS(addingActivitiesAndConstraints.getNewActivitysCol()));

    jsResult.set("OptTabs", vec3DDoubleToJS(addingActivitiesAndConstraints.getSolvedTabs()));
    jsResult.set("newOptTabs", vec3DDoubleToJS(addingActivitiesAndConstraints.getNewOptTabs()));
    jsResult.set("pivotRows", vecIntToJS(addingActivitiesAndConstraints.getPivotRows()));
    jsResult.set("pivotCols", vecIntToJS(addingActivitiesAndConstraints.getPivotCols()));

    return jsResult;
}

// emscripten::val runKnapsack(emscripten::val objFunc, emscripten::val jsConstraints, emscripten::val jsIsMin)
emscripten::val runKnapsack(emscripten::val objFunc, emscripten::val jsConstraints)
{
    // Convert 1D array -> std::vector<double>
    std::vector<double> objFuncDouble;
    if (!objFunc.isUndefined() && !objFunc.isNull())
    {
        unsigned len = objFunc["length"].as<unsigned>();
        objFuncDouble.reserve(len);
        for (unsigned i = 0; i < len; ++i)
        {
            objFuncDouble.push_back(objFunc[i].as<double>());
        }
    }

    // Convert 2D array -> std::vector<std::vector<double>>
    std::vector<std::vector<double>> constraintsDouble;
    if (!jsConstraints.isUndefined() && !jsConstraints.isNull())
    {
        unsigned rows = jsConstraints["length"].as<unsigned>();
        constraintsDouble.reserve(rows);
        for (unsigned r = 0; r < rows; ++r)
        {
            emscripten::val row = jsConstraints[r];
            unsigned cols = row["length"].as<unsigned>();
            std::vector<double> rowVec;
            rowVec.reserve(cols);
            for (unsigned c = 0; c < cols; ++c)
            {
                rowVec.push_back(row[c].as<double>());
            }
            constraintsDouble.push_back(rowVec);
        }
    }

    // bool isMin = (!jsIsMin.isUndefined() && !jsIsMin.isNull()) ? jsIsMin.as<bool>() : false;

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
    // Convert 1D array -> std::vector<double>
    std::vector<double> objFuncDouble;
    if (!objFunc.isUndefined() && !objFunc.isNull())
    {
        unsigned len = objFunc["length"].as<unsigned>();
        objFuncDouble.reserve(len);
        for (unsigned i = 0; i < len; ++i)
        {
            objFuncDouble.push_back(objFunc[i].as<double>());
        }
    }

    // Convert 2D array -> std::vector<std::vector<double>>
    std::vector<std::vector<double>> constraintsDouble;
    if (!jsConstraints.isUndefined() && !jsConstraints.isNull())
    {
        unsigned rows = jsConstraints["length"].as<unsigned>();
        constraintsDouble.reserve(rows);
        for (unsigned r = 0; r < rows; ++r)
        {
            emscripten::val row = jsConstraints[r];
            unsigned cols = row["length"].as<unsigned>();
            std::vector<double> rowVec;
            rowVec.reserve(cols);
            for (unsigned c = 0; c < cols; ++c)
            {
                rowVec.push_back(row[c].as<double>());
            }
            constraintsDouble.push_back(rowVec);
        }
    }

    bool isMin = (!jsIsMin.isUndefined() && !jsIsMin.isNull()) ? jsIsMin.as<bool>() : false;

    BranchAndBound branchAndBound(verbose);

    branchAndBound.RunBranchAndBound(objFuncDouble, constraintsDouble, isMin);

    emscripten::val jsResult = emscripten::val::object();

    jsResult.set("json", emscripten::val(branchAndBound.getJSON()));
    jsResult.set("solution", emscripten::val(branchAndBound.getSolutionStr()));

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
    // emscripten::function("runGoalPenaltiesSimplex", &runGoalPenaltiesSimplex);
    // emscripten::function("runGoalPenaltiesSimplex", &runGoalPenaltiesSimplex);
}

// int main()
// {
//     std::cout << "C++ WASM loaded" << std::endl;
//     return 0;
// }

#endif