export function render(formContainer, resultsContainer, Module) {
    // Insert HTML with input section using default browser styles
    formContainer.innerHTML = `
        <h1>Steepest Descent</h1>
        <div>
            <label>
                <input type="radio" name="problemType" value="Max" checked> Max
            </label>
            <label>
                <input type="radio" name="problemType" value="Min"> Min
            </label>
        </div>
        <div id="problemTypeText">Problem is: Max</div>
        <div id="InputSection">
            <div>
                <label>Function</label>
                <input type="text" id="funcInput" placeholder="x^2 + y^2 + 2*x + 4">
            </div>
            <div id="pointsSection">
                <label>Points</label>
                <div id="pointsInputs">
                    <!-- No initial input -->
                </div>
            </div>
        </div>
        <div>
            <button id="solveButton">Solve</button>
            <button id="resetButton">Reset</button>
        </div>
    `;

    // ===== STATE =====
    let problemType = "Max";
    let func = "";
    let points = []; // Array to store point values

    function updateProblemType() {
        problemType = document.querySelector('input[name="problemType"]:checked').value;
        document.getElementById("problemTypeText").innerText = "Problem is: " + problemType;
    }

    // Update points inputs based on number of variables
    function updatePointsInputs(variables) {
        const pointsInputs = document.getElementById("pointsInputs");
        // Preserve existing values
        const currentValues = Array.from(document.querySelectorAll(".pointInput")).map(input => input.value);
        pointsInputs.innerHTML = ''; // Clear existing inputs
        variables.forEach((variable, index) => {
            const newInput = document.createElement("input");
            newInput.type = "number";
            newInput.className = "pointInput";
            newInput.placeholder = `Enter ${variable}`;
            // Restore previous value if available
            if (index < currentValues.length) {
                newInput.value = currentValues[index];
            }
            pointsInputs.appendChild(newInput);
        });
        // Update points array to match current inputs
        points = Array.from(document.querySelectorAll(".pointInput")).map(input => parseFloat(input.value) || 0);
    }

    // Radio button event listeners
    document.querySelectorAll('input[name="problemType"]').forEach(radio => {
        radio.onchange = updateProblemType;
    });

    // Function input event listener
    document.getElementById("funcInput").addEventListener("input", (event) => {
        func = event.target.value;
        // Remove common math functions to avoid false positives
        const mathFunctions = /\b(sin|cos|tan|asin|acos|atan|sinh|cosh|tanh|log|ln|exp|sqrt|abs)\b/gi;
        const cleanFunc = func.replace(mathFunctions, '');
        // Match variable names (one or more letters, not math functions)
        const regex = /\b[a-zA-Z]+\b/g;
        const variables = Array.from(new Set(cleanFunc.match(regex) || []));
        updatePointsInputs(variables);
    });

    function resetRadios() {
        document.querySelector('input[value="Max"]').checked = true;
        document.getElementById("problemTypeText").innerText = "Problem is: Max";
    }

    document.getElementById("resetButton").onclick = () => {
        problemType = "Max";
        func = "";
        points = [];
        document.getElementById("funcInput").value = "";
        updatePointsInputs([]); // Clear points inputs
        resetRadios();
        resultsContainer.innerHTML = "";
    };

    document.getElementById("solveButton").onclick = () => {
        func = document.getElementById("funcInput").value;

        // func = "x^2 + y^2 + 2*x + 4 "
        

        // Extract variables, excluding math functions
        const mathFunctions = /\b(sin|cos|tan|asin|acos|atan|sinh|cosh|tanh|log|ln|exp|sqrt|abs)\b/gi;
        const cleanFunc = func.replace(mathFunctions, '');
        const regex = /\b[a-zA-Z]+\b/g;
        const variables = Array.from(new Set(cleanFunc.match(regex) || []));
        // console.log("Variables:", variables);

        // Collect points values
        points = Array.from(document.querySelectorAll(".pointInput")).map(input => parseFloat(input.value) || 0);


        // points = [2, 1]
        // console.log("Points:", points);

        try {
            if (!func) {
                throw new Error("Function string is empty");
            }
            if (variables.length > 0 && points.length !== variables.length) {
                throw new Error("Number of points must match number of variables");
            }
            if (variables.length === 0 && points.length > 0) {
                throw new Error("No variables detected, but points were provided");
            }
            const isMin = (problemType === "Min");


            const result = Module.runSteepestDescent(func, variables, points, isMin);

            resultsContainer.innerHTML = "";
            resultsContainer.innerHTML = '<pre>' + result.outputString + '</pre>';
        } catch (err) {

            resultsContainer.innerHTML = `<p style="color: red;">Error: ${err.message}</p>`;
        }
    };

    // ===== INITIAL RENDER =====
    resetRadios();
}