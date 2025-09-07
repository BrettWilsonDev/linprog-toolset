#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <limits>
#include <stdexcept>

class GoalPreemptiveSimplex
{
public:
    // Constructor
    GoalPreemptiveSimplex(bool isConsoleOutput = false)
        : isConsoleOutput(isConsoleOutput)
    {
        testInputSelected = 0;

        GuiHeaderRow.clear();
        GuiPivotCols.clear();
        GuiPivotRows.clear();

        // goal constraints
        amtOfObjVars = 2;

        // goal constraints
        goalConstraints = {{0.0, 0.0, 0.0, 0.0}};
        signItems = {"<=", ">=", "="};
        signItemsChoices = {0};

        // goal constraints
        constraints.clear();
        signItemsChoicesC = {0};

        tableaus.clear();

        goals = {"goal 1"};
        goalOrder = {0};

        pivotCol = -1;
        pivotRow = -1;
        tCol = -1;
        tRow = -1;

        goalMetStrings.clear();

        opTable = -1;

        penalties = {0.0};
        penaltiesTotals.clear();
    }

    // Test input function
    struct TestInputResult
    {
        std::vector<std::vector<double>> goals;
        std::vector<std::vector<double>> constraints;
        std::vector<int> orderOverride;
    };

    TestInputResult testInput(int testNum = -1)
    {
        std::vector<std::vector<double>> goals;
        std::vector<std::vector<double>> constraints;
        std::vector<int> orderOverride;

        if (testNum == 0)
        {
            goals = {
                {40, 30, 20, 100, 0},
                {2, 4, 3, 10, 2},
                {5, 8, 4, 30, 1},
            };

            constraints = {
                // {7, 2, 3, 69, 1}
            };

            // orderOverride = {0, 1, 2};
            orderOverride = {2, 1, 0}; // {3, 2, 1};
            // orderOverride = {};
        }

        if (testNum == -1)
        {
            return {{}, {}, {}};
        }
        else
        {
            return {goals, constraints, orderOverride};
        }
    }

    // Build first penalties variable tableau
    struct TableauResult
    {
        std::vector<std::vector<double>> oldTab;
        std::vector<std::vector<double>> newTab;
        int conStart;
    };

