#pragma once

#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <tuple>

#include "dual_simplex.hpp"

// class Logger {
// public:
//     static void Write(const std::string& message) {
//         std::cout << message;
//     }

//     static void WriteLine(const std::string& message = "") {
//         std::cout << message << std::endl;
//     }
// };

class CanonicalFormBuilder
{
public:
    static void BuildCanonicalForm(std::vector<double> &objFunc,
                                   std::vector<std::vector<double>> &constraints)
    {
        // Placeholder implementation
    }
};

class CuttingPlane
{
private:
    bool isConsoleOutput;
    int precision;
    double tolerance;
    int maxIterations;

    DualSimplex dual;

    bool isMin;

    // simplex specific vars
    std::string problemType;
    std::string absProblemType;

    // dual constraints
    // int amtOfObjVars;
    std::vector<double> objFunc;
    std::vector<std::vector<double>> constraints;

public:
    CuttingPlane(bool isConsoleOutput = false)
        : isConsoleOutput(isConsoleOutput),
          precision(4),
          tolerance(1e-6),
          maxIterations(100),
          isMin(false),
          problemType("Max"),
          absProblemType("abs Off")
        //   amtOfObjVars(2)
    {
        objFunc = {0.0, 0.0};
        constraints = {{0.0, 0.0, 0.0, 0.0}};
    }

    double roundValue(double value)
    {
        try
        {
            double multiplier = std::pow(10.0, precision);
            return std::round(value * multiplier) / multiplier;
        }
        catch (...)
        {
            return value;
        }
    }

    void printTableau(const std::vector<std::vector<double>> &tableau, const std::string &title = "Tableau")
    {
        // Build header
        std::vector<std::string> tempHeaderStr;
        for (int i = 0; i < objFunc.size(); ++i)
        {
            tempHeaderStr.push_back("x" + std::to_string(i + 1));
        }
        for (int i = 0; i < (tableau[0].size() - objFunc.size() - 1); ++i)
        {
            tempHeaderStr.push_back("s/e" + std::to_string(i + 1));
        }
        tempHeaderStr.push_back("rhs");

        // Print title
        Logger::WriteLine(title);

        // Print headers
        for (const auto &header : tempHeaderStr)
        {
            std::cout << std::setw(10) << header; // wider for readability
        }
        std::cout << "\n";

        // Print rows
        for (const auto &row : tableau)
        {
            for (const auto &val : row)
            {
                std::cout << std::setw(10) << std::fixed << std::setprecision(4) << roundValue(val);
            }
            std::cout << "\n";
        }

        Logger::WriteLine("");
    }

    // void printTableau(const std::vector<std::vector<double>> &tableau, const std::string &title = "Tableau")
    // {
    //     std::vector<std::string> tempHeaderStr;
    //     for (int i = 0; i < objFunc.size(); ++i)
    //     {
    //         tempHeaderStr.push_back("x" + std::to_string(i + 1));
    //     }
    //     for (int i = 0; i < (tableau[tableau.size() - 1].size() - objFunc.size() - 1); ++i)
    //     {
    //         tempHeaderStr.push_back("s/e" + std::to_string(i + 1));
    //     }
    //     tempHeaderStr.push_back("rhs");

    //     Logger::WriteLine("" + title);
    //     for (const auto &header : tempHeaderStr)
    //     {
    //         // Logger::Write(std::setw(8) + header + "  ");
    //         std::cout << std::setw(8) << header << "  ";
    //     }
    //     Logger::WriteLine("");

    //     for (const auto &row : tableau)
    //     {
    //         for (const auto &val : row)
    //         {
    //             // Logger::Write(std::setw(8) + std::fixed + std::setprecision(4) + std::to_string(roundValue(val)) + "  ");
    //             std::cout << std::setw(8) << std::fixed << std::setprecision(precision) << std::to_string(roundValue(val)) << "  ";
    //         }
    //         Logger::WriteLine("");
    //     }
    //     Logger::WriteLine("");
    // }

    std::vector<std::vector<double>> roundMatrix(const std::vector<std::vector<double>> &matrix)
    {
        std::vector<std::vector<double>> result;
        for (const auto &row : matrix)
        {
            std::vector<double> roundedRow;
            for (const auto &val : row)
            {
                roundedRow.push_back(roundValue(val));
            }
            result.push_back(roundedRow);
        }
        return result;
    }

