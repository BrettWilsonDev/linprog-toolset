#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>
#include <iomanip>

class TwoPhaseSimplex
{
private:
    bool isConsoleOutput;
    bool useBlandsRule; // New flag to control Bland's rule usage
    int testInputSelected;

    std::vector<int> IMPivotCols;
    std::vector<int> IMPivotRows;
    std::vector<std::string> IMHeaderRow;
    std::vector<int> IMPhaseType;

    std::string problemType;

    int amtOfObjVars;
    std::vector<double> objFunc;

    std::vector<std::vector<double>> constraints;
    std::vector<std::string> signItems;
    std::vector<int> signItemsChoices;

    int amtOfConstraints;

    std::vector<std::vector<std::vector<double>>> tableaus;

    int pivotCol;
    int pivotRow;
    int tCol;
    int tRow;
    std::vector<std::string> tHeader;

    std::vector<int> phases;
    double prevZ;

    std::string wString;

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
    TwoPhaseSimplex(bool consoleOutput = false, bool blandsRule = false)
        : isConsoleOutput(consoleOutput), useBlandsRule(blandsRule)
    {
        testInputSelected = -1;

        IMPivotCols.clear();
        IMPivotRows.clear();
        IMHeaderRow.clear();
        IMPhaseType.clear();

        problemType = "Max";

        amtOfObjVars = 2;
        objFunc = {0.0, 0.0};

        constraints = {{0.0, 0.0, 0.0, 0.0}};
        signItems = {"<=", ">="};
        signItemsChoices = {0};

        amtOfConstraints = 1;

        tableaus.clear();

        pivotCol = -1;
        pivotRow = -1;
        tCol = -1;
        tRow = -1;
        tHeader.clear();

        phases.clear();
    }

    struct TestInputResult
    {
        std::vector<double> objFunc;
        std::vector<std::vector<double>> constraints;
        bool isMin;
        bool valid;
    };

    TestInputResult testInput(int testNum = -1)
    {
        TestInputResult result;
        result.valid = false;
        result.isMin = false;

        if (testNum == 0)
        {
            result.objFunc = {100, 30};
            result.constraints = {
                {0, 1, 3, 1},
                {1, 1, 7, 0},
                {10, 4, 40, 0}};
            result.valid = true;
        }
        else if (testNum == 1)
        {
            result.objFunc = {10, 50, 80, 100};
            result.constraints = {
                {1, 4, 4, 8, 140, 0},
                {1, 0, 0, 0, 50, 0},
                {1, 0, 0, 0, 50, 1},
                {1, 1, 1, 1, 70, 1}};
            result.valid = true;
        }
        else if (testNum == 2)
        {
            result.objFunc = {48, 20, 8};
            result.constraints = {
                {8, 4, 2, 60, 1},
                {6, 2, 1.5, 30, 1},
                {1, 1.5, 0.5, 20, 1}};
            result.isMin = true;
            result.valid = true;
        }
        else if (testNum == 3)
        {
            result.objFunc = {4, 1};
            result.constraints = {
                {3, 1, 10, 1},
                {1, 1, 5, 1},
                {1, 0, 3, 1}};
            result.isMin = true;
            result.valid = true;
        }

        return result;
    }

    struct FormulationResult
    {
        std::vector<std::vector<double>> tableau;
        std::vector<int> aCols;
    };

