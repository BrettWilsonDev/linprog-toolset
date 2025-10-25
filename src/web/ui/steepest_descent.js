export function render(formContainer, resultsContainer, Module) {
    formContainer.innerHTML = `
        <style>
            /* Wrapper to center all content */
            .wrapper {
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: center;
                max-width: 1200px; /* Optional: limits width for better readability */
                margin: 0 auto;
                padding: 20px;
            }

            /* Existing styles */
            #tree { width: 100%; height: 100vh; overflow: hidden; }
            svg { display: block; }
            .link { fill: none; stroke: #ff0000ff; stroke-width: 5px; }
            .node-box { 
                border-radius: 8px; 
                box-shadow: 0 4px 6px rgba(0,0,0,0.1); 
                padding: 8px; 
                box-sizing: border-box; 
                overflow: auto; 
                border: 20px solid red;
            }
            .node-box.integer { background-color: #d1fae5; }
            .node-box.infeasible { background-color: #fee2e2; }
            .node-box.pruned { background-color: #e5e7eb; }
            .tableau table { border-collapse: collapse; margin-top: 6px; margin-left: auto; margin-right: auto; max-width: 100%; }
            .tableau th, .tableau td { border: 1px solid #d1d5db; padding: 3px; text-align: right; font-size: 9px; word-break: break-all; }
            .tableau .pivot-cell { background-color: #CC3300; font-weight: bold; }
            .tableau .pivot-row-col { background-color: #CC3300; }

            /* Form and button styling */
            .row { 
                display: flex; 
                gap: 10px; 
                margin-bottom: 15px; 
                align-items: center;
                justify-content: center; /* Center items in the row */
            }
            button {
                background: #333333;
                color: #FFFFFF;
                border: none;
                padding: 8px 16px;
                border-radius: 4px;
                cursor: pointer;
                font-size: 0.9rem;
                font-weight: bold;
                transition: background 0.2s ease;
            }
            button:hover {
                background: #CC3300;
                color: #000000;
            }
            button#resetButton {
                background: #1A1A1A;
            }
            button#resetButton:hover {
                background: #992600;
            }
            #form input[type="number"], #form select {
                background: #1A1A1A;
                color: #FFFFFF;
                border: 1px solid #333333;
                padding: 6px;
                border-radius: 4px;
                margin: 5px 0;
            }
            #form input[type="number"] {
                // width: 50px;
                -webkit-appearance: none !important;
                -moz-appearance: textfield !important;
                appearance: none !important;
            }
            #form input[type="number"]::-webkit-inner-spin-button,
            #form input[type="number"]::-webkit-outer-spin-button {
                -webkit-appearance: none !important;
                display: none !important;
                margin: 0 !important;
                opacity: 0 !important;
            }
            #form input[type="number"] {
                -moz-appearance: textfield !important;
            }
            #form select {
                width: auto;
                min-width: 60px;
            }
            #form input[type="number"]:focus, #form select:focus {
                border-color: #CC3300;
                outline: none;
            }
            #form input[type="radio"] {
                -webkit-appearance: none;
                -moz-appearance: none;
                appearance: none;
                width: 16px;
                height: 16px;
                border: 2px solid #333333;
                border-radius: 50%;
                position: relative;
                cursor: pointer;
                transition: all 0.2s ease;
                margin-right: 8px;
            }
            #form input[type="radio"]:checked {
                border-color: #CC3300;
                background: #CC3300;
            }
            #form input[type="radio"]:checked::after {
                content: '';
                width: 8px;
                height: 8px;
                background: #FFFFFF;
                border-radius: 50%;
                position: absolute;
                top: 50%;
                left: 50%;
                transform: translate(-50%, -50%);
            }
            #form input[type="radio"]:hover {
                border-color: #CC3300;
            }
            #form input[type="radio"]:focus {
                outline: none;
                box-shadow: 0 0 0 2px #CC3300;
            }
            #form input[type="checkbox"] {
                -webkit-appearance: none;
                -moz-appearance: none;
                appearance: none;
                width: 16px;
                height: 16px;
                border: 2px solid #333333;
                border-radius: 4px;
                position: relative;
                cursor: pointer;
                transition: all 0.2s ease;
                margin-right: 8px;
            }
            #form input[type="checkbox"]:checked {
                border-color: #CC3300;
                background: #CC3300;
            }
            #form input[type="checkbox"]:checked::after {
                content: '✔';
                color: #FFFFFF;
                font-size: 12px;
                position: absolute;
                top: 50%;
                left: 50%;
                transform: translate(-50%, -50%);
            }
            #form input[type="checkbox"]:hover {
                border-color: #CC3300;
            }
            #form input[type="checkbox"]:focus {
                outline: none;
                box-shadow: 0 0 0 2px #CC3300;
            }
            label {
                margin-right: 15px;
                display: flex;
                align-items: center;
            }
            p {
                color: #000000;
                margin-bottom: 10px;
            }
            h1, h3, h4 {
                color: #CC3300;
                margin-bottom: 10px;
                text-align: center; /* Center headings */
            }
            .constraint {
                display: flex;
                gap: 10px;
                margin-bottom: 10px;
                align-items: center;
                justify-content: center; /* Center constraint rows */
            }
            // #results {
            //     background: #1A1A1A;
            //     border: 1px solid #333333;
            //     padding: 10px;
            //     border-radius: 4px;
            //     text-align: center; /* Center text in results */
            // }
            @media (max-width: 600px) {
                .row {
                    flex-wrap: wrap;
                    gap: 8px;
                }
                button {
                    padding: 6px 12px;
                    font-size: 0.85rem;
                }
                #form input[type="number"] {
                    width: 40px;
                }
                #form input[type="radio"], #form input[type="checkbox"] {
                    width: 14px;
                    height: 14px;
                }
                #form input[type="radio"]:checked::after, #form input[type="checkbox"]:checked::after {
                    width: 7px;
                    height: 7px;
                    font-size: 10px;
                }
            }
        </style>

        <h1 style="margin-top: 60px;">Steepest Descent</h1>
        <div class="row">
            <label>
                <input type="radio" name="problemType" value="Max" checked> Max
            </label>
            <label>
                <input type="radio" name="problemType" value="Min"> Min
            </label>
        </div>
        <div class="row" id="problemTypeText">Problem is: Max</div>
        <div id="InputSection">
            <div>
                <label class="row">Function</label>
                <div class="row">
                    <input class="row" type="text" id="funcInput" placeholder="x^2 + y^2 + 2*x + 4">
                </div>
            </div>
            <div id="pointsSection">
                <label class="row">Points</label>
                <div class="row" id="pointsInputs">
                    <!-- No initial input -->
                </div>
            </div>
        </div>
        <div class="row">
            <button onclick="window.location.href = './src/web/ui/python/steepest-descent/';">Sympy Version</button>
        </div>
        <div class="row">
            <button id="solveButton">Solve</button>
            <button style="background-color: red;" id="resetButton">Reset</button>
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

    function handleImplicitMultiplication(func) {
        // Protect math functions by replacing them temporarily
        const mathFunctions = /\b(sin|cos|tan|asin|acos|atan|sinh|cosh|tanh|log|ln|exp|sqrt|abs)\b/gi;
        let modifiedFunc = func.replace(mathFunctions, match => `_${match}_`);

        // Match numbers followed by variables (e.g., 2x, 22xy) and insert * between number and variables
        modifiedFunc = modifiedFunc.replace(/(\d+)([a-zA-Z]+)/g, '$1*$2');

        // Restore protected math functions
        modifiedFunc = modifiedFunc.replace(/_(sin|cos|tan|asin|acos|atan|sinh|cosh|tanh|log|ln|exp|sqrt|abs)_/gi, '$1');

        return modifiedFunc;
    }

    // Function input event listener
    document.getElementById("funcInput").addEventListener("input", (event) => {
        func = event.target.value;

        func = handleImplicitMultiplication(func);

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

        func = handleImplicitMultiplication(func);

        func = func.replace(/\*\*/g, '^');


        // Extract variables, excluding math functions
        const mathFunctions = /\b(sin|cos|tan|asin|acos|atan|sinh|cosh|tanh|log|ln|exp|sqrt|abs)\b/gi;
        const cleanFunc = func.replace(mathFunctions, '');
        const regex = /\b[a-zA-Z]+\b/g;
        const variables = Array.from(new Set(cleanFunc.match(regex) || []));

        points = Array.from(document.querySelectorAll(".pointInput")).map(input => parseFloat(input.value) || 0);


        // console.log(func);

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