#ifndef HUNGARIAN_ALGORITHM_HPP
#define HUNGARIAN_ALGORITHM_HPP

#include <vector>
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <set>
#include <string>
#include <sstream>

class HungarianAlgorithm {
private:
    std::vector<std::vector<double>> originalMatrix;
    std::vector<std::vector<double>> matrix;
    bool maximize;
    double blankValue;
    bool hasBlankValue;
    size_t nRows;
    size_t nCols;
    int stepCount;
    bool dummyAdded;
    
    static constexpr double INF = std::numeric_limits<double>::infinity();
    static constexpr double NEG_INF = -std::numeric_limits<double>::infinity();

public:
    HungarianAlgorithm() = default;

    // Constructor
    HungarianAlgorithm(const std::vector<std::vector<double>>& valMatrix, 
                      bool maximize = false, 
                      double blankValue = 0.0, 
                      bool hasBlankValue = false)
        : originalMatrix(valMatrix), maximize(maximize), blankValue(blankValue), 
          hasBlankValue(hasBlankValue), stepCount(0), dummyAdded(false) {
        
        // Handle blank values
        if (hasBlankValue) {
            for (auto& row : originalMatrix) {
                for (auto& val : row) {
                    if (val == blankValue) {
                        val = maximize ? NEG_INF : INF;
                    }
                }
            }
        }
        
        // Setup matrix for minimization
        if (maximize) {
            matrix = originalMatrix;
            for (auto& row : matrix) {
                for (auto& val : row) {
                    if (val == NEG_INF) {
                        val = INF;
                    } else {
                        val = -val;
                    }
                }
            }
        } else {
            matrix = originalMatrix;
        }
        
        nRows = matrix.size();
        nCols = nRows > 0 ? matrix[0].size() : 0;
    }
    
    // Print matrix utility
    void printMatrix(const std::vector<std::vector<double>>& mat, const std::string& title = "Matrix") const {
        std::cout << "\n" << title << ":\n";
        std::cout << std::string(50, '-') << "\n";
        
        for (size_t i = 0; i < mat.size(); ++i) {
            std::cout << "Row " << i << ": [";
            for (size_t j = 0; j < mat[i].size(); ++j) {
                if (j > 0) std::cout << ", ";
                
                if (mat[i][j] == INF) {
                    std::cout << "   -  ";
                } else if (mat[i][j] == NEG_INF) {
                    std::cout << "   -  ";
                } else {
                    std::cout << std::setw(6) << std::fixed << std::setprecision(1) << mat[i][j];
                }
            }
            std::cout << "]\n";
        }
        std::cout << "\n";
    }
    
    // Step 1: Add dummy rows/columns if needed
    void step1AddDummy() {
        stepCount++;
        std::cout << "STEP " << stepCount << ": Add dummy row/column if needed\n";
        std::cout << std::string(60, '=') << "\n";
        
        if (nRows != nCols) {
            size_t maxDim = std::max(nRows, nCols);
            
            // Find appropriate dummy value
            double highval = 1000.0;
            bool foundFinite = false;
            
            for (const auto& row : matrix) {
                for (double val : row) {
                    if (val != INF && val != NEG_INF) {
                        if (!foundFinite) {
                            highval = maximize ? val : val;
                            foundFinite = true;
                        } else {
                            highval = maximize ? std::min(highval, val) : std::max(highval, val);
                        }
                    }
                }
            }
            
            // Create new square matrix
            std::vector<std::vector<double>> newMatrix(maxDim, std::vector<double>(maxDim, highval));
            
            if (nRows < nCols) {
                // Add dummy rows
                for (size_t i = 0; i < nRows; ++i) {
                    for (size_t j = 0; j < nCols; ++j) {
                        newMatrix[i][j] = matrix[i][j];
                    }
                }
                std::cout << "Added " << (maxDim - nRows) << " dummy row(s) with val " << highval << "\n";
                dummyAdded = true;
            } else {
                // Add dummy columns
                for (size_t i = 0; i < nRows; ++i) {
                    for (size_t j = 0; j < nCols; ++j) {
                        newMatrix[i][j] = matrix[i][j];
                    }
                }
                std::cout << "Added " << (maxDim - nCols) << " dummy column(s) with val " << highval << "\n";
                dummyAdded = true;
            }
            
            matrix = newMatrix;
            nRows = nCols = maxDim;
        } else {
            std::cout << "Matrix is already square - no dummy needed\n";
        }
        
        printMatrix(matrix, "Matrix after Step 1");
    }
    
