#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#include "symbolicc++.h"

#include "exprtk.hpp"
#include "symbolic_eval.hpp"

using Point = std::vector<double>;

// ---------- FunctionParser (exprtk wrapper) ----------
class FunctionParser
{
public:
    FunctionParser(const std::string &exprStr, const std::vector<std::string> &vars)
    {
        for (const auto &v : vars)
        {
            variableMap[v] = 0.0;
            symbol_table.add_variable(v, variableMap[v]);
        }
        symbol_table.add_constants();
        expression.register_symbol_table(symbol_table);
        if (!parser.compile(exprStr, expression))
        {
            throw std::runtime_error(std::string("Failed to parse expression: ") + exprStr);
        }
        this->exprStr = exprStr;
        this->vars = vars;
    }

    double eval(const Point &p)
    {
        if (p.size() != vars.size())
            throw std::runtime_error("Point dimension mismatch in eval()");
        for (size_t i = 0; i < vars.size(); ++i)
            variableMap[vars[i]] = p[i];
        return expression.value();
    }

    const std::string &getExprString() const { return exprStr; }
    const std::vector<std::string> &getVars() const { return vars; }

private:
    std::map<std::string, double> variableMap;
    exprtk::symbol_table<double> symbol_table;
    exprtk::expression<double> expression;
    exprtk::parser<double> parser;
    std::string exprStr;
    std::vector<std::string> vars;
};

// ---------- DetailedSteepestDescentOptimizer class ----------
class DetailedSteepestDescentOptimizer
{
public:
    DetailedSteepestDescentOptimizer(const std::string &functionExpr,
                                     const std::vector<std::string> &variables,
                                     bool maximize = false)
        : parser(functionExpr, variables), maximize(maximize)
    {
        varNames = variables;
        n = varNames.size();
        OGfunctionExpr = functionExpr;
        oss << std::fixed << std::setprecision(6);
    }

    std::string getDetailedOutput() const
    {
        return oss.str();
    }

    double evaluateFunction(const Point &p)
    {
        return cleanFloat(parser.eval(p));
    }

    std::vector<double> evaluateGradient(const Point &p)
    {
        return computeNumericalGradient(p);
    }

    std::vector<std::vector<double>> evaluateHessian(const Point &p)
    {
        return computeNumericalHessian(p);
    }

    std::string checkCriticalPointNature(const Point &p)
    {
        auto H = evaluateHessian(p);
        auto eigs = computeJacobiEigenvalues(H);
        bool allPos = true, allNeg = true;
        for (double e : eigs)
        {
            if (cleanFloat(e) <= 0)
                allPos = false;
            if (cleanFloat(e) >= 0)
                allNeg = false;
        }
        if (allPos)
            return "Local Minimum";
        if (allNeg)
            return "Local Maximum";
        return "Saddle Point";
    }

    Symbolic recursiveExpand(const Symbolic &expr)
    {
        Symbolic result = expr;

        // Only Numbers or Symbols are left as-is
        if (result.type() == typeid(Number<void>) || result.type() == typeid(Symbol))
            return result;

        // Expand the expression using built-in expand
        result = result.expand();

        // Recursively expand each element if it's a matrix (Symbolic++ uses matrices for sums/products)
        if (result.type() == typeid(SymbolicMatrix))
        {
            CastPtr<SymbolicMatrix> m(result);
            int rows = m->rows();
            int cols = m->cols();
            for (int i = 0; i < rows; i++)
                for (int j = 0; j < cols; j++)
                    (*m)[i][j] = recursiveExpand((*m)[i][j]);
            result = *m;
        }

        return result;
    }

    // double detailedStepSizeCalculation(const Point &currentPoint, const std::vector<double> &gradient)
    // {
    //     oss << "    Finding optimal step size 'h':\n";
    //     oss << "    Current point: " << pointToString(currentPoint) << "\n";
    //     oss << "    Gradient vector: " << vecToString(gradient) << "\n";

    //     std::vector<double> direction = gradient;
    //     for (size_t i = 0; i < direction.size(); ++i)
    //         direction[i] = maximize ? direction[i] : -direction[i];
    //     oss << "    Direction vector (for " << (maximize ? "ascent" : "descent") << "): " << vecToString(direction) << "\n";
    //     oss << "    Formula: x_(i+1) = x_i + h * direction\n";