    double cleanValue(double value, double tolerance = 1e-10)
    {
        return std::abs(value) < tolerance ? 0.0 : value;
    }

    std::vector<double> gomoryCut(const std::vector<double> &row, bool verbose = false)
    {
        if (row.size() < 2)
        {
            throw std::invalid_argument("Input must have at least one coefficient and RHS.");
        }

        std::vector<double> cleanRow;
        for (const auto &x : row)
        {
            cleanRow.push_back(cleanValue(x));
        }

        std::vector<double> coefs(cleanRow.begin(), cleanRow.end() - 1);
        double rhs = cleanRow.back();

        auto getFractionalPart = [](double x)
        {
            int integer_part = static_cast<int>(x);
            double fractional_part = x - integer_part;
            if (fractional_part < 0)
            {
                fractional_part += 1;
            }
            return fractional_part;
        };

        // auto getFractionalPart = [this](double x)
        // {
        //     double roundedX = roundValue(x); // rounds to 4 decimal places
        //     double frac = roundedX - std::floor(roundedX);
        //     return (frac < tolerance || 1.0 - frac < tolerance) ? 0.0 : frac;
        // };

        if (verbose)
        {
            std::string rowStr;
            for (const auto &c : coefs)
            {
                rowStr += std::to_string(roundValue(c)) + ", ";
            }
            Logger::WriteLine("Original row: " + rowStr + "| " + std::to_string(roundValue(rhs)));
        }

        std::vector<double> fracCoefs;
        for (const auto &coef : coefs)
        {
            fracCoefs.push_back(-getFractionalPart(coef));
        }

        double rhsFrac = getFractionalPart(rhs);
        double negFracRhs = -rhsFrac;

        std::vector<double> result;
        for (const auto &x : fracCoefs)
        {
            result.push_back(cleanValue(x));
        }
        result.push_back(cleanValue(negFracRhs));

        if (verbose)
        {
            std::string cutStr;
            for (const auto &r : result)
            {
                cutStr += std::to_string(roundValue(r)) + ", ";
            }
            Logger::WriteLine("Gomory cut coefficients: " + cutStr);
        }

        return result;
    }

    std::vector<int> getBasicVarSpots(const std::vector<std::vector<std::vector<double>>> &tableaus)
    {
        std::vector<int> basicVarSpots;
        const auto &tableau = tableaus.back();

        for (size_t k = 0; k < tableau[0].size(); ++k)
        {
            std::vector<double> column;
            for (const auto &row : tableau)
            {
                column.push_back(cleanValue(row[k]));
            }

            int ones_count = std::count_if(column.begin(), column.end(),
                                           [this](double x)
                                           { return std::abs(x - 1.0) < tolerance; });
            int zeros_count = std::count_if(column.begin(), column.end(),
                                            [this](double x)
                                            { return std::abs(x) < tolerance; });

            if (ones_count == 1 && zeros_count == column.size() - 1)
            {
                basicVarSpots.push_back(k);
            }
        }

        return basicVarSpots;
    }