    // Step 2: Row reduction
    void step2RowReduction() {
        stepCount++;
        std::cout << "STEP " << stepCount << ": Row reduction\n";
        std::cout << std::string(60, '=') << "\n";
        
        for (size_t i = 0; i < nRows; ++i) {
            double minVal = INF;
            bool hasFinite = false;
            
            // Find minimum finite value in row
            for (size_t j = 0; j < nCols; ++j) {
                if (matrix[i][j] != INF && matrix[i][j] != NEG_INF) {
                    minVal = std::min(minVal, matrix[i][j]);
                    hasFinite = true;
                }
            }
            
            if (hasFinite) {
                std::cout << "Row " << i << ": minimum value = " << minVal << "\n";
                for (size_t j = 0; j < nCols; ++j) {
                    if (matrix[i][j] != INF && matrix[i][j] != NEG_INF) {
                        matrix[i][j] -= minVal;
                    }
                }
            } else {
                std::cout << "Row " << i << ": all values are infinite - no reduction needed\n";
            }
        }
        
        printMatrix(matrix, "Matrix after row reduction");
    }
    
    // Step 3: Column reduction
    void step3ColumnReduction() {
        stepCount++;
        std::cout << "STEP " << stepCount << ": Column reduction\n";
        std::cout << std::string(60, '=') << "\n";
        
        for (size_t j = 0; j < nCols; ++j) {
            double minVal = INF;
            bool hasFinite = false;
            
            // Find minimum finite value in column
            for (size_t i = 0; i < nRows; ++i) {
                if (matrix[i][j] != INF && matrix[i][j] != NEG_INF) {
                    minVal = std::min(minVal, matrix[i][j]);
                    hasFinite = true;
                }
            }
            
            if (hasFinite) {
                std::cout << "Column " << j << ": minimum value = " << minVal << "\n";
                for (size_t i = 0; i < nRows; ++i) {
                    if (matrix[i][j] != INF && matrix[i][j] != NEG_INF) {
                        matrix[i][j] -= minVal;
                    }
                }
            } else {
                std::cout << "Column " << j << ": all values are infinite - no reduction needed\n";
            }
        }
        
        printMatrix(matrix, "Matrix after column reduction");
    }
    
    // Find assignment for current zeros
    std::vector<std::pair<int, int>> findAssignment(const std::vector<std::vector<bool>>& zerosMatrix) const {
        std::vector<std::pair<int, int>> assignments;
        std::set<size_t> usedRows, usedCols;
        
        for (size_t i = 0; i < nRows; ++i) {
            for (size_t j = 0; j < nCols; ++j) {
                if (zerosMatrix[i][j] && usedRows.find(i) == usedRows.end() && 
                    usedCols.find(j) == usedCols.end()) {
                    assignments.push_back({static_cast<int>(i), static_cast<int>(j)});
                    usedRows.insert(i);
                    usedCols.insert(j);
                    break;
                }
            }
        }
        
        return assignments;
    }
    
