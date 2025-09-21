#pragma once

#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <numeric>
#include <algorithm>
#include "dual_simplex.hpp"

class DEASolver
{
private:
    DualSimplex dual;
    bool isConsoleOutput;
    int testInputSelected;
    int amtOfItems;
    int amtOfOutputs;
    int amtOfInputs;

    std::vector<std::vector<double>> LpOutputs;
    std::vector<std::vector<double>> LpInputs;
    std::vector<std::vector<std::vector<double>>> tables;
    std::vector<std::string> header;
    std::vector<double> allInputTotals;
    std::vector<double> allOutputTotals;
    std::vector<std::vector<double>> allRangesO;
    std::vector<std::vector<double>> allRangesI;
    std::vector<std::vector<double>> changingVars;

    std::string problemType;
    bool isMin;
    mutable std::ostringstream outputStream; // Made mutable to allow modification in const methods

public:
    DEASolver(bool consoleOutput = false) : isConsoleOutput(consoleOutput)
    {
        dual = DualSimplex();
        testInputSelected = -1;
        amtOfItems = 1;
        amtOfOutputs = 1;
        amtOfInputs = 1;

        LpOutputs = {{0.0}};
        LpInputs = {{0.0}};

        tables.clear();
        header.clear();
        allInputTotals.clear();
        allOutputTotals.clear();
        allRangesO.clear();
        allRangesI.clear();
        changingVars.clear();

        problemType = "Max";
        isMin = false;
        outputStream.str(""); // Initialize the stringstream
        outputStream.clear(); // Ensure the stream is in a good state
    }

    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>
    testInput(int testNum = -1)
    {
        std::vector<std::vector<double>> inputs, outputs;

        if (testNum == 0)
        {
            inputs = {
                {5, 14},
                {8, 15},
                {7, 12}};

            outputs = {
                {9, 4, 16},
                {5, 7, 10},
                {4, 9, 13}};
        }
        else if (testNum == 1)
        {
            inputs = {
                {15, 20, 50},
                {14, 23, 51},
                {16, 19, 51}};

            outputs = {
                {200, 15, 35},
                {220, 18, 45},
                {210, 17, 20}};
        }

        return std::make_pair(inputs, outputs);
    }

    struct TableResult
    {
        std::vector<std::vector<double>> table;
        std::vector<double> zRow;
        std::vector<std::vector<double>> constraints;
        std::vector<double> conRow;
    };

    TableResult buildTable(const std::vector<std::vector<double>> &LpInputs, const std::vector<std::vector<double>> &LpOutputs, int currentSelection = 0)
    {
        int tabWLen = LpInputs.back().size() + LpOutputs.back().size();

        // Build bottom rows
        std::vector<std::vector<double>> bottomRows;
        for (int i = 0; i < LpInputs.back().size() + LpOutputs.back().size(); ++i)
        {
            std::vector<double> row(LpInputs.back().size() + LpOutputs.back().size(), 0.0);
            row[i] = 1.0;
            bottomRows.push_back(row);
        }

        // Build top rows
        std::vector<std::vector<double>> topRows;
        for (int i = 0; i < LpOutputs.size(); ++i)
        {
            topRows.push_back(LpOutputs[i]);
        }

        for (int i = 0; i < LpInputs.size(); ++i)
        {
            for (int j = 0; j < LpInputs[i].size(); ++j)
            {
                topRows[i].push_back(-LpInputs[i][j]);
            }
        }

        // Build middle row
        std::vector<double> middleRow(tabWLen, 0.0);
        const auto &currentMiddleRow = LpInputs[currentSelection];

        // Replace the last elements with currentMiddleRow values
        int startIdx = tabWLen - currentMiddleRow.size();
        for (int i = 0; i < currentMiddleRow.size(); ++i)
        {
            middleRow[startIdx + i] = currentMiddleRow[i];
        }

        // Build zRow
        std::vector<double> zRow = LpOutputs[currentSelection];
        while (zRow.size() < tabWLen)
        {
            zRow.push_back(0.0);
        }

        // Build table
        std::vector<std::vector<double>> table;
        table.push_back(zRow);

        for (const auto &row : topRows)
        {
            table.push_back(row);
        }

        table.push_back(middleRow);

        for (const auto &row : bottomRows)
        {
            table.push_back(row);
        }

        // Build constraints
        std::vector<std::vector<double>> constraints;

        for (int i = 0; i < topRows.size(); ++i)
        {
            auto constraint = topRows[i];
            constraint.push_back(0.0);
            constraint.push_back(0.0);
            constraints.push_back(constraint);
        }

        auto tempMiddleRow = middleRow;
        tempMiddleRow.push_back(1.0);
        tempMiddleRow.push_back(0.0);
        constraints.push_back(tempMiddleRow);

        tempMiddleRow = middleRow;
        tempMiddleRow.push_back(1.0);
        tempMiddleRow.push_back(1.0);
        constraints.push_back(tempMiddleRow);

        for (int i = 0; i < bottomRows.size(); ++i)
        {
            auto tempCon = bottomRows[i];
            tempCon.push_back(0.0001);
            tempCon.push_back(1.0);
            constraints.push_back(tempCon);
        }

        return {table, zRow, constraints, LpInputs[currentSelection]};
    }