    TableauResult buildFirstPreemptiveTableau(
        const std::vector<std::vector<double>> &goalConstraints,
        const std::vector<std::vector<double>> &constraints,
        const std::vector<int> &orderOverride = {})
    {
        std::vector<std::vector<double>> oldTab;
        std::vector<std::vector<double>> newTab;
        int conStart = 0;

        // height = (goals + goalConstraints) = goals * 2 + constraints
        int tabSizeH = (goalConstraints.size() * 2) + constraints.size();

        // width = z + rhs + x + (g+,-) * 2 + slack
        int tabSizeW = 1 + 1 + (goalConstraints.back().size() - 2) +
                       (goalConstraints.size() * 2) + constraints.size();

        int amtOfObjVars = goalConstraints.back().size() - 2;
        int amtOfGoals = goalConstraints.size();

        // build Gui header row
        GuiHeaderRow.clear();
        GuiHeaderRow.push_back("z");

        for (int i = 0; i < amtOfObjVars; i++)
        {
            GuiHeaderRow.push_back("x" + std::to_string(i + 1));
        }

        for (int i = 0; i < amtOfGoals; i++)
        {
            GuiHeaderRow.push_back("g" + std::to_string(i + 1) + "-");
            GuiHeaderRow.push_back("g" + std::to_string(i + 1) + "+");
        }

        for (int i = 0; i < static_cast<int>(constraints.size()); i++)
        {
            GuiHeaderRow.push_back("c" + std::to_string(i + 1));
        }

        GuiHeaderRow.push_back("Rhs");

        // fix the equalities constraints sizes
        for (const auto &goal : goalConstraints)
        {
            if (goal.back() == 2)
            {
                tabSizeH += 1;
                amtOfGoals += 1;
            }
        }

        // initialize table with zeros
        oldTab.resize(tabSizeH, std::vector<double>(tabSizeW, 0.0));

        // put in lhs in the z col
        for (int i = 0; i < amtOfGoals; i++)
        {
            oldTab[i][0] = 1;
        }

        // put in neg 1s in their spots
        int gCtr = amtOfObjVars + 1;
        int ExtraCtr = 0;
        for (int i = 0; i < static_cast<int>(goalConstraints.size()); i++)
        {
            if (goalConstraints[i].back() == 0)
            {
                oldTab[i + ExtraCtr][gCtr + 1] = -1;
            }
            else if (goalConstraints[i].back() == 1)
            {
                oldTab[i + ExtraCtr][gCtr] = -1;
            }
            else if (goalConstraints[i].back() == 2)
            {
                oldTab[i + ExtraCtr][gCtr] = -1;
                oldTab[i + 1 + ExtraCtr][gCtr + 1] = -1;
                ExtraCtr += 1;
            }
            gCtr += 2;
        }

        // put in the slacks
        for (int i = 0; i < static_cast<int>(constraints.size()); i++)
        {
            if (constraints[i].back() == 0)
            {
                oldTab[tabSizeH - i - 1][oldTab[0].size() - i - 2] = 1;
            }
            else if (constraints[i].back() == 1)
            {
                oldTab[tabSizeH - i - 1][oldTab[0].size() - i - 2] = -1;
            }
        }

        // put the 1 -1 for goal constraints in
        int onesCtr = amtOfObjVars + 1;
        for (int i = amtOfGoals; i < tabSizeH - static_cast<int>(constraints.size()); i++)
        {
            oldTab[i][onesCtr] = 1;
            oldTab[i][onesCtr + 1] = -1;
            onesCtr += 2;
        }

        // put in goal constraints
        for (int i = 0; i < static_cast<int>(goalConstraints.size()); i++)
        {
            for (int j = 0; j < amtOfObjVars; j++)
            {
                oldTab[i + amtOfGoals][j + 1] = goalConstraints[i][j]; // lhs
            }
            oldTab[i + amtOfGoals][oldTab[0].size() - 1] =
                goalConstraints[i][goalConstraints[i].size() - 2]; // rhs
        }

        // put in constraints
        for (int i = 0; i < static_cast<int>(constraints.size()); i++)
        {
            for (int j = 0; j < amtOfObjVars; j++)
            {
                oldTab[i + amtOfGoals + goalConstraints.size()][j + 1] = constraints[i][j]; // lhs
            }
            oldTab[i + amtOfGoals + goalConstraints.size()][oldTab[0].size() - 1] =
                constraints[i][constraints[i].size() - 2]; // rhs
        }

        // first tab done move on to second tab
        newTab = oldTab;

        // rows at the top of the old table are the goals
        std::vector<std::vector<double>> topRows;
        for (int i = 0; i < amtOfGoals; i++)
        {
            topRows.push_back(oldTab[i]);
        }

        // rows at the bottom of the old table are the goal constraints
        std::vector<std::vector<double>> bottomRows;
        for (int i = 0; i < static_cast<int>(goalConstraints.size()); i++)
        {
            bottomRows.push_back(oldTab[i + amtOfGoals]);
        }

        // fix the equalities issues by duplicating the goal constraint rows
        std::vector<std::vector<double>> tempBottomRows;
        for (int i = 0; i < static_cast<int>(goalConstraints.size()); i++)
        {
            if (goalConstraints[i].back() == 2)
            {
                tempBottomRows.push_back(bottomRows[i]);
                tempBottomRows.push_back(bottomRows[i]);
            }
            else
            {
                tempBottomRows.push_back(bottomRows[i]);
            }
        }
        bottomRows = tempBottomRows;

        auto backUpGoals = goalConstraints;

        // fix the equalities issues by duplicating the goal constraint rows
        std::vector<std::vector<double>> tempGoals;
        for (const auto &goal : goalConstraints)
        {
            if (goal.back() == 2)
            {
                auto listCopy1 = goal;
                auto listCopy2 = goal;

                listCopy1[listCopy1.size() - 1] = 0;
                listCopy2[listCopy2.size() - 1] = 1;

                tempGoals.push_back(listCopy2);
                tempGoals.push_back(listCopy1);
            }
            else
            {
                tempGoals.push_back(goal);
            }
        }

        auto modifiedGoalConstraints = tempGoals;

        // calculate the new table goal rows (NO penalties in preemptive GP)
        for (int i = 0; i < static_cast<int>(modifiedGoalConstraints.size()); i++)
        {
            for (int j = 0; j < static_cast<int>(newTab[i].size()); j++)
            {
                if (modifiedGoalConstraints[i].back() == 0)
                {
                    newTab[i][j] = -1 * (topRows[i][j] - bottomRows[i][j]);
                }
                else if (modifiedGoalConstraints[i].back() == 1)
                {
                    newTab[i][j] = topRows[i][j] + bottomRows[i][j];
                }
            }
        }

        // fix the order of the equalities
        int equalCtr = 0;
        for (int i = 0; i < static_cast<int>(backUpGoals.size()); i++)
        {
            if (backUpGoals[i].back() == 2)
            {
                auto tempRow = newTab[i + equalCtr];
                newTab[i + equalCtr] = newTab[i + 1 + equalCtr];
                newTab[i + 1 + equalCtr] = tempRow;
                equalCtr += 1;
            }
        }

        // reorder the goals according to user input
        if (!orderOverride.empty())
        {
            std::vector<std::vector<double>> tempNewTab;
            for (int i = 0; i < static_cast<int>(orderOverride.size()); i++)
            {
                tempNewTab.push_back(newTab[orderOverride[i]]);
            }

            for (int i = 0; i < static_cast<int>(tempNewTab.size()); i++)
            {
                newTab[i] = tempNewTab[i];
            }
        }

        conStart = amtOfGoals;

        TableauResult result;
        result.oldTab = oldTab;
        result.newTab = newTab;
        result.conStart = conStart;

        return result;
    }