    // Find minimum lines covering all zeros
    std::pair<std::set<size_t>, std::set<size_t>> findMinimumLines() const {
        // Create zeros matrix
        std::vector<std::vector<bool>> zeros(nRows, std::vector<bool>(nCols, false));
        for (size_t i = 0; i < nRows; ++i) {
            for (size_t j = 0; j < nCols; ++j) {
                zeros[i][j] = (matrix[i][j] == 0.0 && matrix[i][j] != INF);
            }
        }
        
        std::set<size_t> markedRows, markedCols;
        auto assignments = findAssignment(zeros);
        std::set<size_t> assignedRows, assignedCols;
        
        for (const auto& assignment : assignments) {
            if (assignment.first != -1 && assignment.second != -1) {
                assignedRows.insert(assignment.first);
                assignedCols.insert(assignment.second);
            }
        }
        
        // Mark unassigned rows
        for (size_t i = 0; i < nRows; ++i) {
            if (assignedRows.find(i) == assignedRows.end()) {
                markedRows.insert(i);
            }
        }
        
        // Iteratively mark rows and columns
        bool changed = true;
        while (changed) {
            changed = false;
            
            // Mark columns that have zeros in marked rows
            for (size_t row : markedRows) {
                for (size_t col = 0; col < nCols; ++col) {
                    if (zeros[row][col] && markedCols.find(col) == markedCols.end()) {
                        markedCols.insert(col);
                        changed = true;
                    }
                }
            }
            
            // Mark rows that are assigned to marked columns
            for (const auto& assignment : assignments) {
                size_t row = assignment.first;
                size_t col = assignment.second;
                if (markedCols.find(col) != markedCols.end() && 
                    markedRows.find(row) == markedRows.end()) {
                    markedRows.insert(row);
                    changed = true;
                }
            }
        }
        
        // Lines are unmarked rows and marked columns
        std::set<size_t> lineRows, lineCols;
        for (size_t i = 0; i < nRows; ++i) {
            if (markedRows.find(i) == markedRows.end()) {
                lineRows.insert(i);
            }
        }
        lineCols = markedCols;
        
        return {lineRows, lineCols};
    }
    
    // Step 4: Check optimality
    bool step4CheckOptimality(std::set<size_t>& lineRows, std::set<size_t>& lineCols) {
        stepCount++;
        std::cout << "STEP " << stepCount << ": Check optimality\n";
        std::cout << std::string(60, '=') << "\n";
        
        auto lines = findMinimumLines();
        lineRows = lines.first;
        lineCols = lines.second;
        
        size_t numLines = lineRows.size() + lineCols.size();
        
        std::cout << "Lines covering all zeros:\n";
        std::cout << "Row lines: ";
        for (size_t row : lineRows) {
            std::cout << row << " ";
        }
        std::cout << "\nColumn lines: ";
        for (size_t col : lineCols) {
            std::cout << col << " ";
        }
        std::cout << "\nTotal lines: " << numLines << "\n";
        std::cout << "Matrix size: " << nRows << "\n";
        
        if (numLines == nRows) {
            std::cout << "Number of lines equals matrix size - OPTIMAL!\n";
            return true;
        } else {
            std::cout << "Number of lines ≠ matrix size - NOT OPTIMAL\n";
            return false;
        }
    }
    
    // Step 5: Improve solution
    void step5ImproveSolution(const std::set<size_t>& lineRows, const std::set<size_t>& lineCols) {
        stepCount++;
        std::cout << "STEP " << stepCount << ": Improve solution\n";
        std::cout << std::string(60, '=') << "\n";
        
        // Create coverage matrix
        std::vector<std::vector<bool>> covered(nRows, std::vector<bool>(nCols, false));
        
        for (size_t i : lineRows) {
            for (size_t j = 0; j < nCols; ++j) {
                covered[i][j] = true;
            }
        }
        for (size_t j : lineCols) {
            for (size_t i = 0; i < nRows; ++i) {
                covered[i][j] = true;
            }
        }
        
        // Find minimum uncovered value
        std::vector<double> uncoveredValues;
        for (size_t i = 0; i < nRows; ++i) {
            for (size_t j = 0; j < nCols; ++j) {
                if (!covered[i][j] && matrix[i][j] != INF && matrix[i][j] != NEG_INF) {
                    uncoveredValues.push_back(matrix[i][j]);
                }
            }
        }
        
        if (uncoveredValues.empty()) {
            std::cout << "No uncovered finite values found - algorithm may not converge\n";
            return;
        }
        
        double a = *std::min_element(uncoveredValues.begin(), uncoveredValues.end());
        std::cout << "Step 5a: Smallest uncovered value a = " << a << "\n";
        std::cout << "Step 5b: Subtract a from uncovered elements\n";
        std::cout << "Step 5c: Add a to doubly covered elements\n";
        
        // Apply improvement
        for (size_t i = 0; i < nRows; ++i) {
            for (size_t j = 0; j < nCols; ++j) {
                if (matrix[i][j] != INF && matrix[i][j] != NEG_INF) {
                    bool rowCovered = lineRows.find(i) != lineRows.end();
                    bool colCovered = lineCols.find(j) != lineCols.end();
                    
                    if (!rowCovered && !colCovered) {
                        matrix[i][j] -= a;
                    } else if (rowCovered && colCovered) {
                        matrix[i][j] += a;
                    }
                }
            }
        }
        
        printMatrix(matrix, "Matrix after improvement");
    }
    
