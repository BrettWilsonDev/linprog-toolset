// #include <vector>
// #include <limits>
// #include <iostream>
// #include <string>
// #include <algorithm>
// #include <stdexcept>
// #include <sstream>
// #include "symbolicc++.h"
// #include "math_preliminaries.hpp"

// // using namespace std;

// // Revised solveLinear to extract coefficients more reliably
// Symbolic solveLinear(const Symbolic &expr, const Symbol &d)
// {
//     // Assume expr is of the form a*d + b, solve for d: a*d + b = 0 => d = -b/a
//     // Get coefficient of d (degree 1) and constant term (degree 0)
//     // Symbolic a = expr.coeff(d, 1); // Coefficient of d
//     // Symbolic b = expr.coeff(d, 0); // Constant term
//     // if (a == Symbolic(0))
//     // {
//     //     throw runtime_error("Expression is not linear in d or constant non-zero.");
//     // }
//     // return -b / a;

//     std::cout << solve(expr, d).back().rhs << std::endl;

//     return solve(expr, d).back().rhs;
// }

// // Revised toDouble for robust conversion
// double toDouble(const Symbolic &sym)
// {
//     try
//     {
//         // Convert Symbolic to string
//         std::ostringstream oss;
//         oss << sym;
//         std::string str = oss.str();
//         str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
//         // std::cerr << "Converting string to double: " << str << std::endl;

//         if (str.empty())
//         {
//             throw std::invalid_argument("Empty string cannot be converted to double");
//         }

//         bool negative = false;
//         if (!str.empty() && str[0] == '-')
//         {
//             negative = true;
//             str = str.substr(1);
//         }

//         double result;
//         size_t slash_pos = str.find('/');
//         if (slash_pos != std::string::npos)
//         {
//             std::string num_str = str.substr(0, slash_pos);
//             std::string den_str = str.substr(slash_pos + 1);
//             if (num_str.empty() || den_str.empty())
//             {
//                 throw std::invalid_argument("Invalid fraction format: " + str);
//             }
//             double num = std::stod(num_str);
//             double den = std::stod(den_str);
//             if (den == 0)
//             {
//                 throw std::invalid_argument("Division by zero in fraction: " + str);
//             }
//             result = num / den;
//         }
//         else
//         {
//             result = std::stod(str);
//         }

//         return negative ? -result : result;
//     }
//     catch (const std::exception &e)
//     {
//         std::cerr << "Error in toDouble: " << e.what() << std::endl;
//         throw;
//     }
// }

// class SensitivityAnalysis
// {
// public:
//     SensitivityAnalysis(bool isConsoleOutput = false)
//         : isConsoleOutput(isConsoleOutput), d("d"), mathPrelim()
//     {
//         testInputSelected = 0;
//         globalOptimalTab.clear();
//         globalHeaderRow.clear();
//         problemType = "Max";
//         absProblemType = "abs Off";
//         amtOfObjVars = 2;
//         objFunc = {Symbolic(0), Symbolic(0)};
//         constraints = {{Symbolic(0), Symbolic(0), Symbolic(0), Symbolic(0)}};
//         signItems = {"<=", ">="};
//         signItemsChoices = {0};
//         amtOfConstraints = 1;
//         changingTable.clear();
//         posDelta = numeric_limits<double>::infinity();
//         negDelta = numeric_limits<double>::infinity();
//         posRange = numeric_limits<double>::infinity();
//         negRange = numeric_limits<double>::infinity();
//         termPos = "?";
//         currentDeltaSelection = "o0";
//     }

