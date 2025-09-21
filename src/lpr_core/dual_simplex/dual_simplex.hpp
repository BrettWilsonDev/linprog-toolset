#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <limits>
#include <iomanip>

class DualSimplex
{
private:
    bool isConsoleOutput;
    std::vector<int> IMPivotCols;
    std::vector<int> IMPivotRows;
    std::vector<std::string> IMHeaderRow;
    std::vector<int> phases;

    static constexpr double EPS = 1e-12;

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

    LPRResult result;

public:
    DualSimplex(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput) {}

    std::vector<std::vector<double>> DoFormulationOperation(const std::vector<double> &objFunc, std::vector<std::vector<double>> constraints)
    {
        int excessCount = 0;
        int slackCount = 0;

        for (const auto &constraint : constraints)
        {
            if (constraint.back() == 1)
                excessCount++;
            else
                slackCount++;
        }

        for (auto &constraint : constraints)
        {
            if (constraint.back() == 1)
            {
                for (double &val : constraint)
                {
                    val *= -1;
                }
            }
        }

        for (auto &constraint : constraints)
        {
            constraint.pop_back();
        }

        int tableSizeH = constraints.size() + 1;
        int imCtr = 1;
        for (size_t i = 0; i < objFunc.size(); i++)
        {
            IMHeaderRow.push_back("x" + std::to_string(imCtr++));
        }

        imCtr = 1;
        if (excessCount > 0)
        {
            for (int i = 0; i < excessCount; i++)
            {
                IMHeaderRow.push_back("e" + std::to_string(imCtr++));
            }
        }

        if (slackCount > 0)
        {
            for (int i = 0; i < slackCount; i++)
            {
                IMHeaderRow.push_back("s" + std::to_string(imCtr++));
            }
        }

        IMHeaderRow.push_back("rhs");

        int tableSizeW = excessCount + slackCount + 1 + objFunc.size();
        std::vector<std::vector<double>> opTable(tableSizeH, std::vector<double>(tableSizeW, 0.0));

        for (size_t i = 0; i < objFunc.size(); i++)
        {
            opTable[0][i] = -objFunc[i];
        }

        for (size_t i = 0; i < constraints.size(); i++)
        {
            for (size_t j = 0; j < constraints[i].size() - 1; j++)
            {
                opTable[i + 1][j] = constraints[i][j];
            }
            opTable[i + 1][tableSizeW - 1] = constraints[i].back();
        }

        for (size_t i = 1; i < opTable.size(); i++)
        {
            opTable[i][i + objFunc.size() - 1] = 1;
        }

        return opTable;
    }

    std::pair<std::vector<std::vector<double>>, std::vector<double>> DoDualPivotOperation(const std::vector<std::vector<double>> &tab)
    {
        std::vector<double> thetaRow;
        std::vector<double> rhs;
        for (const auto &row : tab)
            rhs.push_back(row.back());

        // find the most negative RHS (choose smallest index in ties)
        int pivotRow = -1;
        double minRhs = std::numeric_limits<double>::infinity();
        for (size_t i = 0; i < rhs.size(); ++i)
        {
            if (rhs[i] < -EPS && (rhs[i] < minRhs - EPS || (std::abs(rhs[i] - minRhs) <= EPS && static_cast<int>(i) < pivotRow)))
            {
                minRhs = rhs[i];
                pivotRow = static_cast<int>(i);
            }
        }
        if (pivotRow == -1)
            return {tab, {}}; // no negative rhs -> done

        int m = static_cast<int>(tab[pivotRow].size()) - 1;

        // find candidate pivot columns (tab[pivotRow][j] < 0)
        std::vector<int> candidates;
        for (int j = 0; j < m; ++j)
            if (tab[pivotRow][j] < -EPS)
                candidates.push_back(j);
        if (candidates.empty())
            return {tab, {}}; // no eligible pivot column

        // compute dual pivot thetas and pick smallest positive theta; tie-break by smallest column index (Bland)
        double bestTheta = std::numeric_limits<double>::infinity();
        int bestCol = -1;
        thetaRow.assign(m, std::numeric_limits<double>::infinity());
        for (int col : candidates)
        {
            double denom = tab[pivotRow][col];
            if (std::abs(denom) < EPS)
                continue;
            double theta = std::abs(tab[0][col] / denom);
            thetaRow[col] = theta;
            if (theta > EPS)
            {
                if (theta < bestTheta - EPS || (std::abs(theta - bestTheta) <= EPS && col < bestCol))
                {
                    bestTheta = theta;
                    bestCol = col;
                }
            }
        }

        // if no positive theta found, but zeros exist, prefer smallest column index with theta == 0
        if (bestCol == -1)
        {
            for (int col : candidates)
            {
                if (std::abs(thetaRow[col]) <= EPS)
                {
                    bestCol = col;
                    break;
                }
            }
            if (bestCol == -1)
                return {tab, {}}; // nothing usable
        }

        int rowIndex = pivotRow;
        int colIndex = bestCol;

        std::vector<std::vector<double>> newTab = tab;
        double divNumber = tab[rowIndex][colIndex];
        if (std::abs(divNumber) < EPS)
            return {tab, {}};

        for (size_t j = 0; j < newTab[rowIndex].size(); j++)
        {
            newTab[rowIndex][j] = newTab[rowIndex][j] / divNumber;
            if (newTab[rowIndex][j] == -0.0)
                newTab[rowIndex][j] = 0.0;
        }

        std::vector<double> pivotMathRow = newTab[rowIndex];
        for (size_t i = 0; i < newTab.size(); i++)
        {
            if (i == static_cast<size_t>(rowIndex))
                continue;
            double factor = tab[i][colIndex];
            for (size_t j = 0; j < newTab[i].size(); j++)
            {
                newTab[i][j] -= factor * newTab[rowIndex][j];
            }
        }

        newTab[rowIndex] = pivotMathRow;

        if (isConsoleOutput)
        {
            std::cout << "the pivot col in Dual is " << colIndex + 1 << " and the pivot row is " << rowIndex + 1 << std::endl;
        }

        IMPivotCols.push_back(colIndex);
        IMPivotRows.push_back(rowIndex);

        return {newTab, thetaRow};
    }