    //     oss << "    New point expressions:\n";
    //     for (size_t i = 0; i < varNames.size(); ++i)
    //     {
    //         oss << "      " << varNames[i] << "_(i+1) = " << currentPoint[i] << " + h * (" << direction[i] << ")\n";
    //     }

    //     try
    //     {
    //         std::vector<std::string> newPointStr;
    //         for (size_t i = 0; i < currentPoint.size(); ++i)
    //         {
    //             std::ostringstream ss;
    //             ss << currentPoint[i] << " + h * (" << direction[i] << ")";
    //             newPointStr.push_back(ss.str());
    //         }

    //         // 1) First pass: collect unique variables in order of appearance
    //         std::vector<char> uniqueVars;
    //         for (size_t i = 0; i < OGfunctionExpr.size(); ++i)
    //         {
    //             char c = OGfunctionExpr[i];
    //             if (c >= 'a' && c <= 'z')
    //             {
    //                 bool found = false;
    //                 for (char u : uniqueVars)
    //                     if (u == c)
    //                     {
    //                         found = true;
    //                         break;
    //                     }
    //                 if (!found)
    //                     uniqueVars.push_back(c);
    //             }
    //         }

    //         // 2) Second pass: substitute using uniqueVars -> newPointStr
    //         std::string funcSub;
    //         for (size_t i = 0; i < OGfunctionExpr.size(); ++i)
    //         {
    //             char c = OGfunctionExpr[i];
    //             if (c >= 'a' && c <= 'z')
    //             {
    //                 size_t idx = 0;
    //                 for (; idx < uniqueVars.size(); ++idx)
    //                     if (uniqueVars[idx] == c)
    //                         break;
    //                 if (idx < newPointStr.size())
    //                     funcSub += "(" + newPointStr[idx] + ")";
    //                 else
    //                     funcSub += c; // fallback if no replacement
    //             }
    //             else
    //             {
    //                 funcSub += c;
    //             }
    //         }

    //         oss << "\nSubstituting new point into function f(x, y)\nOriginal function:\nSubstituting:" << OGfunctionExpr << std::endl;
    //         for (size_t i = 0; i < newPointStr.size(); i++)
    //         {
    //             oss << "  " << newPointStr[i] << std::endl;
    //         }
    //         oss << "f(x_(i+1)) = " << funcSub << std::endl;

    //         funcSub += ";";

    //         Symbolic gh = SymbolicCpp::evaluateformula(funcSub);

    //         Symbolic h("h");

    //         Symbolic dgdh = df(gh, h); // derivative dg/dh

    //         Equations sol = solve(dgdh, h);

    //         oss << "g(h) (expanded) = " << gh << std::endl;
    //         oss << "\nTaking derivative with respect to h:\ndg/dh = " << dgdh << std::endl;
    //         oss << "Setting dg/dh = 0 to find optimal h:\nSolving: " << dgdh << " = 0" << std::endl;
    //         oss << "h = " << sol.back().rhs << std::endl;

    //         if (fabs(cleanFloat(sol.back().rhs)) < 1e-8)
    //         {
    //             return 0.0;
    //         }

    //         return cleanFloat(sol.back().rhs);
    //     }
    //     catch (...)
    //     {
    //         oss << "Symbolic expression could not be evaluated falling back to golden section search" << std::endl;
    //     }