    FormulationResult formulateFirstTab1(const std::vector<double> &objFunc,
                                         const std::vector<std::vector<double>> &constraints)
    {
        FormulationResult result;
        std::vector<std::vector<double>> &tab = result.tableau;

        int objFuncSize = objFunc.size();

        // tableH = constraints + w + z
        int tableH = constraints.size() + 1 + 1;
        // tableW = obj vars + rhs + artificial + constraints
        int tableW = objFuncSize + 1 + 0 + constraints.size();

        int excessCount = 0;
        int slackCount = 0;
        for (int i = 0; i < constraints.size(); i++)
        {
            if (constraints[i].back() == 1)
            {
                excessCount++;
            }
            else
            {
                slackCount++;
            }
        }

        // Build display header row
        IMHeaderRow.clear();
        int imCtr = 1;
        for (int i = 0; i < objFunc.size(); i++)
        {
            IMHeaderRow.push_back("x" + std::to_string(imCtr));
            imCtr++;
        }

        imCtr = 1;

        if (excessCount > 0)
        {
            for (int i = 0; i < excessCount; i++)
            {
                IMHeaderRow.push_back("a" + std::to_string(imCtr));
                IMHeaderRow.push_back("e" + std::to_string(imCtr));
                imCtr++;
            }
        }

        if (slackCount > 0)
        {
            for (int i = 0; i < slackCount; i++)
            {
                IMHeaderRow.push_back("s" + std::to_string(imCtr));
                imCtr++;
            }
        }

        IMHeaderRow.push_back("rhs");

        std::vector<std::vector<double>> eCons;

        for (int i = 0; i < constraints.size(); i++)
        {
            if (constraints[i].back() == 1)
            {
                std::vector<double> temp = constraints[i];
                for (int j = 0; j < temp.size(); j++)
                {
                    temp[j] = temp[j] * -1;
                }
                eCons.push_back(temp);
            }
        }

        // Calculate summed w row
        std::vector<double> summedW;
        for (int i = 0; i < objFuncSize + 1; i++)
        {
            double temp = 0;
            for (int j = 0; j < eCons.size(); j++)
            {
                temp += eCons[j][i];
            }
            summedW.push_back(temp);
        }

        if (isConsoleOutput)
        {
            std::cout << std::endl;
        }

        // Negate the w row
        for (int i = 0; i < summedW.size(); i++)
        {
            summedW[i] = summedW[i] * -1;
        }

        // Make the w row string
        if (isConsoleOutput)
        {
            std::string wStr = "";
            for (int i = 0; i < summedW.size() - 1; i++)
            {
                wStr += " + " + std::to_string(summedW[i]) + "x" + std::to_string(i + 1);
            }

            for (int i = 0; i < eCons.size(); i++)
            {
                wStr += " - e" + std::to_string(i + 1);
            }

            std::cout << "w" << wStr << " = " << summedW.back() << std::endl
                      << std::endl;
        }

        std::string wStr = "";
        for (int i = 0; i < summedW.size() - 1; i++)
        {
            wStr += " + " + std::to_string(summedW[i]) + "x" + std::to_string(i + 1);
        }

        for (int i = 0; i < eCons.size(); i++)
        {
            wStr += " - e" + std::to_string(i + 1);
        }

        wString = "w" + wStr + " = " + std::to_string(summedW.back());

        // Fill the table with zeros
        tab.resize(tableH);
        for (int i = 0; i < tableH; i++)
        {
            tab[i].resize(tableW + eCons.size(), 0.0);
        }

        // Add w row
        for (int i = 0; i < objFuncSize; i++)
        {
            tab[0][i] = summedW[i];
        }
        tab[0].back() = summedW.back();

        // Add z row
        for (int i = 0; i < objFuncSize; i++)
        {
            tab[1][i] = objFunc[i] * -1;
        }

        std::vector<std::vector<double>> tempAllCons;
        for (int i = 0; i < tableH - 2; i++)
        {
            std::vector<double> tempCons(tableW + eCons.size(), 0.0);
            tempAllCons.push_back(tempCons);
        }

        std::vector<int> &aCols = result.aCols;
        int aCtr = objFuncSize;
        for (int i = 0; i < tempAllCons.size(); i++)
        {
            for (int k = 0; k < objFuncSize; k++)
            {
                tempAllCons[i][k] = constraints[i][k];
            }
            tempAllCons[i].back() = constraints[i][constraints[i].size() - 2];

            if (constraints[i].back() == 1)
            {
                tempAllCons[i][aCtr] = 1;
                tempAllCons[i][aCtr + 1] = -1;
                aCols.push_back(aCtr);
                aCtr += 2;
            }
            else
            {
                tempAllCons[i][aCtr] = 1;
                aCtr++;
            }
        }

        for (int i = 2; i < tab.size(); i++)
        {
            for (int j = 0; j < tab[i].size(); j++)
            {
                tab[i][j] = tempAllCons[i - 2][j];
            }
        }

        for (int i = 2; i < tab.size(); i++)
        {
            for (int j = objFuncSize; j < tab[i].size(); j++)
            {
                if (tempAllCons[i - 2][j] == -1)
                {
                    tab[0][j] = -1;
                }
            }
        }

        return result;
    }

