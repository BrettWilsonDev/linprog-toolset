#include <vector>
#include <string>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <numeric>

#include "dual_simplex.hpp"

class Duality
{
private:
    bool isConsoleOutput;
    DualSimplex *dual;

    std::vector<double> objFunc;
    double optimalSolution;
    std::vector<double> changingVars;
    std::vector<std::vector<double>> constraintsLhs;
    std::vector<double> cellRef;
    std::vector<double> dualObjFunc;
    double dualOptimalSolution;
    std::vector<double> dualChangingVars;

    std::vector<std::vector<double>> dualConstraintsLhs;
    std::vector<double> dualCellRef;

    // dual constraints
    std::vector<std::vector<double>> constraints;
    std::vector<std::vector<double>> dualConstraints;
    std::vector<std::string> headerString;
    std::vector<std::string> dualHeaderString;
    std::vector<double> tObjFunc;
    std::vector<double> tDualObjFunc;
    std::string strMin;
    std::string strDualMin;

public:
    Duality(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput)
    {
        dual = new DualSimplex();

        objFunc = {0.0, 0.0};
        optimalSolution = 0.0;
        constraints = {{0.0, 0.0, 0.0, 0.0}};
    }

    ~Duality()
    {
        delete dual;
    }

    template <typename T>
    std::vector<std::vector<T>> TransposeMat(const std::vector<std::vector<T>> &matrix)
    {
        if (matrix.empty())
            return {};

        size_t rows = matrix.size();
        size_t cols = matrix[0].size();

        std::vector<std::vector<T>> result(cols, std::vector<T>(rows));
        for (size_t j = 0; j < cols; ++j)
        {
            for (size_t i = 0; i < rows; ++i)
            {
                result[j][i] = matrix[i][j];
            }
        }
        return result;
    }