    //     double hGoldRatio = performGoldenSectionSearch(currentPoint, direction, -2.0, 2.0, 1e-8);
    //     oss << "    Selected step size h = " << hGoldRatio << "\n";
    //     return cleanFloat(hGoldRatio);
    // }

double detailedStepSizeCalculation(const Point &currentPoint, const std::vector<double> &gradient)
{
    oss << "    Finding optimal step size 'h':\n";
    oss << "    Current point: " << pointToString(currentPoint) << "\n";
    oss << "    Gradient vector: " << vecToString(gradient) << "\n";

    std::vector<double> direction = gradient;
    for (size_t i = 0; i < direction.size(); ++i)
        direction[i] = maximize ? direction[i] : -direction[i];
    oss << "    Direction vector (for " << (maximize ? "ascent" : "descent") << "): " << vecToString(direction) << "\n";
    oss << "    Formula: x_(i+1) = x_i + h * direction\n";

    oss << "    New point expressions:\n";
    for (size_t i = 0; i < varNames.size(); ++i)
    {
        oss << "      " << varNames[i] << "_(i+1) = " << currentPoint[i] << " + h * (" << direction[i] << ")\n";
    }

    try
    {
        std::vector<std::string> newPointStr;
        for (size_t i = 0; i < currentPoint.size(); ++i)
        {
            std::ostringstream ss;
            ss << currentPoint[i] << " + h * (" << direction[i] << ")";
            newPointStr.push_back(ss.str());
        }

        // 1) First pass: collect unique variables in order of appearance, excluding 'e'
        std::vector<char> uniqueVars;
        for (size_t i = 0; i < OGfunctionExpr.size(); ++i)
        {
            char c = OGfunctionExpr[i];
            if (c >= 'a' && c <= 'z' && c != 'e') // Exclude 'e' from variable substitution
            {
                bool found = false;
                for (char u : uniqueVars)
                    if (u == c)
                    {
                        found = true;
                        break;
                    }
                if (!found)
                    uniqueVars.push_back(c);
            }
        }

        // 2) Second pass: substitute using uniqueVars -> newPointStr
        std::string funcSub;
        for (size_t i = 0; i < OGfunctionExpr.size(); ++i)
        {
            char c = OGfunctionExpr[i];
            if (c >= 'a' && c <= 'z' && c != 'e') // Substitute only non-'e' variables
            {
                size_t idx = 0;
                for (; idx < uniqueVars.size(); ++idx)
                    if (uniqueVars[idx] == c)
                        break;
                if (idx < newPointStr.size())
                    funcSub += "(" + newPointStr[idx] + ")";
                else
                    funcSub += c; // fallback if no replacement
            }
            else
            {
                funcSub += c; // Keep 'e' and other characters unchanged
            }
        }

        oss << "\nSubstituting new point into function f(x, y)\nOriginal function: " << OGfunctionExpr << std::endl;
        for (size_t i = 0; i < newPointStr.size(); i++)
        {
            oss << "  " << newPointStr[i] << std::endl;
        }
        oss << "f(x_(i+1)) = " << funcSub << std::endl;

        funcSub += ";";

        Symbolic gh = SymbolicCpp::evaluateformula(funcSub);

        Symbolic h("h");

        Symbolic dgdh = df(gh, h); // derivative dg/dh

        Equations sol = solve(dgdh, h);

        oss << "g(h) (expanded) = " << gh << std::endl;
        oss << "\nTaking derivative with respect to h:\ndg/dh = " << dgdh << std::endl;
        oss << "Setting dg/dh = 0 to find optimal h:\nSolving: " << dgdh << " = 0" << std::endl;
        oss << "h = " << sol.back().rhs << std::endl;

        // Convert symbolic expression to string
        std::ostringstream h_stream;
        h_stream << sol.back().rhs;
        std::string h_expr = h_stream.str();

        // Replace 'e' with '2.718281828' in the string, ensuring it's not part of another word
        std::string substituted_h_expr;
        size_t pos = 0;
        while (pos < h_expr.size())
        {
            if (h_expr[pos] == 'e' && 
                (pos == 0 || !isalnum(h_expr[pos - 1])) && // Ensure 'e' is not part of a word
                (pos + 1 == h_expr.size() || !isalnum(h_expr[pos + 1])))
            {
                substituted_h_expr += "2.718281828";
                pos++;
            }
            else
            {
                substituted_h_expr += h_expr[pos];
                pos++;
            }
        }

        // oss << "h (after replacing e with 2.718281828) = " << substituted_h_expr << std::endl;

        // Create an lvalue for the expression to avoid rvalue issues
        std::string substituted_h_expr_with_semicolon = substituted_h_expr + ";";

        // Re-parse the substituted expression into a Symbolic object
        Symbolic substituted_h = SymbolicCpp::evaluateformula(substituted_h_expr_with_semicolon);

        double h_value = cleanFloat(substituted_h);

        if (fabs(h_value) < 1e-8)
        {
            oss << "    Step size h is near zero (" << h_value << "), returning 0.0\n";
            return 0.0;
        }

        oss << "    Selected step size h = " << h_value << "\n";
        return h_value;
    }
    catch (const std::exception& ex)
    {
        oss << "Symbolic expression could not be evaluated: " << ex.what() << "\nFalling back to golden section search\n";
    }
    catch (...)
    {
        oss << "Symbolic expression could not be evaluated (unknown error). Falling back to golden section search\n";
    }

    double hGoldRatio = performGoldenSectionSearch(currentPoint, direction, -2.0, 2.0, 1e-8);
    oss << "    Selected step size h = " << hGoldRatio << "\n";
    return cleanFloat(hGoldRatio);
}

