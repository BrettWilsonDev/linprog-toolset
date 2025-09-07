#ifndef NEAREST_NEIGHBOUR_TSP_HPP
#define NEAREST_NEIGHBOUR_TSP_HPP

#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <set>
#include <string>
#include <sstream>
#include <utility>

class NearestNeighbourTSP {
private:
    std::vector<std::vector<double>> distanceMatrix;
    size_t numCities;

public:
    NearestNeighbourTSP() = default;

    // Constructor
    explicit NearestNeighbourTSP(const std::vector<std::vector<double>>& distMatrix)
        : distanceMatrix(distMatrix) {
        numCities = distanceMatrix.size();
    }
    
    // Print TSP formulation
    void printFormulation(const std::vector<std::vector<double>>& distMatrix) const {
        size_t n = distMatrix.size();
        
        // Objective function
        std::cout << "Objective function:\n";
        std::vector<std::string> terms;
        
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if (i != j) {
                    std::ostringstream term;
                    term << distMatrix[i][j] << "x" << (i+1) << (j+1);
                    terms.push_back(term.str());
                }
            }
        }
        
        std::cout << "min z = ";
        for (size_t i = 0; i < terms.size(); ++i) {
            if (i > 0) std::cout << " + ";
            std::cout << terms[i];
        }
        std::cout << "\n\n";
        
        // Arriving once in a city constraints
        std::cout << "Arriving once in a city constraints:\n";
        
        // Each city is entered exactly once
        for (size_t j = 0; j < n; ++j) {
            std::vector<std::string> constraintTerms;
            for (size_t i = 0; i < n; ++i) {
                if (i != j) {
                    std::ostringstream term;
                    term << "x" << (i+1) << (j+1);
                    constraintTerms.push_back(term.str());
                }
            }
            
            for (size_t i = 0; i < constraintTerms.size(); ++i) {
                if (i > 0) std::cout << " + ";
                std::cout << constraintTerms[i];
            }
            std::cout << " = 1\n";
        }
        
        // Each city is left exactly once
        for (size_t i = 0; i < n; ++i) {
            std::vector<std::string> constraintTerms;
            for (size_t j = 0; j < n; ++j) {
                if (i != j) {
                    std::ostringstream term;
                    term << "x" << (i+1) << (j+1);
                    constraintTerms.push_back(term.str());
                }
            }
            
            for (size_t i = 0; i < constraintTerms.size(); ++i) {
                if (i > 0) std::cout << " + ";
                std::cout << constraintTerms[i];
            }
            std::cout << " = 1\n";
        }
        std::cout << "\n";
        
        // Sub-tour elimination constraints (MTZ)
        std::cout << "Sub-tour constraints:\n";
        for (size_t i = 1; i < n; ++i) {       // Ui for cities 2..n
            for (size_t j = 1; j < n; ++j) {
                if (i != j) {
                    std::cout << "U" << (i+1) << " - U" << (j+1) << " + " 
                             << n << "x" << (i+1) << (j+1) << " <= " << (n-1) << "\n";
                }
            }
        }
    }
    
    // Get distance between two cities (1-indexed)
    double getDistance(int fromCity, int toCity) const {
        return distanceMatrix[fromCity-1][toCity-1];
    }
    
    // Solve using Nearest Neighbour Heuristic with verbose output
    std::pair<std::vector<int>, double> solveNnhVerbose(int startCity = 1) {
        std::cout << "\n=== Formulation ===\n\n";
        printFormulation(distanceMatrix);
        
        std::vector<int> route = {startCity};
        std::set<int> remainingCities;
        
        // Initialize remaining cities (1-indexed)
        for (int i = 1; i <= static_cast<int>(numCities); ++i) {
            if (i != startCity) {
                remainingCities.insert(i);
            }
        }
        
        int step = 1;
        std::cout << "\nInitial city: x" << startCity << "\n";
        
        while (!remainingCities.empty()) {
            int lastCity = route.back();
            
            // Build list of available next steps
            std::vector<std::pair<int, double>> candidates;
            for (int city : remainingCities) {
                candidates.push_back({city, getDistance(lastCity, city)});
            }
            
            // Find nearest city
            auto minElement = std::min_element(candidates.begin(), candidates.end(),
                [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                    return a.second < b.second;
                });
            
            int nearestCity = minElement->first;
            // double nearestDist = minElement->second;
            
            // Print available options
            std::cout << "\nRoute " << step << ": Current city x" << lastCity << "\n";
            std::cout << "Available next moves:\n";
            
            for (const auto& candidate : candidates) {
                int city = candidate.first;
                double dist = candidate.second;
                std::string marker = (city == nearestCity) ? " <-- chosen" : "";
                std::cout << "  x" << lastCity << city << " : " << dist << marker << "\n";
            }
            
            route.push_back(nearestCity);
            remainingCities.erase(nearestCity);
            
            // Print updated route
            std::cout << "Updated route after step " << step << ": ";
            for (size_t i = 0; i < route.size() - 1; ++i) {
                if (i > 0) std::cout << " => ";
                std::cout << "x" << route[i] << route[i+1];
            }
            std::cout << "\n";
            
            step++;
        }
        
        // Return to start
        route.push_back(startCity);
        std::cout << "\nReturn to start:\n";
        
        // Print final route
        for (size_t i = 0; i < route.size() - 1; ++i) {
            if (i > 0) std::cout << " => ";
            std::cout << "x" << route[i] << route[i+1];
        }
        std::cout << "\n";
        
        // Calculate distances
        std::vector<double> distances;
        for (size_t i = 0; i < route.size() - 1; ++i) {
            distances.push_back(getDistance(route[i], route[i+1]));
        }
        
        // Print calculation
        std::cout << "\nZ = ";
        for (size_t i = 0; i < distances.size(); ++i) {
            if (i > 0) std::cout << " + ";
            std::cout << distances[i];
        }
        
        double totalDistance = 0;
        for (double dist : distances) {
            totalDistance += dist;
        }
        
        std::cout << "\nZ = " << totalDistance << "\n";
        
        return {route, totalDistance};
    }
    
    // Solve without verbose output (returns just the result)
    std::pair<std::vector<int>, double> solve(int startCity = 1) {
        std::vector<int> route = {startCity};
        std::set<int> remainingCities;
        
        // Initialize remaining cities (1-indexed)
        for (int i = 1; i <= static_cast<int>(numCities); ++i) {
            if (i != startCity) {
                remainingCities.insert(i);
            }
        }
        
        while (!remainingCities.empty()) {
            int lastCity = route.back();
            
            // Find nearest unvisited city
            int nearestCity = -1;
            double nearestDist = std::numeric_limits<double>::infinity();
            
            for (int city : remainingCities) {
                double dist = getDistance(lastCity, city);
                if (dist < nearestDist) {
                    nearestDist = dist;
                    nearestCity = city;
                }
            }
            
            route.push_back(nearestCity);
            remainingCities.erase(nearestCity);
        }
        
        // Return to start
        route.push_back(startCity);
        
        // Calculate total distance
        double totalDistance = 0;
        for (size_t i = 0; i < route.size() - 1; ++i) {
            totalDistance += getDistance(route[i], route[i+1]);
        }
        
        return {route, totalDistance};
    }
    
    // Print distance matrix
    void printDistanceMatrix() const {
        std::cout << "\nDistance Matrix:\n";
        std::cout << std::string(50, '-') << "\n";
        
        // Header
        std::cout << std::setw(8) << " ";
        for (size_t j = 0; j < numCities; ++j) {
            std::cout << std::setw(8) << ("City" + std::to_string(j+1));
        }
        std::cout << "\n";
        
        // Rows
        for (size_t i = 0; i < numCities; ++i) {
            std::cout << std::setw(8) << ("City" + std::to_string(i+1));
            for (size_t j = 0; j < numCities; ++j) {
                std::cout << std::setw(8) << std::fixed << std::setprecision(0) << distanceMatrix[i][j];
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
    
    // Get number of cities
    size_t getNumCities() const {
        return numCities;
    }
    
    // Get distance matrix
    const std::vector<std::vector<double>>& getDistanceMatrix() const {
        return distanceMatrix;
    }
    // Example usage function
    void runTSPExample() {
        std::cout << "NEAREST NEIGHBOUR TSP SOLVER - EXAMPLE\n";
        std::cout << std::string(80, '=') << "\n";
        
        std::vector<std::vector<double>> distanceMatrix = {
            {0, 520, 980, 450, 633},  // City 1
            {520, 0, 204, 888, 557},  // City 2
            {980, 204, 0, 446, 1020}, // City 3
            {450, 888, 446, 0, 249},  // City 4
            {633, 557, 1020, 249, 0}  // City 5
        };
        
        NearestNeighbourTSP solver(distanceMatrix);
        
        // Print the distance matrix
        solver.printDistanceMatrix();
        
        // Solve with verbose output
        auto [finalRoute, totalCost] = solver.solveNnhVerbose(1);
        
        std::cout << "\nFinal Results:\n";
        std::cout << "Route: ";
        for (size_t i = 0; i < finalRoute.size(); ++i) {
            if (i > 0) std::cout << " => ";
            std::cout << finalRoute[i];
        }
        std::cout << "\nTotal distance: " << totalCost << "\n";
        
        // // Example of solving without verbose output
        // std::cout << "\n" << std::string(80, '=') << "\n";
        // std::cout << "QUICK SOLVE (NO VERBOSE OUTPUT):\n";
        // auto [quickRoute, quickCost] = solver.solve(1);
        
        // std::cout << "Route: ";
        // for (size_t i = 0; i < quickRoute.size(); ++i) {
        //     if (i > 0) std::cout << " => ";
        //     std::cout << quickRoute[i];
        // }
        // std::cout << "\nTotal distance: " << quickCost << "\n";
    }
};


#endif // NEAREST_NEIGHBOUR_TSP_HPP