    struct PivotResult
    {
        std::vector<std::vector<double>> tableau;
        bool isOptimal;
        bool valid;
    };

    PivotResult DoPivotOperationsPhase1(const std::vector<std::vector<double>> &tab)
    {
        PivotResult result;
        result.valid = false;
        result.isOptimal = false;

        // Choose pivot column based on Bland's rule flag
        int pivotCol = -1;

        if (useBlandsRule)
        {
            // Bland's rule: Choose smallest index among positive elements
            for (int i = 0; i < tab[0].size() - 1; i++)
            {
                if (tab[0][i] > 1e-12)
                {
                    pivotCol = i;
                    break;
                }
            }
        }
        else
        {
            // Standard rule: Choose largest positive element
            double largestW = -1;
            for (int i = 0; i < tab[0].size() - 1; i++)
            {
                if (tab[0][i] > largestW)
                {
                    largestW = tab[0][i];
                    pivotCol = i;
                }
            }
        }

        // If no positive elements, we're optimal
        if (pivotCol == -1)
        {
            result.isOptimal = true;
            return result;
        }

        std::vector<double> thetas;
        for (int i = 2; i < tab.size(); i++)
        {
            if (tab[i][pivotCol] <= 1e-12)
            { // Use epsilon for numerical stability
                thetas.push_back(std::numeric_limits<double>::infinity());
            }
            else
            {
                thetas.push_back(tab[i].back() / tab[i][pivotCol]);
            }
        }

        // Find minimum positive theta
        double theta = std::numeric_limits<double>::infinity();
        for (double x : thetas)
        {
            if (x > 1e-12 && x != std::numeric_limits<double>::infinity())
            {
                theta = std::min(theta, x);
            }
        }

        // Apply pivot row selection (always use smallest index for ties, regardless of Bland's rule flag)
        int pivotRow = -1;
        for (int i = 0; i < thetas.size(); i++)
        {
            if (std::abs(thetas[i] - theta) < 1e-12)
            {
                pivotRow = i + 2;
                break;
            }
        }

        std::vector<std::vector<double>> newTab = tab;

        // Initialize newTab with zeros
        for (int i = 0; i < newTab.size(); i++)
        {
            for (int j = 0; j < newTab[i].size(); j++)
            {
                newTab[i][j] = 0;
            }
        }

        // The div row
        double divNum = tab[pivotRow][pivotCol];

        if (divNum == 0)
        {
            if (isConsoleOutput)
            {
                std::cout << "Divide by 0 error" << std::endl;
            }
            return result;
        }

        for (int j = 0; j < tab[pivotRow].size(); j++)
        {
            newTab[pivotRow][j] = tab[pivotRow][j] / divNum;
        }

        // Apply pivot formula
        for (int i = 0; i < tab.size(); i++)
        {
            for (int j = 0; j < tab[i].size(); j++)
            {
                if (i != pivotRow)
                {
                    newTab[i][j] = tab[i][j] - (tab[i][pivotCol] * newTab[pivotRow][j]);
                }
            }
        }

        // Clean up near-zero values
        for (int i = 0; i < newTab[0].size(); i++)
        {
            if (std::abs(newTab[0][i]) < 1e-12)
            {
                newTab[0][i] = 0.0;
            }
        }

        bool isAllNegW = true;
        for (double num : newTab[0])
        {
            if (num > 0)
            {
                isAllNegW = false;
                break;
            }
        }

        if (isConsoleOutput)
        {
            std::cout << "In Phase 1, The pivot row is " << (pivotRow + 1)
                      << " and the pivot col is " << (pivotCol + 1) << std::endl;
        }

        IMPivotCols.push_back(pivotCol);
        IMPivotRows.push_back(pivotRow);

        result.tableau = newTab;
        result.isOptimal = isAllNegW;
        result.valid = true;

        return result;
    }

#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <iostream>