    struct SolveResult
    {
        double outputTotal;
        double inputTotal;
        std::vector<double> outputRange;
        std::vector<double> inputRange;
        std::vector<double> cellRef;
        std::vector<double> changingVars;
    };

    SolveResult solveTable(const std::vector<std::vector<double>> &table, const std::vector<double> &objfunc, const std::vector<std::vector<double>> &constraints, const std::vector<double> &conRow, bool isMin = false)
    {
        auto result = dual.DoDualSimplex(objfunc, constraints, isMin);
        auto changingVars = result.changingVars;
        double optimalSolution = result.optimalSolution;

        // if (isConsoleOutput)
        // {
        // outputStream << "\n"; 
        outputStream << "changingVars: ";
        for (double var : changingVars)
        {
            outputStream << var << " ";
        }
        outputStream << "\n";
        outputStream << "optimalSolution: " << optimalSolution << "\n";
        // outputStream << "\n\n";
        // }

        // Build mathCons (constraints without last column)
        std::vector<std::vector<double>> mathCons;
        for (const auto &constraint : constraints)
        {
            std::vector<double> mathCon;
            for (int j = 0; j < constraint.size() - 1; ++j)
            {
                mathCon.push_back(constraint[j]);
            }
            mathCons.push_back(mathCon);
        }

        // Build cellRef
        std::vector<double> cellRef;
        cellRef.push_back(optimalSolution);

        for (const auto &mathCon : mathCons)
        {
            double sum = 0.0;
            for (int i = 0; i < std::min(changingVars.size(), mathCon.size()); ++i)
            {
                sum += changingVars[i] * mathCon[i];
            }
            cellRef.push_back(sum);
        }

        // Get objFunc length (count non-zero elements)
        int objFuncLen = 0;
        for (double val : objfunc)
        {
            if (val != 0.0)
            {
                objFuncLen++;
            }
        }

        // Get outputs multiplied by changing vars
        std::vector<double> outputRange;
        for (int i = 0; i < objFuncLen; ++i)
        {
            outputRange.push_back(objfunc[i] * changingVars[i]);
        }

        // Get inputs multiplied by changing vars
        std::vector<double> inputRange;
        for (int i = 0; i < objfunc.size() - objFuncLen; ++i)
        {
            inputRange.push_back(conRow[i] * changingVars[i + objFuncLen]);
        }

        // Calculate totals
        double outputTotal = std::accumulate(outputRange.begin(), outputRange.end(), 0.0);
        double inputTotal = std::accumulate(inputRange.begin(), inputRange.end(), 0.0);

        return {outputTotal, inputTotal, outputRange, inputRange, cellRef, changingVars};
    }

