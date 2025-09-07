#ifndef BRANCH_AND_BOUND_HPP
#define BRANCH_AND_BOUND_HPP

#include <vector>
#include <string>
#include <stack>
#include <map>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <optional>
#include <numeric>

#include "dual_simplex.hpp"

class Logger
{
public:
    static void WriteLine(const std::string &message)
    {
        std::cout << message << std::endl;
    }

    static void Write(const std::string &message)
    {
        std::cout << message;
    }
};

enum class VariableSignType
{
    Positive,
    NonNegative,
    Unrestricted
};

class BranchAndBound
{
private:
    bool isConsoleOutput;
    int precision = 4;
    double tolerance = 1e-6;

    DualSimplex dual;
    std::vector<double> objFunc = {0.0, 0.0};
    std::vector<std::vector<double>> constraints = {{0.0, 0.0, 0.0, 0.0}};
    std::vector<std::vector<std::vector<double>>> newTableaus;
    bool isMin = false;

    // Tree traversal specific variables
    std::vector<double> bestSolution;
    double bestObjective;
    int nodeCounter = 0;
    std::vector<std::pair<std::vector<double>, double>> allSolutions;
    bool enablePruning = false;

    std::string bestSolutionNodeNum;
    std::vector<std::vector<double>> bestSolutionTableau;
    std::vector<std::vector<std::vector<double>>> displayTableausMin;

public:
    BranchAndBound(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput)
    {
        objFunc = {0.0, 0.0};
        constraints = {{0.0, 0.0, 0.0, 0.0}};
        bestObjective = isMin ? std::numeric_limits<double>::infinity()
                              : -std::numeric_limits<double>::infinity();
    }

    double RoundValue(double value)
    {
        try
        {
            return std::round(value * std::pow(10.0, precision)) / std::pow(10.0, precision);
        }
        catch (...)
        {
            return value;
        }
    }

// double RoundValue(double value)
// {
//     try
//     {
//         double multiplier = std::pow(10.0, precision);
//         double roundedValue = std::round(value * multiplier);
//         if (std::fmod(roundedValue, 2) == 0.5) {
//             roundedValue = std::floor(roundedValue / 2) * 2;
//         }
//         return roundedValue / multiplier;
//     }
//     catch (...)
//     {
//         return value;
//     }
// }

    std::vector<std::vector<double>> RoundMatrix(const std::vector<std::vector<double>> &matrix)
    {
        if (matrix.empty())
            return matrix;

        std::vector<std::vector<double>> result;
        for (const auto &row : matrix)
        {
            std::vector<double> roundedRow;
            for (const auto &val : row)
            {
                roundedRow.push_back(RoundValue(val));
            }
            result.push_back(roundedRow);
        }
        return result;
    }

    std::vector<double> RoundArray(const std::vector<double> &array)
    {
        if (array.empty())
            return array;

        std::vector<double> result;
        for (const auto &val : array)
        {
            result.push_back(RoundValue(val));
        }
        return result;
    }

    std::vector<std::vector<std::vector<double>>> RoundTableaus(
        const std::vector<std::vector<std::vector<double>>> &tableaus)
    {
        if (tableaus.empty())
            return tableaus;

        std::vector<std::vector<std::vector<double>>> roundedTableaus;
        for (const auto &tableau : tableaus)
        {
            roundedTableaus.push_back(RoundMatrix(tableau));
        }
        return roundedTableaus;
    }

    bool IsIntegerValue(double value)
    {
        double roundedVal = RoundValue(value);
        return std::abs(roundedVal - std::round(roundedVal)) <= tolerance;
    }

