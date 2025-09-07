#ifndef SENSITIVITY_ANALYSIS_HPP
#define SENSITIVITY_ANALYSIS_HPP

#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <stdexcept>
#include <string>
#include <sstream>

#include "dual_simplex.hpp"

class AddingActivitiesAndConstraints
{
private:
    bool isConsoleOutput;
    DualSimplex *dual;
    std::vector<double> objFunc;
    std::vector<std::vector<double>> constraints;

    // Store matrices for sensitivity analysis
    std::vector<std::vector<double>> matrixB;
    std::vector<std::vector<double>> matrixBNegOne;
    std::vector<std::vector<double>> matrixCbv;
    std::vector<std::vector<double>> matrixCbvNegOne;
    std::vector<int> basicVarSpots;
    std::vector<std::vector<double>> firstTab;
    std::vector<std::vector<double>> revisedTab;
    std::vector<std::string> headerStr;

public:
    explicit AddingActivitiesAndConstraints(bool isConsoleOutput = false)
        : isConsoleOutput(isConsoleOutput), dual(nullptr)
    {

        // Initialize with default values
        objFunc = {0.0, 0.0};
        constraints = {{0.0, 0.0, 0.0, 0.0}};

        // Initialize matrices as empty
        matrixB.clear();
        matrixBNegOne.clear();
        matrixCbv.clear();
        matrixCbvNegOne.clear();
        basicVarSpots.clear();
        firstTab.clear();
        revisedTab.clear();
        headerStr.clear();

        dual = new DualSimplex();
    }

    ~AddingActivitiesAndConstraints()
    {
        delete dual;
    }

    std::vector<std::vector<double>> DoFormulationOperation(
        const std::vector<double> &objFunc,
        std::vector<std::vector<double>> constraints)
    {

        int excessCount = 0;
        int slackCount = 0;

        for (size_t i = 0; i < constraints.size(); i++)
        {
            if (constraints[i][constraints[i].size() - 1] == 1)
            {
                excessCount++;
            }
            else
            {
                slackCount++;
            }
        }

        // Multiply by -1 for excess constraints
        for (size_t i = 0; i < constraints.size(); i++)
        {
            if (constraints[i][constraints[i].size() - 1] == 1)
            {
                for (size_t j = 0; j < constraints[i].size(); j++)
                {
                    constraints[i][j] = -1 * constraints[i][j];
                }
            }
        }

        // Remove the last element from each constraint
        for (size_t i = 0; i < constraints.size(); i++)
        {
            constraints[i].pop_back();
        }

        int tableSizeH = constraints.size() + 1;
        int tableSizeW = excessCount + slackCount + 1 + objFunc.size();

        std::vector<std::vector<double>> opTable(tableSizeH, std::vector<double>(tableSizeW, 0.0));

        // Fill objective function row (negated)
        for (size_t i = 0; i < objFunc.size(); i++)
        {
            opTable[0][i] = -objFunc[i];
        }

        // Fill constraint rows
        for (size_t i = 0; i < constraints.size(); i++)
        {
            for (size_t j = 0; j < constraints[i].size() - 1; j++)
            {
                opTable[i + 1][j] = constraints[i][j];
            }
            opTable[i + 1][opTable[i + 1].size() - 1] = constraints[i][constraints[i].size() - 1];
        }

        // Add slack and excess variables (1s on diagonal)
        for (size_t i = 1; i < opTable.size(); i++)
        {
            for (size_t j = objFunc.size(); j < opTable[i].size() - 1; j++)
            {
                if (j == i + objFunc.size() - 1)
                {
                    opTable[i][j] = 1.0;
                }
            }
        }

        return opTable;
    }

    std::vector<std::vector<double>> MatTranspose(const std::vector<std::vector<double>> &matrix)
    {
        if (matrix.empty())
            return {};

        size_t rows = matrix.size();
        size_t cols = matrix[0].size();
        std::vector<std::vector<double>> result(cols, std::vector<double>(rows));

        for (size_t j = 0; j < cols; j++)
        {
            for (size_t i = 0; i < rows; i++)
            {
                result[j][i] = matrix[i][j];
            }
        }
        return result;
    }

