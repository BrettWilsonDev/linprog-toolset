#ifndef BRANCH_AND_BOUND_KNAPSACK_HPP
#define BRANCH_AND_BOUND_KNAPSACK_HPP

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <iostream>
#include <iomanip>

class KnapsackItem
{
public:
    int index;
    int value;
    int weight;
    double ratio;
    std::string name;

    KnapsackItem(int idx, int val, int wgt) : index(idx), value(val), weight(wgt)
    {
        ratio = static_cast<double>(value) / weight;
        name = "x" + std::to_string(index + 1);
    }
};

class KnapsackNode
{
public:
    int level;
    double profit;
    int weight;
    double bound;
    std::map<int, int> fixedVariables;
    KnapsackNode *parent;
    std::vector<KnapsackNode *> children;

    KnapsackNode(int lvl, double prf, int wgt, double bnd,
                 const std::map<int, int> &fixedVars = std::map<int, int>(),
                 KnapsackNode *prnt = nullptr)
        : level(lvl), profit(prf), weight(wgt), bound(bnd), fixedVariables(fixedVars), parent(prnt) {}

    ~KnapsackNode()
    {
        for (auto child : children)
        {
            delete child;
        }
    }
};

class BranchAndBoundKnapsack
{
private:
    std::vector<int> values;
    std::vector<int> weights;
    int capacity;
    std::vector<KnapsackItem> items;
    double bestValue;
    std::vector<int> bestSolution;
    std::vector<int> branchingOrder;

public:
    BranchAndBoundKnapsack(const std::vector<int> &vals, const std::vector<int> &wgts, int cap)
        : values(vals), weights(wgts), capacity(cap), bestValue(0)
    {
        for (size_t i = 0; i < vals.size(); ++i)
        {
            items.emplace_back(i, vals[i], wgts[i]);
        }
        bestSolution.resize(vals.size(), 0);

        std::vector<size_t> indices(vals.size());
        for (size_t i = 0; i < vals.size(); ++i)
            indices[i] = i;
        std::sort(indices.begin(), indices.end(),
                  [&](size_t a, size_t b)
                  { return static_cast<double>(vals[a]) / wgts[a] > static_cast<double>(vals[b]) / wgts[b]; });
        branchingOrder = std::vector<int>(indices.begin(), indices.end());
    }

    std::vector<KnapsackItem> DisplayRatioTest()
    {
        Logger::WriteLine("Branch & Bound Algorithm - Knapsack Method");
        Logger::WriteLine("Ratio Test");
        Logger::WriteLine("Item   z_i/c_i   Rank");

        auto sortedItems = items;
        std::sort(sortedItems.begin(), sortedItems.end(),
                  [](const KnapsackItem &a, const KnapsackItem &b)
                  { return a.ratio > b.ratio; });

        std::map<int, int> rankMap;
        for (size_t rank = 0; rank < sortedItems.size(); ++rank)
        {
            rankMap[sortedItems[rank].index] = rank + 1;
        }

        for (const auto &item : items)
        {
            Logger::WriteLine(item.name + "    " + std::to_string(item.value) + "/" +
                              std::to_string(item.weight) + " = " +
                              std::to_string(item.ratio).substr(0, std::to_string(item.ratio).find(".") + 4) +
                              "    " + std::to_string(rankMap[item.index]));
        }
        Logger::WriteLine("");
        return sortedItems;
    }

    double CalculateUpperBound(const KnapsackNode &node, const std::vector<KnapsackItem> &sortedItems)
    {
        int remainingCapacity = capacity - node.weight;
        double upperBound = node.profit;

        for (const auto &item : sortedItems)
        {
            if (node.fixedVariables.find(item.index) != node.fixedVariables.end())
                continue;

            if (remainingCapacity >= item.weight)
            {
                remainingCapacity -= item.weight;
                upperBound += item.value;
            }
            else
            {
                if (remainingCapacity > 0)
                {
                    upperBound += (static_cast<double>(remainingCapacity) / item.weight) * item.value;
                }
                break;
            }
        }
        return upperBound;
    }

