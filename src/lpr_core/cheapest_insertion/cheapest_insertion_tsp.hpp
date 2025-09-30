#pragma once

#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <string>
#include <sstream>
#include <utility>
#include <limits>

struct InsertionOption
{
    int city;
    double detourCost;
    std::string routePart;
    std::string detourPart;
    std::string calcPart;
    std::vector<int> newRoute;
    int insertPosition;
};

class CheapestInsertionTSP
{
private:
    std::vector<std::vector<double>> distanceMatrix;
    size_t numCities;
    std::map<int, std::string> cities;
    std::ostringstream oss; // To collect output

public:
    CheapestInsertionTSP() = default;

    // Constructor
    explicit CheapestInsertionTSP(const std::vector<std::vector<double>> &distMatrix)
        : distanceMatrix(distMatrix)
    {
        numCities = distanceMatrix.size();
        for (size_t i = 0; i < numCities; ++i)
        {
            cities[i + 1] = "City " + std::to_string(i + 1);
        }
    }

    // Get collected output
    std::string getCollectedOutput() const
    {
        return oss.str();
    }

    // Print TSP formulation
    void printFormulation(const std::vector<std::vector<double>> &distMatrix)
    {
        size_t n = distMatrix.size();

        // Objective function
        oss << "Objective function:\n";
        std::vector<std::string> terms;

        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = 0; j < n; ++j)
            {
                if (i != j)
                {
                    std::ostringstream term;
                    term << distMatrix[i][j] << "x" << (i + 1) << (j + 1);
                    terms.push_back(term.str());
                }
            }
        }

        oss << "min z = ";
        for (size_t i = 0; i < terms.size(); ++i)
        {
            if (i > 0 && (i % 10 == 0))
                oss << "\n";
            if (i > 0)
                oss << " + ";
            oss << terms[i];
        }
        oss << "\n\n";

        // Arriving once in a city constraints
        oss << "Arriving once in a city constraints:\n";

        // Each city is entered exactly once
        for (size_t j = 0; j < n; ++j)
        {
            std::vector<std::string> constraintTerms;
            for (size_t i = 0; i < n; ++i)
            {
                if (i != j)
                {
                    std::ostringstream term;
                    term << "x" << (i + 1) << (j + 1);
                    constraintTerms.push_back(term.str());
                }
            }

            for (size_t i = 0; i < constraintTerms.size(); ++i)
            {
                if (i > 0)
                    oss << " + ";
                oss << constraintTerms[i];
            }
            oss << " = 1\n";
        }

        // Each city is left exactly once
        for (size_t i = 0; i < n; ++i)
        {
            std::vector<std::string> constraintTerms;
            for (size_t j = 0; j < n; ++j)
            {
                if (i != j)
                {
                    std::ostringstream term;
                    term << "x" << (i + 1) << (j + 1);
                    constraintTerms.push_back(term.str());
                }
            }

            for (size_t i = 0; i < constraintTerms.size(); ++i)
            {
                if (i > 0)
                    oss << " + ";
                oss << constraintTerms[i];
            }
            oss << " = 1\n";
        }
        oss << "\n";

        // Sub-tour elimination constraints (MTZ)
        oss << "Sub-tour constraints:\n";
        for (size_t i = 1; i < n; ++i)
        { // Ui for cities 2..n
            for (size_t j = 1; j < n; ++j)
            {
                if (i != j)
                {
                    oss << "U" << (i + 1) << " - U" << (j + 1) << " + "
                        << n << "x" << (i + 1) << (j + 1) << " <= " << (n - 1) << "\n";
                }
            }
        }
    }

    // Get distance between two cities (1-indexed)
    double getDistance(int fromCity, int toCity) const
    {
        return distanceMatrix[fromCity - 1][toCity - 1];
    }

    // Find initial 2-city route
    std::vector<int> findInitialRoute(int startCity = -1)
    {
        int c1, c2;

        if (startCity != -1)
        {
            c1 = startCity;
            std::vector<std::pair<int, double>> distances;
            for (int i = 1; i <= static_cast<int>(numCities); ++i)
            {
                if (i != c1)
                {
                    distances.push_back({i, getDistance(c1, i)});
                }
            }

            auto minElement = std::min_element(distances.begin(), distances.end(),
                                               [](const std::pair<int, double> &a, const std::pair<int, double> &b)
                                               {
                                                   return a.second < b.second;
                                               });

            c2 = minElement->first;
        }
        else
        {
            double minDist = std::numeric_limits<double>::infinity();
            c1 = c2 = -1;

            for (int i = 1; i <= static_cast<int>(numCities); ++i)
            {
                for (int j = i + 1; j <= static_cast<int>(numCities); ++j)
                {
                    double d = getDistance(i, j);
                    if (d < minDist)
                    {
                        minDist = d;
                        c1 = i;
                        c2 = j;
                    }
                }
            }
        }

        oss << "Initial route chosen: " << cities.at(c1) << " => " << cities.at(c2)
            << " with distance " << std::fixed << std::setprecision(0) << getDistance(c1, c2) << "\n";

        return {c1, c2};
    }

    // Get insertion options for 2-city route
    std::vector<InsertionOption> getInsertionOptions(const std::vector<int> &route,
                                                     const std::vector<int> &remainingCities)
    {
        if (route.size() != 2)
        {
            throw std::runtime_error("This method is for 2-city route only.");
        }

        int a = route[0];
        int b = route[1];
        std::vector<std::pair<int, int>> edges = {{a, b}, {b, a}};

        if (a > b)
        {
            edges = {{b, a}, {a, b}};
        }

        std::vector<InsertionOption> options;

        for (const auto &edge : edges)
        {
            int currentFrom = edge.first;
            int currentTo = edge.second;
            double currentDist = getDistance(currentFrom, currentTo);

            for (int newCity : remainingCities)
            {
                double newDist1 = getDistance(currentFrom, newCity);
                double newDist2 = getDistance(newCity, currentTo);
                double detourCost = newDist1 + newDist2 - currentDist;

                std::ostringstream routePart, detourPart, calcPart;
                routePart << "x" << currentFrom << currentTo << " " << std::fixed << std::setprecision(0) << currentDist;
                detourPart << "x" << currentFrom << newCity << " x" << newCity << currentTo
                           << " " << std::fixed << std::setprecision(0) << newDist1 << " " << newDist2;
                calcPart << "= (" << std::fixed << std::setprecision(0) << newDist1 << " + " << newDist2 << ") - "
                         << currentDist << " = " << detourCost;

                std::vector<int> newRoute;
                if (currentFrom == route[0] && currentTo == route[1])
                {
                    newRoute = route;
                    newRoute.insert(newRoute.begin() + 1, newCity);
                }
                else
                {
                    newRoute = {currentFrom, newCity, currentTo};
                }

                options.push_back({newCity,
                                   detourCost,
                                   routePart.str(),
                                   detourPart.str(),
                                   calcPart.str(),
                                   newRoute,
                                   1});
            }
        }

        return options;
    }

    // Get insertion options for 3+ city routes
    std::vector<InsertionOption> getGeneralInsertionOptions(const std::vector<int> &route,
                                                            const std::vector<int> &remainingCities)
    {
        std::vector<InsertionOption> options;

        for (size_t i = 0; i < route.size(); ++i)
        {
            int currentFrom = route[i];
            int currentTo = route[(i + 1) % route.size()];
            double currentDist = getDistance(currentFrom, currentTo);

            for (int newCity : remainingCities)
            {
                double newDist1 = getDistance(currentFrom, newCity);
                double newDist2 = getDistance(newCity, currentTo);
                double detourCost = newDist1 + newDist2 - currentDist;

                std::ostringstream routePart, detourPart, calcPart;
                routePart << "x" << currentFrom << currentTo << " " << std::fixed << std::setprecision(0) << currentDist;
                detourPart << "x" << currentFrom << newCity << " x" << newCity << currentTo
                           << " " << std::fixed << std::setprecision(0) << newDist1 << " " << newDist2;
                calcPart << "= (" << std::fixed << std::setprecision(0) << newDist1 << " + " << newDist2 << ") - "
                         << currentDist << " = " << detourCost;

                std::vector<int> newRoute = route;
                newRoute.insert(newRoute.begin() + i + 1, newCity);

                options.push_back({newCity,
                                   detourCost,
                                   routePart.str(),
                                   detourPart.str(),
                                   calcPart.str(),
                                   newRoute,
                                   static_cast<int>(i + 1)});
            }
        }

        return options;
    }

    // Calculate total distance for a route
    double calculateTotalDistance(const std::vector<int> &route) const
    {
        double total = 0;
        for (size_t i = 0; i < route.size(); ++i)
        {
            total += getDistance(route[i], route[(i + 1) % route.size()]);
        }
        return total;
    }

    // Format route for display with optional anchor
    std::string formatRoute(std::vector<int> route, int anchor = -1) const
    {
        if (anchor != -1)
        {
            auto it = std::find(route.begin(), route.end(), anchor);
            if (it != route.end())
            {
                size_t k = std::distance(route.begin(), it);
                std::vector<int> newRoute;
                newRoute.insert(newRoute.end(), route.begin() + k, route.end());
                newRoute.insert(newRoute.end(), route.begin(), route.begin() + k);
                route = newRoute;
            }
        }

        std::ostringstream result;
        for (size_t i = 0; i < route.size(); ++i)
        {
            if (i > 0)
                result << " => ";
            result << cities.at(route[i]);
        }
        return result.str();
    }

    // Format remaining cities list
    std::string formatRemainingCities(const std::vector<int> &remainingCities) const
    {
        std::ostringstream result;
        result << "[";
        for (size_t i = 0; i < remainingCities.size(); ++i)
        {
            if (i > 0)
                result << ", ";
            result << cities.at(remainingCities[i]);
        }
        result << "]";
        return result.str();
    }

    // Main solve function
    std::pair<std::vector<int>, double> solve(int startCity = -1)
    {
        oss << "\n=== Formulation ===\n\n";
        printFormulation(distanceMatrix);

        oss << "\n=== Solving TSP using Cheapest Insertion Heuristic ===\n\n";

        std::vector<int> route = findInitialRoute(startCity);
        std::vector<int> remainingCities;

        for (int c = 1; c <= static_cast<int>(numCities); ++c)
        {
            if (std::find(route.begin(), route.end(), c) == route.end())
            {
                remainingCities.push_back(c);
            }
        }

        oss << "\nInitial route: " << formatRoute(route) << "\n";
        oss << "Remaining cities: " << formatRemainingCities(remainingCities) << "\n\n";

        while (!remainingCities.empty())
        {
            std::vector<InsertionOption> insertionOptions;

            if (route.size() == 2)
            {
                insertionOptions = getInsertionOptions(route, remainingCities);
            }
            else
            {
                insertionOptions = getGeneralInsertionOptions(route, remainingCities);
            }

            // Print table header
            oss << std::left
                << std::setw(20) << "Route"
                << std::setw(35) << "Detour"
                << std::setw(25) << "Detour Length" << "\n";
            oss << std::string(80, '-') << "\n";

            // Print insertion options
            for (const auto &option : insertionOptions)
            {
                oss << std::left
                    << std::setw(20) << option.routePart
                    << std::setw(35) << option.detourPart
                    << std::setw(25) << option.calcPart << "\n";
            }

            // Pick cheapest insertion
            auto bestOption = std::min_element(insertionOptions.begin(), insertionOptions.end(),
                                               [](const InsertionOption &a, const InsertionOption &b)
                                               {
                                                   return a.detourCost < b.detourCost;
                                               });

            route = bestOption->newRoute;
            remainingCities.erase(
                std::remove(remainingCities.begin(), remainingCities.end(), bestOption->city),
                remainingCities.end());

            int anchor = (std::find(route.begin(), route.end(), 1) != route.end()) ? 1 : route[0];
            oss << "\nUpdated route: " << formatRoute(route, anchor) << "\n";

            if (!remainingCities.empty())
            {
                oss << "\n";
            }
        }

        double totalDistance = calculateTotalDistance(route);
        int anchor = (std::find(route.begin(), route.end(), 1) != route.end()) ? 1 : route[0];
        oss << "\nFinal route: " << formatRoute(route, anchor) << "\n";

        // Print calculation
        std::vector<std::string> distances;
        for (size_t i = 0; i < route.size(); ++i)
        {
            std::ostringstream dist;
            dist << getDistance(route[i], route[(i + 1) % route.size()]);
            distances.push_back(dist.str());
        }

        oss << "z = ";
        for (size_t i = 0; i < distances.size(); ++i)
        {
            if (i > 0)
                oss << " + ";
            oss << distances[i];
        }
        oss << "\nz = " << totalDistance << "\n\n";

        return {route, totalDistance};
    }

    // Print distance matrix
    void printDistanceMatrix()
    {
        oss << "\nDistance Matrix:\n";
        oss << std::string(50, '-') << "\n";

        // Header
        oss << std::setw(8) << " ";
        for (size_t j = 0; j < numCities; ++j)
        {
            oss << std::setw(8) << ("City" + std::to_string(j + 1));
        }
        oss << "\n";

        // Rows
        for (size_t i = 0; i < numCities; ++i)
        {
            oss << std::setw(8) << ("City" + std::to_string(i + 1));
            for (size_t j = 0; j < numCities; ++j)
            {
                oss << std::setw(8) << std::fixed << std::setprecision(0) << distanceMatrix[i][j];
            }
            oss << "\n";
        }
        oss << "\n";
    }

    // Getters
    size_t getNumCities() const
    {
        return numCities;
    }

    const std::vector<std::vector<double>> &getDistanceMatrix() const
    {
        return distanceMatrix;
    }

    const std::map<int, std::string> &getCities() const
    {
        return cities;
    }

    // Example usage function
    void runCheapestInsertionExample()
    {
        oss << "CHEAPEST INSERTION TSP SOLVER - EXAMPLE\n";
        oss << std::string(80, '=') << "\n";

        std::vector<std::vector<double>> distanceMatrix = {
            {0, 520, 980, 450, 633},  // City 1
            {520, 0, 204, 888, 557},  // City 2
            {980, 204, 0, 446, 1020}, // City 3
            {450, 888, 446, 0, 249},  // City 4
            {633, 557, 1020, 249, 0}  // City 5
        };

        CheapestInsertionTSP solver(distanceMatrix);

        // Print the distance matrix
        solver.printDistanceMatrix();

        // Solve with starting city 1
        auto [finalRoute, totalCost] = solver.solve(1);

        oss << "Final Results Summary:\n";
        oss << "Route: ";
        for (size_t i = 0; i < finalRoute.size(); ++i)
        {
            if (i > 0)
                oss << " => ";
            oss << finalRoute[i];
        }
        oss << "\nTotal distance: " << std::fixed << std::setprecision(0) << totalCost << "\n";
    }
};

class CheapestInsertion
{
private:
    bool isConsoleOutput;
    std::ostringstream oss;

public:
    CheapestInsertion(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput) {}
    ~CheapestInsertion() = default;

    // Get collected output
    std::string getCollectedOutput() const
    {
        return oss.str();
    }

    void runCheapestInsertion(std::vector<std::vector<double>> distanceMatrix, int startCity = -1)
    {
        oss << "CHEAPEST INSERTION TSP SOLVER\n";
        oss << std::string(80, '=') << "\n";

        CheapestInsertionTSP solver(distanceMatrix);

        // Print the distance matrix
        solver.printDistanceMatrix();

        auto [finalRoute, totalCost] = solver.solve(startCity);

        oss << "Final Results Summary:\n";
        oss << "Route: ";
        for (size_t i = 0; i < finalRoute.size(); ++i)
        {
            if (i > 0)
                oss << " => ";
            oss << finalRoute[i];
        }
        oss << "\nTotal distance: " << std::fixed << std::setprecision(0) << totalCost << "\n";

        oss << solver.getCollectedOutput();

        if (isConsoleOutput)
        {
            std::cout << oss.str();
        }
    }
};