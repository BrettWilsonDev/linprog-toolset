#pragma once

#include <vector>
#include <limits>
#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include "symbolicc++.h"
#include "math_preliminaries.hpp"

class SensitivityAnalysis
{
public:
    bool isConsoleOutput;
    int testInputSelected;
    MathPreliminaries mathPrelim;
    Symbolic d;
    std::vector<Symbolic> globalOptimalTab;
    std::vector<Symbolic> globalHeaderRow;
    std::string problemType;
    std::string absProblemType;
    int amtOfObjVars;
    std::vector<double> objFunc;
    std::vector<std::vector<double>> constraints;
    std::vector<std::string> signItems;
    std::vector<int> signItemsChoices;
    int amtOfConstraints;
    std::vector<std::vector<Symbolic>> changingTable;
    double posDelta;
    double negDelta;
    double posRange;
    double negRange;
    std::string termPos;
    std::string currentDeltaSelection;

    std::string outputString;

    SensitivityAnalysis(bool consoleOutput = false) : isConsoleOutput(consoleOutput)
    {
        testInputSelected = 4;
        mathPrelim = MathPreliminaries();
        d = Symbolic("d");
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
        posDelta = std::numeric_limits<double>::infinity();
        negDelta = std::numeric_limits<double>::infinity();
        posRange = std::numeric_limits<double>::infinity();
        negRange = std::numeric_limits<double>::infinity();
        termPos = "?";
        currentDeltaSelection = "o0";

        outputString = "";
    }

    struct TestData
    {
        std::vector<double> objFunc;
        std::vector<std::vector<double>> constraints;
        bool isMin;
    };

    TestData testInput(int testNum)
    {
        TestData data;
        data.isMin = false;
        if (testNum == 0)
        {
            data.objFunc = {60, 30, 20};
            data.constraints = {{8, 6, 1, 48, 0},
                                {4, 2, 1.5, 20, 0},
                                {2, 1.5, 0.5, 8, 0}};
        }
        else if (testNum == 1)
        {
            data.objFunc = {100, 30};
            data.constraints = {{0, 1, 3, 1},
                                {1, 1, 7, 0},
                                {10, 4, 40, 0}};
        }
        else if (testNum == 2)
        {
            data.objFunc = {30, 28, 26, 30};
            data.constraints = {{8, 8, 4, 4, 160, 0},
                                {1, 0, 0, 0, 5, 0},
                                {1, 0, 0, 0, 5, 1},
                                {1, 1, 1, 1, 20, 1}};
        }
        else if (testNum == 3)
        {
            data.objFunc = {10, 50, 80, 100};
            data.constraints = {{1, 4, 4, 8, 140, 0},
                                {1, 0, 0, 0, 50, 0},
                                {1, 0, 0, 0, 50, 1},
                                {1, 1, 1, 1, 70, 1}};
        }
        else if (testNum == 4)
        {
            data.objFunc = {3, 2};
            data.constraints = {{2, 1, 100, 0},
                                {1, 1, 80, 0},
                                {1, 0, 40, 0}};
        }
        else if (testNum == 5)
        {
            data.objFunc = {120, 80};
            data.constraints = {{8, 4, 160, 0},
                                {4, 4, 100, 0},
                                {1, 0, 17, 0},
                                {1, 0, 5, 1},
                                {0, 1, 17, 0},
                                {0, 1, 2, 1},
                                {1, -1, 0, 1},
                                {1, -4, 0, 0}};
        }
        else
        {
            data.objFunc.clear();
            data.constraints.clear();
        }
        return data;
    }