    double DisplaySubProblem(const KnapsackNode &node, const std::vector<KnapsackItem> &sortedItems,
                             const std::string &subProblemNumber, bool isRoot = false)
    {
        Logger::WriteLine(subProblemNumber.empty() ? "Sub-Problem" : "Sub-Problem " + subProblemNumber);

        int remainingCapacity = capacity;
        double totalValue = 0;
        std::vector<int> sortedFixedKeys;
        for (const auto &kv : node.fixedVariables)
            sortedFixedKeys.push_back(kv.first);
        std::sort(sortedFixedKeys.begin(), sortedFixedKeys.end());

        std::vector<int> fixedItems;
        for (int itemIndex : sortedFixedKeys)
        {
            int value = node.fixedVariables.at(itemIndex);
            const auto &item = items[itemIndex];

            if (value == 1)
            {
                Logger::WriteLine("* " + item.name + " = " + std::to_string(value) + "    " +
                                  std::to_string(remainingCapacity) + "-" + std::to_string(item.weight) +
                                  "=" + std::to_string(remainingCapacity - item.weight));
                remainingCapacity -= item.weight;
                totalValue += item.value;
                fixedItems.push_back(itemIndex);
            }
            else if (value == 0)
            {
                Logger::WriteLine("* " + item.name + " = " + std::to_string(value) + "    " +
                                  std::to_string(remainingCapacity) + "-0=" + std::to_string(remainingCapacity));
                fixedItems.push_back(itemIndex);
            }
        }

        for (const auto &item : sortedItems)
        {
            if (std::find(fixedItems.begin(), fixedItems.end(), item.index) != fixedItems.end())
                continue;

            if (remainingCapacity >= item.weight)
            {
                Logger::WriteLine(item.name + " = 1    " + std::to_string(remainingCapacity) + "-" +
                                  std::to_string(item.weight) + "=" + std::to_string(remainingCapacity - item.weight));
                remainingCapacity -= item.weight;
                totalValue += item.value;
            }
            else if (remainingCapacity > 0)
            {
                std::string fractionalDisplay = std::to_string(remainingCapacity) + "/" + std::to_string(item.weight);
                Logger::WriteLine(item.name + " = " + fractionalDisplay + "    " +
                                  std::to_string(remainingCapacity) + "-" + std::to_string(item.weight));
                totalValue += (static_cast<double>(remainingCapacity) / item.weight) * item.value;
                remainingCapacity = 0;
            }
            else
            {
                Logger::WriteLine(item.name + " = 0");
            }
        }
        Logger::WriteLine("");
        return totalValue;
    }

    void DisplayIntegerModel()
    {
        Logger::WriteLine("Integer Programming Model");
        std::string valueStr;
        for (size_t i = 0; i < values.size(); ++i)
        {
            valueStr += std::to_string(values[i]) + "x" + std::to_string(i + 1);
            if (i < values.size() - 1)
                valueStr += " + ";
        }
        Logger::WriteLine("max z = " + valueStr);

        std::string weightStr;
        for (size_t i = 0; i < weights.size(); ++i)
        {
            weightStr += std::to_string(weights[i]) + "x" + std::to_string(i + 1);
            if (i < weights.size() - 1)
                weightStr += " + ";
        }
        Logger::WriteLine("s.t " + weightStr + " ≤ " + std::to_string(capacity));
        Logger::WriteLine("xi = 0 or 1");
        Logger::WriteLine("");
    }

    bool IsFeasible(const KnapsackNode &node)
    {
        return node.weight <= capacity;
    }

    bool IsComplete(const KnapsackNode &node)
    {
        return node.fixedVariables.size() == items.size();
    }

    int GetNextVariableToBranch(const KnapsackNode &node, const std::vector<KnapsackItem> &sortedItems)
    {
        int remainingCapacity = capacity - node.weight;

        for (const auto &item : sortedItems)
        {
            if (node.fixedVariables.find(item.index) != node.fixedVariables.end())
                continue;

            if (remainingCapacity >= item.weight)
            {
                remainingCapacity -= item.weight;
            }
            else if (remainingCapacity > 0)
            {
                return item.index;
            }
            else
            {
                break;
            }
        }

        for (int varIndex : branchingOrder)
        {
            if (node.fixedVariables.find(varIndex) == node.fixedVariables.end())
            {
                return varIndex;
            }
        }
        return -1;
    }