    struct DEAResult
    {
        std::vector<std::vector<std::vector<double>>> tables;
        std::vector<std::string> header;
        std::vector<double> allInputTotals;
        std::vector<double> allOutputTotals;
        std::vector<std::vector<double>> allRangesO;
        std::vector<std::vector<double>> allRangesI;
        std::vector<std::vector<double>> allChangingVars;
    };

    DEAResult doDEA(const std::vector<std::vector<double>> &LpInputs, const std::vector<std::vector<double>> &LpOutputs, bool isMin = false)
    {
        // Clear the output stream at the start of doDEA to ensure fresh output
        outputStream.str("");
        outputStream.clear();

        std::vector<std::vector<std::vector<double>>> tables;
        std::vector<std::vector<double>> allRangesO;
        std::vector<std::vector<double>> allRangesI;
        std::vector<double> allOutputTotals;
        std::vector<double> allInputTotals;
        std::vector<std::vector<double>> allChangingVars;

        for (int i = 0; i < LpInputs.size(); ++i)
        {
            auto tableResult = buildTable(LpInputs, LpOutputs, i);
            auto solveResult = solveTable(tableResult.table, tableResult.zRow,
                                          tableResult.constraints, tableResult.conRow, isMin);

            allRangesO.push_back(solveResult.outputRange);
            allRangesI.push_back(solveResult.inputRange);
            allOutputTotals.push_back(solveResult.outputTotal);
            allInputTotals.push_back(solveResult.inputTotal);
            allChangingVars.push_back(solveResult.changingVars);

            // Add cellRef to table
            auto table = tableResult.table;
            for (int j = 0; j < solveResult.cellRef.size() - 1; ++j)
            {
                table[j].push_back(solveResult.cellRef[j]);
            }

            // Add sign column
            for (int j = 0; j < LpOutputs.size() + 1; ++j)
            {
                table[j].push_back(0.0);
            }
            table[LpOutputs.size() + 1].push_back(2.0);

            for (int j = 0; j < LpInputs.back().size() + LpOutputs.back().size(); ++j)
            {
                table[j + LpOutputs.size() + 2].push_back(1.0);
            }

            // Add RHS column
            for (int j = 0; j < LpOutputs.size() + 1; ++j)
            {
                table[j].push_back(0.0);
            }
            table[LpOutputs.size() + 1].push_back(1.0);

            for (int j = 0; j < LpInputs.back().size() + LpOutputs.back().size(); ++j)
            {
                table[j + LpOutputs.size() + 2].push_back(0.0001);
            }

            tables.push_back(table);
        }

        // Build header
        std::vector<std::string> header;
        for (int i = 0; i < LpOutputs.back().size(); ++i)
        {
            header.push_back("o" + std::to_string(i + 1));
        }
        for (int i = 0; i < LpInputs.back().size(); ++i)
        {
            header.push_back("i" + std::to_string(i + 1));
        }
        header.push_back("ref");
        header.push_back("sign");
        header.push_back("rhs");

        // Console output if enabled
        // if (isConsoleOutput)
        // {
            outputStream << "\n";
            for (int i = 0; i < tables.size(); ++i)
            {
                // Print changing variables
                for (double var : allChangingVars[i])
                {
                    outputStream << std::setw(10) << std::setprecision(4) << std::fixed << var << " ";
                }
                outputStream << "\n";

                // Print header
                for (const std::string &h : header)
                {
                    outputStream << std::setw(10) << h << " ";
                }
                outputStream << "\n";

                // Print table
                for (int j = 0; j < tables[i].size(); ++j)
                {
                    for (int k = 0; k < tables[i][j].size(); ++k)
                    {
                        if (k == LpOutputs.back().size() + LpInputs.back().size() + 1)
                        {
                            // Sign column
                            if (tables[i][j][k] == 0.0)
                            {
                                outputStream << std::setw(10) << "    <=" << " ";
                            }
                            else if (tables[i][j][k] == 1.0)
                            {
                                outputStream << std::setw(10) << "    >=" << " ";
                            }
                            else
                            {
                                outputStream << std::setw(10) << "     =" << " ";
                            }
                        }
                        else
                        {
                            outputStream << std::setw(10) << std::setprecision(4) << std::fixed
                                         << tables[i][j][k] << " ";
                        }
                    }
                    outputStream << "\n";
                }

                outputStream << "\nranges:\n\n";

                // Output ranges
                for (int j = 0; j < LpOutputs.back().size(); ++j)
                {
                    outputStream << std::setw(10) << ("o" + std::to_string(j + 1)) << " ";
                }
                outputStream << "\n";

                for (int j = 0; j < LpOutputs.back().size(); ++j)
                {
                    outputStream << std::setw(10) << std::setprecision(6) << std::fixed
                                 << allRangesO[i][j] << " ";
                }
                outputStream << "  Output total: " << allOutputTotals[i] << "\n";

                outputStream << "\n";

                // Input ranges
                for (int j = 0; j < LpInputs.back().size(); ++j)
                {
                    outputStream << std::setw(10) << ("i" + std::to_string(j + 1)) << " ";
                }
                outputStream << "\n";

                for (int j = 0; j < LpInputs.back().size(); ++j)
                {
                    outputStream << std::setw(10) << std::setprecision(6) << std::fixed
                                 << allRangesI[i][j] << " ";
                }
                outputStream << "   Input total: " << allInputTotals[i] << "\n";

                outputStream << "\n\nTotals:\n\n"
                             << allOutputTotals[i]
                             << "\ndivided by\n"
                             << allInputTotals[i]
                             << "\n\n= " << (allOutputTotals[i] / allInputTotals[i]) << "\n\n";
            }
        // }

        // Store results in member variables for getter access
        this->tables = tables;
        this->header = header;
        this->allInputTotals = allInputTotals;
        this->allOutputTotals = allOutputTotals;
        this->allRangesO = allRangesO;
        this->allRangesI = allRangesI;
        this->changingVars = allChangingVars;

        printOutput();

        return {tables, header, allInputTotals, allOutputTotals, allRangesO, allRangesI, allChangingVars};
    }

