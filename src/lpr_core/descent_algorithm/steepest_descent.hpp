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

using Point = vector<double>;

// ---------- Utility to handle floating-point noise ----------
static double cleanFloat(double x, double threshold = 1e-10)
{
    return (fabs(x) < threshold) ? 0.0 : x;
}

// ---------- FunctionParser (exprtk wrapper) ----------
class FunctionParser
{
public:
    FunctionParser(const string &exprStr, const vector<string> &vars)
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
            throw runtime_error(string("Failed to parse expression: ") + exprStr);
        }
        this->exprStr = exprStr;
        this->vars = vars;
    }

    double eval(const Point &p)
    {
        if (p.size() != vars.size())
            throw runtime_error("Point dimension mismatch in eval()");
        for (size_t i = 0; i < vars.size(); ++i)
            variableMap[vars[i]] = p[i];
        return cleanFloat(expression.value());
    }

    const string &getExprString() const { return exprStr; }
    const vector<string> &getVars() const { return vars; }

private:
    map<string, double> variableMap;
    exprtk::symbol_table<double> symbol_table;
    exprtk::expression<double> expression;
    exprtk::parser<double> parser;
    string exprStr;
    vector<string> vars;
};

// ---------- Utility formatting ----------
static string pointToString(const Point &p, int prec = 6)
{
    ostringstream oss;
    oss << fixed << setprecision(prec) << "(";
    for (size_t i = 0; i < p.size(); ++i)
    {
        oss << cleanFloat(p[i]);
        if (i + 1 < p.size())
            oss << ", ";
    }
    oss << ")";
    return oss.str();
}

static string vecToString(const vector<double> &v, int prec = 6)
{
    ostringstream oss;
    oss << fixed << setprecision(prec) << "[";
    for (size_t i = 0; i < v.size(); ++i)
    {
        oss << cleanFloat(v[i]);
        if (i + 1 < v.size())
            oss << ", ";
    }
    oss << "]";
    return oss.str();
}

// ---------- Numerical derivatives ----------
static vector<double> numericalGradient(FunctionParser &f, const Point &x, double eps = 1e-5)
{
    size_t n = x.size();
    vector<double> grad(n, 0.0);
    Point x1 = x, x2 = x;
    for (size_t i = 0; i < n; ++i)
    {
        x1[i] = x[i] + eps;
        x2[i] = x[i] - eps;
        double f1 = f.eval(x1);
        double f2 = f.eval(x2);
        grad[i] = cleanFloat((f1 - f2) / (2.0 * eps));
        x1[i] = x[i];
        x2[i] = x[i];
    }
    return grad;
}

static vector<vector<double>> numericalHessian(FunctionParser &f, const Point &x, double eps = 1e-4)
{
    size_t n = x.size();
    vector<vector<double>> H(n, vector<double>(n, 0.0));
    Point xp = x, xm = x;
    // diagonal second derivatives
    for (size_t i = 0; i < n; ++i)
    {
        xp[i] = x[i] + eps;
        xm[i] = x[i] - eps;
        double fpp = f.eval(xp);
        double fmm = f.eval(xm);
        double f0 = f.eval(x);
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
            double fpp = f.eval(xpp);
            double fpm = f.eval(xpm);
            double fmp = f.eval(xmp);
            double fmm = f.eval(xmm);
            double val = cleanFloat((fpp - fpm - fmp + fmm) / (4.0 * eps * eps));
            H[i][j] = val;
            H[j][i] = val;
        }
    }
    return H;
}

// ---------- Jacobi eigenvalue algorithm for symmetric matrices ----------
static vector<double> jacobiEigenvalues(vector<vector<double>> A, int maxIter = 100, double tol = 1e-10)
{
    size_t n = A.size();
    vector<vector<double>> V(n, vector<double>(n, 0.0));
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

    vector<double> evals(n);
    for (size_t i = 0; i < n; ++i)
        evals[i] = cleanFloat(A[i][i]);
    return evals;
}

