#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <tuple>
#include <algorithm>
#include <cmath>
#include <iomanip>

#include "symbolicc++.h"
#include "dual_simplex.hpp"

class MathPreliminaries
{
private:
    bool isConsoleOutput;
    int testInputSelected;
    std::unique_ptr<DualSimplex> dual;
    Symbolic d;

    std::vector<std::vector<std::vector<Symbolic>>> globalOptimalTab;
    std::vector<std::string> globalHeaderRow;

    std::string problemType;
    std::string absProblemType;

    int amtOfObjVars;
    std::vector<double> objFunc;

    std::vector<std::vector<double>> constraints;
    std::vector<std::string> signItems;
    std::vector<int> signItemsChoices;

    int amtOfConstraints;
    std::vector<std::vector<Symbolic>> changingTable;

    Symbolic matCbv;
    Symbolic matB;
    Symbolic matBNegOne;
    Symbolic matCbvNegOne;

    bool absRule;
    std::string lockOptTab;
    bool optTabLockState;

    bool isAllDeltaCRow;
    bool isSingleDeltaCRow;
    bool isSingleDeltaARow;
    int singleCIndex;
    int singleAIndex;
    bool isDeltaZCol;
    bool isAllDeltaRows;
    bool isFormulaDeltaChanged;

    std::string currentDeltaSelection;

    bool solveDelta;

    std::vector<std::vector<std::string>> changingTableOut;
    std::vector<std::vector<std::string>> optimalTableOut;
    std::vector<std::vector<std::string>> initialTableOut;

    Symbolic matrixCbvOut;
    Symbolic matrixBOut;
    Symbolic matrixBNegOneOut;
    Symbolic matrixCbvNegOneOut;

    std::vector<std::string> headerRowOut;

    double valueRange;

    std::vector<std::vector<std::vector<double>>> reOptTableaus;
    std::vector<int> pivotCols;
    std::vector<int> pivotRows;

    bool shouldReoptimize;
    
    std::vector<std::string> influencedFormulasOutput;

public:
    MathPreliminaries(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput), d("d")
    {
        testInputSelected = -1;
        dual = std::make_unique<DualSimplex>();

        globalOptimalTab.clear();
        globalHeaderRow.clear();

        problemType = "Max";
        absProblemType = "abs Off";

        amtOfObjVars = 2;
        objFunc = {0.0, 0.0};

        constraints = {{0.0, 0.0, 0.0, 0.0}};
        signItems = {"<=", ">="};
        signItemsChoices = {0};

        amtOfConstraints = 1;
        changingTable.clear();

        matCbv = Symbolic("", 1, 1);
        matB = Symbolic("", 1, 1);
        matBNegOne = Symbolic("", 1, 1);
        matCbvNegOne = Symbolic("", 1, 1);

        absRule = false;
        lockOptTab = "off";
        optTabLockState = false;

        solveDelta = false;
        // deltaSolve = "off";

        isAllDeltaCRow = false;
        isSingleDeltaCRow = false;
        isSingleDeltaARow = false;
        singleCIndex = -1;
        singleAIndex = -1;
        isDeltaZCol = false;
        isAllDeltaRows = false;
        isFormulaDeltaChanged = false;

        currentDeltaSelection = "dStore0";

        valueRange = 0;

        shouldReoptimize = false;
    }

    std::tuple<std::vector<double>, std::vector<std::vector<double>>, bool> testInput(int testNum = -1)
    {
        bool isMin = false;
        std::vector<double> objFunc;
        std::vector<std::vector<double>> constraints;

        if (testNum == 0)
        {
            objFunc = {60, 30, 20};
            constraints = {{8, 6, 1, 48, 0},
                           {4, 2, 1.5, 20, 0},
                           {2, 1.5, 0.5, 8, 0}};
        }
        else if (testNum == 1)
        {
            objFunc = {100, 30};
            constraints = {{0, 1, 3, 1},
                           {1, 1, 7, 0},
                           {10, 4, 40, 0}};
        }
        else if (testNum == 2)
        {
            objFunc = {30, 28, 26, 30};
            constraints = {{8, 8, 4, 4, 160, 0},
                           {1, 0, 0, 0, 5, 0},
                           {1, 0, 0, 0, 5, 1},
                           {1, 1, 1, 1, 20, 1}};
        }
        else if (testNum == 3)
        {
            objFunc = {10, 50, 80, 100};
            constraints = {{1, 4, 4, 8, 140, 0},
                           {1, 0, 0, 0, 50, 0},
                           {1, 0, 0, 0, 50, 1},
                           {1, 1, 1, 1, 70, 1}};
        }
        else if (testNum == 4)
        {
            objFunc = {3, 2};
            constraints = {{2, 1, 100, 0},
                           {1, 1, 80, 0},
                           {1, 0, 40, 0}};
        }
        else if (testNum == 5)
        {
            objFunc = {120, 80};
            constraints = {{8, 4, 160, 0},
                           {4, 4, 100, 0},
                           {1, 0, 17, 0},
                           {1, 0, 5, 1},
                           {0, 1, 17, 0},
                           {0, 1, 2, 1},
                           {1, -1, 0, 1},
                           {1, -4, 0, 0}};
        }

        if (testNum == -1)
        {
            return std::make_tuple(std::vector<double>(), std::vector<std::vector<double>>(), false);
        }
        else
        {
            return make_tuple(objFunc, constraints, isMin);
        }
    }