    int findMostFractionalRow(const std::vector<std::vector<double>> &tableau)
    {
        double bestFractionalScore = -1;
        int selectedRow = 1;
        std::vector<std::tuple<int, double, double>> candidates;

        if (isConsoleOutput)
        {
            Logger::WriteLine("\nAnalyzing fractional parts:");
        }

        for (size_t i = 1; i < tableau.size(); ++i)
        {
            double rhsValue = cleanValue(tableau[i].back());
            double fractionalPart = rhsValue - std::floor(rhsValue);
            double fractionalScore = std::min(fractionalPart, 1 - fractionalPart);

            if (isConsoleOutput)
            {
                Logger::WriteLine("Row " + std::to_string(i) + ": RHS = " +
                                  std::to_string(roundValue(rhsValue)) + ", fractional part = " +
                                  std::to_string(roundValue(fractionalPart)) + ", score = " +
                                  std::to_string(roundValue(fractionalScore)));
            }

            if (fractionalPart > tolerance && fractionalPart < (1 - tolerance))
            {
                if (fractionalScore > bestFractionalScore)
                {
                    bestFractionalScore = fractionalScore;
                    candidates = {std::make_tuple(i, fractionalPart, rhsValue)};
                }
                else if (std::abs(fractionalScore - bestFractionalScore) < tolerance)
                {
                    candidates.emplace_back(i, fractionalPart, rhsValue);
                }
            }
        }

        if (!candidates.empty())
        {
            std::sort(candidates.begin(), candidates.end(),
                      [](const auto &a, const auto &b)
                      {
                          return std::get<1>(a) > std::get<1>(b) ||
                                 (std::get<1>(a) == std::get<1>(b) && std::get<2>(a) < std::get<2>(b));
                      });
            selectedRow = std::get<0>(candidates[0]);

            if (isConsoleOutput)
            {
                std::string candidateStr;
                for (const auto &c : candidates)
                {
                    candidateStr += "(" + std::to_string(std::get<0>(c)) + ", RHS=" +
                                    std::to_string(roundValue(std::get<2>(c))) + ", frac=" +
                                    std::to_string(roundValue(std::get<1>(c))) + "), ";
                }
                Logger::WriteLine("Candidates with score " + std::to_string(roundValue(bestFractionalScore)) + ": " + candidateStr);
            }
        }

        if (isConsoleOutput)
        {
            Logger::WriteLine("Selected row " + std::to_string(selectedRow));
        }

        return selectedRow;
    }

    bool hasFractionalSolution(const std::vector<std::vector<double>> &tableau)
    {
        // for (size_t i = 1; i < tableau.size(); ++i) {
        //     double rhsValue = cleanValue(tableau[i].back());
        //     double fractionalPart = rhsValue - std::floor(rhsValue);
        //     if (fractionalPart > tolerance) {
        //         return true;
        //     }
        // }
        // return false;
        auto getFractionalPart = [this](double x)
        {
            double roundedX = roundValue(x); // rounds to 4 decimal places
            double frac = roundedX - std::floor(roundedX);
            return (frac < tolerance || 1.0 - frac < tolerance) ? 0.0 : frac;
        };

        for (size_t i = 1; i < tableau.size(); ++i)
        {
            double rhsValue = cleanValue(tableau[i].back());
            double fractionalPart = getFractionalPart(rhsValue);
            if (fractionalPart > 0.0)
                return true;
        }
        return false;
    }

    std::vector<std::vector<double>> cleanTableau(const std::vector<std::vector<double>> &tableau)
    {
        std::vector<std::vector<double>> cleanedTableau;
        for (const auto &row : tableau)
        {
            std::vector<double> cleanedRow;
            for (const auto &val : row)
            {
                cleanedRow.push_back(cleanValue(val));
            }
            cleanedTableau.push_back(cleanedRow);
        }
        return cleanedTableau;
    }

    void PrintBasicVars(const std::vector<std::vector<double>> &tableau)
    {
        Logger::WriteLine("\n=== BASIC VARIABLE VALUES (Decision Variables Only) ===");
        std::vector<int> basicVarColumns;

        for (size_t col = 0; col < objFunc.size(); ++col)
        {
            std::vector<double> column;
            for (size_t row = 1; row < tableau.size(); ++row)
            {
                column.push_back(cleanValue(tableau[row][col]));
            }

            int onesCount = std::count_if(column.begin(), column.end(),
                                          [this](double x)
                                          { return std::abs(x - 1.0) < tolerance; });
            int zerosCount = std::count_if(column.begin(), column.end(),
                                           [this](double x)
                                           { return std::abs(x) < tolerance; });

            if (onesCount == 1 && zerosCount == column.size() - 1)
            {
                basicVarColumns.push_back(col);
            }
        }

        for (const auto &col : basicVarColumns)
        {
            int basicRow = -1;
            for (size_t row = 1; row < tableau.size(); ++row)
            {
                if (std::abs(cleanValue(tableau[row][col]) - 1.0) < tolerance)
                {
                    basicRow = row;
                    break;
                }
            }

            if (basicRow != -1)
            {
                double rhsValue = cleanValue(tableau[basicRow].back());
                std::string varName = "x" + std::to_string(col + 1);
                Logger::WriteLine(varName + " = " + std::to_string(roundValue(rhsValue)));
            }
        }
        Logger::WriteLine("");
    }