// ---------- Golden section search with verbose steps (numeric) ----------
static double goldenSectionSearchDetailed(FunctionParser &f,
                                          const Point &currentPoint,
                                          const vector<double> &direction,
                                          bool maximize,
                                          double a = -2.0, double b = 2.0,
                                          double tol = 1e-8,
                                          bool verbose = true,
                                          int maxStepsToPrint = 20)
{
    const double phi = (1.0 + sqrt(5.0)) / 2.0;
    const double resPhi = 2.0 - phi;

    auto obj = [&](double h) -> double
    {
        Point newP(currentPoint.size());
        for (size_t i = 0; i < currentPoint.size(); ++i)
            newP[i] = cleanFloat(currentPoint[i] + h * direction[i]);
        double v = f.eval(newP);
        return cleanFloat(maximize ? -v : v);
    };

    double x1 = cleanFloat(a + resPhi * (b - a));
    double x2 = cleanFloat(a + (1.0 - resPhi) * (b - a));
    double f1 = cleanFloat(obj(x1)), f2 = cleanFloat(obj(x2));

    int iter = 0;
    if (verbose)
    {
        std::cout << "    Golden-section search for h in [" << a << ", " << b << "], tol=" << tol << "\n";
        std::cout << "      initial: x1=" << x1 << " f1=" << f1 << " | x2=" << x2 << " f2=" << f2 << "\n";
    }

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
        if (verbose && iter <= maxStepsToPrint)
        {
            std::cout << "      iter " << setw(2) << iter << ": a=" << a << " b=" << b
                      << " x1=" << x1 << " f1=" << f1 << " x2=" << x2 << " f2=" << f2 << "\n";
        }
        if (iter > 10000)
            break; // safety
    }
    double hOpt = cleanFloat(0.5 * (a + b));
    if (verbose)
        std::cout << "    Golden search done. h_opt ~ " << hOpt << "\n";
    return hOpt;
}

// ---------- DetailedSteepestDescentOptimizer class ----------
class DetailedSteepestDescentOptimizer
{
public:
    DetailedSteepestDescentOptimizer(const string &functionExpr,
                                     const vector<string> &variables,
                                     bool maximize = false)
        : parser(functionExpr, variables), maximize(maximize)
    {
        varNames = variables;
        n = varNames.size();
        OGfunctionExpr = functionExpr;
    }

    double evaluateFunction(const Point &p)
    {
        return cleanFloat(parser.eval(p));
    }

    vector<double> evaluateGradient(const Point &p)
    {
        return numericalGradient(parser, p);
    }

    vector<vector<double>> evaluateHessian(const Point &p)
    {
        return numericalHessian(parser, p);
    }

    string checkCriticalPointNature(const Point &p)
    {
        auto H = evaluateHessian(p);
        auto eigs = jacobiEigenvalues(H);
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

    double detailedStepSizeCalculation(const Point &currentPoint, const vector<double> &gradient, bool verbose = true)
    {
        if (verbose)
        {
            std::cout << "    Finding optimal step size 'h':\n";
            std::cout << "    Current point: " << pointToString(currentPoint) << "\n";
            std::cout << "    Gradient vector: " << vecToString(gradient) << "\n";
        }

        vector<double> direction = gradient;
        for (size_t i = 0; i < direction.size(); ++i)
            direction[i] = maximize ? direction[i] : -direction[i];
        if (verbose)
        {
            std::cout << "    Direction vector (for " << (maximize ? "ascent" : "descent") << "): " << vecToString(direction) << "\n";
            std::cout << "    Formula: x_(i+1) = x_i + h * direction\n";
        }

        if (verbose)
        {
            std::cout << "    New point expressions:\n";
            for (size_t i = 0; i < varNames.size(); ++i)
            {
                std::cout << "      " << varNames[i] << "_(i+1) = " << currentPoint[i] << " + h * (" << direction[i] << ")\n";
            }
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

            // 1) First pass: collect unique variables in order of appearance
            std::vector<char> uniqueVars;
            for (size_t i = 0; i < OGfunctionExpr.size(); ++i)
            {
                char c = OGfunctionExpr[i];
                if (c >= 'a' && c <= 'z')
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
                if (c >= 'a' && c <= 'z')
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
                    funcSub += c;
                }
            }

            if (verbose)
            {
                std::cout << "Substituting new point into function f(x, y)\nOriginal function:" << OGfunctionExpr << std::endl;
                for (size_t i = 0; i < newPointStr.size(); i++)
                {
                    std::cout << "  " << newPointStr[i] << std::endl;
                }
                std::cout << "f(x_(i+1)) = " << funcSub << std::endl;
            }

            funcSub += ";";

            Symbolic gh = SymbolicCpp::evaluateformula(funcSub);

            Symbolic h("h");

            Symbolic dgdh = df(gh, h); // derivative dg/dh

            Equations sol = solve(dgdh, h);

            if (verbose)
            {
                std::cout << "g(h) (expanded) = " << gh << std::endl;
                std::cout << "\nTaking derivative with respect to h:\ndg/dh = " << dgdh << std::endl;
                std::cout << "Setting dg/dh = 0 to find optimal h:\nSolving: " << dgdh << " = 0" << std::endl;
                std::cout << "h = " << sol.back().rhs << std::endl;
            }

            if (fabs(cleanFloat(sol.back().rhs)) < 1e-8)
            {
                return 0.0;
            }

            return cleanFloat(sol.back().rhs);
        }
        catch (...)
        {
            std::cout << "Symbolic expression could not be evaluated falling back to golden section search" << std::endl;
        }

        double hGoldRatio = goldenSectionSearchDetailed(parser, currentPoint, direction, maximize, -2.0, 2.0, 1e-8, verbose);
        if (verbose)
            std::cout << "    Selected step size h = " << hGoldRatio << "\n";
        return cleanFloat(hGoldRatio);
    }