    std::vector<double> scrubDelta(const std::vector<Symbolic> &lst)
    {
        std::vector<double> cleaned;
        cleaned.reserve(lst.size());

        for (const auto &elem : lst)
        {
            double val = (double)(elem[d == 0]);
            cleaned.push_back(val);
        }

        return cleaned;
    }

    // bool isAllDeltaCRow;
    // bool isSingleDeltaCRow;
    // bool isSingleDeltaARow;
    // int singleCIndex;
    // int singleAIndex;
    // bool isDeltaZCol;
    // bool isAllDeltaRows;
    // bool isFormulaDeltaChanged;
    // std::vector<std::string> influencedFormulasOutput;

    void detectAndPrintInfluencedFormulas(
        const std::vector<std::vector<Symbolic>> &changingTable,
        const Symbolic &matrixB,
        const Symbolic &matrixCbvNegOne,
        size_t objFuncSize)
    {
        // Clear previous output
        influencedFormulasOutput.clear();

        // Reset flags
        isAllDeltaCRow = false;
        isSingleDeltaCRow = false;
        isSingleDeltaARow = false;
        isDeltaZCol = false;
        isAllDeltaRows = false;
        isFormulaDeltaChanged = false;
        singleCIndex = -1;
        singleAIndex = -1;

        // Check if any element in changingTable contains delta
        for (size_t i = 0; i < changingTable.size(); i++)
        {
            for (size_t j = 0; j < changingTable[i].size(); j++)
            {
                try
                {
                    const Symbolic &expr = changingTable[i][j];
                    std::string tmpName = "**sym_tmp*" + std::to_string(i) + "*" + std::to_string(j);
                    Symbolic tmp(tmpName.c_str());

                    std::ostringstream oss;
                    oss << expr;
                    std::string before = oss.str();
                    oss.str("");
                    oss.clear();
                    oss << expr[d == tmp];
                    std::string after = oss.str();

                    bool contains_d = (before != after);

                    if (!contains_d)
                    {
                        try
                        {
                            double v0 = double(expr[d == 0]);
                            double v1 = double(expr[d == 1]);
                            if (v0 != v1)
                                contains_d = true;
                        }
                        catch (...)
                        {
                            contains_d = true;
                        }
                    }

                    if (contains_d)
                    {
                        isFormulaDeltaChanged = true;
                        break;
                    }
                }
                catch (...)
                {
                }
            }
            if (isFormulaDeltaChanged)
                break;
        }

        if (!isFormulaDeltaChanged)
            return;

        influencedFormulasOutput.push_back("Mathematical Preliminary formulas that will be influenced");

        if (isConsoleOutput)
        {
            std::cout << "\nMathematical Preliminary formulas that will be influenced" << std::endl;
        }

        // Check if all rows in matrixB contain delta
        int rows = matrixB.rows();
        int cols = matrixB.columns();
        for (int i = 0; i < rows && !isAllDeltaRows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                try
                {
                    const Symbolic &expr = matrixB(i, j);
                    std::string tmpName = "**sym_tmp_b*" + std::to_string(i) + "*" + std::to_string(j);
                    Symbolic tmp(tmpName.c_str());

                    std::ostringstream oss;
                    oss << expr;
                    std::string before = oss.str();
                    oss.str("");
                    oss.clear();
                    oss << expr[d == tmp];
                    std::string after = oss.str();

                    if (before != after)
                    {
                        isAllDeltaRows = true;
                        break;
                    }

                    try
                    {
                        double v0 = double(expr[d == 0]);
                        double v1 = double(expr[d == 1]);
                        if (v0 != v1)
                        {
                            isAllDeltaRows = true;
                            break;
                        }
                    }
                    catch (...)
                    {
                    }
                }
                catch (...)
                {
                }
            }
        }

        if (isAllDeltaRows)
        {
            influencedFormulasOutput.push_back("All formulas are influenced");

            if (isConsoleOutput)
            {
                std::cout << "All formulas are influenced" << std::endl;
            }
            return;
        }

        // Check if CbvNegOne contains delta (all C row)
        int cbvRows = matrixCbvNegOne.rows();
        for (int i = 0; i < cbvRows; i++)
        {
            try
            {
                const Symbolic &expr = matrixCbvNegOne(i, 0);
                std::string tmpName = "**sym_tmp_cbv*" + std::to_string(i);
                Symbolic tmp(tmpName.c_str());

                std::ostringstream oss;
                oss << expr;
                std::string before = oss.str();
                oss.str("");
                oss.clear();
                oss << expr[d == tmp];
                std::string after = oss.str();

                if (before != after)
                {
                    isAllDeltaCRow = true;
                    break;
                }

                try
                {
                    double v0 = double(expr[d == 0]);
                    double v1 = double(expr[d == 1]);
                    if (v0 != v1)
                    {
                        isAllDeltaCRow = true;
                        break;
                    }
                }
                catch (...)
                {
                }
            }
            catch (...)
            {
            }
        }