    // Getters for accessing internal state
    const std::vector<std::vector<std::vector<double>>> &getTables() const { return tables; }
    const std::vector<std::string> &getHeader() const { return header; }
    const std::vector<double> &getAllInputTotals() const { return allInputTotals; }
    const std::vector<double> &getAllOutputTotals() const { return allOutputTotals; }
    const std::vector<std::vector<double>> &getAllRangesO() const { return allRangesO; }
    const std::vector<std::vector<double>> &getAllRangesI() const { return allRangesI; }
    const std::vector<std::vector<double>> &getChangingVars() const { return changingVars; }

    // Setters for configuration
    void setConsoleOutput(bool enable)
    {
        isConsoleOutput = enable;
        if (!enable)
        {
            clearOutput(); // Clear output if console output is disabled
        }
    }
    void setProblemType(const std::string &type)
    {
        problemType = type;
        isMin = (type == "Min");
    }

    // Utility methods
    void printEfficiencyScores() const
    {
        // if (isConsoleOutput)
        // {
            outputStream << "\nEfficiency Scores:\n";
            for (int i = 0; i < allOutputTotals.size(); ++i)
            {
                double efficiency = allOutputTotals[i] / allInputTotals[i];
                outputStream << "DMU " << (i + 1) << ": " << std::setprecision(6) << std::fixed
                             << efficiency << "\n";
            }
        // }
    }

    std::vector<double> getEfficiencyScores() const
    {
        std::vector<double> scores;
        for (int i = 0; i < allOutputTotals.size(); ++i)
        {
            scores.push_back(allOutputTotals[i] / allInputTotals[i]);
        }
        return scores;
    }

    // New method to retrieve the accumulated output
    std::string getOutput() const
    {
        return outputStream.str();
    }

    // New method to print the accumulated output to console
    void printOutput() const
    {
        if (isConsoleOutput)
        {
            std::cout << outputStream.str();
        }
    }

    // New method to clear the output stream
    void clearOutput()
    {
        outputStream.str("");
        outputStream.clear();
    }
};