    std::tuple<Point, double, std::vector<std::map<std::string, std::string>>> optimize(const Point &initialPoint, int maxIterations = 100, double tolerance = 1e-6)
    {
        Point currentPoint = initialPoint;
        std::vector<std::map<std::string, std::string>> history;

        std::string method = maximize ? "Steepest Ascent" : "Steepest Descent";
        oss << "\n"
            << std::string(80, '=') << "\n";
        oss << method << " Algorithm - Detailed Steps\n";
        oss << std::string(80, '=') << "\n";
        oss << "Function: f(";
        for (size_t i = 0; i < varNames.size(); ++i)
        {
            oss << varNames[i] << (i + 1 == varNames.size() ? "" : ", ");
        }
        oss << ") = " << parser.getExprString() << "\n";
        oss << "Objective: Find " << (maximize ? "maximum" : "minimum") << "\n";
        oss << "Initial point: " << pointToString(currentPoint) << "\n";
        oss << "Initial function value: f" << pointToString(currentPoint) << " = " << evaluateFunction(currentPoint) << "\n\n";
        oss << "Gradient/Hessian will be computed numerically (central differences).\n\n";
        oss << std::string(80, '-') << "\n";

        for (int iteration = 0; iteration < maxIterations; ++iteration)
        {
            oss << "\n ITERATION " << (iteration + 1) << ":\n";
            oss << std::string(40, '=') << "\n";

            std::vector<double> grad = evaluateGradient(currentPoint);
            double gradNorm = 0.0;
            for (double g : grad)
                gradNorm += g * g;
            gradNorm = cleanFloat(sqrt(gradNorm));

            oss << "Step 1: Calculate gradient at current point " << pointToString(currentPoint) << "\n";
            for (size_t i = 0; i < grad.size(); ++i)
            {
                oss << "  d/d" << varNames[i] << " ~ (f(";
                oss << varNames[i] << " +/- eps) -> approx = " << grad[i] << "\n";
            }
            oss << "  Gradient vector: ∇f = " << vecToString(grad) << "\n";
            oss << "  Gradient norm: ||∇f|| = " << gradNorm << "\n";

            std::map<std::string, std::string> iterInfo;
            iterInfo["iteration"] = std::to_string(iteration + 1);
            iterInfo["point"] = pointToString(currentPoint);
            iterInfo["functionValue"] = std::to_string(evaluateFunction(currentPoint));
            iterInfo["gradient"] = vecToString(grad);
            iterInfo["gradientNorm"] = std::to_string(gradNorm);

            if (gradNorm < tolerance)
            {
                oss << "\n CONVERGED! Gradient norm " << gradNorm << " < tolerance " << tolerance << "\n";
                oss << "The gradient is approximately zero, indicating we're at a critical point.\n";
                history.push_back(iterInfo);
                break;
            }

            oss << "\nStep 2: Find optimal step size\n";
            double stepSize = detailedStepSizeCalculation(currentPoint, grad);
            iterInfo["stepSize"] = std::to_string(stepSize);

            std::vector<double> direction = grad;
            for (size_t i = 0; i < direction.size(); ++i)
                direction[i] = maximize ? direction[i] : -direction[i];

            Point newPoint = currentPoint;
            for (size_t i = 0; i < newPoint.size(); ++i)
                newPoint[i] = cleanFloat(currentPoint[i] + stepSize * direction[i]);

            oss << "\nStep 3: Update point using optimal step size\n";
            oss << "  Step size h = " << stepSize << "\n";
            oss << "  Direction vector: " << vecToString(direction) << "\n";
            oss << "  New point calculation:\n";
            for (size_t i = 0; i < varNames.size(); ++i)
            {
                oss << "    " << varNames[i] << "_(new) = " << currentPoint[i]
                    << " + (" << stepSize << ") * (" << direction[i] << ") = "
                    << newPoint[i] << "\n";
            }
            double oldValue = evaluateFunction(currentPoint);
            double newValue = evaluateFunction(newPoint);
            oss << "\nStep 4: Verify improvement\n";
            oss << "  f(oldPoint) = f" << pointToString(currentPoint) << " = " << oldValue << "\n";
            oss << "  f(newPoint) = f" << pointToString(newPoint) << " = " << newValue << "\n";
            if (maximize)
            {
                oss << "  " << (newValue > oldValue ? " Better: " : " Worse: ") << newValue << (newValue > oldValue ? " > " : " <= ") << oldValue << "\n";
            }
            else
            {
                oss << "  " << (newValue < oldValue ? " Better: " : " Worse: ") << newValue << (newValue < oldValue ? " < " : " >= ") << oldValue << "\n";
            }

            currentPoint = newPoint;
            history.push_back(iterInfo);
        }

        double optimalValue = evaluateFunction(currentPoint);

        oss << "\n"
            << std::string(80, '=') << "\n";
        oss << " FINAL RESULTS\n";
        oss << std::string(80, '=') << "\n";
        std::ostringstream fp;
        fp << std::fixed << std::setprecision(6);
        for (size_t i = 0; i < currentPoint.size(); ++i)
        {
            fp << varNames[i] << "=" << cleanFloat(currentPoint[i]);
            if (i + 1 < currentPoint.size())
                fp << ", ";
        }
        oss << "Optimal point: (" << fp.str() << ")\n";
        oss << "Optimal value: " << std::fixed << std::setprecision(6) << cleanFloat(optimalValue) << "\n\n";

        oss << "Hessian Analysis:\n";
        oss << "Hessian (numeric, central differences):\n";
        auto H = evaluateHessian(currentPoint);
        for (size_t i = 0; i < H.size(); ++i)
        {
            oss << "  [ ";
            for (size_t j = 0; j < H.size(); ++j)
            {
                oss << std::setw(10) << cleanFloat(H[i][j]) << (j + 1 == H.size() ? " " : ", ");
            }
            oss << "]\n";
        }
        auto eigs = computeJacobiEigenvalues(H);
        oss << "Eigenvalues: " << vecToString(eigs) << "\n";
        std::string nature = checkCriticalPointNature(currentPoint);
        oss << "Nature of critical point: " << nature << "\n";
        if (eigs.size() == 2)
        {
            double det = cleanFloat(H[0][0] * H[1][1] - H[0][1] * H[1][0]);
            double tr = cleanFloat(H[0][0] + H[1][1]);
            oss << "Determinant: " << det << "\n";
            oss << "Trace: " << tr << "\n";
            if (det > 0 && tr > 0)
                oss << " det(H) > 0 and tr(H) > 0 -> Local Minimum\n";
            else if (det > 0 && tr < 0)
                oss << " det(H) > 0 and tr(H) < 0 -> Local Maximum\n";
            else if (det < 0)
                oss << " det(H) < 0 -> Saddle Point\n";
            else
                oss << " Inconclusive test\n";
        }

        return std::make_tuple(currentPoint, optimalValue, history);
    }

private:
    static double cleanFloat(double x, double threshold = 1e-10)
    {
        return (fabs(x) < threshold) ? 0.0 : x;
    }