        if (isAllDeltaCRow)
        {
            influencedFormulasOutput.push_back("CBVB^-1");
            influencedFormulasOutput.push_back("Z* = CBVB^-1.b");

            if (isConsoleOutput)
            {
                std::cout << "CBVB^-1" << std::endl;
                std::cout << "Z* = CBVB^-1.b" << std::endl;
            }

            for (size_t i = 0; i < changingTable[0].size() - 1; i++)
            {
                std::string formula;
                if (i < objFuncSize)
                {
                    formula = "C" + std::to_string(i + 1) + "* = (CBVB^-1.A" +
                              std::to_string(i + 1) + ") - C" + std::to_string(i + 1);
                }
                else
                {
                    formula = "S" + std::to_string(i - objFuncSize + 1) +
                              "* = (CBVB^-1.A" + std::to_string(i + 1) + ") - C" +
                              std::to_string(i + 1);
                }
                influencedFormulasOutput.push_back(formula);

                if (isConsoleOutput)
                {
                    std::cout << formula << std::endl;
                }
            }
        }

        // Check for single delta C row
        if (!isAllDeltaCRow)
        {
            for (size_t i = 1; i < changingTable.size(); i++)
            {
                for (size_t j = 0; j < changingTable[i].size() - 1; j++)
                {
                    try
                    {
                        const Symbolic &expr = changingTable[0][j];
                        std::string tmpName = "**sym_tmp_c*" + std::to_string(j);
                        Symbolic tmp(tmpName.c_str());

                        std::ostringstream oss;
                        oss << expr;
                        std::string before = oss.str();
                        oss.str("");
                        oss.clear();
                        oss << expr[d == tmp];
                        std::string after = oss.str();

                        if (before != after)
                        {
                            isSingleDeltaCRow = true;
                            singleCIndex = j + 1;
                            break;
                        }

                        try
                        {
                            double v0 = double(expr[d == 0]);
                            double v1 = double(expr[d == 1]);
                            if (v0 != v1)
                            {
                                isSingleDeltaCRow = true;
                                singleCIndex = j + 1;
                                break;
                            }
                        }
                        catch (...)
                        {
                        }
                    }
                    catch (...)
                    {
                    }
                }
                if (isSingleDeltaCRow)
                    break;
            }
        }

        // Check for single delta A row
        if (!isAllDeltaCRow)
        {
            for (size_t i = 1; i < changingTable.size(); i++)
            {
                for (size_t j = 0; j < changingTable[i].size() - 1; j++)
                {
                    try
                    {
                        const Symbolic &expr = changingTable[i][j];
                        std::string tmpName = "**sym_tmp_a*" + std::to_string(i) + "*" + std::to_string(j);
                        Symbolic tmp(tmpName.c_str());

                        std::ostringstream oss;
                        oss << expr;
                        std::string before = oss.str();
                        oss.str("");
                        oss.clear();
                        oss << expr[d == tmp];
                        std::string after = oss.str();

                        if (before != after)
                        {
                            isSingleDeltaARow = true;
                            singleAIndex = j + 1;
                            break;
                        }

                        try
                        {
                            double v0 = double(expr[d == 0]);
                            double v1 = double(expr[d == 1]);
                            if (v0 != v1)
                            {
                                isSingleDeltaARow = true;
                                singleAIndex = j + 1;
                                break;
                            }
                        }
                        catch (...)
                        {
                        }
                    }
                    catch (...)
                    {
                    }
                }
                if (isSingleDeltaARow)
                    break;
            }
        }

        if (isSingleDeltaCRow)
        {
            std::string formula = "C" + std::to_string(singleCIndex) +
                                  "* = (CBVB^-1.A" + std::to_string(singleCIndex) +
                                  ") - C" + std::to_string(singleCIndex);
            influencedFormulasOutput.push_back(formula);

            if (isConsoleOutput)
            {
                std::cout << formula << std::endl;
            }
        }

        if (isSingleDeltaARow)
        {
            std::string formula = "A" + std::to_string(singleAIndex) +
                                  "* = B^-1.A" + std::to_string(singleAIndex);
            influencedFormulasOutput.push_back(formula);

            if (isConsoleOutput)
            {
                std::cout << formula << std::endl;
            }
        }

        // Check for delta in Z column (RHS)
        for (size_t i = 1; i < changingTable.size(); i++)
        {
            try
            {
                const Symbolic &expr = changingTable[i].back();
                std::string tmpName = "**sym_tmp_rhs*" + std::to_string(i);
                Symbolic tmp(tmpName.c_str());

                std::ostringstream oss;
                oss << expr;
                std::string before = oss.str();
                oss.str("");
                oss.clear();
                oss << expr[d == tmp];
                std::string after = oss.str();

                if (before != after)
                {
                    isDeltaZCol = true;
                    break;
                }

                try
                {
                    double v0 = double(expr[d == 0]);
                    double v1 = double(expr[d == 1]);
                    if (v0 != v1)
                    {
                        isDeltaZCol = true;
                        break;
                    }
                }
                catch (...)
                {
                }
            }
            catch (...)
            {
            }
        }