    void PrintDuality()
    {
        tObjFunc = objFunc;
        tDualObjFunc = dualObjFunc;
        tObjFunc.push_back(optimalSolution);
        tDualObjFunc.push_back(dualOptimalSolution);

        // print header
        for (const auto &header : headerString)
        {
            std::cout << std::setw(8) << header << "  ";
        }
        std::cout << "\n\n";

        // display the objective function
        std::cout << std::setw(8) << strDualMin + " z" << "  ";
        for (const auto &val : tObjFunc)
        {
            std::cout << std::setw(8) << std::fixed << std::setprecision(3) << val << "  ";
        }
        std::cout << "\n\n";

        auto displayCons = constraintsLhs;

        // build display cons
        for (size_t i = 0; i < constraintsLhs.size(); ++i)
        {
            displayCons[i].push_back(cellRef[i]);
            // displayCons[i].push_back(constraints[i][constraints[i].size() - 1] == 0 ? 0.0 : 1.0);
            if (constraints[i][constraints[i].size() - 1] == 1)
            {
                displayCons[i].push_back(0);
                constraints[i][constraints[i].size() - 2] *= -1;
            }
            else
            {
                displayCons[i].push_back(0);
            }
            displayCons[i].push_back(constraints[i][constraints[i].size() - 2]);

            double tSlack = displayCons[i][displayCons[i].size() - 1] - displayCons[i][displayCons[i].size() - 3];

            displayCons[i].push_back(tSlack);
        }

        // display the constraints
        for (size_t i = 0; i < displayCons.size(); ++i)
        {
            std::cout << std::setw(8) << "c" + std::to_string(i + 1) << "  ";
            for (size_t j = 0; j < displayCons[i].size(); ++j)
            {
                if (j == displayCons[i].size() - 3)
                {
                    std::string sign = displayCons[i][j] == 0 ? "<=" : ">=";
                    std::cout << std::setw(8) << sign << "  ";
                }
                else
                {
                    std::cout << std::setw(8) << std::fixed << std::setprecision(3)
                              << displayCons[i][j] << "  ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";

        // optimal variables
        std::cout << std::setw(8) << "opt" << "  ";
        for (const auto &var : changingVars)
        {
            std::cout << std::setw(8) << std::fixed << std::setprecision(3) << var << "  ";
        }
        std::cout << "\n\n";

        // dual ==============================================
        std::cout << std::string(100, '=') << "\n\n";

        // print dual header
        for (const auto &header : dualHeaderString)
        {
            std::cout << std::setw(8) << header << "  ";
        }
        std::cout << "\n\n";

        // display the dual objective function
        std::cout << std::setw(8) << strMin + " z" << "  ";
        for (const auto &val : tDualObjFunc)
        {
            std::cout << std::setw(8) << std::fixed << std::setprecision(3) << val << "  ";
        }
        std::cout << "\n\n";

        auto dualDisplayCons = dualConstraintsLhs;

        // build dual display cons
        for (size_t i = 0; i < dualConstraintsLhs.size(); ++i)
        {
            dualDisplayCons[i].push_back(dualCellRef[i]);
            dualDisplayCons[i].push_back(1);
            dualDisplayCons[i].push_back(objFunc[i]);
            double tSlack = dualDisplayCons[i][dualDisplayCons[i].size() - 1] - dualDisplayCons[i][dualDisplayCons[i].size() - 3];
            dualDisplayCons[i].push_back(tSlack);
        }

        // display the dual constraints
        for (size_t i = 0; i < dualDisplayCons.size(); ++i)
        {
            std::cout << std::setw(8) << "c" + std::to_string(i + 1) << "  ";
            for (size_t j = 0; j < dualDisplayCons[i].size(); ++j)
            {
                if (j == dualDisplayCons[i].size() - 3)
                {
                    std::string sign = dualDisplayCons[i][j] == 0 ? "<=" : ">=";
                    std::cout << std::setw(8) << sign << "  ";
                }
                else
                {
                    std::cout << std::setw(8) << std::fixed << std::setprecision(3)
                              << dualDisplayCons[i][j] << "  ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";

        // optimal dual variables
        std::cout << std::setw(8) << "opt" << "  ";
        for (const auto &var : dualChangingVars)
        {
            std::cout << std::setw(8) << std::fixed << std::setprecision(3) << var << "  ";
        }
        std::cout << "\n";
    }

    std::tuple<std::vector<double>, double, std::vector<double>,
               std::vector<std::vector<double>>, std::vector<double>,
               std::vector<double>, double, std::vector<double>,
               std::vector<std::vector<double>>, std::vector<double>>
    DoDuality(std::vector<double> objFunc,
              std::vector<std::vector<double>> constraints,
              bool isMin)
    {
        auto a = objFunc;
        auto b = constraints;

        auto [tableaus, changingVars, optimalSolution, _, __, ___] =
            dual->DoDualSimplex(a, b, isMin);

        auto constraintsLhs = constraints;
        for (auto &row : constraintsLhs)
        {
            row.pop_back();
            row.pop_back();
        }

        std::vector<double> cellRef;
        for (size_t i = 0; i < constraintsLhs.size(); ++i)
        {
            double tSum = 0.0;
            for (size_t j = 0; j < changingVars.size() && j < constraintsLhs[i].size(); ++j)
            {
                tSum += changingVars[j] * constraintsLhs[i][j];
            }
            cellRef.push_back(tSum);
        }

        int slackCtr = 0, excessCtr = 0;
        for (const auto &constraint : constraints)
        {
            if (constraint[constraint.size() - 1] == 1)
            {
                excessCtr++;
            }
            else
            {
                slackCtr++;
            }
        }

        if (slackCtr != constraints.size() && excessCtr != constraints.size())
        {
            for (auto &constraint : constraints)
            {
                if (constraint[constraint.size() - 1] == 1)
                {
                    constraint[constraint.size() - 1] = 0;
                    for (size_t j = 0; j < constraint.size() - 1; ++j)
                    {
                        constraint[j] = -constraint[j];
                    }
                }
            }
        }

        auto dualConstraints = constraints;
        std::vector<double> dualObjFunc;

        for (int i = 0; i < constraintsLhs.size(); i++)
        {
            dualObjFunc.push_back(constraints[i][constraints[i].size() - 2]);
        }

        for (const auto &value : dualObjFunc)
        {
            std::cout << value << " ";
        }
        std::cout << "\n"
                  << std::endl;

        dualConstraints = TransposeMat(constraintsLhs);

        for (size_t i = 0; i < dualConstraints.size(); ++i)
        {
            dualConstraints[i].push_back(objFunc[i]);
        }

        for (size_t i = 0; i < constraints.size() && i < dualConstraints.size(); ++i)
        {
            dualConstraints[i].push_back(constraints[i][constraints[i].size() - 1] == 1 ? 0.0 : 1.0);
        }

        for (size_t i = 0; i < dualObjFunc.size(); ++i)
        {
            if (dualObjFunc[i] < 0)
            {
                for (auto &constraint : dualConstraints)
                {
                    constraint[i] = -constraint[i];
                }
            }
        }

        isMin = !isMin;

        auto a2 = dualObjFunc;
        auto b2 = dualConstraints;

        std::cout << "\ndual obj\n"
                  << std::endl;

        auto [tableaus2, dualChangingVars, dualOptimalSolution, _A, _B, __C] =
            dual->DoDualSimplex(a2, b2, isMin);

        auto dualConstraintsLhs = dualConstraints;
        for (auto &row : dualConstraintsLhs)
        {
            row.pop_back();
            row.pop_back();
        }

        std::vector<double> dualCellRef;
        for (size_t i = 0; i < dualConstraintsLhs.size(); ++i)
        {
            double tSum = 0.0;
            for (size_t j = 0; j < dualChangingVars.size() && j < dualConstraintsLhs[i].size(); ++j)
            {
                tSum += dualChangingVars[j] * dualConstraintsLhs[i][j];
            }
            dualCellRef.push_back(tSum);
        }

        this->optimalSolution = optimalSolution;
        this->changingVars = changingVars;
        this->constraintsLhs = constraintsLhs;
        this->cellRef = cellRef;

        this->dualObjFunc = dualObjFunc;
        this->dualOptimalSolution = dualOptimalSolution;
        this->dualChangingVars = dualChangingVars;
        this->dualConstraintsLhs = dualConstraintsLhs;
        this->dualCellRef = dualCellRef;

        headerString.clear();
        for (size_t i = 0; i < objFunc.size(); ++i)
        {
            headerString.push_back("x" + std::to_string(i + 1));
        }
        headerString.push_back("z");
        headerString.insert(headerString.begin(), "primal");

        dualHeaderString.clear();
        for (size_t i = 0; i < dualObjFunc.size(); ++i)
        {
            dualHeaderString.push_back("y" + std::to_string(i + 1));
        }
        dualHeaderString.push_back("z");
        dualHeaderString.insert(dualHeaderString.begin(), "dual");

        if (isConsoleOutput)
        {
            PrintDuality();
            std::cout << "\n" << std::endl; 
            if (optimalSolution == dualOptimalSolution)
            {
                std::cout << "Strong Duality " << optimalSolution
                          << " is equal to " << dualOptimalSolution << "\n";
            }
            else
            {
                std::cout << "Weak Duality " << optimalSolution
                          << " is not equal to " << dualOptimalSolution << "\n";
            }
        }

        return {objFunc, optimalSolution, changingVars, constraintsLhs, cellRef,
                dualObjFunc, dualOptimalSolution, dualChangingVars, dualConstraintsLhs, dualCellRef};
    }

    void RunDuality(const std::vector<double> &objFuncPassed,
                    const std::vector<std::vector<double>> &constraintsPassed,
                    bool isMin)
    {
        objFunc = objFuncPassed;
        constraints = constraintsPassed;

        DoDuality(objFunc, constraints, isMin);
    }

    void PrintShadowPrice()
    {
        std::cout << std::setw(8) << "Shadow Price:" << "  ";
        for (const auto &var : dualChangingVars)
        {
            std::cout << std::setw(8) << std::fixed << std::setprecision(3) << var << "  ";
        }
        std::cout << "\n";
    }
};