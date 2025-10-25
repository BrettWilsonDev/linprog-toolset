import sympy as sp
import numpy as np
from typing import List, Tuple, Optional, Callable

class DetailedSteepestDescentOptimizer:
    """
    Implements the Steepest Ascent/Descent Algorithm with detailed step-by-step output.
    """
    
    def __init__(self, functionExpr: str, variables: List[str], maximize: bool = False):
        """
        Initialize the optimizer.
        
        Args:
            functionExpr: String representation of the function (e.g., "x**2 + y**2 + 2*x + 4")
            variables: List of variable names (e.g., ["x", "y"])
            maximize: If True, finds maximum; if False, finds minimum
        """
        self.variables = [sp.Symbol(var) for var in variables]
        self.varNames = variables
        self.function = sp.sympify(functionExpr)
        self.maximize = maximize
        self.gradient = self._computeGradient()
        self.hessian = self._computeHessian()
        
        self.outputStr = ""  # Initialize once, never reset automatically

    def _computeGradient(self) -> List[sp.Expr]:
        """Compute the gradient of the function."""
        return [sp.diff(self.function, var) for var in self.variables]
    
    def _computeHessian(self) -> sp.Matrix:
        """Compute the Hessian matrix of the function."""
        n = len(self.variables)
        hessian = sp.zeros(n, n)
        for i in range(n):
            for j in range(n):
                hessian[i, j] = sp.diff(self.function, self.variables[i], self.variables[j])
        return hessian
    
    def evaluateFunction(self, point: List[float]) -> float:
        """Evaluate the function at a given point."""
        substitutions = {var: val for var, val in zip(self.variables, point)}
        return float(self.function.subs(substitutions))
    
    def evaluateGradient(self, point: List[float]) -> List[float]:
        """Evaluate the gradient at a given point."""
        substitutions = {var: val for var, val in zip(self.variables, point)}
        return [float(grad.subs(substitutions)) for grad in self.gradient]
    
    def evaluateHessian(self, point: List[float]) -> np.ndarray:
        """Evaluate the Hessian matrix at a given point."""
        substitutions = {var: val for var, val in zip(self.variables, point)}
        hessianVals = self.hessian.subs(substitutions)
        return np.array(hessianVals).astype(float)
    
    def checkCriticalPointNature(self, point: List[float]) -> str:
        """
        Check if a critical point is a minimum, maximum, or saddle point using the Hessian.
        """
        H = self.evaluateHessian(point)
        eigenVals = np.linalg.eigvals(H)
        
        if all(eig > 0 for eig in eigenVals):
            return "Local Minimum"
        elif all(eig < 0 for eig in eigenVals):
            return "Local Maximum"
        else:
            return "Saddle Point"
    
    def detailedStepSizeCalculation(self, currentPoint: List[float], gradient: List[float], verbose: bool = True) -> float:
        """
        Find the optimal step size with detailed algebraic steps shown.
        """
        h = sp.Symbol('h')
        
        self.outputStr += f"    Finding optimal step size 'h':\n"
        self.outputStr += f"    Current point: {currentPoint}\n"
        self.outputStr += f"    Gradient vector: {gradient}\n"
        
        direction = gradient if self.maximize else [-g for g in gradient]
        directionType = "ascent" if self.maximize else "descent"
        
        self.outputStr += f"    Direction vector (for {directionType}): {direction}\n"
        self.outputStr += f"    Formula: x_(i+1) = x_i + h * direction\n"
        
        newPoint = [currentPoint[i] + h * direction[i] for i in range(len(currentPoint))]
        
        self.outputStr += f"    New point expressions:\n"
        for i, varName in enumerate(self.varNames):
            self.outputStr += f"      {varName}_(i+1) = {currentPoint[i]} + h * ({direction[i]}) = {newPoint[i]}\n"
        
        substitutions = {var: val for var, val in zip(self.variables, newPoint)}
        gH = self.function.subs(substitutions)
        
        self.outputStr += f"\n    Substituting new point into function f({', '.join(self.varNames)}):\n"
        originalFuncStr = str(self.function)
        self.outputStr += f"    Original function: f({', '.join(self.varNames)}) = {originalFuncStr}\n"
        
        self.outputStr += f"    Substituting:\n"
        for i, varName in enumerate(self.varNames):
            self.outputStr += f"      {varName} = {newPoint[i]}\n"
        
        self.outputStr += f"    f(x_(i+1)) = {gH}\n"
        
        gHExpanded = sp.expand(gH)
        self.outputStr += f"    Expanded: g(h) = {gHExpanded}\n"
        
        gHCollected = sp.collect(gHExpanded, h)
        self.outputStr += f"    Collected: g(h) = {gHCollected}\n"
        
        dgDh = sp.diff(gH, h)
        
        self.outputStr += f"\n    Taking derivative with respect to h:\n"
        self.outputStr += f"    dg/dh = {dgDh}\n"
        self.outputStr += f"    Setting dg/dh = 0 to find optimal h:\n"
        
        try:
            hOptimalSolutions = sp.solve(dgDh, h)
            if hOptimalSolutions:
                hOptimal = float(hOptimalSolutions[0])
                self.outputStr += f"    Solving: {dgDh} = 0\n"
                self.outputStr += f"    Solution: h = {hOptimal}\n"
                return hOptimal
        except:
            self.outputStr += f"    Analytical solution failed, using numerical method\n"
            return self._goldenSectionSearch(currentPoint, gradient)
        
        self.outputStr += f"    No analytical solution found, using numerical method\n"
        return self._goldenSectionSearch(currentPoint, gradient)
    
    def _goldenSectionSearch(self, currentPoint: List[float], gradient: List[float], 
                             a: float = -2.0, b: float = 2.0, tol: float = 1e-6) -> float:
        phi = (1 + np.sqrt(5)) / 2
        resPhi = 2 - phi
        
        direction = gradient if self.maximize else [-g for g in gradient]
        
        def objective(h):
            newPoint = [currentPoint[i] + h * direction[i] for i in range(len(currentPoint))]
            value = self.evaluateFunction(newPoint)
            return -value if self.maximize else value
        
        x1 = a + resPhi * (b - a)
        x2 = a + (1 - resPhi) * (b - a)
        f1 = objective(x1)
        f2 = objective(x2)
        
        while abs(b - a) > tol:
            if f1 < f2:
                b = x2
                x2 = x1
                f2 = f1
                x1 = a + resPhi * (b - a)
                f1 = objective(x1)
            else:
                a = x1
                x1 = x2
                f1 = f2
                x2 = a + (1 - resPhi) * (b - a)
                f2 = objective(x2)
        
        return (a + b) / 2
    
    def optimize(self, initialPoint: List[float], maxIterations: int = 100, 
                 tolerance: float = 1e-6, verbose: bool = True) -> Tuple[List[float], float, List[dict]]:
        currentPoint = initialPoint.copy()
        history = []
        
        method = "Steepest Ascent" if self.maximize else "Steepest Descent"
        self.outputStr += f"\n{'='*80}\n"
        self.outputStr += f"{method} Algorithm - Detailed Steps\n"
        self.outputStr += f"{'='*80}\n"
        self.outputStr += f"Function: f({', '.join(self.varNames)}) = {self.function}\n"
        self.outputStr += f"Objective: Find {'maximum' if self.maximize else 'minimum'}\n"
        self.outputStr += f"Initial point: {currentPoint}\n"
        self.outputStr += f"Initial function value: f({', '.join([str(p) for p in currentPoint])}) = {self.evaluateFunction(currentPoint)}\n"
        
        self.outputStr += f"\nGradient formulas:\n"
        for i, grad in enumerate(self.gradient):
            self.outputStr += f"  ∂f/∂{self.varNames[i]} = {grad}\n"
        
        self.outputStr += "-" * 80 + "\n"
        
        for iteration in range(maxIterations):
            self.outputStr += f"\n ITERATION {iteration + 1}:\n"
            self.outputStr += f"{'='*40}\n"
            
            grad = self.evaluateGradient(currentPoint)
            gradNorm = np.linalg.norm(grad)
            
            self.outputStr += f"Step 1: Calculate gradient at current point {currentPoint}\n"
            for i, (gradVal, varName) in enumerate(zip(grad, self.varNames)):
                substitutionStr = str(self.gradient[i]).replace(str(self.variables[i]), str(currentPoint[i]))
                for j, (var, val) in enumerate(zip(self.variables, currentPoint)):
                    if j != i:
                        substitutionStr = substitutionStr.replace(str(var), str(val))
                self.outputStr += f"  ∂f/∂{varName} = {self.gradient[i]} = {substitutionStr} = {gradVal}\n"
            
            self.outputStr += f"  Gradient vector: ∇f = [{', '.join([f'{g:.6f}' for g in grad])}]\n"
            self.outputStr += f"  Gradient norm: ||∇f|| = {gradNorm:.6f}\n"
            
            iterInfo = {
                'iteration': iteration + 1,
                'point': currentPoint.copy(),
                'functionValue': self.evaluateFunction(currentPoint),
                'gradient': grad.copy(),
                'gradientNorm': gradNorm
            }
            
            if gradNorm < tolerance:
                self.outputStr += f"\n CONVERGED! Gradient norm {gradNorm:.6f} < tolerance {tolerance}\n"
                self.outputStr += f"The gradient is approximately zero, indicating we're at a critical point.\n"
                history.append(iterInfo)
                break
            
            self.outputStr += f"\nStep 2: Find optimal step size\n"
            
            stepSize = self.detailedStepSizeCalculation(currentPoint, grad, verbose)
            iterInfo['stepSize'] = stepSize
            
            direction = grad if self.maximize else [-g for g in grad]
            newPoint = [currentPoint[i] + stepSize * direction[i] for i in range(len(currentPoint))]
            
            self.outputStr += f"\nStep 3: Update point using optimal step size\n"
            self.outputStr += f"  Step size h = {stepSize:.6f}\n"
            self.outputStr += f"  Direction vector: {direction}\n"
            self.outputStr += f"  New point calculation:\n"
            for i, varName in enumerate(self.varNames):
                self.outputStr += f"    {varName}_(new) = {currentPoint[i]} + ({stepSize:.6f}) * ({direction[i]:.6f}) = {newPoint[i]:.6f}\n"
            
            oldValue = self.evaluateFunction(currentPoint)
            newValue = self.evaluateFunction(newPoint)
            self.outputStr += f"\nStep 4: Verify improvement\n"
            self.outputStr += f"  f(oldPoint) = f({currentPoint}) = {oldValue:.6f}\n"
            self.outputStr += f"  f(newPoint) = f({[round(p, 6) for p in newPoint]}) = {newValue:.6f}\n"
            
            if self.maximize:
                improvement = newValue > oldValue
                self.outputStr += f"  {' Better' if improvement else ' Worse'}: {newValue:.6f} {'>' if improvement else '<='} {oldValue:.6f}\n"
            else:
                improvement = newValue < oldValue
                self.outputStr += f"  {' Better' if improvement else ' Worse'}: {newValue:.6f} {'<' if improvement else '>='} {oldValue:.6f}\n"
            
            currentPoint = newPoint
            history.append(iterInfo)
        
        optimalValue = self.evaluateFunction(currentPoint)
        
        self.outputStr += f"\n{'='*80}\n"
        self.outputStr += " FINAL RESULTS\n"
        self.outputStr += f"{'='*80}\n"
        finalPointStr = ", ".join([f"{self.varNames[i]}={currentPoint[i]:.6f}" for i in range(len(currentPoint))])
        self.outputStr += f"Optimal point: ({finalPointStr})\n"
        self.outputStr += f"Optimal value: {optimalValue:.6f}\n"
        
        self.outputStr += f"\nHessian Analysis:\n"
        self.outputStr += f"Hessian matrix symbolic form:\n"
        self.outputStr += f"{self.hessian}\n"
        
        H = self.evaluateHessian(currentPoint)
        self.outputStr += f"Hessian matrix at optimal point:\n"
        self.outputStr += f"{H}\n"
        
        eigenVals = np.linalg.eigvals(H)
        self.outputStr += f"Eigenvalues: {eigenVals}\n"
        
        nature = self.checkCriticalPointNature(currentPoint)
        self.outputStr += f"Nature of critical point: {nature}\n"
        
        if len(eigenVals) == 2:
            detH = np.linalg.det(H)
            traceH = np.trace(H)
            self.outputStr += f"Determinant: {detH:.6f}\n"
            self.outputStr += f"Trace: {traceH:.6f}\n"
            if detH > 0 and traceH > 0:
                self.outputStr += " det(H) > 0 and tr(H) > 0 → Local Minimum\n"
            elif detH > 0 and traceH < 0:
                self.outputStr += " det(H) > 0 and tr(H) < 0 → Local Maximum\n"
            elif detH < 0:
                self.outputStr += " det(H) < 0 → Saddle Point\n"
            else:
                self.outputStr += "  Inconclusive test\n"
        
        if verbose:
            print(self.outputStr)
        
        return self.outputStr


if __name__ == "__main__":
    custom_optimizer = DetailedSteepestDescentOptimizer("2*x*y + 4*x - 2*x**2 - y**2", ["x", "y"], maximize=False)
    custom_result = custom_optimizer.optimize([0.5, 0.5], verbose=False)

    print(custom_result)