    std::pair<std::vector<std::vector<double>>, bool> DoPivotOperationsPhase2(const std::vector<std::vector<double>> &tab, bool isMin)
    {

        double largestZ;
        if (isMin)
        {
            // Get all elements except the last one from row 1
            std::vector<double> row1_subset(tab[1].begin(), tab[1].end() - 1);
            largestZ = *std::max_element(row1_subset.begin(), row1_subset.end());
        }
        else
        {
            std::vector<double> row1_subset(tab[1].begin(), tab[1].end() - 1);
            largestZ = *std::min_element(row1_subset.begin(), row1_subset.end());
        }

        // Find pivot column (index of largestZ in the subset)
        int pivotCol = -1;
        for (int i = 0; i < tab[1].size() - 1; i++)
        {
            if (tab[1][i] == largestZ)
            {
                pivotCol = i;
                break;
            }
        }

        // Calculate thetas
        std::vector<double> thetas;
        for (int i = 2; i < tab.size(); i++)
        {
            if (tab[i][pivotCol] == 0)
            {
                thetas.push_back(std::numeric_limits<double>::infinity());
            }
            else
            {
                thetas.push_back(tab[i].back() / tab[i][pivotCol]);
            }
        }

        // Check if all thetas are negative
        bool allNegativeThetas = true;
        for (double theta : thetas)
        {
            if (theta >= 0)
            {
                allNegativeThetas = false;
                break;
            }
        }

        if (allNegativeThetas)
        {
            return std::make_pair(std::vector<std::vector<double>>(), false);
        }

        // Handle very small values (close to zero)
        for (int i = 0; i < thetas.size(); i++)
        {
            if (std::abs(thetas[i]) < 1e-12)
            {
                thetas[i] = 0.0;
            }
        }

        // Check if any positive thetas exist (excluding 0 and inf)
        bool hasPositiveTheta = false;
        for (double theta : thetas)
        {
            if (theta > 0 && !std::isinf(theta))
            {
                hasPositiveTheta = true;
                break;
            }
        }

        double theta;
        if (!hasPositiveTheta)
        {
            // Check if 0 is in thetas
            bool hasZero = std::find(thetas.begin(), thetas.end(), 0.0) != thetas.end();
            if (hasZero)
            {
                theta = 0.0;
            }
            else
            {
                return std::make_pair(std::vector<std::vector<double>>(), false);
            }
        }
        else
        {
            // Find minimum positive theta (excluding inf)
            theta = std::numeric_limits<double>::infinity();
            for (double x : thetas)
            {
                if (x > 0 && !std::isinf(x) && x < theta)
                {
                    theta = x;
                }
            }
        }

        // Find pivot row
        int pivotRow = -1;
        for (int i = 0; i < thetas.size(); i++)
        {
            if (thetas[i] == theta)
            {
                pivotRow = i;
                break;
            }
        }
        pivotRow += 2; // Adjust for the offset

        // Create newTab (deep copy initialized with zeros)
        std::vector<std::vector<double>> newTab(tab.size());
        for (int i = 0; i < tab.size(); i++)
        {
            newTab[i].resize(tab[i].size(), 0.0);
        }

        // Division row operation
        double divNum = tab[pivotRow][pivotCol];
        if (divNum == 0)
        {
            if (isConsoleOutput)
            {
                std::cout << "Divide by 0 error" << std::endl;
            }
            return std::make_pair(std::vector<std::vector<double>>(), false);
        }

        // Divide the pivot row
        for (int j = 0; j < tab[pivotRow].size(); j++)
        {
            newTab[pivotRow][j] = tab[pivotRow][j] / divNum;
        }

        // Apply the pivot formula to all other rows
        for (int i = 0; i < tab.size(); i++)
        {
            for (int j = 0; j < tab[i].size(); j++)
            {
                if (i != pivotRow)
                {
                    newTab[i][j] = tab[i][j] - (tab[i][pivotCol] * newTab[pivotRow][j]);
                }
            }
        }

        // Check termination condition
        bool isAllNegZ;
        if (isMin)
        {
            isAllNegZ = true;
            for (int i = 0; i < newTab[1].size() - 1; i++)
            {
                if (newTab[1][i] > 0)
                {
                    isAllNegZ = false;
                    break;
                }
            }
        }
        else
        {
            isAllNegZ = true;
            for (int i = 0; i < newTab[1].size() - 1; i++)
            {
                if (newTab[1][i] < 0)
                {
                    isAllNegZ = false;
                    break;
                }
            }
        }

        if (isConsoleOutput)
        {
            std::cout << "In Phase 2, The pivot row is " << (pivotRow + 1)
                      << " and the pivot col is " << (pivotCol + 1) << std::endl;
        }

        // Add to member vectors (assuming these are class members)
        IMPivotCols.push_back(pivotCol);
        IMPivotRows.push_back(pivotRow);

        return std::make_pair(newTab, isAllNegZ);
    }

