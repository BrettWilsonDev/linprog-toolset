
#pragma once

#include <vector>
#include <string>
#include <stack>
#include <map>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <optional>
#include <numeric>
#include <memory>
#include <sstream>
#include <fstream>

#include "dual_simplex.hpp"

class Logger
{
public:
    static void writeLine(const std::string &message)
    {
        std::cout << message << std::endl;
    }

    static void write(const std::string &message)
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

struct TreeNode
{
    std::string name;
    std::optional<double> objective;
    std::vector<double> solution;
    bool isInteger = false;
    bool infeasible = false;
    bool pruned = false;
    std::vector<std::string> constraintsPath;
    std::vector<std::unique_ptr<TreeNode>> children;
    std::string unfixedTabStr = "";
    std::string fixedTabStr = "";
    std::string finalTableauStr = "";
    std::vector<std::string> intermediateTableausStr = {};
    std::vector<int> pivotCols = {};
    std::vector<int> pivotRows = {};
};

class BranchAndBound
{
private:
    bool isConsoleOutput = false;
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

    std::string escapeJson(const std::string &s) const
    {
        std::ostringstream o;
        for (auto c = s.cbegin(); c != s.cend(); c++)
        {
            switch (*c)
            {
            case '"':
                o << "\\\"";
                break;
            case '\\':
                o << "\\\\";
                break;
            case '\b':
                o << "\\b";
                break;
            case '\f':
                o << "\\f";
                break;
            case '\n':
                o << "\\n";
                break;
            case '\r':
                o << "\\r";
                break;
            case '\t':
                o << "\\t";
                break;
            default:
                if ('\x00' <= *c && *c <= '\x1f')
                {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
                }
                else
                {
                    o << *c;
                }
            }
        }
        return o.str();
    }

public:
    BranchAndBound(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput)
    {
        objFunc = {0.0, 0.0};
        constraints = {{0.0, 0.0, 0.0, 0.0}};
        bestObjective = isMin ? std::numeric_limits<double>::infinity()
                              : -std::numeric_limits<double>::infinity();
    }

    double roundValue(double value)
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