        if (isDeltaZCol)
        {
            influencedFormulasOutput.push_back("Z* = CBVB^-1.b");
            influencedFormulasOutput.push_back("b* = B^-1.b");

            if (isConsoleOutput)
            {
                std::cout << "Z* = CBVB^-1.b" << std::endl;
                std::cout << "b* = B^-1.b" << std::endl;
            }
        }
    }

    std::vector<std::vector<Symbolic>> doFormulationOperation(const std::vector<Symbolic> &objFunc,
                                                              const std::vector<std::vector<Symbolic>> &constraints,
                                                              bool absRule = false)
    {
        int excessCount = 0;
        int slackCount = 0;

        // Count excess and slack variables
        for (size_t i = 0; i < constraints.size(); i++)
        {
            double signVal = (double)(constraints[i].back()[d == 0]); // Evaluate to double
            if (std::abs(signVal - 1.0) < 1e-10)                      // Check if close to 1.0 for >=
            {
                excessCount++;
            }
            else
            {
                slackCount++;
            }
        }

        // Make a copy of constraints for modification
        std::vector<std::vector<Symbolic>> modConstraints = constraints;

        // Multiply >= constraints by -1
        for (size_t i = 0; i < modConstraints.size(); i++)
        {
            double signVal = (double)(modConstraints[i].back()[d == 0]); // Evaluate to double
            if (std::abs(signVal - 1.0) < 1e-10)                         // Check if close to 1.0 for >=
            {
                for (size_t j = 0; j < modConstraints[i].size(); j++)
                {
                    modConstraints[i][j] = -1 * modConstraints[i][j];
                }
            }
        }

        // Remove the sign indicator (last element)
        for (size_t i = 0; i < modConstraints.size(); i++)
        {
            modConstraints[i].pop_back();
        }

        int tableSizeH = modConstraints.size() + 1;
        int tableSizeW = excessCount + slackCount + 1 + objFunc.size();

        std::vector<std::vector<Symbolic>> opTable(tableSizeH, std::vector<Symbolic>(tableSizeW, 0));

        // Set objective function coefficients
        for (size_t i = 0; i < objFunc.size(); i++)
        {
            opTable[0][i] = -objFunc[i];
        }

        // Set constraint coefficients and RHS
        for (size_t i = 0; i < modConstraints.size(); i++)
        {
            for (size_t j = 0; j < modConstraints[i].size() - 1; j++)
            {
                opTable[i + 1][j] = modConstraints[i][j];
            }
            opTable[i + 1][tableSizeW - 1] = modConstraints[i].back();
        }

        // Add slack and excess variable coefficients (identity matrix)
        for (size_t i = 1; i < opTable.size(); i++)
        {
            for (size_t j = objFunc.size(); j < opTable[i].size() - 1; j++)
            {
                if (j == i + objFunc.size() - 1)
                {
                    opTable[i][j] = 1;
                }
            }
        }

        // Apply absolute value rule if needed
        if (absRule)
        {
            // Apply abs to objective function row
            for (size_t i = 0; i < opTable[0].size(); i++)
            {
                double val = (double)(opTable[0][i][d == 0]); // Evaluate at d=0
                opTable[0][i] = Symbolic(std::abs(val));      // Apply std::abs and convert back to Symbolic
            }

            // Apply abs to constraint coefficients for decision variables
            for (size_t i = 0; i < opTable.size(); i++)
            {
                for (size_t j = 0; j < objFunc.size(); j++)
                {
                    double val = (double)(opTable[i][j][d == 0]); // Evaluate at d=0
                    opTable[i][j] = Symbolic(std::abs(val));      // Apply std::abs and convert back to Symbolic
                }
            }

            // Apply abs to RHS
            for (size_t i = 0; i < opTable.size(); i++)
            {
                double val = (double)(opTable[i].back()[d == 0]); // Evaluate at d=0
                opTable[i].back() = Symbolic(std::abs(val));      // Apply std::abs and convert back to Symbolic
            }

            // Negate excess variables
            for (size_t i = 0; i < opTable.size(); i++)
            {
                for (size_t j = tableSizeW - excessCount - 1; j < opTable[i].size() - 1; j++)
                {
                    if (opTable[i][j] != 0)
                    {
                        opTable[i][j] = -opTable[i][j];
                    }
                }
            }
        }

        // Build header row
        globalHeaderRow.clear();
        for (size_t i = 0; i < objFunc.size(); i++)
        {
            globalHeaderRow.push_back("x" + std::to_string(i + 1));
        }
        for (int i = 0; i < excessCount; i++)
        {
            globalHeaderRow.push_back("e" + std::to_string(i + 1));
        }
        for (int i = 0; i < slackCount; i++)
        {
            globalHeaderRow.push_back("s" + std::to_string(i + 1));
        }
        globalHeaderRow.push_back("rhs");

        if (isConsoleOutput)
        {
            for (const auto &header : globalHeaderRow)
            {
                std::cout << header << " ";
            }
            std::cout << std::endl;
        }

        return opTable;
    }

    std::tuple<std::vector<std::vector<Symbolic>>, Symbolic, Symbolic, Symbolic, Symbolic, std::vector<int>>
    DoPreliminaries(std::vector<Symbolic> &objFunc, std::vector<std::vector<Symbolic>> &constraints, bool isMin, bool absRule = false, bool optTabLockState = false)
    {
        // Scrub delta from inputs for dual simplex (convert Symbolic expressions to doubles)
        std::vector<double> tObjFunc = scrubDelta(objFunc);
        std::vector<std::vector<double>> tConstraints;
        for (size_t i = 0; i < constraints.size(); i++)
        {
            tConstraints.push_back(scrubDelta(constraints[i]));
        }

        // The dual simplex works with doubles and returns results we need to convert back
        // std::vector<std::vector<std::vector<double>>> doubleTableaus;
        // std::vector<double> doubleChangingVars;

        // Call dual simplex with double inputs
        auto [doubleTableaus, doubleChangingVars, doubleOptimalSolution, _a, __b, headerRow] = dual->DoDualSimplex(tObjFunc, tConstraints, isMin);

        this->headerRowOut = headerRow;

        // Convert double tableaus back to Symbolic expressions for globalOptimalTab
        globalOptimalTab.clear();
        for (const auto &doubleTab : doubleTableaus)
        {
            std::vector<std::vector<Symbolic>> exTab;
            for (const auto &doubleRow : doubleTab)
            {
                std::vector<Symbolic> exRow;
                for (double val : doubleRow)
                {
                    exRow.push_back(Symbolic(val));
                }
                exTab.push_back(exRow);
            }
            globalOptimalTab.push_back(exTab);
        }

        if (optTabLockState)
        {

            // Update objFunc
            for (size_t i = 0; i < objFunc.size(); ++i)
            {
                if (currentDeltaSelection == "o" + std::to_string(i))
                {
                    objFunc[i] = valueRange;
                }
            }

            // Update constraints
            for (size_t i = 0; i < constraints.size(); ++i)
            {
                for (size_t j = 0; j < constraints[i].size(); ++j)
                {
                    if (currentDeltaSelection == "c" + std::to_string(i) + std::to_string(j))
                    {
                        constraints[i][j] = valueRange;
                    }
                }
            }

            // Update constraints right-hand side (RHS)
            for (size_t i = 0; i < constraints.size(); ++i)
            {
                if (currentDeltaSelection == "cRhs" + std::to_string(i))
                {
                    constraints[i][constraints[i].size() - 2] = valueRange;
                    // constraints[i][constraints[i].size() - 2] = Symbolic(std::to_string(valueRange));
                    // constraints[i][constraints[i].size() - 2] = constraints[i][constraints[i].size() - 2];
                }
            }

            // currentDeltaSelection = "dStore0";
        }

        // Use globalOptimalTab for the rest of the computation
        std::vector<std::vector<std::vector<Symbolic>>> tableaus = globalOptimalTab;

        // Keep delta in the table
        std::vector<std::vector<Symbolic>> deltaTab = doFormulationOperation(objFunc, constraints, absRule);

        // Get basic variable positions
        std::vector<int> basicVarSpots;
        for (size_t k = 0; k < tableaus.back()[0].size(); k++)
        {
            std::vector<Symbolic> tCVars;
            for (size_t i = 0; i < tableaus.back().size(); i++)
            {
                tCVars.push_back(tableaus.back()[i][k]);
            }

            Symbolic sum = 0;
            for (const auto &val : tCVars)
            {
                sum += val;
            }

            if (sum == 1)
            {
                basicVarSpots.push_back(k);
            }
        }

        // Get columns of basic variables
        std::vector<std::vector<Symbolic>> basicVarCols;
        for (size_t i = 0; i < tableaus.back()[0].size(); i++)
        {
            if (find(basicVarSpots.begin(), basicVarSpots.end(), i) != basicVarSpots.end())
            {
                std::vector<Symbolic> tLst;
                for (size_t j = 0; j < tableaus.back().size(); j++)
                {
                    tLst.push_back(tableaus.back()[j][i]);
                }
                basicVarCols.push_back(tLst);
            }
        }

        // Sort basic variables by position of 1 in column
        std::vector<std::pair<std::vector<Symbolic>, int>> zippedCbv;
        for (size_t i = 0; i < basicVarCols.size(); i++)
        {
            zippedCbv.push_back({basicVarCols[i], basicVarSpots[i]});
        }

        sort(zippedCbv.begin(), zippedCbv.end(),
             [](const std::pair<std::vector<Symbolic>, int> &a, const std::pair<std::vector<Symbolic>, int> &b)
             {
                 auto it_a = find(a.first.begin(), a.first.end(), 1);
                 auto it_b = find(b.first.begin(), b.first.end(), 1);
                 size_t pos_a = (it_a != a.first.end()) ? distance(a.first.begin(), it_a) : a.first.size();
                 size_t pos_b = (it_b != a.first.end()) ? distance(b.first.begin(), it_b) : b.first.size();
                 return pos_a < pos_b;
             });

        basicVarSpots.clear();
        for (const auto &pair : zippedCbv)
        {
            basicVarSpots.push_back(pair.second);
        }

        // Replace first tableau with delta tableau
        tableaus[0] = deltaTab;

        // Build cbv vector
        std::vector<Symbolic> cbv;
        for (size_t i = 0; i < basicVarSpots.size(); i++)
        {
            if (absRule)
            {
                cbv.push_back(tableaus[0][0][basicVarSpots[i]]);
            }
            else
            {
                cbv.push_back(-tableaus[0][0][basicVarSpots[i]]);
            }
        }

        if (isConsoleOutput)
        {
            for (int spot : basicVarSpots)
            {
                std::cout << spot << " ";
            }
            std::cout << std::endl;
        }

        // Build matrix B
        std::vector<std::vector<Symbolic>> matBData;
        for (size_t i = 0; i < basicVarSpots.size(); i++)
        {
            std::vector<Symbolic> tLst;
            for (size_t j = 1; j < tableaus[0].size(); j++)
            {
                tLst.push_back(tableaus[0][j][basicVarSpots[i]]);
            }
            matBData.push_back(tLst);
        }

        // Convert to Symbolic matrices
        Symbolic matrixCbv("", cbv.size(), 1);
        for (size_t i = 0; i < cbv.size(); i++)
        {
            matrixCbv(i, 0) = cbv[i];
        }

        Symbolic matrixB("", matBData.size(), matBData[0].size());
        for (size_t i = 0; i < matBData.size(); i++)
        {
            for (size_t j = 0; j < matBData[i].size(); j++)
            {
                matrixB(i, j) = matBData[i][j];
            }
        }

        // Corrected matrix inversion
        // Symbolic matrixBNegOne = ~matrixB; // Use ~ operator for inversion
        Symbolic matrixBNegOne = matrixB.inverse();
        Symbolic matrixCbvNegOne = matrixBNegOne * matrixCbv;

        if (isConsoleOutput)
        {
            std::cout << "cbv" << std::endl;
            std::cout << matrixCbv << std::endl;
            std::cout << "B" << std::endl;
            std::cout << matrixB << std::endl;
            std::cout << "B^-1" << std::endl;
            std::cout << matrixBNegOne << std::endl;
            std::cout << "cbvB^-1" << std::endl;
            std::cout << matrixCbvNegOne << std::endl;
        }

        // Calculate changing Z row
        std::vector<Symbolic> changingZRow;
        for (size_t j = 0; j < deltaTab[0].size() - 1; j++)
        {
            std::vector<Symbolic> tLst;
            for (size_t i = 1; i < deltaTab.size(); i++)
            {
                tLst.push_back(deltaTab[i][j]);
            }

            Symbolic colMatrix("", tLst.size(), 1);
            for (size_t i = 0; i < tLst.size(); i++)
            {
                colMatrix(i, 0) = tLst[i];
            }

            Symbolic mmultCbvNegOneBCol = colMatrix.transpose() * matrixCbvNegOne;
            Symbolic matNegValue = mmultCbvNegOneBCol(0, 0);

            if (absRule)
            {
                changingZRow.push_back(matNegValue - deltaTab[0][j]);
            }
            else
            {
                changingZRow.push_back(matNegValue - (-deltaTab[0][j]));
            }
        }

        // Calculate RHS optimal value
        std::vector<Symbolic> tRhsCol;
        for (size_t i = 1; i < deltaTab.size(); i++)
        {
            tRhsCol.push_back(deltaTab[i].back());
        }

        Symbolic rhsMatrix("", tRhsCol.size(), 1);
        for (size_t i = 0; i < tRhsCol.size(); i++)
        {
            rhsMatrix(i, 0) = tRhsCol[i];
        }

        Symbolic tRhsOptimal = rhsMatrix.transpose() * matrixCbvNegOne;
        Symbolic changingOptimal = tRhsOptimal(0, 0);

        // Calculate changing B values
        std::vector<std::vector<Symbolic>> tChangingBv;
        for (size_t j = 0; j < deltaTab[0].size(); j++)
        {
            std::vector<Symbolic> tLst;
            for (size_t i = 1; i < deltaTab.size(); i++)
            {
                tLst.push_back(deltaTab[i][j]);
            }

            Symbolic colMatrix("", tLst.size(), 1);
            for (size_t i = 0; i < tLst.size(); i++)
            {
                colMatrix(i, 0) = tLst[i];
            }

            Symbolic tMatrix = colMatrix.transpose() * matrixBNegOne;

            std::vector<Symbolic> resultRow;
            for (unsigned i = 0; i < tMatrix.columns(); i++)
            {
                resultRow.push_back(tMatrix(0, i));
            }
            tChangingBv.push_back(resultRow);
        }

        // Build changing table
        std::vector<std::vector<Symbolic>> changingTable = tableaus.back();

        changingZRow.push_back(changingOptimal);
        changingTable[0] = changingZRow;

        // Transpose and set B values
        for (size_t i = 0; i < changingTable.size() - 1; i++)
        {
            for (size_t j = 0; j < changingTable[0].size(); j++)
            {
                changingTable[i + 1][j] = tChangingBv[j][i];
            }
        }

        if (optTabLockState)
        {
            // auto [doubleTableaus, doubleChangingVars, doubleOptimalSolution, _a, __b, headerRow] = dual->DoDualSimplex(tObjFunc, tConstraints, isMin);

            std::vector<std::vector<double>> tChangingTable;
            for (size_t i = 0; i < changingTable.size(); ++i)
            {
                tChangingTable.push_back(std::vector<double>());
                for (size_t j = 0; j < changingTable[i].size(); ++j)
                {
                    // tChangingTable[i].push_back(changingTable[i][j].as_double());
                    tChangingTable[i].push_back((double)changingTable[i][j]);
                }
            }

            try
            {

                auto [reOptTableaus, _aa, _bb, pivotCols, pivotRows, _ee] = dual->DoDualSimplex(tObjFunc, tConstraints, isMin, &tChangingTable);
                this->reOptTableaus = reOptTableaus;
                this->pivotCols = pivotCols;
                this->pivotRows = pivotRows;

                if (reOptTableaus.size() != 1)
                {
                    this->shouldReoptimize = true;
                }
            }
            catch (...)
            {
            }

            for (size_t i = 0; i < reOptTableaus.size(); ++i)
            {
                for (size_t j = 0; j < reOptTableaus[i].size(); ++j)
                {
                    for (size_t k = 0; k < reOptTableaus[i][j].size(); ++k)
                    {
                        std::cout << std::setw(10) << reOptTableaus[i][j][k] << " ";
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl
                          << std::endl;
            }

            std::cout << reOptTableaus.size() << std::endl;
        }

        if (solveDelta)
        {
            for (size_t i = 0; i < changingTable.size(); ++i)
            {
                for (size_t j = 0; j < changingTable[i].size(); ++j)
                {
                    try
                    {
                        Symbolic &expr = changingTable[i][j];
                        std::string tmpName = "__sym_tmp_" + std::to_string(i) + "_" + std::to_string(j);
                        Symbolic tmp(tmpName.c_str());

                        std::ostringstream oss;
                        oss << expr;
                        std::string before = oss.str();

                        oss.str("");
                        oss.clear();
                        oss << expr[d == tmp];
                        std::string after = oss.str();

                        bool contains_d = (before != after);

                        if (!contains_d)
                        {
                            try
                            {
                                double v0 = double(expr[d == 0]);
                                double v1 = double(expr[d == 1]);
                                if (v0 != v1)
                                    contains_d = true;
                            }
                            catch (...)
                            {
                                contains_d = true;
                            }
                        }

                        if (contains_d)
                        {
                            auto solutions = solve(expr, d);
                            if (!solutions.empty())
                            {
                                // expr = solutions.back().rhs; // your original pattern
                                // expr = solutions.back();
                                std::ostringstream oss;
                                oss << solutions.back().rhs;
                                std::string temp = oss.str();
                                temp.insert(0, "d = ");

                                expr = Symbolic(temp.c_str());
                            }
                        }
                    }
                    catch (...)
                    {
                    }
                }
            }
        }

        std::vector<std::vector<std::string>> strTable(tableaus[0].size());
        std::vector<size_t> colWidths(tableaus[0][0].size(), 0);

        for (size_t i = 0; i < tableaus[0].size(); i++)
        {
            strTable[i].resize(tableaus[0][i].size());
            for (size_t j = 0; j < tableaus[0][i].size(); j++)
            {
                std::ostringstream oss;
                oss << tableaus[0][i][j];
                strTable[i][j] = oss.str();

                if (strTable[i][j].size() > colWidths[j])
                    colWidths[j] = strTable[i][j].size();
            }
        }

        this->initialTableOut = strTable;

        if (isConsoleOutput)
        {
            std::cout << "\ninitial table\n"
                      << std::endl;
            for (size_t i = 0; i < strTable.size(); i++)
            {
                for (size_t j = 0; j < strTable[i].size(); j++)
                {
                    std::cout << std::setw(colWidths[j] + 15) << std::left << strTable[i][j];
                }
                std::cout << std::endl;
            }
        }

        // Print optimal table
        strTable.clear();
        strTable.resize(tableaus.back().size());
        colWidths.clear();
        colWidths.resize(tableaus.back()[0].size(), 0);

        for (size_t i = 0; i < tableaus.back().size(); i++)
        {
            strTable[i].resize(tableaus.back()[i].size());
            for (size_t j = 0; j < tableaus.back()[i].size(); j++)
            {
                std::ostringstream oss;
                oss << tableaus.back()[i][j];
                strTable[i][j] = oss.str();

                if (strTable[i][j].size() > colWidths[j])
                    colWidths[j] = strTable[i][j].size();
            }
        }

        this->optimalTableOut = strTable;

        if (isConsoleOutput)
        {
            std::cout << "\noptimal table\n"
                      << std::endl;
            for (size_t i = 0; i < strTable.size(); i++)
            {
                for (size_t j = 0; j < strTable[i].size(); j++)
                {
                    std::cout << std::setw(colWidths[j] + 15) << std::left << strTable[i][j];
                }
                std::cout << std::endl;
            }
        }

        // Print optimal changing table
        strTable.clear();
        strTable.resize(changingTable.size());
        colWidths.clear();
        colWidths.resize(changingTable[0].size(), 0);

        for (size_t i = 0; i < changingTable.size(); i++)
        {
            strTable[i].resize(changingTable[i].size());
            for (size_t j = 0; j < changingTable[i].size(); j++)
            {
                std::ostringstream oss;
                oss << changingTable[i][j];
                strTable[i][j] = oss.str();

                if (strTable[i][j].size() > colWidths[j])
                    colWidths[j] = strTable[i][j].size();
            }
        }

        this->changingTableOut = strTable;

        if (isConsoleOutput)
        {
            std::cout << "\noptimal changing table\n"
                      << std::endl;
            for (size_t i = 0; i < strTable.size(); i++)
            {
                for (size_t j = 0; j < strTable[i].size(); j++)
                {
                    std::cout << std::setw(colWidths[j] + 15) << std::left << strTable[i][j];
                }
                std::cout << std::endl;
            }
        }

        this->matrixCbvOut = matrixCbv;
        this->matrixBOut = matrixB;
        this->matrixBNegOneOut = matrixBNegOne;
        this->matrixCbvNegOneOut = matrixCbvNegOne;

        if (!optTabLockState)
        {
            detectAndPrintInfluencedFormulas(changingTable, matrixB, matrixCbvNegOne, objFunc.size());
        }

        return make_tuple(changingTable, matrixCbv, matrixB, matrixBNegOne, matrixCbvNegOne, basicVarSpots);
    }

    void DoMathPreliminaries(std::vector<double> &objFuncDouble, std::vector<std::vector<double>> &constraintsDouble, bool isMin = false, std::string currentDeltaSelection = "dStore0", bool absRule = false, bool optTabLockState = false, bool solveDelta = false, double valueRange = 0)
    {
        this->valueRange = valueRange;

        std::vector<Symbolic> objFunc;
        objFunc.reserve(objFuncDouble.size());
        for (double val : objFuncDouble)
        {
            objFunc.push_back(Symbolic(val));
        }

        std::vector<std::vector<Symbolic>> constraints;
        constraints.reserve(constraintsDouble.size());
        for (const auto &row : constraintsDouble)
        {
            std::vector<Symbolic> symbolicRow;
            symbolicRow.reserve(row.size());
            for (double val : row)
            {
                symbolicRow.push_back(Symbolic(val));
            }
            constraints.push_back(std::move(symbolicRow));
        }

        // currentDeltaSelection = "o1";       // Updates objFunc[1]
        // currentDeltaSelection = "c12";     // Updates constraints[1][2]
        // currentDeltaSelection = "cRhs0";   // Updates constraints[0][size()-2]
        // currentDeltaSelection = "dStore0"; // Does nothing

        if (optTabLockState)
        {
            this->currentDeltaSelection = currentDeltaSelection;
        }

        if (currentDeltaSelection != "dStore0" && !optTabLockState)
        {
            // Update objFunc
            for (size_t i = 0; i < objFunc.size(); ++i)
            {
                if (currentDeltaSelection == "o" + std::to_string(i))
                {
                    objFunc[i] = objFunc[i] + d;
                }
            }

            // Update constraints
            for (size_t i = 0; i < constraints.size(); ++i)
            {
                for (size_t j = 0; j < constraints[i].size(); ++j)
                {
                    if (currentDeltaSelection == "c" + std::to_string(i) + std::to_string(j))
                    {
                        constraints[i][j] = constraints[i][j] + d;
                    }
                }
            }

            // Update constraints right-hand side (RHS)
            for (size_t i = 0; i < constraints.size(); ++i)
            {
                if (currentDeltaSelection == "cRhs" + std::to_string(i))
                {
                    constraints[i][constraints[i].size() - 2] =
                        constraints[i][constraints[i].size() - 2] + d;
                }
            }
        }

        this->solveDelta = solveDelta;
        auto [changingTable, matCbv, matB, matBNegOne, matCbvNegOne, basicVarSpots] = DoPreliminaries(objFunc, constraints, isMin, absRule, optTabLockState);
    }

    void
    testDoPreliminaries()
    {
        std::cout << "\n=== Testing DoPreliminaries (requires DualSimplex) ===\n"
                  << std::endl;

        MathPreliminaries mp(true); // Enable console output for debugging

        std::vector<double> objFunc = {3.0, 2.0};
        std::vector<std::vector<double>> constraints = {
            {2.0, 1.0, 100.0, 0.0},
            {1.0, 1.0, 80.0, 0.0},
            {1.0, 0.0, 45.0, 0.0}};

        DoMathPreliminaries(objFunc, constraints, false, currentDeltaSelection);
    }

    std::vector<std::string> symbolicToStringVector1d(const Symbolic &sym) const
    {
        std::vector<std::string> result;
        int rows = sym.rows();
        int cols = sym.columns();

        result.reserve(rows);

        for (int i = 0; i < rows; i++)
        {
            std::ostringstream oss;
            // oss << "[";
            for (int j = 0; j < cols; j++)
            {
                oss << sym(i, j);
                if (j + 1 < cols)
                    oss << " ";
            }
            // oss << "]";
            result.push_back(oss.str());
        }

        return result;
    }

    std::vector<std::vector<std::string>> symbolicToStringVector2d(const Symbolic &sym) const
    {
        std::vector<std::vector<std::string>> result;
        int rows = sym.rows();
        int cols = sym.columns();

        result.reserve(rows);

        for (int i = 0; i < rows; i++)
        {
            std::vector<std::string> row;
            row.reserve(cols);
            for (int j = 0; j < cols; j++)
            {
                std::ostringstream oss;
                oss << sym(i, j); // Each element as a single string
                row.push_back(oss.str());
            }
            result.push_back(row);
        }

        return result;
    }

    std::vector<std::vector<std::string>> GetChangingTable() { return changingTableOut; }
    std::vector<std::vector<std::string>> GetOptimalTable() { return optimalTableOut; }
    std::vector<std::vector<std::string>> GetInitialTable() { return initialTableOut; }

    std::vector<std::string> getMatrixCbv() const // 1d
    {
        return symbolicToStringVector1d(matrixCbvOut);
    }

    std::vector<std::vector<std::string>> getMatrixB() const // 2d
    {
        return symbolicToStringVector2d(matrixBOut);
    }

    std::vector<std::vector<std::string>> getMatrixBNegOne() const // 2d
    {
        return symbolicToStringVector2d(matrixBNegOneOut);
    }

    std::vector<std::string> getMatrixCbvNegOne() const // 1d
    {
        return symbolicToStringVector1d(matrixCbvNegOneOut);
    }

    std::vector<std::string> getHeaderRow() { return headerRowOut; }

    bool getShouldReOptimize() { return shouldReoptimize; }

    std::vector<int> getPivotCols() { return pivotCols; }
    std::vector<int> getPivotRows() { return pivotRows; }

    std::vector<std::vector<std::vector<double>>> getReOptTableaus() { return reOptTableaus; }

    std::vector<std::string> getInfluencedFormulas() { return influencedFormulasOutput; }
};