    std::vector<std::vector<std::vector<double>>> DoTwoPhase(const std::vector<double> &objFunc, const std::vector<std::vector<double>> &constraints, bool isMin)
    {
        std::vector<std::vector<std::vector<double>>> tabs;
        bool isAllNegW = false;
        std::vector<int> aCols;

        // Formulate first tableau
        auto [tab, cols] = formulateFirstTab1(objFunc, constraints);
        aCols = cols;
        tabs.push_back(tab);

        // Check if all elements in first row are non-positive
        isAllNegW = tabs.back()[0].empty() ? false : std::all_of(tabs.back()[0].begin(), tabs.back()[0].end(), [](double num)
                                                                 { return num <= 0; });

        int phase1Ctr = 0;
        while (!isAllNegW)
        {
            auto [newTab, newIsAllNegW, pivotValid] = DoPivotOperationsPhase1(tabs.back());
            if (!pivotValid)
            {
                break;
            }
            if (!newTab.size() && newIsAllNegW == false)
            {
                break;
            }

            tabs.push_back(newTab);
            isAllNegW = newIsAllNegW;

            phase1Ctr++;
            IMPhaseType.push_back(0);

            if (isAllNegW || phase1Ctr > 10)
            {
                break;
            }
        }

        int tabPhaseNum = phase1Ctr + 1;

        // Deep copy last tableau
        std::vector<std::vector<double>> newTab = tabs.back();

        // Zero out columns specified in aCols
        for (int k = 0; k < aCols.size(); k++)
        {
            for (size_t i = 0; i < newTab.size(); i++)
            {
                newTab[i][aCols[k]] = 0.0;
            }
        }

        tabs.push_back(newTab);

        // Check objective row (second row, excluding last element)
        bool AllPosZ = isMin ? std::all_of(tabs.back()[1].begin(), tabs.back()[1].end() - 1,
                                           [](double num)
                                           { return num <= 0; })
                             : std::all_of(tabs.back()[1].begin(), tabs.back()[1].end() - 1,
                                           [](double num)
                                           { return num >= 0; });

        // Find index of duplicate tableau
        int indexOfDupe = -1;
        for (size_t i = 0; i < tabs.size(); i++)
        {
            if (tabs[i] == tabs.back())
            {
                indexOfDupe = i;
                break;
            }
        }

        int phase2Ctr = 0;
        phases.push_back(0);

        while (!AllPosZ)
        {
            prevZ = tabs.back()[1].back();
            auto [newTab, newAllPosZ] = DoPivotOperationsPhase2(tabs.back(), isMin);

            if (!newTab.size() && newAllPosZ == false)
            {
                break;
            }

            tabs.push_back(newTab);
            AllPosZ = newAllPosZ;
            phases.push_back(1);

            if (AllPosZ || phase2Ctr > 100)
            {
                break;
            }

            phase2Ctr++;
            IMPhaseType.push_back(2);
        }

        // Remove duplicate tableau
        if (indexOfDupe != -1 && indexOfDupe < tabs.size())
        {
            tabs.erase(tabs.begin() + indexOfDupe);
        }

        // Final optimal check
        isAllNegW = tabs.back()[0].empty() ? false : std::all_of(tabs.back()[0].begin(), tabs.back()[0].end(), [](double num)
                                                                 { return num <= 0; });

        if (!isAllNegW)
        {
            tabs.pop_back();
            if (!IMPivotCols.empty())
                IMPivotCols.pop_back();
            if (!IMPivotRows.empty())
                IMPivotRows.pop_back();
        }

        IMPhaseType.push_back(2);
        int currentPhase = 1;

        // Output tableaus
        for (size_t i = 0; i < tabs.size(); i++)
        {
            if (i == tabPhaseNum)
            {
                currentPhase = 2;
            }

            if (isConsoleOutput)
            {
                std::cout << "Phase " << currentPhase << std::endl;
                std::cout << "Tableau " << (i + 1) << std::endl;
                for (const auto &row : tabs[i])
                {
                    for (double val : row)
                    {
                        std::cout << std::fixed << std::setprecision(3) << std::setw(10) << val << " ";
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;
            }
        }

        phases.push_back(-1);

        size_t nVars = objFunc.size(); // number of decision variables
        std::vector<double> changingVars(nVars, 0.0);

        // Reference the last tableau
        const auto &lastTable = tabs.back();

        for (size_t col = 0; col < nVars; ++col)
        {
            int onesCount = 0;
            int nonZeroCount = 0;
            size_t rowIndex = 0;

            for (size_t row = 0; row < lastTable.size(); ++row)
            {
                if (lastTable[row][col] == 1.0)
                {
                    ++onesCount;
                    rowIndex = row;
                }
                if (lastTable[row][col] != 0.0)
                {
                    ++nonZeroCount;
                }
            }

            // Valid if exactly one 1 and all others are zero
            if (onesCount == 1 && nonZeroCount == 1)
            {
                changingVars[col] = lastTable[rowIndex].back(); // RHS
            }
        }

        result.tableaus = tabs;
        result.pivotCols = IMPivotCols;
        result.pivotRows = IMPivotRows;
        result.headerRow = IMHeaderRow;
        result.phases = phases;
        result.optimalSolution = tabs.back().at(1).back();
        result.changingVars = changingVars;

        return tabs;
    }

    // std::vector<std::vector<std::vector<double>>> DoTwoPhase(const std::vector<double> &objFunc, const std::vector<std::vector<double>> &constraints, bool isMin)
    // {
    //     std::vector<std::vector<std::vector<double>>> tabs;

    //     FormulationResult formResult = formulateFirstTab1(objFunc, constraints);
    //     std::vector<std::vector<double>> tab = formResult.tableau;
    //     std::vector<int> aCols = formResult.aCols;

    //     tabs.push_back(tab);

    //     bool isAllNegW = true;
    //     for (double num : tabs.back()[0])
    //     {
    //         if (num > 0)
    //         {
    //             isAllNegW = false;
    //             break;
    //         }
    //     }

    //     int phase1Ctr = 0;
    //     while (!isAllNegW)
    //     {
    //         PivotResult pivotResult = DoPivotOperationsPhase1(tabs.back());
    //         if (!pivotResult.valid)
    //         {
    //             break;
    //         }

    //         tab = pivotResult.tableau;
    //         isAllNegW = pivotResult.isOptimal;

    //         tabs.push_back(tab);

    //         phase1Ctr++;
    //         IMPhaseType.push_back(0);
    //         if (isAllNegW || phase1Ctr > 10)
    //         {
    //             break;
    //         }
    //     }

    //     int tabPhaseNum = phase1Ctr + 1;

    //     std::vector<std::vector<double>> newTab = tabs.back();

    //     // Remove artificial variable columns
    //     for (int k = 0; k < aCols.size(); k++)
    //     {
    //         for (int i = 0; i < newTab.size(); i++)
    //         {
    //             if (aCols[k] < newTab[i].size())
    //             {
    //                 newTab[i][aCols[k]] = 0.0;
    //             }
    //         }
    //     }

    //     tabs.push_back(newTab);

    //     bool AllPosZ;
    //     if (!isMin)
    //     {
    //         AllPosZ = true;
    //         for (int i = 0; i < tabs.back()[1].size() - 1; i++)
    //         {
    //             if (tabs.back()[1][i] < 0)
    //             {
    //                 AllPosZ = false;
    //                 break;
    //             }
    //         }
    //     }
    //     else
    //     {
    //         AllPosZ = true;
    //         for (int i = 0; i < tabs.back()[1].size() - 1; i++)
    //         {
    //             if (tabs.back()[1][i] > 0)
    //             {
    //                 AllPosZ = false;
    //                 break;
    //             }
    //         }
    //     }

    //     // Find and remove duplicate
    //     // int indexOfDupe = -1;
    //     // for (int i = 0; i < tabs.size() - 1; i++)
    //     // {
    //     //     if (tabs[i] == tabs.back())
    //     //     {
    //     //         indexOfDupe = i;
    //     //         break;
    //     //     }
    //     // }

    //     int phase2Ctr = 0;
    //     phases.push_back(0);

    //     while (!AllPosZ)
    //     {
    //         prevZ = tabs.back()[1].back();
    //         auto pivotResult = DoPivotOperationsPhase2(tabs.back(), isMin);

    //         tab = pivotResult.first;
    //         AllPosZ = pivotResult.second;

    //         tabs.push_back(tab);
    //         phases.push_back(1);

    //         if (AllPosZ || phase2Ctr > 100)
    //         {
    //             break;
    //         }

    //         phase2Ctr++;
    //         IMPhaseType.push_back(2);
    //     }

    //     if (indexOfDupe != -1 && indexOfDupe < tabs.size())
    //     {
    //         tabs.erase(tabs.begin() + indexOfDupe);
    //     }

    //     // Final optimal check
    //     isAllNegW = true;
    //     for (double num : tabs.back()[0])
    //     {
    //         if (num > 0)
    //         {
    //             isAllNegW = false;
    //             break;
    //         }
    //     }

    //     if (!isAllNegW)
    //     {
    //         tabs.pop_back();
    //         if (!IMPivotCols.empty())
    //             IMPivotCols.pop_back();
    //         if (!IMPivotRows.empty())
    //             IMPivotRows.pop_back();
    //     }

    //     IMPhaseType.push_back(2);
    //     int currentPhase = 1;

    //     for (int i = 0; i < tabs.size(); i++)
    //     {
    //         if (i == tabPhaseNum)
    //         {
    //             currentPhase = 2;
    //         }

    //         if (isConsoleOutput)
    //         {
    //             std::cout << "Phase " << currentPhase << std::endl;
    //             std::cout << "Tableau " << (i + 1) << std::endl;
    //             for (int j = 0; j < tabs[i].size(); j++)
    //             {
    //                 for (int k = 0; k < tabs[i][j].size(); k++)
    //                 {
    //                     std::cout << std::setw(10) << std::fixed << std::setprecision(3)
    //                               << tabs[i][j][k] << " ";
    //                 }
    //                 std::cout << std::endl;
    //             }
    //             std::cout << std::endl;
    //         }
    //     }

    //     phases.push_back(-1);

    //     result.tableaus = tabs;
    //     result.pivotCols = IMPivotCols;
    //     result.pivotRows = IMPivotRows;
    //     result.headerRow = IMHeaderRow;
    //     result.phases = phases;
    //     result.optimalSolution = tabs.back().at(1).back();

    //     return tabs;
    // }

    // Setter to enable/disable Bland's rule
    void SetBlandsRule(bool enable) { useBlandsRule = enable; }

    // Getters for accessing private members
    const std::vector<int> &GetPivotCols() const { return IMPivotCols; }
    const std::vector<int> &GetPivotRows() const { return IMPivotRows; }
    const std::vector<std::string> &GetHeaderRow() const { return IMHeaderRow; }
    const std::vector<int> &GetPhaseTypes() const { return IMPhaseType; }
    const std::vector<int> &GetPhases() const { return phases; }
    const std::string &GetWString() const { return wString; }

    LPRResult GetResult()
    {
        return result;
    }
};