    std::vector<std::vector<double>> doCuttingPlane(std::vector<std::vector<double>> workingTableau)
    {
        auto currentTableau = cleanTableau(workingTableau);
        int iteration = 1;

        while (hasFractionalSolution(currentTableau) && iteration <= maxIterations)
        {
            Logger::WriteLine("\n=== CUTTING PLANE ITERATION " + std::to_string(iteration) + " ===");
            currentTableau = cleanTableau(currentTableau);
            int pickedRow = findMostFractionalRow(currentTableau);

            std::string rowStr;
            for (const auto &x : currentTableau[pickedRow])
            {
                rowStr += std::to_string(roundValue(x)) + ", ";
            }
            Logger::WriteLine("Selected row " + std::to_string(pickedRow) + " for Gomory cut: " + rowStr);

            std::vector<double> tempList(currentTableau[pickedRow].begin() + objFunc.size(), currentTableau[pickedRow].end());
            auto newCon = gomoryCut(tempList, true);
            for (auto &x : newCon)
            {
                x = cleanValue(x);
            }

            for (size_t i = 0; i < objFunc.size(); ++i)
            {
                newCon.insert(newCon.begin(), 0);
            }
            newCon.insert(newCon.end() - 1, 1);

            std::string conStr;
            for (const auto &x : newCon)
            {
                conStr += std::to_string(roundValue(x)) + ", ";
            }
            Logger::WriteLine("Generated cutting plane constraint: " + conStr);

            Logger::WriteLine("Adding new slack variable column...\n");
            for (auto &row : currentTableau)
            {
                row.insert(row.end() - 1, 0);
            }

            currentTableau.push_back(newCon);
            printTableau(currentTableau, "Tableau with cutting plane constraint " + std::to_string(iteration));

            auto result = dual.DoDualSimplex({}, {}, isMin, &currentTableau);
            auto finalTableaus = result.tableaus;
            auto headerStr = result.headerRow;

            for (size_t i = 0; i < finalTableaus.size(); ++i)
            {
                printTableau(finalTableaus[i], "Iteration " + std::to_string(iteration) + " - Tableau " + std::to_string(i + 1));
            }

            currentTableau = cleanTableau(finalTableaus.back());
            ++iteration;
        }

        if (iteration >= maxIterations)
        {
            Logger::WriteLine("\nsomething is very wrong");
        }

        if (!hasFractionalSolution(currentTableau))
        {
            Logger::WriteLine("\n=== OPTIMAL INTEGER SOLUTION FOUND ===");
            Logger::WriteLine("Solution achieved after " + std::to_string(iteration - 1) + " cutting plane iterations");
        }
        else
        {
            Logger::WriteLine("\n=== MAXIMUM ITERATIONS REACHED ===");
            Logger::WriteLine("Stopped after " + std::to_string(maxIterations) + " iterations");
        }

        printTableau(currentTableau, "Final Optimal Tableau");
        double optimalSolution = currentTableau[0].back();
        Logger::WriteLine("Optimal Solution: " + std::to_string(roundValue(optimalSolution)));
        PrintBasicVars(currentTableau);

        return currentTableau;
    }

    void RunCuttingPlane(std::vector<double> objFuncPassed,
                         std::vector<std::vector<double>> constraintsPassed,
                         bool isMinPassed)
    {
        objFunc = objFuncPassed;
        constraints = constraintsPassed;
        isMin = isMinPassed;

        // std::tie(objFunc, constraints) = SetUpProblem(objFunc, constraints, varSigns);

        CanonicalFormBuilder::BuildCanonicalForm(objFunc, constraints);

        auto result = dual.DoDualSimplex(objFunc, constraints, isMin);

        auto workingTableaus = result.tableaus;

        for (size_t i = 0; i < workingTableaus.size(); ++i)
        {
            printTableau(workingTableaus[i], "Initial Tableau " + std::to_string(i + 1));
        }

        doCuttingPlane(workingTableaus.back());
    }
};