    void PrintTableau(const std::vector<std::vector<double>> &tableau,
                      const std::string &title = "Tableau")
    {
        std::vector<std::string> tempHeaderStr;
        for (size_t i = 0; i < objFunc.size(); ++i)
        {
            tempHeaderStr.push_back("x" + std::to_string(i + 1));
        }
        for (size_t i = 0; i < (tableau[tableau.size() - 1].size() - objFunc.size() - 1); ++i)
        {
            tempHeaderStr.push_back("s" + std::to_string(i + 1));
        }
        tempHeaderStr.push_back("rhs");

        if (isConsoleOutput)
        {
            Logger::WriteLine("\n" + title);
            for (const auto &header : tempHeaderStr)
            {
                // Logger::Write(std::setw(8) + header + "  ");
                std::cout << std::setw(8) << header << "  ";
            }
            Logger::WriteLine("");

            for (const auto &row : tableau)
            {
                for (const auto &val : row)
                {
                    // Logger::Write(std::setw(8) << std::fixed << std::setprecision(4) <<
                    //              std::to_string(RoundValue(val)) << "  ");
                    std::cout << std::setw(8) << std::fixed << std::setprecision(4) << std::to_string(RoundValue(val)) << "  ";
                }
                Logger::WriteLine("");
            }
            Logger::WriteLine("");
        }
    }

    std::vector<int> GetBasicVarSpots(const std::vector<std::vector<std::vector<double>>> &tableaus)
    {
        std::vector<int> basicVarSpots;
        const auto &lastTableau = tableaus[tableaus.size() - 1];

        for (size_t k = 0; k < lastTableau[lastTableau.size() - 1].size(); ++k)
        {
            std::vector<double> tCVars;
            for (size_t i = 0; i < lastTableau.size(); ++i)
            {
                tCVars.push_back(RoundValue(lastTableau[i][k]));
            }

            double sumVals = RoundValue(std::accumulate(tCVars.begin(), tCVars.end(), 0.0));
            if (std::abs(sumVals - 1.0) <= tolerance)
            {
                basicVarSpots.push_back(k);
            }
        }

        std::vector<std::vector<double>> basicVarCols;
        for (size_t i = 0; i < lastTableau[lastTableau.size() - 1].size(); ++i)
        {
            if (std::find(basicVarSpots.begin(), basicVarSpots.end(), i) != basicVarSpots.end())
            {
                std::vector<double> tLst;
                for (size_t j = 0; j < lastTableau.size(); ++j)
                {
                    tLst.push_back(RoundValue(lastTableau[j][i]));
                }
                basicVarCols.push_back(tLst);
            }
        }

        std::vector<std::pair<std::vector<double>, int>> zippedCbv;
        for (size_t i = 0; i < basicVarCols.size(); ++i)
        {
            zippedCbv.push_back({basicVarCols[i], basicVarSpots[i]});
        }

        std::sort(zippedCbv.begin(), zippedCbv.end(),
                  [](const auto &a, const auto &b)
                  {
                      auto aOne = std::find(a.first.begin(), a.first.end(), 1.0);
                      auto bOne = std::find(b.first.begin(), b.first.end(), 1.0);
                      size_t aIndex = aOne != a.first.end() ? std::distance(a.first.begin(), aOne) : a.first.size();
                      size_t bIndex = bOne != b.first.end() ? std::distance(b.first.begin(), bOne) : b.first.size();
                      return aIndex < bIndex;
                  });

        std::vector<int> sortedBasicVarSpots;
        for (const auto &item : zippedCbv)
        {
            sortedBasicVarSpots.push_back(item.second);
        }

        return sortedBasicVarSpots.empty() ? std::vector<int>() : sortedBasicVarSpots;
    }

    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>
    DoAddConstraint(const std::vector<std::vector<double>> &addedConstraints,
                    const std::vector<std::vector<double>> &overRideTab = {})
    {
        std::vector<std::vector<double>> changingTable;
        std::vector<int> basicVarSpots;

        if (!overRideTab.empty())
        {
            changingTable = overRideTab;
            changingTable = RoundMatrix(changingTable);
            basicVarSpots = GetBasicVarSpots({changingTable});
        }
        else
        {
            Logger::WriteLine("needs an input table");
            return {{}, {}};
        }

        auto newTab = changingTable;

        for (size_t k = 0; k < addedConstraints.size(); ++k)
        {
            for (auto &row : newTab)
            {
                row.insert(row.end() - 1, 0.0);
            }

            std::vector<double> newCon(newTab[0].size(), 0.0);
            for (size_t i = 0; i < addedConstraints[k].size() - 2; ++i)
            {
                newCon[i] = RoundValue(addedConstraints[k][i]);
            }
            newCon[newCon.size() - 1] = RoundValue(addedConstraints[k][addedConstraints[k].size() - 2]);
            int slackSpot = (newCon.size() - addedConstraints.size()) - 1 + k;
            newCon[slackSpot] = addedConstraints[k][addedConstraints[k].size() - 1] == 1 ? -1.0 : 1.0;

            newTab.push_back(newCon);
        }

        newTab = RoundMatrix(newTab);
        PrintTableau(newTab, "unfixed tab");

        auto displayTab = newTab;

        for (size_t k = 0; k < addedConstraints.size(); ++k)
        {
            size_t constraintRowIndex = newTab.size() - addedConstraints.size() + k;

            for (int colIndex : basicVarSpots)
            {
                double coefficientInNewRow = RoundValue(displayTab[constraintRowIndex][colIndex]);

                if (std::abs(coefficientInNewRow) > tolerance)
                {
                    std::optional<size_t> pivotRow;
                    for (size_t rowIndex = 0; rowIndex < displayTab.size() - addedConstraints.size(); ++rowIndex)
                    {
                        if (std::abs(RoundValue(displayTab[rowIndex][colIndex]) - 1.0) <= tolerance)
                        {
                            pivotRow = rowIndex;
                            break;
                        }
                    }

                    if (pivotRow)
                    {
                        bool autoReverse = addedConstraints[k][addedConstraints[k].size() - 1] == 1;
                        for (size_t col = 0; col < displayTab[0].size(); ++col)
                        {
                            double pivotVal = RoundValue(displayTab[*pivotRow][col]);
                            double constraintVal = RoundValue(displayTab[constraintRowIndex][col]);
                            double newVal = autoReverse ? pivotVal - coefficientInNewRow * constraintVal
                                                        : constraintVal - coefficientInNewRow * pivotVal;
                            displayTab[constraintRowIndex][col] = RoundValue(newVal);
                        }
                    }
                }
            }
        }

        displayTab = RoundMatrix(displayTab);
        PrintTableau(displayTab, "fixed tab");

        return {displayTab, newTab};
    }