    // Pivot operations
    struct PivotResult
    {
        std::vector<std::vector<double>> newTab;
        std::vector<double> zRhs;
    };

    PivotResult doPivotOperations(const std::vector<std::vector<double>> &tab,
                                  int conStartRow, int zRow, int tabNum = 1)
    {
        auto oldTab = tab;
        const int R = static_cast<int>(tab.size());
        const int C = static_cast<int>(tab[0].size());

        std::vector<std::vector<double>> newTab(R, std::vector<double>(C, 0.0));

        // exclude the z col and rhs
        std::vector<double> currentZ = tab[zRow];
        currentZ[0] = 0.0;
        currentZ.pop_back(); // drop RHS

        // Entering variable (first index of max)
        double largestZ = *std::max_element(currentZ.begin(), currentZ.end());
        int pivotCol = -1;
        for (int j = 0; j < static_cast<int>(currentZ.size()); ++j)
        {
            if (std::abs(currentZ[j] - largestZ) < 1e-12)
            {
                pivotCol = j;
                break;
            }
        }

        if (pivotCol == -1)
        {
            throw std::runtime_error("No entering variable found.");
        }

        // Ratios
        bool useZero = false;
        std::vector<double> ratios(R, std::numeric_limits<double>::infinity());
        for (int i = conStartRow; i < R; ++i)
        {
            double div = tab[i][pivotCol];
            if (std::abs(div) < 1e-10)
            {
                ratios[i] = std::numeric_limits<double>::infinity();
            }
            else
            {
                double r = tab[i][C - 1] / div;
                ratios[i] = r;
                if (r == 0.0 && div > 0.0)
                {
                    useZero = true;
                }
            }
        }

        // Positive ratios
        std::vector<double> positiveRatios;
        for (int i = conStartRow; i < R; ++i)
        {
            double r = ratios[i];
            if ((useZero ? (r >= 0.0) : (r > 0.0)) && std::isfinite(r))
            {
                positiveRatios.push_back(r);
            }
        }

        if (positiveRatios.empty())
        {
            throw std::runtime_error("Unbounded problem: no valid leaving row.");
        }

        // Min ratio
        double minVal = *std::min_element(positiveRatios.begin(), positiveRatios.end());

        // Leaving row (first index)
        int pivotRow = -1;
        for (int i = conStartRow; i < R; ++i)
        {
            if (std::abs(ratios[i] - minVal) < 1e-12)
            {
                pivotRow = i;
                break;
            }
        }

        if (pivotRow == -1)
        {
            throw std::runtime_error("No leaving row found.");
        }

        // Normalize pivot row
        double divNumber = tab[pivotRow][pivotCol];
        for (int j = 0; j < C; ++j)
        {
            double v = tab[pivotRow][j] / divNumber;
            newTab[pivotRow][j] = (v == -0.0 ? 0.0 : v);
        }

        // Pivot operations
        for (int i = 0; i < R; ++i)
        {
            if (i == pivotRow)
                continue;
            double mult = oldTab[i][pivotCol];
            for (int j = 0; j < C; ++j)
            {
                newTab[i][j] = oldTab[i][j] - mult * newTab[pivotRow][j];
            }
        }

        // Clean tiny values
        for (auto &row : newTab)
        {
            for (double &v : row)
            {
                if (std::abs(v) < 1e-10)
                    v = 0.0;
                if (v == 0.0 && std::signbit(v))
                    v = 0.0;
            }
        }

        // z RHS
        std::vector<double> zRhs;
        for (int i = 0; i < conStartRow; ++i)
        {
            zRhs.push_back(newTab[i][C - 1]);
        }

        GuiPivotRows.push_back(pivotRow);
        GuiPivotCols.push_back(pivotCol);

        PivotResult result;
        result.newTab = newTab;
        result.zRhs = zRhs;
        return result;
    }