    std::vector<std::vector<double>> MatMultiply(
        const std::vector<std::vector<double>> &A,
        const std::vector<std::vector<double>> &B)
    {

        size_t rowsA = A.size();
        size_t colsA = A[0].size();
        size_t rowsB = B.size();
        size_t colsB = B[0].size();

        if (colsA != rowsB)
        {
            throw std::invalid_argument("Incompatible dimensions for multiplication");
        }

        std::vector<std::vector<double>> result(rowsA, std::vector<double>(colsB, 0.0));

        for (size_t i = 0; i < rowsA; i++)
        {
            for (size_t j = 0; j < colsB; j++)
            {
                double sum = 0.0;
                for (size_t k = 0; k < colsA; k++)
                {
                    sum += A[i][k] * B[k][j];
                }
                result[i][j] = sum;
            }
        }
        return result;
    }

    std::vector<std::vector<double>> MatIdentity(int n)
    {
        std::vector<std::vector<double>> result(n, std::vector<double>(n, 0.0));
        for (int i = 0; i < n; i++)
        {
            result[i][i] = 1.0;
        }
        return result;
    }

    std::vector<std::vector<double>> MatInverse(const std::vector<std::vector<double>> &matrix)
    {
        size_t n = matrix.size();
        size_t m = matrix[0].size();
        if (n != m)
        {
            throw std::invalid_argument("Matrix must be square for inversion");
        }

        // Create augmented matrix
        std::vector<std::vector<double>> aug(n, std::vector<double>(2 * n));
        auto identity = MatIdentity(n);

        for (size_t i = 0; i < n; i++)
        {
            for (size_t j = 0; j < n; j++)
            {
                aug[i][j] = matrix[i][j];
                aug[i][j + n] = identity[i][j];
            }
        }

        // Forward elimination
        for (size_t i = 0; i < n; i++)
        {
            double pivot = aug[i][i];
            if (std::abs(pivot) < 1e-10)
            {
                // Swap with a lower row
                bool found = false;
                for (size_t r = i + 1; r < n; r++)
                {
                    if (std::abs(aug[r][i]) > 1e-10)
                    {
                        std::swap(aug[i], aug[r]);
                        pivot = aug[i][i];
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    throw std::invalid_argument("Matrix is singular and cannot be inverted");
                }
            }

            // Normalize pivot row
            for (size_t j = 0; j < 2 * n; j++)
            {
                aug[i][j] /= pivot;
            }

            // Eliminate other rows
            for (size_t r = 0; r < n; r++)
            {
                if (r != i)
                {
                    double factor = aug[r][i];
                    for (size_t j = 0; j < 2 * n; j++)
                    {
                        aug[r][j] -= factor * aug[i][j];
                    }
                }
            }
        }

        // Extract inverse
        std::vector<std::vector<double>> result(n, std::vector<double>(n));
        for (size_t i = 0; i < n; i++)
        {
            for (size_t j = 0; j < n; j++)
            {
                result[i][j] = aug[i][j + n];
            }
        }
        return result;
    }

    std::vector<std::vector<double>> MatFromColVector(const std::vector<double> &lst)
    {
        std::vector<std::vector<double>> result;
        for (double x : lst)
        {
            result.push_back({x});
        }
        return result;
    }

    void PrintMatrix(const std::vector<std::vector<double>> &M, const std::string &name = "Matrix")
    {
        Logger::WriteLine(name);
        auto transposed = MatTranspose(M);
        for (const auto &row : transposed)
        {
            for (double val : row)
            {
                Logger::Write(std::to_string(val) + "    ");
            }
            Logger::WriteLine("");
        }
        Logger::WriteLine("");
    }

    void DoPreliminaries(const std::vector<double> &objFunc,
                         const std::vector<std::vector<double>> &constraints,
                         bool isMin)
    {

        // Make temporary copies
        auto tObjFunc = objFunc;
        auto tConstraints = constraints;

        auto result = dual->DoDualSimplex(tObjFunc, tConstraints, isMin);
        auto &tableaus = result.tableaus;
        auto &headerStr = result.headerRow;

        this->headerStr = headerStr;

        // Get the spots of the basic variables
        std::vector<int> basicVarSpots;
        auto &finalTableau = tableaus[tableaus.size() - 1];

        for (size_t k = 0; k < finalTableau[finalTableau.size() - 1].size(); k++)
        {
            std::vector<double> tCVars;

            for (size_t i = 0; i < finalTableau.size(); i++)
            {
                double columnValue = finalTableau[i][k];
                tCVars.push_back(columnValue);
            }

            double sum = std::accumulate(tCVars.begin(), tCVars.end(), 0.0);
            if (std::abs(sum - 1.0) < 1e-10)
            {
                basicVarSpots.push_back(k);
            }
        }

        this->basicVarSpots = basicVarSpots;

        // Get the columns of the basic variables
        std::vector<std::vector<double>> basicVarCols;
        for (size_t i = 0; i < finalTableau[finalTableau.size() - 1].size(); i++)
        {
            if (std::find(basicVarSpots.begin(), basicVarSpots.end(), i) != basicVarSpots.end())
            {
                std::vector<double> tLst;
                for (size_t j = 0; j < finalTableau.size(); j++)
                {
                    tLst.push_back(finalTableau[j][i]);
                }
                basicVarCols.push_back(tLst);
            }
        }

        // Sort the cbv according to basic var positions
        std::vector<std::pair<std::vector<double>, int>> zippedCbv;
        for (size_t i = 0; i < basicVarCols.size(); i++)
        {
            zippedCbv.push_back({basicVarCols[i], basicVarSpots[i]});
        }

        std::sort(zippedCbv.begin(), zippedCbv.end(),
                  [](const auto &a, const auto &b)
                  {
                      auto it_a = std::find(a.first.begin(), a.first.end(), 1.0);
                      auto it_b = std::find(b.first.begin(), b.first.end(), 1.0);
                      int idx_a = (it_a != a.first.end()) ? std::distance(a.first.begin(), it_a) : a.first.size();
                      int idx_b = (it_b != b.first.end()) ? std::distance(b.first.begin(), it_b) : b.first.size();
                      return idx_a < idx_b;
                  });

        std::vector<std::vector<double>> sortedBasicVars;
        basicVarSpots.clear();
        for (const auto &pair : zippedCbv)
        {
            sortedBasicVars.push_back(pair.first);
            basicVarSpots.push_back(pair.second);
        }

        // Populate matrices
        std::vector<double> cbv;
        for (int spot : basicVarSpots)
        {
            cbv.push_back(-tableaus[0][0][spot]);
        }

        std::vector<std::vector<double>> matB;
        for (int spot : basicVarSpots)
        {
            std::vector<double> tLst;
            for (size_t j = 1; j < tableaus[0].size(); j++)
            {
                tLst.push_back(tableaus[0][j][spot]);
            }
            matB.push_back(tLst);
        }

        this->matrixCbv = MatFromColVector(cbv);
        this->matrixB = matB;
        this->matrixBNegOne = MatInverse(matB);
        this->matrixCbvNegOne = MatMultiply(matrixBNegOne, matrixCbv);

        if (isConsoleOutput)
        {
            PrintMatrix(matrixCbv, "cbv");
            PrintMatrix(matrixB, "B");
            PrintMatrix(matrixBNegOne, "B^-1");
            PrintMatrix(matrixCbvNegOne, "cbvB^-1");
            Logger::WriteLine("");
        }

        // Work with the final tableau directly
        this->firstTab = tableaus[0];

        // Get the z values of the new changing table
        std::vector<double> changingZRow;
        for (size_t j = 0; j < firstTab[firstTab.size() - 1].size() - 1; j++)
        { // skip RHS column
            // Column under variable j (skip row 0)
            std::vector<double> tCol;
            for (size_t i = 1; i < firstTab.size(); i++)
            {
                tCol.push_back(firstTab[i][j]);
            }

            // Make row vector and multiply with cbvB^-1
            std::vector<std::vector<double>> tRow = {tCol};
            auto mmultCbvNegOneBCol = MatMultiply(tRow, matrixCbvNegOne);
            double matNegValue = mmultCbvNegOneBCol[0][0];
            changingZRow.push_back(matNegValue - (-firstTab[0][j]));
        }

        // Get the rhs optimal value
        std::vector<double> rhsCol;
        for (size_t i = 1; i < firstTab.size(); i++)
        {
            rhsCol.push_back(firstTab[i][firstTab[i].size() - 1]);
        }
        std::vector<std::vector<double>> rhsRow = {rhsCol};
        auto rhsOptimal = MatMultiply(rhsRow, matrixCbvNegOne);
        double changingOptimal = rhsOptimal[0][0];

        // Get the b values of the new changing table
        std::vector<std::vector<double>> changingBvRows;
        for (size_t j = 0; j < firstTab[firstTab.size() - 1].size(); j++)
        { // all columns, including RHS
            std::vector<double> col;
            for (size_t i = 1; i < firstTab.size(); i++)
            {
                col.push_back(firstTab[i][j]);
            }
            std::vector<std::vector<double>> row = {col};   // make 1×n
            auto product = MatMultiply(row, matrixBNegOne); // (1×n) @ (n×n)
            changingBvRows.push_back(product[0]);           // flatten row
        }

        // Transpose to match tableau format
        auto transposeChangingB = MatTranspose(changingBvRows);

        // Rebuild changing table from final tableau
        this->revisedTab = firstTab;

        changingZRow.push_back(changingOptimal);
        revisedTab[0] = changingZRow;

        // Fill in rows under Z row
        for (size_t i = 0; i < revisedTab.size() - 1; i++)
        {
            for (size_t j = 0; j < revisedTab[i].size(); j++)
            {
                revisedTab[i + 1][j] = transposeChangingB[i][j];
            }
        }

        Logger::WriteLine("");
        PrintTableau(firstTab, "Initial Table");
        Logger::WriteLine("");

        // Logger::WriteLine("Initial Table\n");
        // for (const auto &header : headerStr)
        // {
        //     Logger::Write(header + "               ");
        // }
        // Logger::WriteLine("");
        // for (size_t i = 0; i < firstTab.size(); i++)
        // {
        //     for (size_t j = 0; j < firstTab[i].size(); j++)
        //     {
        //         std::ostringstream oss;
        //         oss << std::fixed << std::setprecision(4) << firstTab[i][j];
        //         Logger::Write(oss.str() + "           ");
        //     }
        //     Logger::WriteLine("");
        // }
        
        Logger::WriteLine("");
        PrintTableau(revisedTab, "Optimal Changing Table");
        Logger::WriteLine("");

        // Logger::WriteLine("\nOptimal Changing Table\n");
        // for (const auto &header : headerStr)
        // {
        //     Logger::Write(header + "               ");
        // }
        // Logger::WriteLine("");
        // for (size_t i = 0; i < revisedTab.size(); i++)
        // {
        //     for (size_t j = 0; j < revisedTab[i].size(); j++)
        //     {
        //         std::ostringstream oss;
        //         oss << std::fixed << std::setprecision(4) << revisedTab[i][j];
        //         Logger::Write(oss.str() + "           ");
        //     }
        //     Logger::WriteLine("");
        // }
    }

    std::vector<double> DoAddActivity(const std::vector<double> &activity)
    {
        // Convert activity (skip first element) into a column matrix
        std::vector<double> activitySkipped(activity.begin() + 1, activity.end());
        auto matrixAct = MatFromColVector(activitySkipped);

        // c = matrixAct^T * matrixCbvNegOne
        auto c = MatMultiply(MatTranspose(matrixAct), matrixCbvNegOne);

        // Top value of the objective row
        double cTop = c[0][0] - activity[0];

        // b = matrixAct^T * matrixBNegOne
        auto b = MatMultiply(MatTranspose(matrixAct), matrixBNegOne);

        // Flatten the last row of b for display
        std::vector<double> displayCol = {cTop};
        displayCol.insert(displayCol.end(), b.back().begin(), b.back().end());

        std::ostringstream oss;
        oss << "Display Column: ";
        for (size_t i = 0; i < displayCol.size(); i++)
        {
            if (i > 0)
                oss << ", ";
            oss << displayCol[i];
        }
        Logger::WriteLine(oss.str());

        return displayCol;
    }

    double RoundValue(double value)
    {
        return std::round(value * 10000.0) / 10000.0;
    }

    std::vector<std::vector<double>> RoundMatrix(const std::vector<std::vector<double>> &matrix)
    {
        if (matrix.empty())
        {
            return {{}};
        }

        std::vector<std::vector<double>> result;
        for (const auto &row : matrix)
        {
            std::vector<double> roundedRow;
            for (double val : row)
            {
                roundedRow.push_back(RoundValue(val));
            }
            result.push_back(roundedRow);
        }
        return result;
    }

    void PrintTableau(const std::vector<std::vector<double>> &tableau, const std::string &title = "Tableau")
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
                std::cout << std::setw(10) << std::fixed << std::setprecision(4) << RoundValue(val);
            }
            std::cout << "\n";
        }

        Logger::WriteLine("");
    }


    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>
    DoAddConstraint(const std::vector<std::vector<double>> &addedConstraints,
                    const std::vector<std::vector<double>> &overRideTab = {})
    {

        if (overRideTab.empty())
        {
            Logger::WriteLine("needs an input table");
            return {{}, {}};
        }

        auto newTab = overRideTab;

        // Add new constraint rows to the tableau
        for (size_t k = 0; k < addedConstraints.size(); k++)
        {
            // Add new column for slack/excess variable
            for (size_t i = 0; i < overRideTab.size(); i++)
            {
                newTab[i].insert(newTab[i].end() - 1, 0.0);
            }

            std::vector<double> newCon(overRideTab[0].size() + addedConstraints.size(), 0.0);

            // Fill constraint coefficients
            for (size_t i = 0; i < addedConstraints[k].size() - 2; i++)
            {
                newCon[i] = RoundValue(addedConstraints[k][i]);
            }

            // Set RHS value
            newCon[newCon.size() - 1] = RoundValue(addedConstraints[k][addedConstraints[k].size() - 2]);

            // Set slack/excess variable
            size_t slackSpot = (newCon.size() - addedConstraints.size()) - 1 + k;
            if (addedConstraints[k].back() == 1)
            { // >= constraint
                newCon[slackSpot] = -1.0;
            }
            else
            {
                newCon[slackSpot] = 1.0;
            }

            newTab.push_back(newCon);
        }

        newTab = RoundMatrix(newTab);
        PrintTableau(newTab, "unfixed tab");

        auto displayTab = newTab;

        // Process each added constraint
        for (size_t k = 0; k < addedConstraints.size(); k++)
        {
            size_t constraintRowIndex = newTab.size() - addedConstraints.size() + k;

            for (int colIndex : basicVarSpots)
            {
                double coefficientInNewRow = RoundValue(displayTab[constraintRowIndex][colIndex]);

                if (std::abs(coefficientInNewRow) > 1e-6)
                {
                    int pivotRow = -1;
                    for (size_t rowIndex = 0; rowIndex < displayTab.size() - addedConstraints.size(); rowIndex++)
                    {
                        if (std::abs(RoundValue(displayTab[rowIndex][colIndex]) - 1.0) <= 1e-6)
                        {
                            pivotRow = rowIndex;
                            break;
                        }
                    }

                    if (pivotRow != -1)
                    {
                        int constraintType = static_cast<int>(addedConstraints[k].back());
                        bool autoReverse = (constraintType == 1);

                        for (size_t col = 0; col < displayTab[0].size(); col++)
                        {
                            double pivotVal = RoundValue(displayTab[pivotRow][col]);
                            double constraintVal = RoundValue(displayTab[constraintRowIndex][col]);

                            double newVal;
                            if (autoReverse)
                            {
                                newVal = pivotVal - coefficientInNewRow * constraintVal;
                            }
                            else
                            {
                                newVal = constraintVal - coefficientInNewRow * pivotVal;
                            }

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

    void RunSensitivityAnalysis(const std::vector<double> &objFuncPassed,
                                const std::vector<std::vector<double>> &constraintsPassed,
                                bool isMinPassed)
    {

        objFunc = objFuncPassed;
        constraints = constraintsPassed;
        bool isMin = isMinPassed;

        DoPreliminaries(objFunc, constraints, isMin);


        // std::vector<std::vector<double>> addedCon = {
        //     {1, 0, 0, 2, 1},
        //     {0, 1, 0, 2, 1},
        //     {0, 0, 1, 2, 1},
        // };
        // DoAddConstraint(addedCon, revisedTab);

        std::vector<double> act = {1, 2, 4, 5};
        DoAddActivity(act);
    }

    // Getters for accessing private members if needed
    const std::vector<std::vector<double>> &getMatrixB() const { return matrixB; }
    const std::vector<std::vector<double>> &getMatrixBNegOne() const { return matrixBNegOne; }
    const std::vector<std::vector<double>> &getMatrixCbv() const { return matrixCbv; }
    const std::vector<std::vector<double>> &getMatrixCbvNegOne() const { return matrixCbvNegOne; }
    const std::vector<int> &getBasicVarSpots() const { return basicVarSpots; }
    const std::vector<std::vector<double>> &getFirstTab() const { return firstTab; }
    const std::vector<std::vector<double>> &getRevisedTab() const { return revisedTab; }
    const std::vector<std::string> &getHeaderStr() const { return headerStr; }
};

#endif // SENSITIVITY_ANALYSIS_HPP