    std::pair<std::vector<std::vector<double>>, std::vector<double>> DoPrimalPivotOperation(
        const std::vector<std::vector<double>> &tab, bool isMin)
    {

        std::vector<double> thetasCol;
        std::vector<double> testRow(tab[0].begin(), tab[0].end() - 1);

        double largestNegativeNumber;
        bool foundNumber = false;

        if (isMin)
        {
            for (double num : testRow)
            {
                if (num > 0 && num != 0)
                {
                    if (!foundNumber || num < largestNegativeNumber)
                    {
                        largestNegativeNumber = num;
                        foundNumber = true;
                    }
                }
            }
        }
        else
        {
            for (double num : testRow)
            {
                if (num < 0 && num != 0)
                {
                    if (!foundNumber || num < largestNegativeNumber)
                    {
                        largestNegativeNumber = num;
                        foundNumber = true;
                    }
                }
            }
        }

        if (!foundNumber)
        {
            return {std::vector<std::vector<double>>(), std::vector<double>()};
        }

        int colIndex = -1;
        for (int i = 0; i < tab[0].size(); i++)
        {
            if (tab[0][i] == largestNegativeNumber)
            {
                colIndex = i;
                break;
            }
        }

        std::vector<double> thetas;
        for (int i = 1; i < tab.size(); i++)
        {
            if (tab[i][colIndex] != 0)
            {
                thetas.push_back(tab[i].back() / tab[i][colIndex]);
            }
            else
            {
                thetas.push_back(std::numeric_limits<double>::infinity());
            }
        }

        thetasCol = thetas;

        bool allNegativeThetas = true;
        for (double num : thetas)
        {
            if (num >= 0)
            {
                allNegativeThetas = false;
                break;
            }
        }

        if (allNegativeThetas)
        {
            return {std::vector<std::vector<double>>(), std::vector<double>()};
        }

        bool hasPositiveNonInf = false;
        for (double num : thetas)
        {
            if (num > 0 && num != std::numeric_limits<double>::infinity() && num != 0)
            {
                hasPositiveNonInf = true;
                break;
            }
        }

        double minTheta;
        if (!hasPositiveNonInf)
        {
            bool hasZero = false;
            for (double num : thetas)
            {
                if (num == 0)
                {
                    hasZero = true;
                    break;
                }
            }
            if (hasZero)
            {
                minTheta = 0.0;
            }
            else
            {
                return {std::vector<std::vector<double>>(), std::vector<double>()};
            }
        }
        else
        {
            minTheta = std::numeric_limits<double>::infinity();
            for (double x : thetas)
            {
                if (x > 0 && x != std::numeric_limits<double>::infinity())
                {
                    minTheta = std::min(minTheta, x);
                }
            }
        }

        if (minTheta == std::numeric_limits<double>::infinity())
        {
            bool hasZero = false;
            for (double num : thetas)
            {
                if (num == 0)
                {
                    hasZero = true;
                    break;
                }
            }
            if (hasZero)
            {
                minTheta = 0;
            }
            else
            {
                return {std::vector<std::vector<double>>(), std::vector<double>()};
            }
        }

        int rowIndex = -1;
        for (int i = 0; i < thetas.size(); i++)
        {
            if (thetas[i] == minTheta)
            {
                rowIndex = i + 1;
                break;
            }
        }

        double divNumber = tab[rowIndex][colIndex];
        if (divNumber == 0)
        {
            return {std::vector<std::vector<double>>(), std::vector<double>()};
        }

        std::vector<std::vector<double>> operationTab(tab.size());
        for (int i = 0; i < tab.size(); i++)
        {
            operationTab[i].resize(tab[i].size(), 0.0);
        }

        // Divide pivot row
        for (int j = 0; j < tab[rowIndex].size(); j++)
        {
            operationTab[rowIndex][j] = tab[rowIndex][j] / divNumber;
            if (operationTab[rowIndex][j] == -0.0)
            {
                operationTab[rowIndex][j] = 0.0;
            }
        }

        // Apply pivot operation to other rows
        for (int i = 0; i < tab.size(); i++)
        {
            if (i == rowIndex)
            {
                continue;
            }
            for (int j = 0; j < tab[i].size(); j++)
            {
                double mathItem = tab[i][j] - (tab[i][colIndex] * operationTab[rowIndex][j]);
                operationTab[i][j] = mathItem;
            }
        }

        // Optional console output (uncomment if needed)
        // std::cout << "the pivot col in primal is " << (colIndex + 1)
        //           << " and the pivot row is " << (rowIndex + 1) << std::endl;

        IMPivotCols.push_back(colIndex);
        IMPivotRows.push_back(rowIndex);

        return {operationTab, thetasCol};
    }

