#ifndef MATH_PRELIMINARIES_HPP
#define MATH_PRELIMINARIES_HPP

#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <tuple>
#include <algorithm>
#include <cmath>
#include <iomanip>
// #include <../vendor/SymbolicCpp/headers/symbolicc++.h>
// #include <symbolicc++.h>
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

    std::vector<std::vector<std::vector<Symbolic>>> newTableaus;

    std::vector<int> IMPivotCols;
    std::vector<int> IMPivotRows;
    std::vector<std::string> IMHeaderRow;

    int pivotCol;
    int pivotRow;

    bool isAllDeltaCRow;
    bool isSingleDeltaCRow;
    bool isSingleDeltaARow;
    int singleCIndex;
    int singleAIndex;
    bool isDeltaZCol;
    bool isAllDeltaRows;
    bool isFormulaDeltaChanged;

    std::string currentDeltaSelection;

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

        newTableaus.clear();

        IMPivotCols.clear();
        IMPivotRows.clear();
        IMHeaderRow.clear();

        pivotCol = -1;
        pivotRow = -1;

        // solveDelta = false;
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
    DoPreliminaries(const std::vector<Symbolic> &objFunc, const std::vector<std::vector<Symbolic>> &constraints, bool isMin, bool absRule = false, bool optTabLockState = false)
    {
        // Scrub delta from inputs for dual simplex (convert Symbolic expressions to doubles)
        std::vector<double> tObjFunc = scrubDelta(objFunc);
        std::vector<std::vector<double>> tConstraints;
        for (size_t i = 0; i < constraints.size(); i++)
        {
            tConstraints.push_back(scrubDelta(constraints[i]));
        }

        // The dual simplex works with doubles and returns results we need to convert back
        std::vector<std::vector<std::vector<double>>> doubleTableaus;
        std::vector<double> doubleChangingVars;

        if (!optTabLockState)
        {
            // Call dual simplex with double inputs
            auto [doubleTableaus, doubleChangingVars, doubleOptimalSolution, _a, __b, __c] = dual->DoDualSimplex(tObjFunc, tConstraints, isMin);

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

        // Convert numeric values to double where possible
        for (size_t i = 0; i < changingTable.size(); i++)
        {
            for (size_t j = 0; j < changingTable[i].size(); j++)
            {
                // Symbolic does not have evalf; expressions are kept symbolic
            }
        }

        if (isConsoleOutput)
        {
            // Print initial table
            std::cout << "\ninitial table\n"
                      << std::endl;
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

            for (size_t i = 0; i < strTable.size(); i++)
            {
                for (size_t j = 0; j < strTable[i].size(); j++)
                {
                    std::cout << std::setw(colWidths[j] + 15) << std::left << strTable[i][j];
                }
                std::cout << std::endl;
            }

            // Print optimal table
            std::cout << "\noptimal table\n"
                      << std::endl;
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

            for (size_t i = 0; i < strTable.size(); i++)
            {
                for (size_t j = 0; j < strTable[i].size(); j++)
                {
                    std::cout << std::setw(colWidths[j] + 15) << std::left << strTable[i][j];
                }
                std::cout << std::endl;
            }

            // Print optimal changing table
            std::cout << "\noptimal changing table\n"
                      << std::endl;
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

            for (size_t i = 0; i < strTable.size(); i++)
            {
                for (size_t j = 0; j < strTable[i].size(); j++)
                {
                    std::cout << std::setw(colWidths[j] + 15) << std::left << strTable[i][j];
                }
                std::cout << std::endl;
            }
        }

        return make_tuple(changingTable, matrixCbv, matrixB, matrixBNegOne, matrixCbvNegOne, basicVarSpots);
    }

    void testDoPreliminaries()
    {
        std::cout << "\n=== Testing DoPreliminaries (requires DualSimplex) ===\n"
                  << std::endl;
        // try
        // {
        MathPreliminaries mp(true); // Enable console output for debugging

        // Test with simple problem using Symbolic objects
        std::vector<Symbolic> objFunc = {Symbolic(3.0), Symbolic(2.0)};
        std::vector<std::vector<Symbolic>> constraints = {
            {Symbolic(2.0), Symbolic(1.0), Symbolic(100.0), Symbolic(0.0)},
            {Symbolic(1.0), Symbolic(1.0), Symbolic(80.0), Symbolic(0.0)},
            {Symbolic(1.0), Symbolic(0.0), Symbolic(40.0+ mp.d), Symbolic(0.0)} // Use mp.d for symbolic variable
        };

        std::cout << "Calling DoPreliminaries..." << std::endl;
        auto [changingTable, matCbv, matB, matBNegOne, matCbvNegOne, basicVarSpots] =
            mp.DoPreliminaries(objFunc, constraints, false, false, false);
        std::cout << "DoPreliminaries completed successfully" << std::endl;

        std::cout << "  Basic variable positions: ";
        for (int pos : basicVarSpots)
        {
            std::cout << pos << " ";
        }
        std::cout << std::endl;

        // Print some results to verify the symbolic computation
        std::cout << "  Changing table dimensions: " << changingTable.size()
                  << "x" << (changingTable.empty() ? 0 : changingTable[0].size()) << std::endl;

        if (!changingTable.empty() && !changingTable[0].empty())
        {
            std::cout << "  Sample changing table entry [0][0]: " << changingTable[0][0] << std::endl;
        }

        std::cout << "  Matrix Cbv dimensions: " << matCbv.rows() << "x" << matCbv.columns() << std::endl;
        std::cout << "  Matrix B dimensions: " << matB.rows() << "x" << matB.columns() << std::endl;
        // }
        // catch (const std::exception &e)
        // {
        //     std::cout << "DoPreliminaries test failed" << std::endl;
        //     std::cout << "  Error: " << e.what() << std::endl;
        // }
        // catch (...)
        // {
        //     std::cout << "DoPreliminaries test failed with unknown error" << std::endl;
        // }
    }
};

#endif // MATH_PRELIMINARIES_HPP