    bool IsIntegerRelaxation(KnapsackNode &node, const std::vector<KnapsackItem> &sortedItems)
    {
        int remainingCapacity = capacity - node.weight;

        for (const auto &item : sortedItems)
        {
            if (node.fixedVariables.find(item.index) != node.fixedVariables.end())
                continue;

            if (remainingCapacity >= item.weight)
            {
                remainingCapacity -= item.weight;
            }
            else if (remainingCapacity > 0)
            {
                return false;
            }
        }

        std::map<int, int> newFixedVars = node.fixedVariables;
        remainingCapacity = capacity - node.weight;
        double newProfit = node.profit;

        for (const auto &item : sortedItems)
        {
            if (newFixedVars.find(item.index) != newFixedVars.end())
                continue;

            if (remainingCapacity >= item.weight)
            {
                newFixedVars[item.index] = 1;
                remainingCapacity -= item.weight;
                newProfit += item.value;
            }
            else
            {
                newFixedVars[item.index] = 0;
            }
        }

        node.fixedVariables = newFixedVars;
        node.profit = newProfit;
        return true;
    }

    // void SolveRecursive(KnapsackNode &node, const std::vector<KnapsackItem> &sortedItems,
    //                     const std::string &nodeLabel = "", std::vector<int> &candidateCounter = std::vector<int>{0})
void SolveRecursive(KnapsackNode &node, const std::vector<KnapsackItem> &sortedItems,
                    const std::string &nodeLabel = "", std::vector<int> candidateCounter = std::vector<int>{0})
{
    if (candidateCounter.empty())
    {
        candidateCounter = std::vector<int>{0};
    }


        if (!IsFeasible(node))
        {
            Logger::WriteLine("Infeasible\n");
            return;
        }

        if (IsComplete(node))
        {
            FinalizeCandidate(node, candidateCounter);
            return;
        }

        int remainingCapacity = capacity - node.weight;
        bool fractionalFound = false;

        for (const auto &item : sortedItems)
        {
            if (node.fixedVariables.find(item.index) != node.fixedVariables.end())
                continue;

            if (remainingCapacity >= item.weight)
            {
                remainingCapacity -= item.weight;
            }
            else if (remainingCapacity > 0)
            {
                fractionalFound = true;
                break;
            }
        }

        if (!fractionalFound)
        {
            std::map<int, int> newFixedVars = node.fixedVariables;
            remainingCapacity = capacity - node.weight;
            double newProfit = node.profit;

            for (const auto &item : sortedItems)
            {
                if (newFixedVars.find(item.index) != newFixedVars.end())
                    continue;

                if (remainingCapacity >= item.weight)
                {
                    newFixedVars[item.index] = 1;
                    remainingCapacity -= item.weight;
                    newProfit += item.value;
                }
                else
                {
                    newFixedVars[item.index] = 0;
                }
            }

            node.fixedVariables = newFixedVars;
            node.profit = newProfit;
            FinalizeCandidate(node, candidateCounter);
            return;
        }

        node.bound = CalculateUpperBound(node, sortedItems);
        int nextVarIndex = GetNextVariableToBranch(node, sortedItems);
        if (nextVarIndex == -1)
            return;

        std::vector<std::string> childLabels = nodeLabel.empty() ? std::vector<std::string>{"1", "2"} : std::vector<std::string>{nodeLabel + ".1", nodeLabel + ".2"};
        std::string branchDisplay = nodeLabel.empty() ? "Sub-P 1: x" + std::to_string(nextVarIndex + 1) + " = 0    Sub-P 2: x" + std::to_string(nextVarIndex + 1) + " = 1" : "Sub-P " + nodeLabel + ".1: x" + std::to_string(nextVarIndex + 1) + " = 0    Sub-P " + nodeLabel + ".2: x" + std::to_string(nextVarIndex + 1) + " = 1";

        Logger::WriteLine(branchDisplay);
        Logger::WriteLine("==========================================================");

        for (int i = 0; i < 2; ++i)
        {
            int branchValue = i;
            std::map<int, int> newFixedVars = node.fixedVariables;
            newFixedVars[nextVarIndex] = branchValue;

            int newWeight = node.weight;
            double newProfit = node.profit;

            if (branchValue == 1)
            {
                newWeight += items[nextVarIndex].weight;
                newProfit += items[nextVarIndex].value;
            }

            KnapsackNode *newNode = new KnapsackNode(node.level + 1, newProfit, newWeight, 0, newFixedVars, &node);
            std::string currentLabel = childLabels[i];
            DisplaySubProblem(*newNode, sortedItems, "Sub-P " + currentLabel);

            if (!IsFeasible(*newNode))
            {
                Logger::WriteLine("Infeasible\n");
            }
            else
            {
                if (IsIntegerRelaxation(*newNode, sortedItems))
                {
                    FinalizeCandidate(*newNode, candidateCounter);
                    delete newNode;
                    continue;
                }
                newNode->bound = CalculateUpperBound(*newNode, sortedItems);
                SolveRecursive(*newNode, sortedItems, currentLabel, candidateCounter);
            }
            node.children.push_back(newNode);
        }
    }