    // Find optimal assignment from zeros
    std::vector<std::pair<int, int>> findOptimalAssignment(const std::vector<std::vector<bool>>& zerosMatrix) const {
        std::vector<std::pair<int, int>> assignment;
        std::vector<std::vector<bool>> tempZeros = zerosMatrix;
        
        while (true) {
            bool assigned = false;
            
            // Look for rows with exactly one zero
            for (size_t i = 0; i < nRows; ++i) {
                std::vector<size_t> zeroCols;
                for (size_t j = 0; j < nCols; ++j) {
                    if (tempZeros[i][j]) {
                        zeroCols.push_back(j);
                    }
                }
                
                if (zeroCols.size() == 1) {
                    size_t j = zeroCols[0];
                    assignment.push_back({static_cast<int>(i), static_cast<int>(j)});
                    // Remove this row and column
                    for (size_t k = 0; k < nCols; ++k) tempZeros[i][k] = false;
                    for (size_t k = 0; k < nRows; ++k) tempZeros[k][j] = false;
                    assigned = true;
                    break;
                }
            }
            
            if (assigned) continue;
            
            // Look for columns with exactly one zero
            for (size_t j = 0; j < nCols; ++j) {
                std::vector<size_t> zeroRows;
                for (size_t i = 0; i < nRows; ++i) {
                    if (tempZeros[i][j]) {
                        zeroRows.push_back(i);
                    }
                }
                
                if (zeroRows.size() == 1) {
                    size_t i = zeroRows[0];
                    assignment.push_back({static_cast<int>(i), static_cast<int>(j)});
                    // Remove this row and column
                    for (size_t k = 0; k < nCols; ++k) tempZeros[i][k] = false;
                    for (size_t k = 0; k < nRows; ++k) tempZeros[k][j] = false;
                    assigned = true;
                    break;
                }
            }
            
            if (assigned) continue;
            
            // Arbitrary choice if no unique assignments
            bool found = false;
            for (size_t i = 0; i < nRows && !found; ++i) {
                for (size_t j = 0; j < nCols; ++j) {
                    if (tempZeros[i][j]) {
                        assignment.push_back({static_cast<int>(i), static_cast<int>(j)});
                        // Remove this row and column
                        for (size_t k = 0; k < nCols; ++k) tempZeros[i][k] = false;
                        for (size_t k = 0; k < nRows; ++k) tempZeros[k][j] = false;
                        found = true;
                        break;
                    }
                }
            }
            
            if (!found) break;
        }
        
        return assignment;
    }
    