    std::pair<std::optional<int>, std::optional<double>>
    TestIfBasicVarIsInt(const std::vector<std::vector<std::vector<double>>> &tabs)
    {
        std::vector<double> decisionVars;
        const auto &lastTableau = tabs[tabs.size() - 1];

        for (size_t i = 0; i < objFunc.size(); ++i)
        {
            bool found = false;
            for (size_t j = 0; j < lastTableau.size(); ++j)
            {
                double val = RoundValue(lastTableau[j][i]);
                if (std::abs(val - 1.0) <= tolerance)
                {
                    decisionVars.push_back(RoundValue(lastTableau[j][lastTableau[j].size() - 1]));
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                decisionVars.push_back(0.0);
            }
        }

        int bestXSpot = -1;
        std::optional<double> bestRhsVal;
        double minDistanceToHalf = std::numeric_limits<double>::infinity();

        for (size_t i = 0; i < decisionVars.size(); ++i)
        {
            if (!IsIntegerValue(decisionVars[i]))
            {
                double fractionalPart = decisionVars[i] - std::floor(decisionVars[i]);
                double distanceToHalf = std::abs(fractionalPart - 0.5);
                if (distanceToHalf < minDistanceToHalf)
                {
                    minDistanceToHalf = distanceToHalf;
                    bestXSpot = i;
                    bestRhsVal = decisionVars[i];
                }
            }
        }

        // return bestXSpot == -1 ? std::make_pair(std::nullopt, std::nullopt)
        //                       : std::make_pair(bestXSpot, bestRhsVal);

        return bestXSpot == -1 ? std::make_pair(std::optional<int>(std::nullopt), std::optional<double>(std::nullopt))
                               : std::make_pair(bestXSpot, bestRhsVal);
    }

    std::pair<std::vector<double>, std::vector<double>>
    MakeBranch(const std::vector<std::vector<std::vector<double>>> &tabs)
    {
        auto [xSpot, rhsVal] = TestIfBasicVarIsInt(tabs);
        if (!xSpot || !rhsVal)
        {
            return {{}, {}};
        }

        if (isConsoleOutput)
        {
            Logger::WriteLine("Branching on x" + std::to_string(*xSpot + 1) +
                              " = " + std::to_string(RoundValue(*rhsVal)));
        }

        int maxInt = std::ceil(*rhsVal);
        int minInt = std::floor(*rhsVal);

        std::vector<double> newConMin(objFunc.size() + 2, 0.0);
        newConMin[*xSpot] = 1.0;
        newConMin[newConMin.size() - 2] = minInt;
        newConMin[newConMin.size() - 1] = 0; // <= constraint

        std::vector<double> newConMax(objFunc.size() + 2, 0.0);
        newConMax[*xSpot] = 1.0;
        newConMax[newConMax.size() - 2] = maxInt;
        newConMax[newConMax.size() - 1] = 1; // >= constraint

        return {newConMin, newConMax};
    }

    std::optional<double> GetObjectiveValue(const std::vector<std::vector<std::vector<double>>> &tabs)
    {
        if (tabs.empty())
            return std::nullopt;
        return RoundValue(tabs[tabs.size() - 1][0][tabs[tabs.size() - 1][0].size() - 1]);
    }

    std::vector<double> GetCurrentSolution(const std::vector<std::vector<std::vector<double>>> &tabs)
    {
        std::vector<double> solution(objFunc.size(), 0.0);
        const auto &lastTableau = tabs[tabs.size() - 1];

        for (size_t i = 0; i < objFunc.size(); ++i)
        {
            for (size_t j = 0; j < lastTableau.size(); ++j)
            {
                double val = RoundValue(lastTableau[j][i]);
                if (std::abs(val - 1.0) <= tolerance)
                {
                    solution[i] = RoundValue(lastTableau[j][lastTableau[j].size() - 1]);
                    break;
                }
            }
        }
        return solution;
    }

    bool IsIntegerSolution(const std::vector<double> &solution)
    {
        for (double val : solution)
        {
            if (!IsIntegerValue(val))
            {
                return false;
            }
        }
        return true;
    }

    bool UpdateBestSolution(const std::vector<std::vector<std::vector<double>>> &tabs,
                            const std::string &nodeLabel = "")
    {
        auto objVal = GetObjectiveValue(tabs);
        auto solution = GetCurrentSolution(tabs);

        if (!objVal)
            return false;

        if (IsIntegerSolution(solution))
        {
            allSolutions.push_back({solution, *objVal});

            bool isBetter = isMin ? *objVal < bestObjective : *objVal > bestObjective;

            if (isBetter)
            {
                bestObjective = *objVal;
                bestSolution = solution;
                bestSolutionTableau = tabs[tabs.size() - 1];
                bestSolutionNodeNum = nodeLabel;

                if (isConsoleOutput)
                {
                    std::string solStr = "[";
                    for (size_t i = 0; i < solution.size(); ++i)
                    {
                        solStr += std::to_string(solution[i]);
                        if (i < solution.size() - 1)
                            solStr += ", ";
                    }
                    solStr += "]";
                    Logger::WriteLine("New best integer solution Candidate found: " +
                                      solStr + " with objective " + std::to_string(*objVal));
                }
                return true;
            }
            else if (isConsoleOutput)
            {
                std::string solStr = "[";
                for (size_t i = 0; i < solution.size(); ++i)
                {
                    solStr += std::to_string(solution[i]);
                    if (i < solution.size() - 1)
                        solStr += ", ";
                }
                solStr += "]";
                Logger::WriteLine("Integer solution Candidate found: " + solStr +
                                  " with objective " + std::to_string(*objVal) +
                                  " (not better than current best)");
            }
        }
        return false;
    }

    bool ShouldPrune(const std::vector<std::vector<std::vector<double>>> &tabs)
    {
        if (!enablePruning)
            return false;

        auto objVal = GetObjectiveValue(tabs);
        if (!objVal)
            return true;

        if (!bestSolution.empty())
        {
            return isMin ? *objVal >= bestObjective : *objVal <= bestObjective;
        }
        return false;
    }

    std::pair<std::vector<double>, double>
    DoBranchAndBound(std::vector<std::vector<std::vector<double>>> initialTabs,
                     bool enablePruning = false)
    {
        this->enablePruning = enablePruning;

        if (isConsoleOutput)
        {
            Logger::WriteLine("Starting Branch and Bound Algorithm");
            Logger::WriteLine(enablePruning ? "Pruning: ENABLED (standard branch and bound)"
                                            : "Pruning: DISABLED (complete tree exploration)");
            Logger::WriteLine(std::string(50, '='));
        }

        initialTabs = RoundTableaus(initialTabs);
        bestSolution.clear();
        bestObjective = isMin ? std::numeric_limits<double>::infinity()
                              : -std::numeric_limits<double>::infinity();
        nodeCounter = 0;
        allSolutions.clear();

        struct Node
        {
            std::vector<std::vector<std::vector<double>>> tabs;
            int depth;
            std::string nodeLabel;
            std::vector<std::string> constraintsPath;
            std::string parentLabel;
        };

        std::stack<Node> nodeStack;
        nodeStack.push({initialTabs, 0, "0", {}, ""});
        std::map<std::string, int> childCounters;

        int ctr = 0;
        while (!nodeStack.empty())
        {
            if (++ctr > 100)
            {
                Logger::WriteLine("Something is very wrong unless you need more than 100 nodes");
                break;
            }

            auto [currentTabs, depth, nodeLabel, constraintsPath, parentLabel] = nodeStack.top();
            nodeStack.pop();
            nodeCounter++;

            currentTabs = RoundTableaus(currentTabs);

            if (isConsoleOutput)
            {
                Logger::WriteLine("\n--- Processing Node " + nodeLabel + " (Depth " +
                                  std::to_string(depth) + ") ---");
                if (!parentLabel.empty())
                {
                    Logger::WriteLine("Parent: " + parentLabel);
                }
                Logger::WriteLine("Constraints path: [" +
                                  (constraintsPath.empty() ? "" : std::accumulate(constraintsPath.begin() + 1, constraintsPath.end(), constraintsPath[0], [](const auto &a, const auto &b)
                                                                                  { return a + ", " + b; })) +
                                  "]");
            }

            if (ShouldPrune(currentTabs))
            {
                if (isConsoleOutput)
                {
                    Logger::WriteLine("Node " + nodeLabel + " pruned by bound");
                }
                continue;
            }

            UpdateBestSolution(currentTabs, nodeLabel);

            auto [newConMin, newConMax] = MakeBranch(currentTabs);
            if (newConMin.empty() && newConMax.empty())
            {
                if (isConsoleOutput)
                {
                    auto solution = GetCurrentSolution(currentTabs);
                    auto objVal = GetObjectiveValue(currentTabs);
                    std::string solStr = "[";
                    for (size_t i = 0; i < solution.size(); ++i)
                    {
                        solStr += std::to_string(solution[i]);
                        if (i < solution.size() - 1)
                            solStr += ", ";
                    }
                    solStr += "]";
                    Logger::WriteLine("Node " + nodeLabel + ": Integer solution " +
                                      solStr + " with objective " +
                                      (objVal ? std::to_string(*objVal) : "null"));
                }
                continue;
            }

            if (childCounters.find(nodeLabel) == childCounters.end())
            {
                childCounters[nodeLabel] = 0;
            }

            std::vector<Node> childNodes;

            try
            {
                childCounters[nodeLabel]++;
                std::string childLabel = nodeLabel == "0" ? "1" : nodeLabel + "." + std::to_string(childCounters[nodeLabel]);

                if (isConsoleOutput)
                {
                    Logger::Write("\nTrying MIN branch (Node " + childLabel + "): [" +
                                  std::to_string(newConMin[0]));
                    for (size_t i = 1; i < newConMin.size(); ++i)
                    {
                        Logger::Write(", " + std::to_string(newConMin[i]));
                    }
                    Logger::Write("] ");
                    for (size_t i = 0; i < newConMin.size() - 2; ++i)
                    {
                        if (newConMin[i] == 0.0)
                            continue;
                        Logger::Write((newConMin[i] == 1.0 ? "" : std::to_string(newConMin[i]) + "*") +
                                      "x" + std::to_string(i + 1) + " ");
                    }
                    Logger::Write(newConMin[newConMin.size() - 1] == 0 ? "<= " : ">= ");
                    Logger::Write(std::to_string(newConMin[newConMin.size() - 2]) + " ");
                }

                auto [displayTabMin, newTabMin] = DoAddConstraint({newConMin}, currentTabs[currentTabs.size() - 1]);
                auto [newTableausMin, changingVarsMin, optimalSolutionMin, pivotColsMin, pivotRowsMin, headerRowMin] =
                    dual.DoDualSimplex({}, {}, isMin, &displayTabMin);

                if (std::isnan(optimalSolutionMin))
                {
                    PrintTableau(newTableausMin[0], "Node " + childLabel + ": Infeasible tableau");
                    newTableausMin.clear();
                }

                if (!(std::isnan(optimalSolutionMin)))
                {
                    if (!newTableausMin.empty())
                    {
                        newTableausMin = RoundTableaus(newTableausMin);
                        std::string constraintDesc = "x" +
                                                     std::to_string(std::distance(newConMin.begin(),
                                                                                  std::find(newConMin.begin(),
                                                                                            newConMin.end() - 2, 1.0)) +
                                                                    1) +
                                                     " <= " + std::to_string(newConMin[newConMin.size() - 2]);
                        auto newConstraintsPath = constraintsPath;
                        newConstraintsPath.push_back(constraintDesc);
                        childNodes.push_back({newTableausMin, depth + 1, childLabel,
                                              newConstraintsPath, nodeLabel});
                    }
                    else if (isConsoleOutput)
                    {
                        Logger::WriteLine("MIN branch (Node " + childLabel + ") infeasible");
                    }
                }

                if (isConsoleOutput && !newTableausMin.empty())
                {
                    for (size_t i = 0; i < newTableausMin.size() - 1; ++i)
                    {
                        PrintTableau(newTableausMin[i], "Node " + childLabel +
                                                            " MIN branch Tableau " + std::to_string(i + 1));
                        displayTableausMin.push_back(newTableausMin[i]);
                    }
                    PrintTableau(newTableausMin[newTableausMin.size() - 1],
                                 "Node " + childLabel + " MIN branch final tableau");
                    displayTableausMin.push_back(newTableausMin[newTableausMin.size() - 1]);
                }
            }
            catch (const std::exception &e)
            {
                if (isConsoleOutput)
                {
                    Logger::WriteLine("MIN branch (Node " +
                                      std::to_string(childCounters[nodeLabel]) +
                                      ") failed: " + e.what());
                }
            }

            try
            {
                childCounters[nodeLabel]++;
                std::string childLabel = nodeLabel == "0" ? "2" : nodeLabel + "." + std::to_string(childCounters[nodeLabel]);

                if (isConsoleOutput)
                {
                    Logger::Write("\nTrying MAX branch (Node " + childLabel + "): [" +
                                  std::to_string(newConMax[0]));
                    for (size_t i = 1; i < newConMax.size(); ++i)
                    {
                        Logger::Write(", " + std::to_string(newConMax[i]));
                    }
                    Logger::Write("] ");
                    for (size_t i = 0; i < newConMax.size() - 2; ++i)
                    {
                        if (newConMax[i] == 0.0)
                            continue;
                        Logger::Write((newConMax[i] == 1.0 ? "" : std::to_string(newConMax[i]) + "*") +
                                      "x" + std::to_string(i + 1) + " ");
                    }
                    Logger::Write(newConMax[newConMax.size() - 1] == 0 ? "<= " : ">= ");
                    Logger::Write(std::to_string(newConMax[newConMax.size() - 2]) + " ");
                }

                auto [displayTabMax, newTabMax] = DoAddConstraint({newConMax}, currentTabs[currentTabs.size() - 1]);
                auto [newTableausMax, changingVarsMax, optimalSolutionMax, pivotColsMax, pivotRowsMax, headerRowMax] =
                    dual.DoDualSimplex({}, {}, isMin, &displayTabMax);

                if (std::isnan(optimalSolutionMax))
                {
                    PrintTableau(newTableausMax[0], "Node " + childLabel + ": Infeasible tableau");
                    newTableausMax.clear();
                }

                if (!(std::isnan(optimalSolutionMax)))
                {
                    if (!newTableausMax.empty())
                    {
                        newTableausMax = RoundTableaus(newTableausMax);
                        std::string constraintDesc = "x" +
                                                     std::to_string(std::distance(newConMax.begin(),
                                                                                  std::find(newConMax.begin(),
                                                                                            newConMax.end() - 2, 1.0)) +
                                                                    1) +
                                                     " >= " + std::to_string(newConMax[newConMax.size() - 2]);
                        auto newConstraintsPath = constraintsPath;
                        newConstraintsPath.push_back(constraintDesc);
                        childNodes.push_back({newTableausMax, depth + 1, childLabel,
                                              newConstraintsPath, nodeLabel});
                    }
                    else if (isConsoleOutput)
                    {
                        Logger::WriteLine("MAX branch (Node " + childLabel + ") infeasible");
                    }
                }

                if (isConsoleOutput && !newTableausMax.empty())
                {
                    for (size_t i = 0; i < newTableausMax.size() - 1; ++i)
                    {
                        PrintTableau(newTableausMax[i], "Node " + childLabel +
                                                            " MAX branch Tableau " + std::to_string(i + 1));
                    }
                    PrintTableau(newTableausMax[newTableausMax.size() - 1],
                                 "Node " + childLabel + " MAX branch final tableau");
                }
            }
            catch (const std::exception &e)
            {
                if (isConsoleOutput)
                {
                    Logger::WriteLine("MAX branch (Node " +
                                      std::to_string(childCounters[nodeLabel]) +
                                      ") failed: " + e.what());
                }
            }

            for (auto it = childNodes.rbegin(); it != childNodes.rend(); ++it)
            {
                nodeStack.push(*it);
            }
        }

        if (isConsoleOutput)
        {
            Logger::WriteLine("\n" + std::string(50, '='));
            Logger::WriteLine("BRANCH AND BOUND COMPLETED");
            Logger::WriteLine(std::string(50, '='));
            if (!bestSolution.empty())
            {
                PrintTableau(bestSolutionTableau, "Best Candidate solution tableau at node " +
                                                      bestSolutionNodeNum);
                Logger::WriteLine("Node of best solution: " + bestSolutionNodeNum);
                std::string solStr = "[";
                for (size_t i = 0; i < bestSolution.size(); ++i)
                {
                    solStr += std::to_string(bestSolution[i]);
                    if (i < bestSolution.size() - 1)
                        solStr += ", ";
                }
                solStr += "]";
                Logger::WriteLine("Best integer solution: " + solStr);
                Logger::WriteLine("Best objective value: " + std::to_string(bestObjective));
                Logger::WriteLine("Best solution:");
                PrintBasicVars(bestSolutionTableau);
            }
            else
            {
                Logger::WriteLine("No integer solution found");
            }
            Logger::WriteLine("Total nodes processed: " + std::to_string(nodeCounter));

            if (!allSolutions.empty())
            {
                Logger::WriteLine("\nAll integer solutions found (" +
                                  std::to_string(allSolutions.size()) + "):");
                for (size_t i = 0; i < allSolutions.size(); ++i)
                {
                    std::string solStr = "[";
                    for (size_t j = 0; j < allSolutions[i].first.size(); ++j)
                    {
                        solStr += std::to_string(allSolutions[i].first[j]);
                        if (j < allSolutions[i].first.size() - 1)
                            solStr += ", ";
                    }
                    solStr += "]";
                    Logger::WriteLine("  " + std::to_string(i + 1) + ". Solution: " +
                                      solStr + ", Objective: " +
                                      std::to_string(allSolutions[i].second));
                }
            }
        }

        return {bestSolution, bestObjective};
    }

    std::pair<std::vector<double>, std::vector<std::vector<double>>>
    SetUpProblem(std::vector<double> objFunc,
                 std::vector<std::vector<double>> constraints)
    {
        int numConstraints = objFunc.size();
        int vectorLength = objFunc.size() + 3;

        constraints.clear();
        for (int i = 0; i < numConstraints; ++i)
        {
            std::vector<double> row(vectorLength, 0.0);
            row[i] = 1.0;
            row[vectorLength - 2] = 1.0;
            constraints.push_back(row);
        }

        return {objFunc, constraints};
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
                column.push_back(tableau[row][col]);
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

        for (int col : basicVarColumns)
        {
            int basicRow = -1;
            for (size_t row = 1; row < tableau.size(); ++row)
            {
                if (std::abs(tableau[row][col] - 1.0) < tolerance)
                {
                    basicRow = row;
                    break;
                }
            }

            if (basicRow != -1)
            {
                double rhsValue = tableau[basicRow][tableau[basicRow].size() - 1];
                Logger::WriteLine("x" + std::to_string(col + 1) + " = " +
                                  std::to_string(rhsValue));
            }
        }
        Logger::WriteLine("");
    }

    void RunBranchAndBound(const std::vector<double> &objFuncPassed,
                           const std::vector<std::vector<double>> &constraintsPassed,
                           bool isMin)
    {
        std::cout << "running" << std::endl;
        bool enablePruning = false;

        try
        {
            objFunc = objFuncPassed;
            constraints = constraintsPassed;
            // std::tie(objFunc, constraints) = SetUpProblem(objFunc, constraints);

            auto a = objFunc;
            auto b = constraints;

            // Note: CanonicalFormBuilder is not implemented in this conversion
            // You would need to implement the equivalent functionality

            try
            {
                auto [newTableaus, changingVars, optimalSolution, pivotCols, pivotRows, headerRow] =
                    dual.DoDualSimplex(a, b, isMin);

                this->newTableaus = RoundTableaus(newTableaus);

                if (isConsoleOutput)
                {
                    Logger::WriteLine("Initial LP relaxation solved");
                    for (size_t i = 0; i < this->newTableaus.size() - 1; ++i)
                    {
                        PrintTableau(this->newTableaus[i], "Initial Tableau " +
                                                               std::to_string(i + 1));
                    }
                    PrintTableau(this->newTableaus[this->newTableaus.size() - 1],
                                 "Initial tableau solved");
                    auto solution = GetCurrentSolution(this->newTableaus);
                    auto objVal = GetObjectiveValue(this->newTableaus);
                    std::string solStr = "[";
                    for (size_t i = 0; i < solution.size(); ++i)
                    {
                        solStr += std::to_string(solution[i]);
                        if (i < solution.size() - 1)
                            solStr += ", ";
                    }
                    solStr += "]";
                    Logger::WriteLine("Initial solution: " + solStr);
                    Logger::WriteLine("Initial objective: " +
                                      (objVal ? std::to_string(*objVal) : "null"));
                }

                DoBranchAndBound(this->newTableaus, enablePruning);
            }
            catch (const std::exception &e)
            {
                if (isConsoleOutput)
                {
                    Logger::WriteLine("Error in dual simplex: " + std::string(e.what()));
                }
                throw;
            }
        }
        catch (const std::exception &e)
        {
            Logger::WriteLine("math error: " + std::string(e.what()));
            throw;
        }
    }

private:
    std::vector<std::vector<double>> DeepCopy(const std::vector<std::vector<double>> &original)
    {
        if (original.empty())
            return {};
        std::vector<std::vector<double>> copy;
        for (const auto &row : original)
        {
            copy.push_back(row);
        }
        return copy;
    }
};

#endif