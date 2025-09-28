#pragma once

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>

// using namespace std;

struct Point2d
{
    double x, y;
    Point2d(double xx = 0.0, double yy = 0.0) : x(xx), y(yy) {}
    bool operator<(const Point2d &o) const
    {
        if (fabs(x - o.x) > 1e-9)
            return x < o.x;
        return y < o.y;
    }
    bool operator==(const Point2d &o) const
    {
        return fabs(x - o.x) < 1e-9 && fabs(y - o.y) < 1e-9;
    }
};

struct Segment
{
    Point2d start, end;
    Segment(const Point2d &s, const Point2d &e) : start(s), end(e) {}
};

struct GraphData
{
    std::vector<Segment> lineSegments;
    std::vector<Point2d> feasiblePoint2ds;
    std::vector<Point2d> intersectionPoint2ds;
    std::vector<Point2d> feasibleHull;
    Point2d optimalPoint2d;
    double optimalValue;
};

class GraphicalSolver
{
private:
    bool isConsoleOutput;
    int testInputSelected;
    std::string problemType;
    int amtOfObjVars;
    std::vector<double> objFunc;
    int amtOfConstraints;
    std::vector<std::vector<double>> constraints;
    std::vector<std::string> signItems;
    std::vector<int> signItemsChoices;
    double optimalValue;
    Point2d optimalPoint2d;

    double crossProduct(const Point2d &p1, const Point2d &p2, const Point2d &p3) const
    {
        return (p2.x - p1.x) * (p3.y - p2.y) - (p2.y - p1.y) * (p3.x - p2.x);
    }

    std::vector<Point2d> grahamScan(std::vector<Point2d> Point2ds) const
    {
        if (Point2ds.size() < 3)
            return Point2ds;
        sort(Point2ds.begin(), Point2ds.end());
        std::vector<Point2d> lowerHull;
        for (const auto &p : Point2ds)
        {
            while (lowerHull.size() >= 2 && crossProduct(lowerHull[lowerHull.size() - 2], lowerHull.back(), p) < 0)
            {
                lowerHull.pop_back();
            }
            lowerHull.push_back(p);
        }
        std::vector<Point2d> upperHull;
        for (auto it = Point2ds.rbegin(); it != Point2ds.rend(); ++it)
        {
            const auto &p = *it;
            while (upperHull.size() >= 2 && crossProduct(upperHull[upperHull.size() - 2], upperHull.back(), p) < 0)
            {
                upperHull.pop_back();
            }
            upperHull.push_back(p);
        }
        if (lowerHull.size() > 1)
            lowerHull.pop_back();
        if (upperHull.size() > 1)
            upperHull.pop_back();
        lowerHull.insert(lowerHull.end(), upperHull.begin(), upperHull.end());
        return lowerHull;
    }

public:
    GraphicalSolver(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput)
    {
        testInputSelected = -1;
        problemType = "Max";
        amtOfObjVars = 2;
        objFunc = {0.0, 0.0};
        amtOfConstraints = 1;
        constraints = {{0.0, 0.0, 0.0, 0.0}};
        signItems = {"<=", ">=", "="};
        signItemsChoices = {0};
        optimalValue = 0.0;
        optimalPoint2d = Point2d(0.0, 0.0);
    }

    std::tuple<std::vector<double>, std::vector<std::vector<double>>, bool> testInput(int testNum = -1)
    {
        std::vector<double> of;
        std::vector<std::vector<double>> cons;
        bool isMin = false;
        if (testNum == 0)
        {
            isMin = false;
            of = {8.0, 1.0};
            cons = {{1.0, 1.0, 40.0, 0.0}, {2.0, 1.0, 60.0, 0.0}};
        }
        else if (testNum == 1)
        {
            isMin = false;
            of = {100.0, 30.0};
            cons = {{0.0, 1.0, 3.0, 1.0}, {1.0, 1.0, 7.0, 0.0}, {10.0, 4.0, 40.0, 0.0}};
        }
        else if (testNum == 2)
        {
            isMin = false;
            of = {1200.0, 800.0};
            cons = {{8.0, 4.0, 1600.0, 0.0}, {4.0, 4.0, 1000.0, 0.0}, {1.0, 0.0, 170.0, 0.0}, {0.0, 1.0, 200.0, 0.0}, {1.0, 0.0, 40.0, 1.0}, {0.0, 1.0, 25.0, 1.0}, {1.0, -1.0, 0.0, 1.0}, {1.0, -4.0, 0.0, 0.0}};
        }
        else if (testNum == 3)
        {
            isMin = true;
            of = {50.0, 100.0};
            cons = {{7.0, 2.0, 28.0, 1.0}, {2.0, 12.0, 24.0, 1.0}};
        }
        // If testNum == -1, return empty vectors
        return {of, cons, isMin};
    }