    // Step 6: Find final assignment
    std::pair<std::vector<std::pair<int, int>>, double> step6FindAssignment() {
        stepCount++;
        std::cout << "STEP " << stepCount << ": Find optimal assignment\n";
        std::cout << std::string(60, '=') << "\n";
        
        // Create zeros matrix
        std::vector<std::vector<bool>> zeros(nRows, std::vector<bool>(nCols, false));
        for (size_t i = 0; i < nRows; ++i) {
            for (size_t j = 0; j < nCols; ++j) {
                zeros[i][j] = (matrix[i][j] == 0.0 && matrix[i][j] != INF);
            }
        }
        
        auto assignment = findOptimalAssignment(zeros);
        
        std::cout << "Optimal assignment (row, col):\n";
        double totalVal = 0.0;
        std::vector<std::pair<int, int>> validAssignments;
        
        for (size_t i = 0; i < assignment.size(); ++i) {
            int row = assignment[i].first;
            int col = assignment[i].second;
            
            if (row < static_cast<int>(originalMatrix.size()) && 
                col < static_cast<int>(originalMatrix[0].size())) {
                
                double val = originalMatrix[row][col];
                
                if (!hasBlankValue || val != blankValue) {
                    totalVal += val;
                    validAssignments.push_back({row, col});
                    
                    if (hasBlankValue && val == blankValue) {
                        std::cout << "  Assignment " << (i+1) << ": Row " << row 
                                 << " => Column " << col << " (FORBIDDEN - blank slot!)\n";
                    } else {
                        std::cout << "  Assignment " << (i+1) << ": Row " << row 
                                 << " => Column " << col << " (val: " << val << ")\n";
                    }
                } else {
                    std::cout << "  Assignment " << (i+1) << ": Row " << row 
                             << " => Column " << col << " (FORBIDDEN - blank slot!)\n";
                }
            } else {
                std::cout << "  Assignment " << (i+1) << ": Row " << row 
                         << " => Column " << col << " (dummy assignment - ignored)\n";
            }
        }
        
        std::string problemType = maximize ? "maximization" : "minimization";
        std::cout << "\nTotal optimal val (" << problemType << "): " << totalVal << "\n";
        
        return {validAssignments, totalVal};
    }
    
    // Main solve function
    std::pair<std::vector<std::pair<int, int>>, double> solve() {
        std::string problemType = maximize ? "MAXIMIZATION" : "MINIMIZATION";
        std::cout << "HUNGARIAN ALGORITHM - " << problemType << " PROBLEM - STEP BY STEP SOLUTION\n";
        std::cout << std::string(70, '=') << "\n";
        
        printMatrix(originalMatrix, "Original val Matrix");
        
        if (maximize) {
            std::cout << "Converting maximization to minimization problem...\n";
            printMatrix(matrix, "Converted Matrix (for minimization)");
        }
        
        step1AddDummy();
        step2RowReduction();
        step3ColumnReduction();
        
        const int maxIterations = 10;
        int iteration = 0;
        
        while (iteration < maxIterations) {
            std::set<size_t> lineRows, lineCols;
            bool optimal = step4CheckOptimality(lineRows, lineCols);
            
            if (optimal) {
                break;
            } else {
                step5ImproveSolution(lineRows, lineCols);
                iteration++;
            }
        }
        
        if (iteration >= maxIterations) {
            std::cout << "WARNING: Maximum iterations (" << maxIterations << ") reached!\n";
        }
        
        return step6FindAssignment();
    }
    // Example usage function
    void runExamples() {
        std::cout << "EXAMPLE: Problem with Blank Slots (Forbidden Assignments)\n";
        std::cout << std::string(80, '=') << "\n";
        
        const double FORBIDDEN = -999;
        // std::vector<std::vector<double>> costMatrixWithBlanks = {
        //     {22, 18, 30, 18},
        //     {18, FORBIDDEN, 27, 22},
        //     {26, 20, 28, 28},
        //     {16, 22, FORBIDDEN, 14},
        //     {21, FORBIDDEN, 25, 28}
        // };

        std::vector<std::vector<double>> costMatrixWithBlanks = {
            {22, 18, 30, 18},
            {18, FORBIDDEN, 27, 22},
            {26, 20, 28, 28},
            {16, 22, FORBIDDEN, 14},
            {21, FORBIDDEN, 25, 28}
        };
        
        HungarianAlgorithm hungarian(costMatrixWithBlanks, false, FORBIDDEN, true);
        auto result = hungarian.solve();
        
        std::cout << "\nFinal Results:\n";
        std::cout << "Assignments: ";
        for (const auto& assignment : result.first) {
            std::cout << "(" << assignment.first << "," << assignment.second << ") ";
        }
        std::cout << "\nTotal cost: " << result.second << "\n";
    }
};


#endif // HUNGARIAN_ALGORITHM_HPP