    // Main penalties function
    struct PreemptiveResult
    {
        std::vector<std::vector<std::vector<double>>> tableaus;
        std::vector<std::vector<std::string>> goalMetStrings;
        int opTable;
        std::vector<double> penaltiesTotals;
    };

    PreemptiveResult doPreemptive(
        std::vector<std::vector<double>> goals,
        std::vector<std::vector<double>> constraints,
        std::vector<int> orderOverride = {})
    {
        // Deep copy equivalents
        auto a = goals;
        auto b = constraints;
        auto originalGoals = goals;
        std::vector<std::vector<std::vector<double>>> tableaus;

        auto expandedOrder = orderOverride;

        // account for the equalities
        std::vector<std::vector<int>> goalToRows(originalGoals.size());
        int rowCtr = 0;

        for (size_t g = 0; g < originalGoals.size(); g++)
        {
            goalToRows[g].push_back(rowCtr);
            rowCtr++;

            if (originalGoals[g].back() == 2)
            {
                goalToRows[g].push_back(rowCtr);
                rowCtr++;
            }
        }

        expandedOrder.clear();

        if (!orderOverride.empty())
        {
            for (size_t pos = 0; pos < orderOverride.size(); pos++)
            {
                int g = orderOverride[pos];
                expandedOrder.push_back(goalToRows[g][0]);

                if (originalGoals[g].back() == 2)
                {
                    expandedOrder.push_back(goalToRows[g][1]);
                }
            }
        }
        else
        {
            for (int g = 0; g < static_cast<int>(originalGoals.size()); g++)
            {
                expandedOrder.push_back(goalToRows[g][0]);

                if (originalGoals[g].back() == 2)
                {
                    expandedOrder.push_back(goalToRows[g][1]);
                }
            }
        }

        // Build first preemptive tableau (assuming this method exists)
        auto [firstTab, FormulatedTab, conStartRow] = buildFirstPreemptiveTableau(a, b, expandedOrder);
        tableaus.push_back(firstTab);
        tableaus.push_back(FormulatedTab);

        std::vector<std::vector<double>> tempGoalLst;
        for (const auto &goal : goals)
        {
            tempGoalLst.push_back(goal);
            if (goal.back() == 2)
            {
                tempGoalLst.push_back(goal);
            }
        }
        goals = tempGoalLst;

        std::vector<double> zRhs;
        int currentZRow = 0;

        for (int i = 0; i < conStartRow; i++)
        {
            zRhs.push_back(tableaus.back()[i].back());
        }

        int lenObj = goals.back().size() - 2;

        // Sign list to compare to and initial met goal state
        std::vector<bool> metGoals;
        std::vector<int> signLst;
        for (int i = 0; i < goals.size(); i++)
        {
            signLst.push_back(-1);
            signLst.push_back(1);
            metGoals.push_back(false);
        }

        metGoals[0] = true;

        int highestTrueIndex = -1;
        int currentTrueIndex = -1;
        bool isLoopRunning = true;
        std::vector<std::vector<std::string>> goalMetStrings;

        int ctr = 0;
        while (ctr != 100 && isLoopRunning)
        {
            std::vector<std::vector<double> *> basicVarLst;

            for (int k = lenObj + 1; k < (tableaus.back().back().size() - 1) - constraints.size(); k++)
            {
                int columnIndex = k;
                std::vector<double> *tempLst = new std::vector<double>();

                for (int i = conStartRow; i < tableaus.back().size(); i++)
                {
                    double columnValue = tableaus.back()[i][columnIndex];
                    tempLst->push_back(columnValue);
                }

                double sum = 0;
                for (double val : *tempLst)
                {
                    sum += val;
                }

                if (sum == 1)
                {
                    basicVarLst.push_back(tempLst);
                }
                else
                {
                    basicVarLst.push_back(nullptr);
                    delete tempLst;
                }
            }

            std::vector<double *> goalRhs(goals.size(), nullptr);

            // Get the rhs from basic cols
            for (int i = 0; i < basicVarLst.size(); i++)
            {
                if (basicVarLst[i] != nullptr)
                {
                    int sign = signLst[i];
                    for (int j = 0; j < basicVarLst[i]->size(); j++)
                    {
                        if ((*basicVarLst[i])[j] == 1.0)
                        {
                            if (sign == 1)
                            {
                                goalRhs[j] = new double(tableaus.back()[j + conStartRow].back());
                            }
                            else if (sign == -1)
                            {
                                goalRhs[j] = new double(-tableaus.back()[j + conStartRow].back());
                            }
                        }
                    }
                }
            }

            std::vector<std::vector<double> *> sortedLst;

            // Sort according to where goals are
            for (int k = 0; k < basicVarLst.size(); k += 2)
            {
                if (k + 1 < basicVarLst.size())
                {
                    if (basicVarLst[k] == nullptr && basicVarLst[k + 1] == nullptr)
                    {
                        sortedLst.push_back(nullptr);
                    }
                    else
                    {
                        if (basicVarLst[k] != nullptr)
                        {
                            sortedLst.push_back(basicVarLst[k]);
                        }
                        if (basicVarLst[k + 1] != nullptr)
                        {
                            sortedLst.push_back(basicVarLst[k + 1]);
                        }
                    }
                }
            }

            std::vector<double *> tempLst(goalRhs.size(), nullptr);

            for (int i = 0; i < sortedLst.size(); i++)
            {
                if (sortedLst[i] != nullptr)
                {
                    for (int j = 0; j < sortedLst[i]->size(); j++)
                    {
                        if ((*sortedLst[i])[j] == 1)
                        {
                            tempLst[i] = goalRhs[j];
                        }
                    }
                }
            }

            goalRhs = tempLst;

            for (int i = 0; i < goalRhs.size(); i++)
            {
                if (goalRhs[i] != nullptr && *goalRhs[i] == -0.0)
                {
                    *goalRhs[i] = 0.0;
                }
            }

            std::vector<int> EqualitySigns;
            for (int i = 0; i < originalGoals.size(); i++)
            {
                if (originalGoals[i].back() == 2)
                {
                    if (goalRhs[i] != nullptr)
                    {
                        if (*goalRhs[i] > 0)
                        {
                            EqualitySigns.push_back(i);
                            EqualitySigns.push_back(-(i + 1));
                        }
                        else
                        {
                            EqualitySigns.push_back(-i);
                            EqualitySigns.push_back(i + 1);
                        }
                        goalRhs.insert(goalRhs.begin() + i, new double(std::abs(*goalRhs[i])));
                        *goalRhs[i + 1] = -std::abs(*goalRhs[i + 1]);
                    }
                    else
                    {
                        goalRhs.insert(goalRhs.begin() + i, goalRhs[i]);
                    }
                    goalRhs.pop_back();
                }
            }

            std::vector<std::string> goalMetString;

            // Check if goal is met based on constraints conditions
            for (int i = 0; i < goalRhs.size(); i++)
            {
                if (goalRhs[i] == nullptr)
                {
                    metGoals[i] = true;
                    if (goals[i].back() != 2)
                    {
                        goalMetString.push_back(std::to_string(i + 1) + " met: exactly");
                    }
                    continue;
                }

                // Check if goal is met based on constraints conditions
                if (goals[i].back() == 0)
                {
                    if ((*goalRhs[i] + goals[i][goals[i].size() - 2]) <= goals[i][goals[i].size() - 2])
                    {
                        if (*goalRhs[i] > 0)
                        {
                            goalMetString.push_back(std::to_string(i + 1) + " met: over by " + std::to_string(std::abs(*goalRhs[i])));
                        }
                        else
                        {
                            goalMetString.push_back(std::to_string(i + 1) + " met: under by " + std::to_string(std::abs(*goalRhs[i])));
                        }
                        metGoals[i] = true;
                    }
                    else
                    {
                        if (*goalRhs[i] > 0)
                        {
                            goalMetString.push_back(std::to_string(i + 1) + " not met: over by " + std::to_string(std::abs(*goalRhs[i])));
                        }
                        else
                        {
                            goalMetString.push_back(std::to_string(i + 1) + " not met: under by " + std::to_string(std::abs(*goalRhs[i])));
                        }
                        metGoals[i] = false;
                    }
                }
                else if (goals[i].back() == 1)
                {
                    if ((*goalRhs[i] + goals[i][goals[i].size() - 2]) >= goals[i][goals[i].size() - 2])
                    {
                        if (*goalRhs[i] > 0)
                        {
                            goalMetString.push_back(std::to_string(i + 1) + " met: over by " + std::to_string(std::abs(*goalRhs[i])));
                        }
                        else
                        {
                            goalMetString.push_back(std::to_string(i + 1) + " met: under by " + std::to_string(std::abs(*goalRhs[i])));
                        }
                        metGoals[i] = true;
                    }
                    else
                    {
                        if (*goalRhs[i] > 0)
                        {
                            goalMetString.push_back(std::to_string(i + 1) + " not met: over by " + std::to_string(std::abs(*goalRhs[i])));
                        }
                        else
                        {
                            goalMetString.push_back(std::to_string(i + 1) + " not met: under by " + std::to_string(std::abs(*goalRhs[i])));
                        }
                        metGoals[i] = false;
                    }
                }
                else if (goals[i].back() == 2)
                {
                    if (*goalRhs[i] == goals[i][goals[i].size() - 2])
                    {
                        metGoals[i] = true;
                    }
                    else
                    {
                        metGoals[i] = false;
                    }
                }
            }

            auto zRhsBackUp = zRhs;
            if (!expandedOrder.empty())
            {
                std::vector<double> tempZRhs;
                for (int i = 0; i < zRhs.size(); i++)
                {
                    tempZRhs.push_back(zRhs[expandedOrder[i]]);
                }
                zRhs = tempZRhs;
            }

            // 0 in top rhs means goal met regardless of bottom rhs
            for (int i = 0; i < zRhs.size(); i++)
            {
                if (zRhs[i] == 0.0)
                {
                    metGoals[i] = true;
                }
            }

            zRhs = zRhsBackUp;

            if (!expandedOrder.empty())
            {
                std::vector<bool> tempMet;
                for (int i = 0; i < metGoals.size(); i++)
                {
                    tempMet.push_back(metGoals[expandedOrder[i]]);
                }
                metGoals = tempMet;
            }

            // For the equality if z- is the basic var then z+ is false vice versa
            if (!EqualitySigns.empty())
            {
                for (int i = 0; i < EqualitySigns.size(); i++)
                {
                    if (EqualitySigns[i] < 0)
                    {
                        metGoals[std::abs(EqualitySigns[i])] = true;
                    }
                    else
                    {
                        metGoals[EqualitySigns[i]] = false;
                    }
                }
            }

            // Swap to the row of the current goal being worked on
            for (int i = 0; i < metGoals.size(); i++)
            {
                if (metGoals[i] == false)
                {
                    currentZRow = i;
                    break;
                }
            }

            auto tempMetGoals = metGoals;
            for (int i = 0; i < originalGoals.size(); i++)
            {
                if (originalGoals[i].back() == 2)
                {
                    if (!((metGoals[i]) && (metGoals[i + 1])))
                    {
                        if (!metGoals[i])
                        {
                            if (goalRhs[i] != nullptr)
                            {
                                if (*goalRhs[i] > 0)
                                {
                                    goalMetString.push_back(std::to_string(i + 1) + " not met: over by " + std::to_string(std::abs(*goalRhs[i])));
                                }
                                else
                                {
                                    goalMetString.push_back(std::to_string(i + 1) + " not met: under by " + std::to_string(std::abs(*goalRhs[i])));
                                }
                            }
                        }
                        else if (!metGoals[i + 1])
                        {
                            if (goalRhs[i + 1] != nullptr)
                            {
                                if (*goalRhs[i + 1] > 0)
                                {
                                    goalMetString.push_back(std::to_string(i + 1) + " not met: over by " + std::to_string(std::abs(*goalRhs[i + 1])));
                                }
                                else
                                {
                                    goalMetString.push_back(std::to_string(i + 1) + " not met: under by " + std::to_string(std::abs(*goalRhs[i + 1])));
                                }
                            }
                        }
                        metGoals[i] = false;
                        metGoals[i + 1] = false;
                    }
                    else
                    {
                        goalMetString.push_back(std::to_string(i + 1) + " met: exactly");
                        metGoals[i] = true;
                        metGoals[i + 1] = true;
                    }
                }
            }

            goalMetStrings.push_back(goalMetString);

            for (int i = 0; i < metGoals.size(); i++)
            {
                if (!metGoals[i])
                {
                    currentTrueIndex = i;
                    break;
                }
            }

            if (currentTrueIndex > highestTrueIndex)
            {
                highestTrueIndex = currentTrueIndex;
            }
            else
            {
                if (highestTrueIndex > currentTrueIndex)
                {
                    break;
                }
            }

            if (std::all_of(metGoals.begin(), metGoals.end(), [](bool v)
                            { return v; }))
            {
                tableaus.push_back(tableaus.back());
                isLoopRunning = false;
            }

            metGoals = tempMetGoals;

            try
            {
                auto [newTab, newZRhs] = doPivotOperations(tableaus.back(), conStartRow, currentZRow, 1);
                tableaus.push_back(newTab);
                zRhs = newZRhs;
            }
            catch (const std::exception &e)
            {
                goalMetStrings.push_back({"0"});
                break;
            }

            // Clean up dynamically allocated memory
            for (auto ptr : basicVarLst)
            {
                delete ptr;
            }
            for (auto ptr : goalRhs)
            {
                delete ptr;
            }

            ctr++;
        }

        // Sort goal met strings
        std::vector<std::vector<std::string>> sortedGoalMetStrings;
        for (const auto &sublist : goalMetStrings)
        {
            auto sorted = sublist;
            std::sort(sorted.begin(), sorted.end(), [](const std::string &a, const std::string &b)
                      {
                int numA = std::stoi(a.substr(0, a.find(' ')));
                int numB = std::stoi(b.substr(0, b.find(' ')));
                return numA < numB; });
            sortedGoalMetStrings.push_back(sorted);
        }

        for (int i = 0; i < sortedGoalMetStrings.size(); i++)
        {
            for (int j = 0; j < sortedGoalMetStrings[i].size(); j++)
            {
                std::istringstream iss(sortedGoalMetStrings[i][j]);
                std::string word;
                std::vector<std::string> words;
                while (iss >> word)
                {
                    words.push_back(word);
                }
                if (words.size() > 1)
                {
                    std::string result;
                    for (int k = 1; k < words.size(); k++)
                    {
                        if (k > 1)
                            result += " ";
                        result += words[k];
                    }
                    sortedGoalMetStrings[i][j] = result;
                }
            }
        }

        goalMetStrings = sortedGoalMetStrings;
        goalMetStrings.insert(goalMetStrings.begin(), {" "});

        if (isConsoleOutput)
        {
            for (int i = 0; i < tableaus.size(); i++)
            {
                try
                {
                    for (int l = 0; l < goalMetStrings[i].size(); l++)
                    {
                        std::cout << "Goal " << (l + 1) << " " << goalMetStrings[i][l] << std::endl;
                    }
                }
                catch (const std::exception &e)
                {
                    // Handle exception silently like Python pass
                }

                std::cout << "Tableau " << (i + 1) << std::endl;
                for (int j = 0; j < tableaus[i].size(); j++)
                {
                    for (int k = 0; k < tableaus[i][j].size(); k++)
                    {
                        std::cout << std::setw(10) << std::fixed << std::setprecision(3)
                                  << tableaus[i][j][k] << " ";
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
            }

            std::cout << "\noptimal Tableau:\n"
                      << std::endl;
        }

        int opTable = std::distance(tableaus.begin(),
                                    std::find(tableaus.begin(), tableaus.end(), tableaus[tableaus.size() - 2]));

        if (isConsoleOutput)
        {
            for (int l = 0; l < goalMetStrings[opTable].size(); l++)
            {
                std::cout << "Goal " << (l + 1) << " " << goalMetStrings[opTable][l] << std::endl;
            }
            std::cout << "Tableau " << (opTable + 1) << std::endl;
            for (int j = 0; j < tableaus[opTable].size(); j++)
            {
                for (int k = 0; k < tableaus[opTable][j].size(); k++)
                {
                    std::cout << std::setw(10) << std::fixed << std::setprecision(3)
                              << tableaus[opTable][j][k] << " ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }

        return {tableaus, goalMetStrings, opTable};
    }

    // Getters for accessing private members
    const std::vector<std::string> &getGuiHeaderRow() const { return GuiHeaderRow; }
    const std::vector<int> &getGuiPivotCols() const { return GuiPivotCols; }
    const std::vector<int> &getGuiPivotRows() const { return GuiPivotRows; }
    int getOpTable() const { return opTable; }
    const std::vector<double> &getPenaltiesTotals() const { return penaltiesTotals; }

private:
    bool isConsoleOutput;

    int testInputSelected;

    std::vector<std::string> GuiHeaderRow;
    std::vector<int> GuiPivotCols;
    std::vector<int> GuiPivotRows;

    // goal constraints
    int amtOfObjVars;

    // goal constraints
    std::vector<std::vector<double>> goalConstraints;
    std::vector<std::string> signItems;
    std::vector<int> signItemsChoices;

    // goal constraints
    std::vector<std::vector<double>> constraints;
    std::vector<int> signItemsChoicesC;

    std::vector<std::vector<std::vector<double>>> tableaus;

    std::vector<std::string> goals;
    std::vector<int> goalOrder;

    int pivotCol;
    int pivotRow;
    int tCol;
    int tRow;

    std::vector<std::vector<std::string>> goalMetStrings;

    int opTable;

    std::vector<double> penalties;
    std::vector<double> penaltiesTotals;
};