    Point2d findIntersection(double a1, double b1, double c1, double a2, double b2, double c2) const
    {
        double determinant = a1 * b2 - a2 * b1;
        if (fabs(determinant) < 1e-12)
        {
            return Point2d(NAN, NAN); // Invalid Point2d
        }
        double x = (c1 * b2 - c2 * b1) / determinant;
        double y = (a1 * c2 - a2 * c1) / determinant;
        return Point2d(x, y);
    }

    std::vector<Point2d> getEndPoint2ds(const std::vector<double> &con, std::pair<double, double> xBounds, std::pair<double, double> yBounds) const
    {
        double a = con[0], b = con[1], c = con[2];
        std::vector<Point2d> Point2ds;
        if (fabs(b) > 1e-12)
        {
            double y1 = (c - a * xBounds.first) / b;
            if (yBounds.first - 1e-9 <= y1 && y1 <= yBounds.second + 1e-9)
            {
                Point2ds.emplace_back(xBounds.first, y1);
            }
            double y2 = (c - a * xBounds.second) / b;
            if (yBounds.first - 1e-9 <= y2 && y2 <= yBounds.second + 1e-9)
            {
                Point2ds.emplace_back(xBounds.second, y2);
            }
        }
        if (fabs(a) > 1e-12)
        {
            double x1 = (c - b * yBounds.first) / a;
            if (xBounds.first - 1e-9 <= x1 && x1 <= xBounds.second + 1e-9)
            {
                Point2ds.emplace_back(x1, yBounds.first);
            }
            double x2 = (c - b * yBounds.second) / a;
            if (xBounds.first - 1e-9 <= x2 && x2 <= xBounds.second + 1e-9)
            {
                Point2ds.emplace_back(x2, yBounds.second);
            }
        }
        // Remove potential duplicates
        sort(Point2ds.begin(), Point2ds.end());
        auto last = unique(Point2ds.begin(), Point2ds.end(), [](const Point2d &p1, const Point2d &p2)
                           { return p1 == p2; });
        Point2ds.erase(last, Point2ds.end());
        return Point2ds;
    }

    std::tuple<std::vector<Point2d>, std::vector<Point2d>, std::vector<Point2d>> getSortedPoint2ds(const std::vector<std::vector<double>> &cons)
    {
        std::vector<Point2d> intersectionPoint2ds;
        size_t numConstraints = cons.size();
        for (size_t i = 0; i < numConstraints; ++i)
        {
            for (size_t j = i + 1; j < numConstraints; ++j)
            {
                Point2d Point2d = findIntersection(cons[i][0], cons[i][1], cons[i][2],
                                               cons[j][0], cons[j][1], cons[j][2]);
                if (!isnan(Point2d.x))
                {
                    intersectionPoint2ds.push_back(Point2d);
                }
            }
        }

        double maxX = 0.0;
        for (const auto &constraint : cons)
        {
            if (fabs(constraint[0]) > 1e-12)
            {
                maxX = std::max(maxX, constraint[2]);
            }
        }
        double maxY = 0.0;
        for (const auto &constraint : cons)
        {
            if (fabs(constraint[1]) > 1e-12)
            {
                maxY = std::max(maxY, constraint[2]);
            }
        }
        std::pair<double, double> xBounds = {0.0, maxX};
        std::pair<double, double> yBounds = {0.0, maxY};

        std::vector<Point2d> endPoint2ds;
        for (const auto &constraint : cons)
        {
            std::vector<Point2d> pts = getEndPoint2ds(constraint, xBounds, yBounds);
            endPoint2ds.insert(endPoint2ds.end(), pts.begin(), pts.end());
        }

        std::vector<Point2d> allPoint2ds = intersectionPoint2ds;
        allPoint2ds.insert(allPoint2ds.end(), endPoint2ds.begin(), endPoint2ds.end());
        allPoint2ds.emplace_back(0.0, 0.0);

        std::vector<Point2d> feasiblePoint2ds;
        double eps = 1e-9;
        for (const auto &Point2d : allPoint2ds)
        {
            double x = Point2d.x, y = Point2d.y;
            bool isFeasible = true;
            for (const auto &con : cons)
            {
                double val = con[0] * x + con[1] * y;
                int eq = static_cast<int>(con[3]);
                if (eq == 0)
                {
                    if (val > con[2] + eps)
                    {
                        isFeasible = false;
                        break;
                    }
                }
                else if (eq == 1)
                {
                    if (val < con[2] - eps)
                    {
                        isFeasible = false;
                        break;
                    }
                }
                else
                {
                    if (fabs(val - con[2]) > eps)
                    {
                        isFeasible = false;
                        break;
                    }
                }
            }
            if (isFeasible && x >= -eps && y >= -eps)
            {
                feasiblePoint2ds.push_back(Point2d);
            }
        }

        // Remove duplicates and sort
        std::set<Point2d> uniqueFeasible(feasiblePoint2ds.begin(), feasiblePoint2ds.end());
        feasiblePoint2ds.assign(uniqueFeasible.begin(), uniqueFeasible.end());
        sort(feasiblePoint2ds.begin(), feasiblePoint2ds.end());

        // Line segment Point2ds (unique endPoint2ds)
        sort(endPoint2ds.begin(), endPoint2ds.end());
        auto uniqueEndIt = unique(endPoint2ds.begin(), endPoint2ds.end(), [](const Point2d &p1, const Point2d &p2)
                                  { return p1 == p2; });
        endPoint2ds.erase(uniqueEndIt, endPoint2ds.end());
        std::vector<Point2d> lineSegmentPoint2ds = endPoint2ds;

        if (isConsoleOutput)
        {
            std::cout << "\nlineSegments" << std::endl;
            for (size_t i = 0; i < lineSegmentPoint2ds.size(); i += 2)
            {
                if (i + 1 < lineSegmentPoint2ds.size())
                {
                    std::cout << " (start: (" << lineSegmentPoint2ds[i].x << ", " << lineSegmentPoint2ds[i].y
                         << ") end: (" << lineSegmentPoint2ds[i + 1].x << ", " << lineSegmentPoint2ds[i + 1].y << "))" << std::endl;
                }
            }
            std::cout << "\nfeasible region" << std::endl;
            for (const auto &p : feasiblePoint2ds)
            {
                std::cout << "(" << p.x << ", " << p.y << ") ";
            }
            std::cout << "\n"
                 << std::endl;
        }

        return {feasiblePoint2ds, lineSegmentPoint2ds, intersectionPoint2ds};
    }