    void doSensitivityAnalysis(std::vector<Symbolic> objfunc, std::vector<std::vector<Symbolic>> constraints, bool isMin, bool absRule = false, bool optTabLockState = false)
    {
        auto a = objfunc;
        auto b = constraints;
        auto prelimResult = mathPrelim.DoPreliminaries(a, b, isMin, absRule, optTabLockState);
        auto [changingTable, matrixCbv, matrixB, matrixBNegOne, matrixCbvNegOne, basicVarSpots] = prelimResult;
        // changingTable = std::get<0>(prelimResult);
        // auto matrixCbv = std::get<1>(prelimResult);
        // auto matrixB = std::get<2>(prelimResult);
        // auto matrixBNegOne = std::get<3>(prelimResult);
        // auto matrixCbvNegOne = std::get<4>(prelimResult);
        // auto basicVarSpots = std::get<5>(prelimResult);

        Symbolic termWithoutdelta;
        bool objFuncHasDelta = false;
        for (size_t i = 0; i < objfunc.size(); ++i)
        {
            if (objfunc[i] != objfunc[i].subst(d, Symbolic(0.0)))
            {
                termPos = "c" + std::to_string(i + 1);
                termWithoutdelta = objfunc[i].subst(d, Symbolic(0.0));
                objFuncHasDelta = true;
            }
        }
        bool rhsHasDelta = false;
        for (size_t i = 0; i < constraints.size(); ++i)
        {
            auto &rhs = constraints[i][constraints[i].size() - 2];
            if (rhs != rhs.subst(d, Symbolic(0.0)))
            {
                termPos = "b" + std::to_string(i + 1);
                termWithoutdelta = rhs.subst(d, Symbolic(0.0));
                rhsHasDelta = true;
            }
        }
        bool constraintsHasDelta = false;
        size_t conStraintDeltaCol = static_cast<size_t>(-1);
        for (size_t i = 0; i < constraints.size(); ++i)
        {
            for (size_t j = 0; j < constraints[i].size() - 2; ++j)
            {
                if (constraints[i][j] != constraints[i][j].subst(d, Symbolic(0.0)))
                {
                    termPos = "c" + std::to_string(i + 1);
                    termWithoutdelta = constraints[i][j].subst(d, Symbolic(0.0));
                    constraintsHasDelta = true;
                    conStraintDeltaCol = j;
                }
            }
        }

        std::vector<double> deltasList;
        if (objFuncHasDelta)
        {
            for (size_t i = 0; i < changingTable[0].size() - 1; ++i)
            {
                auto &cell = changingTable[0][i];
                if (cell != cell.subst(d, Symbolic(0.0)))
                {
                    auto sol = solve(cell, d).back().rhs;
                    deltasList.push_back(static_cast<double>(sol));
                }
            }
        }
        if (rhsHasDelta)
        {
            deltasList.clear();
            for (size_t i = 0; i < changingTable.size(); ++i)
            {
                auto &cell = changingTable[i].back();
                if (cell != cell.subst(d, Symbolic(0.0)))
                {
                    auto sol = solve(cell, d).back().rhs;
                    deltasList.push_back(static_cast<double>(sol));
                }
            }
        }
        if (constraintsHasDelta)
        {
            if (changingTable[0][conStraintDeltaCol] != changingTable[0][conStraintDeltaCol].subst(d, Symbolic(0.0)))
            {
                try
                {
                    auto sol = solve(changingTable[0][conStraintDeltaCol], d).back().rhs;
                    deltasList.push_back(static_cast<double>(sol));
                }
                catch (...)
                {
                    termPos = "unsolvable";
                }
            }
            else
            {
                negDelta = std::numeric_limits<double>::infinity();
                posDelta = std::numeric_limits<double>::infinity();
                termPos = std::to_string(static_cast<double>(changingTable[0][conStraintDeltaCol]));
            }
        }

        if (!deltasList.empty())
        {
            bool allNegative = true;
            bool allPositive = true;
            for (auto x : deltasList)
            {
                if (x >= 0)
                    allNegative = false;
                if (x <= 0)
                    allPositive = false;
            }

            if (deltasList.size() == 1)
            {
                if (deltasList[0] < 0)
                {
                    posDelta = std::numeric_limits<double>::infinity();
                    negDelta = deltasList[0];
                }
                else if (deltasList[0] > 0)
                {
                    posDelta = deltasList[0];
                    negDelta = -std::numeric_limits<double>::infinity();
                }
            }
            else
            {
                if (allNegative)
                {
                    posDelta = std::numeric_limits<double>::infinity();
                    double minAbsNeg = std::numeric_limits<double>::infinity();
                    double selectedNeg = 0.0;
                    for (auto x : deltasList)
                    {
                        if (x < 0 && std::abs(x) < minAbsNeg)
                        {
                            minAbsNeg = std::abs(x);
                            selectedNeg = x;
                        }
                    }
                    negDelta = selectedNeg;
                }
                else if (allPositive)
                {
                    negDelta = -std::numeric_limits<double>::infinity();
                    double minAbsPos = std::numeric_limits<double>::infinity();
                    double selectedPos = 0.0;
                    for (auto x : deltasList)
                    {
                        if (x > 0 && std::abs(x) < minAbsPos)
                        {
                            minAbsPos = std::abs(x);
                            selectedPos = x;
                        }
                    }
                    posDelta = selectedPos;
                }
                else
                {
                    double minAbsPos = std::numeric_limits<double>::infinity();
                    double selectedPos = 0.0;
                    for (auto x : deltasList)
                    {
                        if (x > 0 && std::abs(x) < minAbsPos)
                        {
                            minAbsPos = std::abs(x);
                            selectedPos = x;
                        }
                    }
                    posDelta = selectedPos;

                    double minAbsNeg = std::numeric_limits<double>::infinity();
                    double selectedNeg = 0.0;
                    for (auto x : deltasList)
                    {
                        if (x < 0 && std::abs(x) < minAbsNeg)
                        {
                            minAbsNeg = std::abs(x);
                            selectedNeg = x;
                        }
                    }
                    negDelta = selectedNeg;
                }
            }
        }

        posRange = static_cast<double>(termWithoutdelta) + posDelta;
        negRange = static_cast<double>(termWithoutdelta) + negDelta;

        if (isConsoleOutput)
        {
            std::cout << negRange << " <= " << termPos << " <= " << posRange << std::endl;
            std::cout << negDelta << " <= d <= " << posDelta << std::endl;
        }

        this->outputString = std::to_string(negRange) + " <= " + termPos + " <= " + std::to_string(posRange) + "\n" + std::to_string(negDelta) + " <= d <= " + std::to_string(posDelta);
    }

    void doSensitivityAnalysisDouble(std::vector<double> &objFuncDouble, std::vector<std::vector<double>> &constraintsDouble, bool isMin = false, std::string currentDeltaSelection = "dStore0")
    {
        try
        {
            std::vector<Symbolic> objFunc;
            for (auto x : objFuncDouble)
            {
                objFunc.push_back(Symbolic(x));
            }
            std::vector<std::vector<Symbolic>> constraints;
            for (auto &row : constraintsDouble)
            {
                std::vector<Symbolic> r;
                for (auto x : row)
                {
                    r.push_back(Symbolic(x));
                }
                constraints.push_back(r);
            }

            if (currentDeltaSelection != "dStore0")
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
                    for (size_t j = 0; j < constraints[i].size() - 2; ++j)
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

            doSensitivityAnalysis(objFunc, constraints, isMin);
        }
        catch (const std::exception &e)
        {
            std::cout << e.what() << '\n'
                      << std::endl;
        }
    }

    std::string getOutputString()
    {
        return this->outputString;
    }
};