    static std::string pointToString(const Point &p, int prec = 6)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(prec) << "(";
        for (size_t i = 0; i < p.size(); ++i)
        {
            ss << cleanFloat(p[i]);
            if (i + 1 < p.size())
                ss << ", ";
        }
        ss << ")";
        return ss.str();
    }

    static std::string vecToString(const std::vector<double> &v, int prec = 6)
    {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(prec) << "[";
        for (size_t i = 0; i < v.size(); ++i)
        {
            ss << cleanFloat(v[i]);
            if (i + 1 < v.size())
                ss << ", ";
        }
        ss << "]";
        return ss.str();
    }

    std::vector<double> computeNumericalGradient(const Point &x, double eps = 1e-5)
    {
        size_t n = x.size();
        std::vector<double> grad(n, 0.0);
        Point x1 = x, x2 = x;
        for (size_t i = 0; i < n; ++i)
        {
            x1[i] = x[i] + eps;
            x2[i] = x[i] - eps;
            double f1 = parser.eval(x1);
            double f2 = parser.eval(x2);
            grad[i] = cleanFloat((f1 - f2) / (2.0 * eps));
            x1[i] = x[i];
            x2[i] = x[i];
        }
        return grad;
    }

    std::vector<std::vector<double>> computeNumericalHessian(const Point &x, double eps = 1e-4)
    {
        size_t n = x.size();
        std::vector<std::vector<double>> H(n, std::vector<double>(n, 0.0));
        Point xp = x, xm = x;
        // diagonal second derivatives
        for (size_t i = 0; i < n; ++i)
        {
            xp[i] = x[i] + eps;
            xm[i] = x[i] - eps;
            double fpp = parser.eval(xp);
            double fmm = parser.eval(xm);
            double f0 = parser.eval(x);
            H[i][i] = cleanFloat((fpp - 2.0 * f0 + fmm) / (eps * eps));
            xp[i] = x[i];
            xm[i] = x[i];
        }
        // off-diagonals
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = i + 1; j < n; ++j)
            {
                Point xpp = x, xpm = x, xmp = x, xmm = x;
                xpp[i] += eps;
                xpp[j] += eps;
                xpm[i] += eps;
                xpm[j] -= eps;
                xmp[i] -= eps;
                xmp[j] += eps;
                xmm[i] -= eps;
                xmm[j] -= eps;
                double fpp = parser.eval(xpp);
                double fpm = parser.eval(xpm);
                double fmp = parser.eval(xmp);
                double fmm = parser.eval(xmm);
                double val = cleanFloat((fpp - fpm - fmp + fmm) / (4.0 * eps * eps));
                H[i][j] = val;
                H[j][i] = val;
            }
        }
        return H;
    }

    static std::vector<double> computeJacobiEigenvalues(std::vector<std::vector<double>> A, int maxIter = 100, double tol = 1e-10)
    {
        size_t n = A.size();
        std::vector<std::vector<double>> V(n, std::vector<double>(n, 0.0));
        for (size_t i = 0; i < n; ++i)
            V[i][i] = 1.0;

        auto maxOffDiag = [&](size_t &p, size_t &q)
        {
            double maxVal = 0.0;
            p = 0;
            q = 1;
            for (size_t i = 0; i < n; ++i)
            {
                for (size_t j = i + 1; j < n; ++j)
                {
                    double v = fabs(cleanFloat(A[i][j]));
                    if (v > maxVal)
                    {
                        maxVal = v;
                        p = i;
                        q = j;
                    }
                }
            }
            return maxVal;
        };

        for (int iter = 0; iter < maxIter; ++iter)
        {
            size_t p, q;
            double maxVal = maxOffDiag(p, q);
            if (maxVal < tol)
                break;

            double app = A[p][p];
            double aqq = A[q][q];
            double apq = A[p][q];

            double phi = 0.5 * atan2(2.0 * apq, (aqq - app));
            double c = cos(phi), s = sin(phi);

            // rotate A
            for (size_t i = 0; i < n; ++i)
            {
                if (i != p && i != q)
                {
                    double aip = A[i][p];
                    double aiq = A[i][q];
                    A[i][p] = cleanFloat(aip * c - aiq * s);
                    A[p][i] = A[i][p];
                    A[i][q] = cleanFloat(aip * s + aiq * c);
                    A[q][i] = A[i][q];
                }
            }
            double new_pp = cleanFloat(c * c * app - 2.0 * s * c * apq + s * s * aqq);
            double new_qq = cleanFloat(s * s * app + 2.0 * s * c * apq + c * c * aqq);
            A[p][p] = new_pp;
            A[q][q] = new_qq;
            A[p][q] = 0.0;
            A[q][p] = 0.0;

            // update eigenvector matrix V
            for (size_t i = 0; i < n; ++i)
            {
                double vip = V[i][p], viq = V[i][q];
                V[i][p] = cleanFloat(vip * c - viq * s);
                V[i][q] = cleanFloat(vip * s + viq * c);
            }
        }

        std::vector<double> evals(n);
        for (size_t i = 0; i < n; ++i)
            evals[i] = cleanFloat(A[i][i]);
        return evals;
    }

    double performGoldenSectionSearch(const Point &currentPoint,
                                      const std::vector<double> &direction,
                                      double a = -2.0, double b = 2.0,
                                      double tol = 1e-8)
    {
        const double phi = (1.0 + sqrt(5.0)) / 2.0;
        const double resPhi = 2.0 - phi;

        auto obj = [&](double h) -> double
        {
            Point newP(currentPoint.size());
            for (size_t i = 0; i < currentPoint.size(); ++i)
                newP[i] = cleanFloat(currentPoint[i] + h * direction[i]);
            double v = parser.eval(newP);
            return cleanFloat(maximize ? -v : v);
        };

        double x1 = cleanFloat(a + resPhi * (b - a));
        double x2 = cleanFloat(a + (1.0 - resPhi) * (b - a));
        double f1 = cleanFloat(obj(x1)), f2 = cleanFloat(obj(x2));

        int iter = 0;
        oss << "    Golden-section search for h in [" << a << ", " << b << "], tol=" << tol << "\n";
        oss << "      initial: x1=" << x1 << " f1=" << f1 << " | x2=" << x2 << " f2=" << f2 << "\n";

        while (fabs(b - a) > tol)
        {
            if (f1 < f2)
            {
                b = x2;
                x2 = x1;
                f2 = f1;
                x1 = cleanFloat(a + resPhi * (b - a));
                f1 = cleanFloat(obj(x1));
            }
            else
            {
                a = x1;
                x1 = x2;
                f1 = f2;
                x2 = cleanFloat(a + (1.0 - resPhi) * (b - a));
                f2 = cleanFloat(obj(x2));
            }
            ++iter;
            oss << "      iter " << std::setw(2) << iter << ": a=" << a << " b=" << b
                << " x1=" << x1 << " f1=" << f1 << " x2=" << x2 << " f2=" << f2 << "\n";
            if (iter > 10000)
                break; // safety
        }
        double hOpt = cleanFloat(0.5 * (a + b));
        oss << "    Golden search done. h_opt ~ " << hOpt << "\n";
        return hOpt;
    }

private:
    FunctionParser parser;
    std::vector<std::string> varNames;
    bool maximize;
    size_t n;
    std::string OGfunctionExpr;
    mutable std::ostringstream oss;
};

class SteepestDescent
{
public:
    SteepestDescent(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput) {}
    ~SteepestDescent() {}

    void DoSteepestDescent(const std::string &exprStr, const std::vector<std::string> &vars, const Point &initialPoint, bool maximize = false)
    {
        DetailedSteepestDescentOptimizer optimizer(exprStr, vars, maximize);
        optimizer.optimize(initialPoint, 100, 1e-5);
        output = optimizer.getDetailedOutput();

        if (isConsoleOutput)
        {
            std::cout << output;
        }
    }

    const std::string &getOutput() const
    {
        return output;
    }

    void test()
    {
        DoSteepestDescent("2*x*y + 4*x - 2*x^2 - y^2", {"x", "y"}, {0.5, 0.5}, true);
        // DoSteepestDescent("x^2 + y^2 + 2*x + 4", {"x", "y"}, {2, 1}, false);
        // DoSteepestDescent("x^2 + y^2 + 2*x + 4 + z", {"x", "y", "z"}, {2, 1, 3}, false);
    }

private:
    std::string output;

    bool isConsoleOutput;
};