    void FinalizeCandidate(const KnapsackNode &node, std::vector<int> &candidateCounter)
    {
        std::vector<int> selectedItems;
        for (size_t i = 0; i < items.size(); ++i)
        {
            if (node.fixedVariables.find(i) != node.fixedVariables.end() && node.fixedVariables.at(i) == 1)
            {
                selectedItems.push_back(i);
            }
        }

        if (!selectedItems.empty())
        {
            std::string valueTerms;
            for (size_t i = 0; i < selectedItems.size(); ++i)
            {
                valueTerms += std::to_string(values[selectedItems[i]]);
                if (i < selectedItems.size() - 1)
                    valueTerms += " + ";
            }
            Logger::WriteLine("z = " + valueTerms + " = " + std::to_string(static_cast<int>(node.profit)));
        }
        else
        {
            Logger::WriteLine("z = 0");
        }

        candidateCounter[0]++;
        char candidateLetter = static_cast<char>(65 + candidateCounter[0] - 1);
        Logger::WriteLine("Candidate " + std::string(1, candidateLetter));

        if (node.profit > bestValue)
        {
            bestValue = node.profit;
            bestSolution.clear();
            bestSolution.resize(items.size(), 0);
            for (size_t i = 0; i < items.size(); ++i)
            {
                bestSolution[i] = node.fixedVariables.find(i) != node.fixedVariables.end() ? node.fixedVariables.at(i) : 0;
            }
            Logger::WriteLine("Best Candidate");
        }
        Logger::WriteLine("");
    }

    std::pair<double, std::vector<int>> Solve()
    {
        Logger::WriteLine(std::string(60, '='));
        auto sortedItems = DisplayRatioTest();
        DisplayIntegerModel();

        KnapsackNode rootNode(0, 0, 0, 0);
        rootNode.bound = CalculateUpperBound(rootNode, sortedItems);
        DisplaySubProblem(rootNode, sortedItems, "", true);

        std::vector<int> candidateCounter = {0};
        SolveRecursive(rootNode, sortedItems, "", candidateCounter);

        return {bestValue, bestSolution};
    }
};

class KnapSack
{
public:
    KnapSack(bool isConsoleOutput = false) {}

    void RunBranchAndBoundKnapSack(const std::vector<double> &objFuncPassed,
                                   const std::vector<std::vector<double>> &constraintsPassed)
    {
        std::vector<int> values(objFuncPassed.begin(), objFuncPassed.end());
        std::vector<int> weights(constraintsPassed[0].begin(), constraintsPassed[0].end() - 2);
        int capacity = static_cast<int>(constraintsPassed[0][constraintsPassed[0].size() - 2]);

        BranchAndBoundKnapsack knapsackSolver(values, weights, capacity);
        auto [bestValue, bestSolution] = knapsackSolver.Solve();

        Logger::WriteLine(std::string(60, '='));
        Logger::WriteLine("FINAL SOLUTION:");
        Logger::WriteLine("Maximum value: " + std::to_string(bestValue));
        Logger::WriteLine("Solution vector:");

        for (size_t i = 0; i < bestSolution.size(); ++i)
        {
            Logger::WriteLine("x" + std::to_string(i + 1) + " = " + std::to_string(bestSolution[i]));
        }

        int totalWeight = 0;
        double totalValue = 0;
        for (size_t i = 0; i < weights.size(); ++i)
        {
            totalWeight += weights[i] * bestSolution[i];
            totalValue += values[i] * bestSolution[i];
        }

        Logger::WriteLine("\nVerification:");
        Logger::WriteLine("Total weight: " + std::to_string(totalWeight) + " (<= " + std::to_string(capacity) + ")");
        Logger::WriteLine("Total value: " + std::to_string(totalValue));
    }
};

#endif // BRANCH_AND_BOUND_KNAPSACK_HPP