//     tuple<vector<Symbolic>, vector<vector<Symbolic>>, bool> testInput(int testNum)
//     {
//         bool isMin = false;
//         vector<Symbolic> objFunc;
//         vector<vector<Symbolic>> constraints;
//         if (testNum == 0)
//         {
//             objFunc = {Symbolic(60), Symbolic(30), Symbolic(20)};
//             constraints = {
//                 {Symbolic(8), Symbolic(6), Symbolic(1), Symbolic(48), Symbolic(0)},
//                 {Symbolic(4), Symbolic(2), Symbolic(3) / Symbolic(2), Symbolic(20), Symbolic(0)},
//                 {Symbolic(2), Symbolic(3) / Symbolic(2), Symbolic(1) / Symbolic(2), Symbolic(8), Symbolic(0)},
//             };
//         }
//         else if (testNum == 1)
//         {
//             objFunc = {Symbolic(100), Symbolic(30)};
//             constraints = {
//                 {Symbolic(0), Symbolic(1), Symbolic(3), Symbolic(1)},
//                 {Symbolic(1), Symbolic(1), Symbolic(7), Symbolic(0)},
//                 {Symbolic(10), Symbolic(4), Symbolic(40), Symbolic(0)},
//             };
//         }
//         else if (testNum == 2)
//         {
//             objFunc = {Symbolic(30), Symbolic(28), Symbolic(26), Symbolic(30)};
//             constraints = {
//                 {Symbolic(8), Symbolic(8), Symbolic(4), Symbolic(4), Symbolic(160), Symbolic(0)},
//                 {Symbolic(1), Symbolic(0), Symbolic(0), Symbolic(0), Symbolic(5), Symbolic(0)},
//                 {Symbolic(1), Symbolic(0), Symbolic(0), Symbolic(0), Symbolic(5), Symbolic(1)},
//                 {Symbolic(1), Symbolic(1), Symbolic(1), Symbolic(1), Symbolic(20), Symbolic(1)},
//             };
//         }
//         else if (testNum == 3)
//         {
//             objFunc = {Symbolic(10), Symbolic(50), Symbolic(80), Symbolic(100)};
//             constraints = {
//                 {Symbolic(1), Symbolic(4), Symbolic(4), Symbolic(8), Symbolic(140), Symbolic(0)},
//                 {Symbolic(1), Symbolic(0), Symbolic(0), Symbolic(0), Symbolic(50), Symbolic(0)},
//                 {Symbolic(1), Symbolic(0), Symbolic(0), Symbolic(0), Symbolic(50), Symbolic(1)},
//                 {Symbolic(1), Symbolic(1), Symbolic(1), Symbolic(1), Symbolic(70), Symbolic(1)},
//             };
//         }
//         else if (testNum == 4)
//         {
//             objFunc = {Symbolic(3), Symbolic(2)};
//             constraints = {
//                 {Symbolic(2), Symbolic(1), Symbolic(100), Symbolic(0)},
//                 {Symbolic(1), Symbolic(1), Symbolic(80), Symbolic(0)},
//                 {Symbolic(1), Symbolic(0), Symbolic(40), Symbolic(0)},
//             };
//         }
//         else if (testNum == 5)
//         {
//             objFunc = {Symbolic(120), Symbolic(80)};
//             constraints = {
//                 {Symbolic(8), Symbolic(4), Symbolic(160), Symbolic(0)},
//                 {Symbolic(4), Symbolic(4), Symbolic(100), Symbolic(0)},
//                 {Symbolic(1), Symbolic(0), Symbolic(17), Symbolic(0)},
//                 {Symbolic(1), Symbolic(0), Symbolic(5), Symbolic(1)},
//                 {Symbolic(0), Symbolic(1), Symbolic(17), Symbolic(0)},
//                 {Symbolic(0), Symbolic(1), Symbolic(2), Symbolic(1)},
//                 {Symbolic(1), Symbolic(-1), Symbolic(0), Symbolic(1)},
//                 {Symbolic(1), Symbolic(-4), Symbolic(0), Symbolic(0)},
//             };
//         }
//         if (testNum == -1)
//         {
//             return make_tuple(vector<Symbolic>{}, vector<vector<Symbolic>>{}, false);
//         }
//         return make_tuple(objFunc, constraints, isMin);
//     }

//     void doSensitivityAnalysis(const vector<Symbolic> &objfunc, const vector<vector<Symbolic>> &constraints, bool isMin)
//     {
//         vector<Symbolic> a = objfunc;
//         vector<vector<Symbolic>> b = constraints;

//         auto [ct, mCbv, mB, mBNegOne, mCbvNegOne, bvs] = mathPrelim.DoPreliminaries(a, b, isMin);
//         changingTable = ct;