    tuple<Point, double, vector<map<string, string>>> optimize(const Point &initialPoint, int maxIterations = 100, double tolerance = 1e-6, bool verbose = true)
    {
        Point currentPoint = initialPoint;
        vector<map<string, string>> history;

        if (verbose)
        {
            string method = maximize ? "Steepest Ascent" : "Steepest Descent";
            std::cout << "\n"
                      << string(80, '=') << "\n";
            std::cout << method << " Algorithm - Detailed Steps\n";
            std::cout << string(80, '=') << "\n";
            std::cout << "Function: f(";
            for (size_t i = 0; i < varNames.size(); ++i)
            {
                std::cout << varNames[i] << (i + 1 == varNames.size() ? "" : ", ");
            }
            std::cout << ") = " << parser.getExprString() << "\n";
            std::cout << "Objective: Find " << (maximize ? "maximum" : "minimum") << "\n";
            std::cout << "Initial point: " << pointToString(currentPoint) << "\n";
            std::cout << "Initial function value: f" << pointToString(currentPoint) << " = " << evaluateFunction(currentPoint) << "\n\n";
            std::cout << "Gradient/Hessian will be computed numerically (central differences).\n\n";
            std::cout << string(80, '-') << "\n";
        }

        for (int iteration = 0; iteration < maxIterations; ++iteration)
        {
            if (verbose)
            {
                std::cout << "\n ITERATION " << (iteration + 1) << ":\n";
                std::cout << string(40, '=') << "\n";
            }

            vector<double> grad = evaluateGradient(currentPoint);
            double gradNorm = 0.0;
            for (double g : grad)
                gradNorm += g * g;
            gradNorm = cleanFloat(sqrt(gradNorm));

            if (verbose)
            {
                std::cout << "Step 1: Calculate gradient at current point " << pointToString(currentPoint) << "\n";
                for (size_t i = 0; i < grad.size(); ++i)
                {
                    std::cout << "  d/d" << varNames[i] << " ~ (f(";
                    std::cout << varNames[i] << " +/- eps) -> approx = " << grad[i] << "\n";
                }
                std::cout << "  Gradient vector: ∇f = " << vecToString(grad) << "\n";
                std::cout << "  Gradient norm: ||∇f|| = " << gradNorm << "\n";
            }

            map<string, string> iterInfo;
            iterInfo["iteration"] = to_string(iteration + 1);
            iterInfo["point"] = pointToString(currentPoint);
            iterInfo["functionValue"] = to_string(evaluateFunction(currentPoint));
            iterInfo["gradient"] = vecToString(grad);
            iterInfo["gradientNorm"] = to_string(gradNorm);

            if (gradNorm < tolerance)
            {
                if (verbose)
                {
                    std::cout << "\n CONVERGED! Gradient norm " << gradNorm << " < tolerance " << tolerance << "\n";
                    std::cout << "The gradient is approximately zero, indicating we're at a critical point.\n";
                }
                history.push_back(iterInfo);
                break;
            }

            if (verbose)
                std::cout << "\nStep 2: Find optimal step size\n";
            double stepSize = detailedStepSizeCalculation(currentPoint, grad, verbose);
            iterInfo["stepSize"] = to_string(stepSize);

            vector<double> direction = grad;
            for (size_t i = 0; i < direction.size(); ++i)
                direction[i] = maximize ? direction[i] : -direction[i];

            Point newPoint = currentPoint;
            for (size_t i = 0; i < newPoint.size(); ++i)
                newPoint[i] = cleanFloat(currentPoint[i] + stepSize * direction[i]);

            if (verbose)
            {
                std::cout << "\nStep 3: Update point using optimal step size\n";
                std::cout << "  Step size h = " << stepSize << "\n";
                std::cout << "  Direction vector: " << vecToString(direction) << "\n";
                std::cout << "  New point calculation:\n";
                for (size_t i = 0; i < varNames.size(); ++i)
                {
                    std::cout << "    " << varNames[i] << "_(new) = " << currentPoint[i]
                              << " + (" << stepSize << ") * (" << direction[i] << ") = "
                              << newPoint[i] << "\n";
                }
                double oldValue = evaluateFunction(currentPoint);
                double newValue = evaluateFunction(newPoint);
                std::cout << "\nStep 4: Verify improvement\n";
                std::cout << "  f(oldPoint) = f" << pointToString(currentPoint) << " = " << oldValue << "\n";
                std::cout << "  f(newPoint) = f" << pointToString(newPoint) << " = " << newValue << "\n";
                if (maximize)
                {
                    std::cout << "  " << (newValue > oldValue ? " Better: " : " Worse: ") << newValue << (newValue > oldValue ? " > " : " <= ") << oldValue << "\n";
                }
                else
                {
                    std::cout << "  " << (newValue < oldValue ? " Better: " : " Worse: ") << newValue << (newValue < oldValue ? " < " : " >= ") << oldValue << "\n";
                }
            }

            currentPoint = newPoint;
            history.push_back(iterInfo);
        }

        double optimalValue = evaluateFunction(currentPoint);

        if (verbose)
        {
            std::cout << "\n"
                      << string(80, '=') << "\n";
            std::cout << " FINAL RESULTS\n";
            std::cout << string(80, '=') << "\n";
            ostringstream fp;
            fp << fixed << setprecision(6);
            for (size_t i = 0; i < currentPoint.size(); ++i)
            {
                fp << varNames[i] << "=" << cleanFloat(currentPoint[i]);
                if (i + 1 < currentPoint.size())
                    fp << ", ";
            }
            std::cout << "Optimal point: (" << fp.str() << ")\n";
            std::cout << "Optimal value: " << fixed << setprecision(6) << cleanFloat(optimalValue) << "\n\n";

            std::cout << "Hessian Analysis:\n";
            std::cout << "Hessian (numeric, central differences):\n";
            auto H = evaluateHessian(currentPoint);
            std::cout << fixed << setprecision(6);
            for (size_t i = 0; i < H.size(); ++i)
            {
                std::cout << "  [ ";
                for (size_t j = 0; j < H.size(); ++j)
                {
                    std::cout << setw(10) << cleanFloat(H[i][j]) << (j + 1 == H.size() ? " " : ", ");
                }
                std::cout << "]\n";
            }
            auto eigs = jacobiEigenvalues(H);
            std::cout << "Eigenvalues: " << vecToString(eigs) << "\n";
            string nature = checkCriticalPointNature(currentPoint);
            std::cout << "Nature of critical point: " << nature << "\n";
            if (eigs.size() == 2)
            {
                double det = cleanFloat(H[0][0] * H[1][1] - H[0][1] * H[1][0]);
                double tr = cleanFloat(H[0][0] + H[1][1]);
                std::cout << "Determinant: " << det << "\n";
                std::cout << "Trace: " << tr << "\n";
                if (det > 0 && tr > 0)
                    std::cout << " det(H) > 0 and tr(H) > 0 -> Local Minimum\n";
                else if (det > 0 && tr < 0)
                    std::cout << " det(H) > 0 and tr(H) < 0 -> Local Maximum\n";
                else if (det < 0)
                    std::cout << " det(H) < 0 -> Saddle Point\n";
                else
                    std::cout << " Inconclusive test\n";
            }
        }

        return make_tuple(currentPoint, optimalValue, history);
    }

private:
    FunctionParser parser;
    vector<string> varNames;
    bool maximize;
    size_t n;
    std::string OGfunctionExpr;
};

class SteepestDescent
{
public:
    SteepestDescent() {}
    ~SteepestDescent() {}

    void DoSteepestDescent(const string &exprStr, const vector<string> &vars, const Point &initialPoint, bool maximize = false)
    {
        DetailedSteepestDescentOptimizer optimizer(exprStr, vars, maximize);
        optimizer.optimize(initialPoint, 100, 1e-5, true);
    }

    void test()
    {
        // DoSteepestDescent("2*x*y + 4*x - 2*x^2 - y^2", {"x", "y"}, {0.5, 0.5}, true);
        DoSteepestDescent("x^2 + y^2 + 2*x + 4", {"x", "y"}, {2, 1}, false);
    }
};