#pragma once

#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include "exprtk.hpp"

class GoldenSectionSearch
{
public:
    GoldenSectionSearch() = default;

    double DoGoldenRatioSearch(const std::string &func, double xLower, double xUpper, double tol, bool findMin = false)
    {
        std::cout << "\nFunction: " << func << "\n";

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

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Iteration | a        | b        | d        | c        | f(c)     | f(d)     | e\n";

        int iter = 1;
        double e = std::abs(b - a);
        std::cout << iter << "         | " << a << " | " << b << " | " << d << " | " << c
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
            std::cout << iter << "         | " << a << " | " << b << " | " << d << " | " << c
                      << " | " << fc << " | " << fd << " | " << e << "\n";
            iter++;
        }

        double xOpt = (b + a) / 2;
        symbolTable.get_variable("x")->ref() = xOpt;
        double fOpt = expression.value();

        std::cout << "\nOptimal x: " << xOpt << "\n";
        std::cout << (findMin ? "Min" : "Max") << " f(x): " << fOpt << "\n";

        return xOpt;
    }

    void Test()
    {
        std::string func = "4*sin(x)*(1+cos(x))";
        double xLower = 0.0;
        double xUpper = 1.570796;
        double tol = 0.05;

        std::cout << "Finding Maximum:\n";
        DoGoldenRatioSearch(func, xLower, xUpper, tol, false);


        func = "(x)^3-6*x"; 
        xLower = 0; 
        xUpper = 3; // Pi/2 
        tol = 0.05;

        std::cout << "Finding Minimum:\n";
        DoGoldenRatioSearch(func, xLower, xUpper, tol, true);
    }
};