//         Symbolic termWithoutdelta = Symbolic(0);
//         bool objFuncHasDelta = false;
//         for (size_t i = 0; i < objfunc.size(); ++i)
//         {
//             if (objfunc[i] != objfunc[i].subst(d == Symbolic(0)))
//             {
//                 termPos = "c" + to_string(i + 1);
//                 termWithoutdelta = objfunc[i].coeff(d, 0);
//                 objFuncHasDelta = true;
//             }
//         }
//         bool rhsHasDelta = false;
//         for (size_t i = 0; i < constraints.size(); ++i)
//         {
//             size_t rhsIdx = constraints[i].size() - 2;
//             if (constraints[i][rhsIdx] != constraints[i][rhsIdx].subst(d == Symbolic(0)))
//             {
//                 termPos = "b" + to_string(i + 1);
//                 termWithoutdelta = constraints[i][rhsIdx].coeff(d, 0);
//                 rhsHasDelta = true;
//             }
//         }
//         bool constraintsHasDelta = false;
//         for (size_t i = 0; i < constraints.size(); ++i)
//         {
//             for (size_t j = 0; j < constraints[i].size() - 2; ++j)
//             {
//                 if (constraints[i][j] != constraints[i][j].subst(d == Symbolic(0)))
//                 {
//                     termPos = "c" + to_string(i + 1);
//                     termWithoutdelta = constraints[i][j].coeff(d, 0);
//                     constraintsHasDelta = true;
//                 }
//             }
//         }

//         vector<Symbolic> deltasList;
//         if (objFuncHasDelta)
//         {
//             for (size_t i = 0; i < changingTable[0].size() - 1; ++i)
//             {
//                 Symbolic e = changingTable[0][i];
//                 if (e != e.subst(d == Symbolic(0)))
//                 {
//                     Symbolic delta = solveLinear(e, d);
//                     deltasList.push_back(delta);
//                 }
//             }
//         }
//         if (rhsHasDelta)
//         {
//             deltasList.clear();
//             for (size_t i = 0; i < changingTable.size(); ++i)
//             {
//                 Symbolic e = changingTable[i].back();
//                 if (e != e.subst(d == Symbolic(0)))
//                 {
//                     Symbolic delta = solveLinear(e, d);
//                     deltasList.push_back(delta);
//                 }
//             }
//         }
//         int conStraintDeltaCol = -1;
//         if (constraintsHasDelta)
//         {
//             for (size_t i = 0; i < changingTable.size(); ++i)
//             {
//                 for (size_t j = 0; j < changingTable[i].size() - 1; ++j)
//                 {
//                     if (changingTable[i][j] != changingTable[i][j].subst(d == Symbolic(0)))
//                     {
//                         conStraintDeltaCol = static_cast<int>(j);
//                     }
//                 }
//             }
//         }

//         if (constraintsHasDelta)
//         {
//             if (conStraintDeltaCol != -1 && changingTable[0][conStraintDeltaCol] != changingTable[0][conStraintDeltaCol].subst(d == Symbolic(0)))
//             {
//                 try
//                 {
//                     Symbolic delta = solveLinear(changingTable[0][conStraintDeltaCol], d);
//                     deltasList.push_back(delta);
//                 }
//                 catch (...)
//                 {
//                     termPos = "unsolvable";
//                 }
//             }
//             else
//             {
//                 negDelta = numeric_limits<double>::infinity();
//                 posDelta = numeric_limits<double>::infinity();
//                 ostringstream oss;
//                 oss << changingTable[0][conStraintDeltaCol];
//                 termPos = oss.str();
//             }
//         }

//         if (!deltasList.empty())
//         {
//             vector<double> all_deltas;
//             for (const auto &del : deltasList)
//             {
//                 all_deltas.push_back(toDouble(del));
//             }
//             if (all_deltas.size() == 1)
//             {
//                 double del_d = all_deltas[0];
//                 if (del_d < 0)
//                 {
//                     posDelta = numeric_limits<double>::infinity();
//                     negDelta = del_d;
//                 }
//                 else if (del_d > 0)
//                 {
//                     posDelta = del_d;
//                     negDelta = numeric_limits<double>::infinity();
//                 }
//             }
//             else
//             {
//                 vector<double> pos_deltas, neg_deltas;
//                 for (double dd : all_deltas)
//                 {
//                     if (dd > 0)
//                         pos_deltas.push_back(dd);
//                     else if (dd < 0)
//                         neg_deltas.push_back(dd);
//                 }
//                 posDelta = pos_deltas.empty() ? numeric_limits<double>::infinity() : *min_element(pos_deltas.begin(), pos_deltas.end());
//                 negDelta = neg_deltas.empty() ? numeric_limits<double>::infinity() : *max_element(neg_deltas.begin(), neg_deltas.end());
//             }
//         }