    std::vector<std::vector<double>> roundMatrix(const std::vector<std::vector<double>> &matrix)
    {
        if (matrix.empty())
            return matrix;

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

    std::vector<double> roundArray(const std::vector<double> &array)
    {
        if (array.empty())
            return array;

        std::vector<double> result;
        for (const auto &val : array)
        {
            result.push_back(roundValue(val));
        }
        return result;
    }

    std::vector<std::vector<std::vector<double>>> roundTableaus(
        const std::vector<std::vector<std::vector<double>>> &tableaus)
    {
        if (tableaus.empty())
            return tableaus;

        std::vector<std::vector<std::vector<double>>> roundedTableaus;
        for (const auto &tableau : tableaus)
        {
            roundedTableaus.push_back(roundMatrix(tableau));
        }
        return roundedTableaus;
    }

    bool isIntegerValue(double value)
    {
        double roundedVal = roundValue(value);
        return std::abs(roundedVal - std::round(roundedVal)) <= tolerance;
    }

    std::string getTableauString(const std::vector<std::vector<double>> &tableau,
                                 const std::string &title = "Tableau")
    {
        std::ostringstream oss;
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

        oss << "\n" + title << std::endl;
        for (const auto &header : tempHeaderStr)
        {
            oss << std::setw(8) << header << "  ";
        }
        oss << std::endl;

        for (const auto &row : tableau)
        {
            for (const auto &val : row)
            {
                oss << std::setw(8) << std::fixed << std::setprecision(4) << std::to_string(roundValue(val)) << "  ";
            }
            oss << std::endl;
        }
        oss << std::endl;

        return oss.str();
    }

    void printTableau(const std::vector<std::vector<double>> &tableau,
                      const std::string &title = "Tableau")
    {
        if (isConsoleOutput)
        {
            std::cout << getTableauString(tableau, title);
        }
    }

    std::vector<int> getBasicVarSpots(const std::vector<std::vector<std::vector<double>>> &tableaus)
    {
        std::vector<int> basicVarSpots;
        const auto &lastTableau = tableaus[tableaus.size() - 1];

        for (size_t k = 0; k < lastTableau[lastTableau.size() - 1].size(); ++k)
        {
            std::vector<double> tCVars;
            for (size_t i = 0; i < lastTableau.size(); ++i)
            {
                tCVars.push_back(roundValue(lastTableau[i][k]));
            }

            double sumVals = roundValue(std::accumulate(tCVars.begin(), tCVars.end(), 0.0));
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
                    tLst.push_back(roundValue(lastTableau[j][i]));
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
    doAddConstraint(const std::vector<std::vector<double>> &addedConstraints,
                    const std::vector<std::vector<double>> &overrideTab = {})
    {
        std::vector<std::vector<double>> changingTable;
        std::vector<int> basicVarSpots;

        if (!overrideTab.empty())
        {
            changingTable = overrideTab;
            changingTable = roundMatrix(changingTable);
            basicVarSpots = getBasicVarSpots({changingTable});
        }
        else
        {
            Logger::writeLine("needs an input table");
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
                newCon[i] = roundValue(addedConstraints[k][i]);
            }
            newCon[newCon.size() - 1] = roundValue(addedConstraints[k][addedConstraints[k].size() - 2]);
            int slackSpot = (newCon.size() - addedConstraints.size()) - 1 + k;
            newCon[slackSpot] = addedConstraints[k][addedConstraints[k].size() - 1] == 1 ? -1.0 : 1.0;

            newTab.push_back(newCon);
        }

        newTab = roundMatrix(newTab);
        printTableau(newTab, "unfixed tab");

        auto displayTab = newTab;

        for (size_t k = 0; k < addedConstraints.size(); ++k)
        {
            size_t constraintRowIndex = newTab.size() - addedConstraints.size() + k;

            for (int colIndex : basicVarSpots)
            {
                double coefficientInNewRow = roundValue(displayTab[constraintRowIndex][colIndex]);

                if (std::abs(coefficientInNewRow) > tolerance)
                {
                    std::optional<size_t> pivotRow;
                    for (size_t rowIndex = 0; rowIndex < displayTab.size() - addedConstraints.size(); ++rowIndex)
                    {
                        if (std::abs(roundValue(displayTab[rowIndex][colIndex]) - 1.0) <= tolerance)
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
                            double pivotVal = roundValue(displayTab[*pivotRow][col]);
                            double constraintVal = roundValue(displayTab[constraintRowIndex][col]);
                            double newVal = autoReverse ? pivotVal - coefficientInNewRow * constraintVal
                                                        : constraintVal - coefficientInNewRow * pivotVal;
                            displayTab[constraintRowIndex][col] = roundValue(newVal);
                        }
                    }
                }
            }
        }

        displayTab = roundMatrix(displayTab);
        printTableau(displayTab, "fixed tab");

        return {displayTab, newTab};
    }

    std::pair<std::optional<int>, std::optional<double>>
    testIfBasicVarIsInt(const std::vector<std::vector<std::vector<double>>> &tabs)
    {
        std::vector<double> decisionVars;
        const auto &lastTableau = tabs[tabs.size() - 1];

        for (size_t i = 0; i < objFunc.size(); ++i)
        {
            bool found = false;
            for (size_t j = 0; j < lastTableau.size(); ++j)
            {
                double val = roundValue(lastTableau[j][i]);
                if (std::abs(val - 1.0) <= tolerance)
                {
                    decisionVars.push_back(roundValue(lastTableau[j][lastTableau[j].size() - 1]));
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
            if (!isIntegerValue(decisionVars[i]))
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

        return bestXSpot == -1 ? std::make_pair(std::optional<int>(std::nullopt), std::optional<double>(std::nullopt))
                               : std::make_pair(bestXSpot, bestRhsVal);
    }

    std::pair<std::vector<double>, std::vector<double>>
    makeBranch(const std::vector<std::vector<std::vector<double>>> &tabs)
    {
        auto [xSpot, rhsVal] = testIfBasicVarIsInt(tabs);
        if (!xSpot || !rhsVal)
        {
            return {{}, {}};
        }

        if (isConsoleOutput)
        {
            Logger::writeLine("Branching on x" + std::to_string(*xSpot + 1) +
                              " = " + std::to_string(roundValue(*rhsVal)));
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

    std::optional<double> getObjectiveValue(const std::vector<std::vector<std::vector<double>>> &tabs)
    {
        if (tabs.empty())
            return std::nullopt;
        return roundValue(tabs[tabs.size() - 1][0][tabs[tabs.size() - 1][0].size() - 1]);
    }

    std::vector<double> getCurrentSolution(const std::vector<std::vector<std::vector<double>>> &tabs)
    {
        std::vector<double> solution(objFunc.size(), 0.0);
        const auto &lastTableau = tabs[tabs.size() - 1];

        for (size_t i = 0; i < objFunc.size(); ++i)
        {
            for (size_t j = 0; j < lastTableau.size(); ++j)
            {
                double val = roundValue(lastTableau[j][i]);
                if (std::abs(val - 1.0) <= tolerance)
                {
                    solution[i] = roundValue(lastTableau[j][lastTableau[j].size() - 1]);
                    break;
                }
            }
        }
        return solution;
    }

    bool isIntegerSolution(const std::vector<double> &solution)
    {
        for (double val : solution)
        {
            if (!isIntegerValue(val))
            {
                return false;
            }
        }
        return true;
    }

    bool updateBestSolution(const std::vector<std::vector<std::vector<double>>> &tabs,
                            const std::string &nodeLabel = "")
    {
        auto objVal = getObjectiveValue(tabs);
        auto solution = getCurrentSolution(tabs);

        if (!objVal)
            return false;

        if (isIntegerSolution(solution))
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
                    Logger::writeLine("New best integer solution Candidate found: " +
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
                Logger::writeLine("Integer solution Candidate found: " + solStr +
                                  " with objective " + std::to_string(*objVal) +
                                  " (not better than current best)");
            }
        }
        return false;
    }

    bool shouldPrune(const std::vector<std::vector<std::vector<double>>> &tabs)
    {
        if (!enablePruning)
            return false;

        auto objVal = getObjectiveValue(tabs);
        if (!objVal)
            return true;

        if (!bestSolution.empty())
        {
            return isMin ? *objVal >= bestObjective : *objVal <= bestObjective;
        }
        return false;
    }

    std::string toJson(const TreeNode *node) const
    {
        std::string json = "{";
        json += "\"name\":\"" + node->name + "\",";
        json += "\"objective\":" + (node->objective.has_value() ? std::to_string((node->objective.value())) : "null") + ",";
        json += "\"solution\":";
        if (node->solution.empty() && node->infeasible)
        {
            json += "null,";
        }
        else
        {
            json += "[";
            for (size_t i = 0; i < node->solution.size(); ++i)
            {
                json += std::to_string((node->solution[i]));
                if (i < node->solution.size() - 1)
                    json += ",";
            }
            json += "],";
        }
        json += "\"isInteger\":" + std::string(node->isInteger ? "true" : "false") + ",";
        json += "\"infeasible\":" + std::string(node->infeasible ? "true" : "false") + ",";
        json += "\"pruned\":" + std::string(node->pruned ? "true" : "false") + ",";
        json += "\"constraintsPath\":[";
        for (size_t i = 0; i < node->constraintsPath.size(); ++i)
        {
            if (i > 0)
                json += ",";
            json += "\"" + escapeJson(node->constraintsPath[i]) + "\"";
        }
        json += "],";
        json += "\"unfixedTab\":\"" + escapeJson(node->unfixedTabStr) + "\",";
        json += "\"fixedTab\":\"" + escapeJson(node->fixedTabStr) + "\",";
        json += "\"finalTableau\":\"" + escapeJson(node->finalTableauStr) + "\",";
        json += "\"intermediateTableaus\":[";
        for (size_t i = 0; i < node->intermediateTableausStr.size(); ++i)
        {
            if (i > 0)
                json += ",";
            json += "\"" + escapeJson(node->intermediateTableausStr[i]) + "\"";
        }
        json += "],";
        json += "\"pivotCols\":[";
        for (size_t i = 0; i < node->pivotCols.size(); ++i)
        {
            if (i > 0)
                json += ",";
            json += std::to_string(node->pivotCols[i]);
        }
        json += "],";
        json += "\"pivotRows\":[";
        for (size_t i = 0; i < node->pivotRows.size(); ++i)
        {
            if (i > 0)
                json += ",";
            json += std::to_string(node->pivotRows[i]);
        }
        json += "],";
        json += "\"children\":[";
        for (size_t i = 0; i < node->children.size(); ++i)
        {
            if (i > 0)
                json += ",";
            json += toJson(node->children[i].get());
        }
        json += "]";
        json += "}";
        return json;
    }

    std::pair<std::vector<double>, double>
    doBranchAndBound(std::vector<std::vector<std::vector<double>>> initialTabs, bool enablePruning = false, const std::vector<int> &initialPivotCols = {}, const std::vector<int> &initialPivotRows = {})
    {
        this->enablePruning = enablePruning;

        if (isConsoleOutput)
        {
            Logger::writeLine("Starting Branch and Bound Algorithm");
            Logger::writeLine(enablePruning ? "Pruning: ENABLED (standard branch and bound)"
                                            : "Pruning: DISABLED (complete tree exploration)");
            Logger::writeLine(std::string(50, '='));
        }

        initialTabs = roundTableaus(initialTabs);
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
            TreeNode *treeNode;
        };

        auto root = std::make_unique<TreeNode>();
        root->name = "0";
        root->constraintsPath = {};
        root->infeasible = false;
        root->pruned = false;
        root->finalTableauStr = getTableauString(initialTabs.back(), "Initial tableau solved");
        for (size_t i = 0; i < initialTabs.size() - 1; ++i)
        {
            root->intermediateTableausStr.push_back(getTableauString(initialTabs[i], "Initial Tableau " + std::to_string(i + 1)));
        }
        root->pivotCols = initialPivotCols;
        root->pivotRows = initialPivotRows;

        std::stack<Node> nodeStack;
        nodeStack.push({initialTabs, 0, "0", {}, "", root.get()});
        std::map<std::string, int> childCounters;

        int ctr = 0;
        while (!nodeStack.empty())
        {
            if (++ctr > 100)
            {
                Logger::writeLine("Something is very wrong unless you need more than 100 nodes");
                break;
            }

            auto current = nodeStack.top();
            nodeStack.pop();
            nodeCounter++;

            auto currentTreeNode = current.treeNode;

            current.tabs = roundTableaus(current.tabs);

            if (isConsoleOutput)
            {
                Logger::writeLine("\n--- Processing Node " + current.nodeLabel + " (Depth " +
                                  std::to_string(current.depth) + ") ---");
                if (!current.parentLabel.empty())
                {
                    Logger::writeLine("Parent: " + current.parentLabel);
                }
                Logger::writeLine("Constraints path: [" +
                                  (current.constraintsPath.empty() ? "" : std::accumulate(current.constraintsPath.begin() + 1, current.constraintsPath.end(), current.constraintsPath[0], [](const auto &a, const auto &b)
                                                                                          { return a + ", " + b; })) +
                                  "]");
            }

            auto sol = getCurrentSolution(current.tabs);
            auto obj = getObjectiveValue(current.tabs);
            currentTreeNode->solution = sol;
            currentTreeNode->objective = obj;
            bool isIntSol = isIntegerSolution(sol);
            currentTreeNode->isInteger = isIntSol;

            if (shouldPrune(current.tabs))
            {
                currentTreeNode->pruned = true;
                if (isConsoleOutput)
                {
                    Logger::writeLine("Node " + current.nodeLabel + " pruned by bound");
                }
                continue;
            }

            updateBestSolution(current.tabs, current.nodeLabel);

            auto [newConMin, newConMax] = makeBranch(current.tabs);
            if (newConMin.empty() && newConMax.empty())
            {
                if (isConsoleOutput)
                {
                    auto solution = getCurrentSolution(current.tabs);
                    auto objVal = getObjectiveValue(current.tabs);
                    std::string solStr = "[";
                    for (size_t i = 0; i < solution.size(); ++i)
                    {
                        solStr += std::to_string(solution[i]);
                        if (i < solution.size() - 1)
                            solStr += ", ";
                    }
                    solStr += "]";
                    Logger::writeLine("Node " + current.nodeLabel + ": Integer solution " +
                                      solStr + " with objective " +
                                      (objVal ? std::to_string(*objVal) : "null"));
                }
                continue;
            }

            if (childCounters.find(current.nodeLabel) == childCounters.end())
            {
                childCounters[current.nodeLabel] = 0;
            }

            std::vector<std::unique_ptr<TreeNode>> newChildren;
            std::vector<Node> childNodes;

            auto [xSpot, rhsVal] = testIfBasicVarIsInt(current.tabs);

            // MIN branch
            try
            {
                childCounters[current.nodeLabel]++;
                std::string childLabel = current.nodeLabel == "0" ? "1" : current.nodeLabel + "." + std::to_string(childCounters[current.nodeLabel]);

                if (isConsoleOutput)
                {
                    Logger::write("\nTrying MIN branch (Node " + childLabel + "): [");
                    for (size_t i = 0; i < newConMin.size(); ++i)
                    {
                        if (i > 0)
                            Logger::write(", ");
                        Logger::write(std::to_string(newConMin[i]));
                    }
                    Logger::write("] ");
                    for (size_t i = 0; i < newConMin.size() - 2; ++i)
                    {
                        if (newConMin[i] == 0.0)
                            continue;
                        Logger::write((newConMin[i] == 1.0 ? "" : std::to_string(newConMin[i]) + "*") +
                                      "x" + std::to_string(i + 1) + " ");
                    }
                    Logger::write(newConMin[newConMin.size() - 1] == 0 ? "<= " : ">= ");
                    Logger::writeLine(std::to_string(newConMin[newConMin.size() - 2]));
                }

                auto [displayTabMin, newTabMin] = doAddConstraint({newConMin}, current.tabs[current.tabs.size() - 1]);
                auto [newTableausMin, changingVarsMin, optimalSolutionMin, pivotColsMin, pivotRowsMin, headerRowMin] =
                    dual.DoDualSimplex({}, {}, isMin, &displayTabMin);

                bool minInfeasible = std::isnan(optimalSolutionMin);
                if (minInfeasible)
                {
                    if (!newTableausMin.empty())
                    {
                        printTableau(newTableausMin[0], "Node " + childLabel + ": Infeasible tableau");
                    }
                    newTableausMin.clear();
                }

                auto childMin = std::make_unique<TreeNode>();
                childMin->name = childLabel;
                std::string constraintDesc = "x" + std::to_string(*xSpot + 1) + " <= " + std::to_string(static_cast<int>(std::floor(*rhsVal)));
                auto newConstraintsPath = current.constraintsPath;
                newConstraintsPath.push_back(constraintDesc);
                childMin->constraintsPath = newConstraintsPath;
                childMin->pruned = false;
                childMin->unfixedTabStr = getTableauString(newTabMin, "unfixed tab");
                childMin->fixedTabStr = getTableauString(displayTabMin, "fixed tab");
                childMin->pivotCols = pivotColsMin;
                childMin->pivotRows = pivotRowsMin;

                if (newTableausMin.empty())
                {
                    childMin->infeasible = true;
                    childMin->isInteger = false;
                    childMin->objective = std::nullopt;
                    childMin->solution = {};
                    if (isConsoleOutput)
                        Logger::writeLine("MIN branch (Node " + childLabel + ") infeasible");
                }
                else
                {
                    childMin->infeasible = false;
                    newTableausMin = roundTableaus(newTableausMin);
                    auto childSol = getCurrentSolution(newTableausMin);
                    auto childObj = getObjectiveValue(newTableausMin);
                    childMin->solution = childSol;
                    childMin->objective = childObj;
                    childMin->isInteger = isIntegerSolution(childSol);
                }

                if (!newTableausMin.empty())
                {
                    if (minInfeasible && !newTableausMin.empty())
                    {
                        childMin->finalTableauStr = getTableauString(newTableausMin[0], "Node " + childLabel + ": Infeasible tableau");
                    }
                    else
                    {
                        for (size_t i = 0; i < newTableausMin.size() - 1; ++i)
                        {
                            std::string title = "Node " + childLabel + " MIN branch Tableau " + std::to_string(i + 1);
                            printTableau(newTableausMin[i], title);
                            childMin->intermediateTableausStr.push_back(getTableauString(newTableausMin[i], title));
                            displayTableausMin.push_back(newTableausMin[i]);
                        }
                        std::string finalTitle = "Node " + childLabel + " MIN branch final tableau";
                        printTableau(newTableausMin[newTableausMin.size() - 1], finalTitle);
                        childMin->finalTableauStr = getTableauString(newTableausMin[newTableausMin.size() - 1], finalTitle);
                        displayTableausMin.push_back(newTableausMin[newTableausMin.size() - 1]);
                    }
                }

                newChildren.push_back(std::move(childMin));
                if (!newTableausMin.empty())
                {
                    childNodes.push_back({newTableausMin, current.depth + 1, childLabel, newChildren.back()->constraintsPath, current.nodeLabel, newChildren.back().get()});
                }
            }
            catch (const std::exception &e)
            {
                if (isConsoleOutput)
                {
                    Logger::writeLine("MIN branch failed: " + std::string(e.what()));
                }
            }

            // MAX branch
            try
            {
                childCounters[current.nodeLabel]++;
                std::string childLabel = current.nodeLabel == "0" ? "2" : current.nodeLabel + "." + std::to_string(childCounters[current.nodeLabel]);

                if (isConsoleOutput)
                {
                    Logger::write("\nTrying MAX branch (Node " + childLabel + "): [");
                    for (size_t i = 0; i < newConMax.size(); ++i)
                    {
                        if (i > 0)
                            Logger::write(", ");
                        Logger::write(std::to_string(newConMax[i]));
                    }
                    Logger::write("] ");
                    for (size_t i = 0; i < newConMax.size() - 2; ++i)
                    {
                        if (newConMax[i] == 0.0)
                            continue;
                        Logger::write((newConMax[i] == 1.0 ? "" : std::to_string(newConMax[i]) + "*") +
                                      "x" + std::to_string(i + 1) + " ");
                    }
                    Logger::write(newConMax[newConMax.size() - 1] == 0 ? "<= " : ">= ");
                    Logger::writeLine(std::to_string(newConMax[newConMax.size() - 2]));
                }

                auto [displayTabMax, newTabMax] = doAddConstraint({newConMax}, current.tabs[current.tabs.size() - 1]);
                auto [newTableausMax, changingVarsMax, optimalSolutionMax, pivotColsMax, pivotRowsMax, headerRowMax] =
                    dual.DoDualSimplex({}, {}, isMin, &displayTabMax);

                bool maxInfeasible = std::isnan(optimalSolutionMax);
                if (maxInfeasible)
                {
                    if (!newTableausMax.empty())
                    {
                        printTableau(newTableausMax[0], "Node " + childLabel + ": Infeasible tableau");
                    }
                    newTableausMax.clear();
                }

                auto childMax = std::make_unique<TreeNode>();
                childMax->name = childLabel;
                std::string constraintDesc = "x" + std::to_string(*xSpot + 1) + " >= " + std::to_string(static_cast<int>(std::ceil(*rhsVal)));
                auto newConstraintsPath = current.constraintsPath;
                newConstraintsPath.push_back(constraintDesc);
                childMax->constraintsPath = newConstraintsPath;
                childMax->pruned = false;
                childMax->unfixedTabStr = getTableauString(newTabMax, "unfixed tab");
                childMax->fixedTabStr = getTableauString(displayTabMax, "fixed tab");
                childMax->pivotCols = pivotColsMax;
                childMax->pivotRows = pivotRowsMax;

                if (newTableausMax.empty())
                {
                    childMax->infeasible = true;
                    childMax->isInteger = false;
                    childMax->objective = std::nullopt;
                    childMax->solution = {};
                    if (isConsoleOutput)
                        Logger::writeLine("MAX branch (Node " + childLabel + ") infeasible");
                }
                else
                {
                    childMax->infeasible = false;
                    newTableausMax = roundTableaus(newTableausMax);
                    auto childSol = getCurrentSolution(newTableausMax);
                    auto childObj = getObjectiveValue(newTableausMax);
                    childMax->solution = childSol;
                    childMax->objective = childObj;
                    childMax->isInteger = isIntegerSolution(childSol);
                }

                if (!newTableausMax.empty())
                {
                    if (maxInfeasible && !newTableausMax.empty())
                    {
                        childMax->finalTableauStr = getTableauString(newTableausMax[0], "Node " + childLabel + ": Infeasible tableau");
                    }
                    else
                    {
                        for (size_t i = 0; i < newTableausMax.size() - 1; ++i)
                        {
                            std::string title = "Node " + childLabel + " MAX branch Tableau " + std::to_string(i + 1);
                            printTableau(newTableausMax[i], title);
                            childMax->intermediateTableausStr.push_back(getTableauString(newTableausMax[i], title));
                        }
                        std::string finalTitle = "Node " + childLabel + " MAX branch final tableau";
                        printTableau(newTableausMax[newTableausMax.size() - 1], finalTitle);
                        childMax->finalTableauStr = getTableauString(newTableausMax[newTableausMax.size() - 1], finalTitle);
                    }
                }

                newChildren.push_back(std::move(childMax));
                if (!newTableausMax.empty())
                {
                    childNodes.push_back({newTableausMax, current.depth + 1, childLabel, newChildren.back()->constraintsPath, current.nodeLabel, newChildren.back().get()});
                }
            }
            catch (const std::exception &e)
            {
                if (isConsoleOutput)
                {
                    Logger::writeLine("MAX branch failed: " + std::string(e.what()));
                }
            }

            for (auto &ch : newChildren)
            {
                currentTreeNode->children.push_back(std::move(ch));
            }

            for (auto it = childNodes.rbegin(); it != childNodes.rend(); ++it)
            {
                nodeStack.push(*it);
            }
        }

        if (isConsoleOutput)
        {
            Logger::writeLine("\n" + std::string(50, '='));
            Logger::writeLine("BRANCH AND BOUND COMPLETED");
            Logger::writeLine(std::string(50, '='));
            if (!bestSolution.empty())
            {
                printTableau(bestSolutionTableau, "Best Candidate solution tableau at node " +
                                                      bestSolutionNodeNum);
                Logger::writeLine("Node of best solution: " + bestSolutionNodeNum);
                std::string solStr = "[";
                for (size_t i = 0; i < bestSolution.size(); ++i)
                {
                    solStr += std::to_string(bestSolution[i]);
                    if (i < bestSolution.size() - 1)
                        solStr += ", ";
                }
                solStr += "]";
                Logger::writeLine("Best integer solution: " + solStr);
                Logger::writeLine("Best objective value: " + std::to_string(bestObjective));
                Logger::writeLine("Best solution:");
                printBasicVars(bestSolutionTableau);
            }
            else
            {
                Logger::writeLine("No integer solution found");
            }
            Logger::writeLine("Total nodes processed: " + std::to_string(nodeCounter));

            if (!allSolutions.empty())
            {
                Logger::writeLine("\nAll integer solutions found (" +
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
                    Logger::writeLine("  " + std::to_string(i + 1) + ". Solution: " +
                                      solStr + ", Objective: " +
                                      std::to_string(allSolutions[i].second));
                }
            }
        }

        this->solution += "\n" + std::string(50, '=');
        this->solution += "\nBRANCH AND BOUND COMPLETED";
        this->solution += "\n" + std::string(50, '=');

        if (!bestSolution.empty())
        {
            this->solution += "\nNode of best solution: " + bestSolutionNodeNum;
            std::string solStr = "[";
            
            for (size_t i = 0; i < bestSolution.size(); ++i)
            {
                solStr += std::to_string(bestSolution[i]);
                if (i < bestSolution.size() - 1)
                    solStr += ", ";
            }
            solStr += "]";

            this->solution += "\nBest integer solution: " + solStr;
            this->solution += "\nBest objective value: " + std::to_string(bestObjective);
        }
        else
        {
            this->solution += "\nNo integer solution found";
        }

        this->solution += "\nTotal nodes processed: " + std::to_string(nodeCounter);

        this->jsonOut = toJson(root.get());

        return {bestSolution, bestObjective};
    }

    std::pair<std::vector<double>, std::vector<std::vector<double>>>
    setUpProblem(std::vector<double> objFunc,
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

    void printBasicVars(const std::vector<std::vector<double>> &tableau)
    {
        if (isConsoleOutput)
        {
            Logger::writeLine("\n=== BASIC VARIABLE VALUES (Decision Variables Only) ===");

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
                    Logger::writeLine("x" + std::to_string(col + 1) + " = " +
                                      std::to_string(rhsValue));
                }
            }
            Logger::writeLine("");
        }
    }

    void
    RunBranchAndBound(const std::vector<double> &objFuncPassed, const std::vector<std::vector<double>> &constraintsPassed, bool isMin)
    {
        // std::cout << "running" << std::endl;
        bool enablePruning = false;

        try
        {
            objFunc = objFuncPassed;
            constraints = constraintsPassed;

            auto a = objFunc;
            auto b = constraints;

            try
            {
                auto [newTableaus, changingVars, optimalSolution, pivotCols, pivotRows, headerRow] =
                    dual.DoDualSimplex(a, b, isMin);

                this->newTableaus = roundTableaus(newTableaus);

                if (isConsoleOutput)
                {
                    Logger::writeLine("Initial LP relaxation solved");
                    for (size_t i = 0; i < this->newTableaus.size() - 1; ++i)
                    {
                        std::string title = "Initial Tableau " + std::to_string(i + 1);
                        printTableau(this->newTableaus[i], title);
                    }
                    printTableau(this->newTableaus[this->newTableaus.size() - 1],
                                 "Initial tableau solved");
                    auto solution = getCurrentSolution(this->newTableaus);
                    auto objVal = getObjectiveValue(this->newTableaus);
                    std::string solStr = "[";
                    for (size_t i = 0; i < solution.size(); ++i)
                    {
                        solStr += std::to_string(solution[i]);
                        if (i < solution.size() - 1)
                            solStr += ", ";
                    }
                    solStr += "]";
                    Logger::writeLine("Initial solution: " + solStr);
                    Logger::writeLine("Initial objective: " +
                                      (objVal ? std::to_string(*objVal) : "null"));
                }

                doBranchAndBound(this->newTableaus, enablePruning, pivotCols, pivotRows);
            }
            catch (const std::exception &e)
            {
                if (isConsoleOutput)
                {
                    Logger::writeLine("Error in dual simplex: " + std::string(e.what()));
                }
                throw;
            }
        }
        catch (const std::exception &e)
        {
            Logger::writeLine("math error: " + std::string(e.what()));
            throw;
        }
    }

    std::string getJSON() const
    {
        return this->jsonOut;
    }

    std::string getSolutionStr() const
    {
        return this->solution;
    }

private:
    std::vector<std::vector<double>> deepCopy(const std::vector<std::vector<double>> &original)
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

    std::string jsonOut = "{}";
    std::string solution;
};
