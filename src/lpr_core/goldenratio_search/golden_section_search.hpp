#pragma once

#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <sstream>
#include "exprtk.hpp"

class GoldenSectionSearch
{
private:
    std::string output;     // Stores the final output
    std::ostringstream oss; // Stream for building output
    bool isConsoleOutput;

public:
    // GoldenSectionSearch() = default;
    GoldenSectionSearch(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput) {}
    ~GoldenSectionSearch() = default;

    // Getter for the output
    std::string getOutput() const
    {
        return output;
    }

    // Clear the output (optional, if you want to reset)
    void clearOutput()
    {
        oss.str("");
        output.clear();
    }

    double DoGoldenRatioSearch(const std::string &func, double xLower, double xUpper, double tol, bool findMin = false)
    {
        oss << "\nFunction: " << func << "\n";

        // ExprTk setup
        exprtk::symbol_table<double> symbolTable;
        double x = xLower;
        symbolTable.add_variable("x", x);

        exprtk::expression<double> expression;
        expression.register_symbol_table(symbolTable);

        exprtk::parser<double> parser;
        if (!parser.compile(func, expression))
        {
            throw std::runtime_error("Error compiling expression: " + func);
        }

        const double phi = (1 + std::sqrt(5)) / 2;
        double a = xLower, b = xUpper;
        double c = b - (b - a) / phi;
        double d = a + (b - a) / phi;

        symbolTable.get_variable("x")->ref() = c;
        double fc = expression.value();
        symbolTable.get_variable("x")->ref() = d;
        double fd = expression.value();

        oss << std::fixed << std::setprecision(6);
        oss << "Iteration | xLower        | xUpper        | x1        | x2        | f(x1)     | f(x2)     | e\n";

        int iter = 1;
        double e = std::abs(b - a);
        oss << iter << "         | " << a << " | " << b << " | " << d << " | " << c
            << " | " << fc << " | " << fd << " | " << e << "\n";
        iter++;

        while (e > tol)
        {
            if ((findMin && fc > fd) || (!findMin && fc < fd))
            {
                a = c;
                c = d;
                fc = fd;
                d = a + (b - a) / phi;
                symbolTable.get_variable("x")->ref() = d;
                fd = expression.value();
            }
            else
            {
                b = d;
                d = c;
                fd = fc;
                c = b - (b - a) / phi;
                symbolTable.get_variable("x")->ref() = c;
                fc = expression.value();
            }

            e = std::abs(b - a);
            oss << iter << "         | " << a << " | " << b << " | " << d << " | " << c
                << " | " << fc << " | " << fd << " | " << e << "\n";
            iter++;
        }

        double xOpt = (b + a) / 2;
        symbolTable.get_variable("x")->ref() = xOpt;
        double fOpt = expression.value();

        oss << "\nOptimal x: " << xOpt << "\n";
        // oss << (findMin ? "Min" : "Max") << " f(x): " << fOpt << "\n";
        oss << (findMin ? "Min" : "Max") << " z: " << fOpt << " Radians\n";

        // Store the output in the class member
        output = oss.str();

        if (isConsoleOutput)
        {
            std::cout << output << std::endl;
        }

        return xOpt;
    }

    void Test()
    {
        std::string func = "4*sin(x)*(1+cos(x))";
        double xLower = 0.0;
        double xUpper = 1.570796;
        double tol = 0.05;

        oss << "Finding Maximum:\n";
        DoGoldenRatioSearch(func, xLower, xUpper, tol, false);

        func = "(x)^3-6*x";
        xLower = 0;
        xUpper = 3;
        tol = 0.05;

        oss << "Finding Minimum:\n";
        DoGoldenRatioSearch(func, xLower, xUpper, tol, true);

        // Update output with the final concatenated output
        output = oss.str();
    }
};