//         double twd = toDouble(termWithoutdelta);
//         posRange = twd + posDelta;
//         negRange = twd + negDelta;

//         if (isConsoleOutput)
//         {
//             cout << negRange << " <= " << termPos << " <= " << posRange << endl;
//             cout << negDelta << " <= d <= " << posDelta << endl;
//         }
//     }

//     void doSensitivityAnalysisDouble(std::vector<double> &objFuncDouble, std::vector<std::vector<double>> &constraintsDouble, bool isMin = false, std::string currentDeltaSelection = "dStore0")
//     {
//         std::vector<Symbolic> objFunc;
//         objFunc.reserve(objFuncDouble.size());
//         for (double val : objFuncDouble)
//         {
//             objFunc.push_back(Symbolic(val));
//         }

//         std::vector<std::vector<Symbolic>> constraints;
//         constraints.reserve(constraintsDouble.size());
//         for (const auto &row : constraintsDouble)
//         {
//             std::vector<Symbolic> symbolicRow;
//             symbolicRow.reserve(row.size());
//             for (double val : row)
//             {
//                 symbolicRow.push_back(Symbolic(val));
//             }
//             constraints.push_back(std::move(symbolicRow));
//         }

//         if (currentDeltaSelection != "dStore0")
//         {
//             // Update objFunc
//             for (size_t i = 0; i < objFunc.size(); ++i)
//             {
//                 if (currentDeltaSelection == "o" + std::to_string(i))
//                 {
//                     objFunc[i] = objFunc[i] + d;
//                 }
//             }

//             // Update constraints
//             for (size_t i = 0; i < constraints.size(); ++i)
//             {
//                 for (size_t j = 0; j < constraints[i].size(); ++j)
//                 {
//                     if (currentDeltaSelection == "c" + std::to_string(i) + std::to_string(j))
//                     {
//                         constraints[i][j] = constraints[i][j] + d;
//                     }
//                 }
//             }

//             // Update constraints right-hand side (RHS)
//             for (size_t i = 0; i < constraints.size(); ++i)
//             {
//                 if (currentDeltaSelection == "cRhs" + std::to_string(i))
//                 {
//                     constraints[i][constraints[i].size() - 2] =
//                         constraints[i][constraints[i].size() - 2] + d;
//                 }
//             }
//         }

//         doSensitivityAnalysis(objFunc, constraints, isMin);
//     }

//     void test()
//     {
//         std::cout << "\nRunning custom test case:\n";
//         vector<Symbolic> customObjFunc = {Symbolic(3), Symbolic(2)};
//         vector<vector<Symbolic>> customConstraints = {
//             {Symbolic(2), Symbolic(1), Symbolic(100), Symbolic(0)},
//             {Symbolic(1), Symbolic(1), Symbolic(80), Symbolic(0)},
//             {Symbolic(1), Symbolic(0), Symbolic(40) + d, Symbolic(0)}};
//         try
//         {
//             doSensitivityAnalysis(customObjFunc, customConstraints, false);
//         }
//         catch (const std::exception &e)
//         {
//             std::cerr << "Error in custom test case: " << e.what() << std::endl;
//         }
//     }

// private:
//     bool isConsoleOutput;
//     Symbol d;

//     int testInputSelected;
//     MathPreliminaries mathPrelim;
//     vector<vector<Symbolic>> globalOptimalTab;
//     vector<Symbolic> globalHeaderRow;
//     string problemType;
//     string absProblemType;
//     int amtOfObjVars;
//     vector<Symbolic> objFunc;
//     vector<vector<Symbolic>> constraints;
//     vector<string> signItems;
//     vector<int> signItemsChoices;
//     int amtOfConstraints;
//     vector<vector<Symbolic>> changingTable;
//     double posDelta;
//     double negDelta;
//     double posRange;
//     double negRange;
//     string termPos;
//     string currentDeltaSelection;
// };