    std::pair<double, Point2d> solveGraphical(const std::vector<double> &objFunc, const std::vector<Point2d> &feasiblePoint2ds, bool isMin = false)
    {
        if (feasiblePoint2ds.empty())
        {
            return {0.0, Point2d()};
        }
        auto evaluateObjective = [&](double x, double y)
        {
            return objFunc[0] * x + objFunc[1] * y;
        };
        std::vector<double> objectiveValues;
        for (const auto &p : feasiblePoint2ds)
        {
            objectiveValues.push_back(evaluateObjective(p.x, p.y));
        }
        double optimalVal;
        size_t optimalIdx;
        if (isMin)
        {
            auto minIt = min_element(objectiveValues.begin(), objectiveValues.end());
            optimalIdx = minIt - objectiveValues.begin();
            optimalVal = *minIt;
        }
        else
        {
            auto maxIt = max_element(objectiveValues.begin(), objectiveValues.end());
            optimalIdx = maxIt - objectiveValues.begin();
            optimalVal = *maxIt;
        }
        Point2d optimalPt = feasiblePoint2ds[optimalIdx];
        if (isConsoleOutput)
        {
            std::cout << "Optimal value: " << optimalVal << " at (" << optimalPt.x << ", " << optimalPt.y << ")" << std::endl;
        }
        return {optimalVal, optimalPt};
    }

    GraphData drawGraph(const std::vector<Point2d> &feasiblePoint2ds, const std::vector<Point2d> &lineSegmentPoint2ds,
                        const std::vector<Point2d> &intersectionPoint2ds, Point2d optimalPoint2d = Point2d(), double optimalValue = NAN)
    {
        // Pair line segment Point2ds every 2 (matching Python logic)
        std::vector<Segment> segments;
        for (size_t i = 0; i < lineSegmentPoint2ds.size(); i += 2)
        {
            if (i + 1 < lineSegmentPoint2ds.size())
            {
                segments.emplace_back(lineSegmentPoint2ds[i], lineSegmentPoint2ds[i + 1]);
            }
        }

        // Compute feasible hull using Graham scan
        std::vector<Point2d> hull = grahamScan(feasiblePoint2ds);

        GraphData data;
        data.lineSegments = segments;
        data.feasiblePoint2ds = feasiblePoint2ds;
        data.intersectionPoint2ds = intersectionPoint2ds;
        data.feasibleHull = hull;
        data.optimalPoint2d = optimalPoint2d;
        data.optimalValue = optimalValue;
        return data;
    }
};