    struct GetInputResult
    {
        std::vector<std::vector<double>> tab;
        bool isMin;
        int amtOfE;
        int amtOfS;
        int lenObj;
    };

    GetInputResult GetInput(const std::vector<double> &objFunc, const std::vector<std::vector<double>> &constraints, bool isMin)
    {
        int amtOfE = 0, amtOfS = 0;
        for (const auto &c : constraints)
        {
            if (c.back() == 1 || c.back() == 2)
                amtOfE++;
            else
                amtOfS++;
        }
        auto tab = DoFormulationOperation(objFunc, constraints);
        return {tab, isMin, amtOfE, amtOfS, static_cast<int>(objFunc.size())};
    }

    struct DoDualSimplexResult
    {
        std::vector<std::vector<std::vector<double>>> tableaus;
        std::vector<double> changingVars;
        double optimalSolution;
        std::vector<int> pivotCols;
        std::vector<int> pivotRows;
        std::vector<std::string> headerRow;
    };

    DoDualSimplexResult DoDualSimplex(const std::vector<double> &objFunc, const std::vector<std::vector<double>> &constraints, bool isMin, const std::vector<std::vector<double>> *tabOverride = nullptr)
    {
        std::vector<std::vector<double>> thetaCols;
        std::vector<std::vector<std::vector<double>>> tableaus;
        auto [tab, isMinLocal, amtOfE, amtOfS, lenObj] = GetInput(objFunc, constraints, isMin);

        if (tabOverride)
        {
            tab = *tabOverride;
            IMPivotCols.clear();
            IMPivotRows.clear();
            IMHeaderRow.pop_back();
        }

        tableaus.push_back(tab);

        while (true)
        {
            for (auto &items : tableaus.back())
            {
                for (double &val : items)
                {
                    if (val == -0.0)
                        val = 0.0;
                }
            }

            std::vector<double> rhsTest;
            for (const auto &row : tableaus.back())
            {
                rhsTest.push_back(row.back());
            }
            const double epsilon = 1e-9;
            bool allRhsPositive = std::all_of(rhsTest.begin(), rhsTest.end(), [epsilon](double num)
                                              { return num >= -epsilon; });

            if (allRhsPositive)
                break;

            auto [newTab, thetaRow] = DoDualPivotOperation(tableaus.back());
            if (thetaRow.empty())
            {
                if (!tabOverride && isConsoleOutput)
                {
                    std::cout << "\nNo Optimal Solution Found" << std::endl;
                }
                return {tableaus, {}, std::numeric_limits<double>::quiet_NaN(), {}, {}, {}};
            }

            for (auto &items : newTab)
            {
                for (double &val : items)
                {
                    if (val == -0.0)
                        val = 0.0;
                }
            }

            tableaus.push_back(newTab);
            phases.push_back(0);
        }

        std::vector<double> objFuncTest(tableaus.back()[0].begin(), tableaus.back()[0].end() - 1);
        bool allObjFuncPositive = isMinLocal
                                      ? std::all_of(objFuncTest.begin(), objFuncTest.end(), [](double num)
                                                    { return num <= 0; })
                                      : std::all_of(objFuncTest.begin(), objFuncTest.end(), [](double num)
                                                    { return num >= 0; });

        if (!allObjFuncPositive)
        {
            while (true)
            {
                for (auto &items : tableaus.back())
                {
                    for (double &val : items)
                    {
                        if (val == -0.0)
                            val = 0.0;
                    }
                }

                if (tableaus.back().empty())
                {
                    if (isConsoleOutput)
                        std::cout << "\nNo Optimal Solution Found" << std::endl;
                    break;
                }

                objFuncTest = std::vector<double>(tableaus.back()[0].begin(), tableaus.back()[0].end() - 1);
                allObjFuncPositive = isMinLocal
                                         ? std::all_of(objFuncTest.begin(), objFuncTest.end(), [](double num)
                                                       { return num <= 0; })
                                         : std::all_of(objFuncTest.begin(), objFuncTest.end(), [](double num)
                                                       { return num >= 0; });

                if (allObjFuncPositive)
                    break;

                auto [newTab, thetaCol] = DoPrimalPivotOperation(tableaus.back(), isMinLocal);
                if (thetaCol.empty() && newTab.empty())
                    break;

                try
                {
                    thetaCols.push_back(thetaCol);
                }
                catch (...)
                {
                    break;
                }
                tableaus.push_back(newTab);
                phases.push_back(1);
            }

            std::vector<double> rhsTest;
            for (const auto &row : tableaus.back())
            {
                rhsTest.push_back(row.back());
            }
            bool allRhsPositive = std::all_of(rhsTest.begin(), rhsTest.end(), [](double num)
                                              { return num >= 0; });

            if (!allRhsPositive)
            {
                tableaus.pop_back();
                IMPivotCols.pop_back();
                IMPivotRows.pop_back();
            }
        }

        if (isConsoleOutput)
        {
            std::cout << "\nOptimal Solution Found" << std::endl;
            if (!tableaus.back().empty())
            {
                std::cout << std::endl;
            }
            else
            {
                std::cout << "\nNo Optimal Solution Found" << std::endl;
            }
        }

        std::vector<std::string> xVars;
        for (int i = 1; i <= lenObj; i++)
        {
            xVars.push_back("x" + std::to_string(i));
        }

        if (isConsoleOutput)
        {
            for (size_t i = 0; i < tableaus.size(); i++)
            {
                std::cout << "Tableau " << i + 1 << std::endl;
                for (const auto &val : IMHeaderRow)
                {
                    std::cout << std::setw(10) << val;
                }
                std::cout << std::endl;
                for (const auto &row : tableaus[i])
                {
                    for (double val : row)
                    {
                        std::cout << std::setw(10) << std::fixed << std::setprecision(3) << val;
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
            }
        }

        std::vector<std::vector<double>> tSCVars;
        for (int k = 0; k < lenObj; k++)
        {
            std::vector<double> tCVars;
            for (const auto &row : tableaus.back())
            {
                tCVars.push_back(row[k]);
            }
            tSCVars.push_back(std::count_if(tCVars.begin(), tCVars.end(), [](double num)
                                            { return num != 0; }) == 1
                                  ? tCVars
                                  : std::vector<double>());
        }

        std::vector<double> changingVars;
        try
        {
            changingVars = std::vector<double>(lenObj, 0.0);
            for (size_t i = 0; i < tSCVars.size(); i++)
            {
                if (!tSCVars[i].empty())
                {
                    std::transform(tSCVars[i].begin(), tSCVars[i].end(), tSCVars[i].begin(), [](double x)
                                   { return std::abs(x); });
                    auto it = std::find(tSCVars[i].begin(), tSCVars[i].end(), 1.0);
                    if (it != tSCVars[i].end())
                    {
                        changingVars[i] = tableaus.back()[std::distance(tSCVars[i].begin(), it)].back();
                    }
                }
            }
        }
        catch (...)
        {
            changingVars = std::vector<double>();
        }

        if (isConsoleOutput)
        {
            std::cout << std::endl;
            for (double val : changingVars)
            {
                std::cout << val << " ";
            }
            std::cout << std::endl
                      << std::endl;
        }

        double optimalSolution = tableaus.back()[0].back();

        result.tableaus = tableaus;
        result.pivotCols = IMPivotCols;
        result.pivotRows = IMPivotRows;
        result.headerRow = IMHeaderRow;
        result.phases = phases;
        result.optimalSolution = optimalSolution;
        result.changingVars = changingVars;

        return {tableaus, changingVars, optimalSolution, IMPivotCols, IMPivotRows, IMHeaderRow};
    }

    std::vector<int> GetPhases()
    {
        return phases;
    }

    LPRResult GetResult()
    {
        return result;
    }
};