export function render(formContainer, resultsContainer, Module) {
    // Insert HTML
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
            width: 50px;
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

    <h1 style="margin-top: 60px;">Cutting Plane</h1>

    <div class="row">
        <label><input type="radio" name="problemType" value="Max" checked> Max</label>
        <label><input type="radio" name="problemType" value="Min"> Min</label>
    </div>

    <p id="problemTypeText">Problem is: Max</p>

    <div class="row">
      <button id="addDecisionVar">decision variables +</button>
      <button id="removeDecisionVar">decision variables -</button>
    </div>

    <div id="objectiveFunction" class="row"></div>

    <div class="row">
      <button id="addConstraint">Constraint +</button>
      <button id="removeConstraint">Constraint -</button>
    </div>

    <div id="constraintsContainer"></div>

    <div class="row">
      <button id="solveButton">Solve</button>
      <button style="background-color: red;" id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
    </div>
  `;

    // ===== STATE =====
    let problemType = "Max";
    let amtOfObjVars = 2;
    let amtOfConstraints = 1;
    let objFunc = [0.0, 0.0];
    let constraints = [[0.0, 0.0, 0.0, 0.0]];
    let signItems = ["<=", ">="];
    let signItemsChoices = [0];

    function updateProblemType() {
        problemType = document.querySelector('input[name="problemType"]:checked').value;
        document.getElementById("problemTypeText").innerText = "Problem is: " + problemType;
    }

    function updateObjectiveFunction() {
        const objFuncContainer = document.getElementById("objectiveFunction");
        objFuncContainer.innerHTML = "";

        for (let i = 0; i < amtOfObjVars; i++) {
            const input = document.createElement("input");
            input.type = "number";
            input.value = objFunc[i];
            input.oninput = (e) => {
                objFunc[i] = parseFloat(e.target.value);
            };

            const label = document.createElement("span");
            label.innerText = `x${i + 1}`;

            objFuncContainer.appendChild(input);
            objFuncContainer.appendChild(label);
        }
    }

    function updateConstraints() {
        const container = document.getElementById("constraintsContainer");
        container.innerHTML = "";

        for (let i = 0; i < amtOfConstraints; i++) {
            const constraintRow = document.createElement("div");
            constraintRow.className = "constraint";

            for (let j = 0; j < amtOfObjVars; j++) {
                const input = document.createElement("input");
                input.type = "number";
                input.value = constraints[i][j];
                input.oninput = (e) => {
                    constraints[i][j] = parseFloat(e.target.value);
                };

                const label = document.createElement("span");
                label.innerText = `x${j + 1}`;

                constraintRow.appendChild(input);
                constraintRow.appendChild(label);
            }

            const signSelect = document.createElement("select");
            signItems.forEach((sign, index) => {
                const option = document.createElement("option");
                option.value = index;
                option.innerText = sign;
                signSelect.appendChild(option);
            });
            signSelect.value = signItemsChoices[i];
            signSelect.onchange = (e) => {
                signItemsChoices[i] = parseInt(e.target.value);
                constraints[i][amtOfObjVars + 1] = signItemsChoices[i];
            };

            const rhsInput = document.createElement("input");
            rhsInput.type = "number";
            rhsInput.value = constraints[i][amtOfObjVars];
            rhsInput.oninput = (e) => {
                constraints[i][amtOfObjVars] = parseFloat(e.target.value);
            };

            constraintRow.appendChild(signSelect);
            constraintRow.appendChild(rhsInput);

            container.appendChild(constraintRow);
        }
    }

    document.querySelectorAll('input[name="problemType"]').forEach(radio => {
        radio.onchange = updateProblemType;
    });

    document.getElementById("addDecisionVar").onclick = () => {
        amtOfObjVars++;
        objFunc.push(0.0);
        constraints.forEach(constraint => constraint.splice(amtOfObjVars - 1, 0, 0.0));
        updateObjectiveFunction();
        updateConstraints();
    };

    document.getElementById("removeDecisionVar").onclick = () => {
        if (amtOfObjVars > 2) {
            amtOfObjVars--;
            objFunc.pop();
            constraints.forEach(constraint => constraint.splice(amtOfObjVars, 1));
            updateObjectiveFunction();
            updateConstraints();
        }
    };

    document.getElementById("addConstraint").onclick = () => {
        amtOfConstraints++;
        const newConstraint = new Array(amtOfObjVars).fill(0.0);
        newConstraint.push(0.0);  // for sign
        newConstraint.push(0.0);  // for rhs
        constraints.push(newConstraint);
        signItemsChoices.push(0);
        updateConstraints();
    };

    document.getElementById("removeConstraint").onclick = () => {
        if (amtOfConstraints > 1) {
            amtOfConstraints--;
            constraints.pop();
            signItemsChoices.pop();
            updateConstraints();
        }
    };

    function resetRadios() {
        document.querySelector('input[value="Max"]').checked = true;
    }

    document.getElementById("resetButton").onclick = () => {
        problemType = "Max";
        amtOfObjVars = 2;
        amtOfConstraints = 1;
        objFunc = [0.0, 0.0];
        constraints = [[0.0, 0.0, 0.0, 0.0]];
        signItems = ["<=", ">="];
        signItemsChoices = [0];
        resultsContainer.innerHTML = "";

        updateObjectiveFunction();
        updateConstraints();
        resetRadios();
    };

    function fmt(num, decimals = 6) {
        return parseFloat(num.toFixed(decimals));
    }

    // Add these helper functions inside the render function, before solveButton.onclick
    // Add these helper functions inside the render function, before solveButton.onclick
    // Add these helper functions inside the render function, before solveButton.onclick
    // Add these helper functions inside the render function, before solveButton.onclick
    // Add these helper functions inside the render function, before solveButton.onclick
    function parseAndRenderTableaus(solutionText, container) {
        const lines = solutionText.trim().split('\n');
        let currentSection = '';
        let currentTableau = [];
        let headers = [];
        let isTableau = false;
        let pendingPivotRow = null;
        let pendingPivotCol = null;
        let currentPivotRow = null;
        let currentPivotCol = null;

        lines.forEach((line, index) => {
            line = line.trim();
            if (!line) return; // Skip empty lines

            if (line.startsWith("Pivot row:")) {
                pendingPivotRow = parseInt(line.split(":")[1].trim());
                return;
            }
            if (line.startsWith("Pivot col:")) {
                pendingPivotCol = parseInt(line.split(":")[1].trim());
                return;
            }

            // Detect tableau start (e.g., "Initial Tableau 1", "Tableau with cutting plane constraint")
            if (line.match(/^(Initial Tableau|Tableau with cutting plane constraint|Iteration \d+ - Tableau|Final Optimal Tableau)/)) {
                if (currentTableau.length > 0 && headers.length > 0) {
                    renderTableau(currentSection, headers, currentTableau, container, currentPivotRow, currentPivotCol);
                    currentTableau = [];
                    headers = [];
                }
                currentSection = line;
                currentPivotRow = pendingPivotRow;
                currentPivotCol = pendingPivotCol;
                pendingPivotRow = null;
                pendingPivotCol = null;
                isTableau = true;
                return;
            }

            // Detect headers (e.g., "x1        x2      s/e1      s/e2       rhs")
            if (isTableau && line.match(/^(x\d+|s\/e\d+|rhs)\b/)) {
                headers = line.trim().split(/\s{2,}/).filter(h => h.match(/^(x\d+|s\/e\d+|rhs)$/));
                return;
            }

            // Detect tableau data rows (lines with space-separated numbers)
            if (isTableau && line.match(/^-?\d+\.\d+/)) {
                const row = line.trim().split(/\s{2,}/).map(num => {
                    const parsed = parseFloat(num);
                    return isNaN(parsed) ? num : parsed.toFixed(4);
                });
                if (row.length === headers.length) {
                    currentTableau.push(row);
                } else {
                    console.warn(`Row ${index + 1} skipped: length ${row.length} does not match headers ${headers.length}`, { row, headers });
                }
                return;
            }

            // Non-tableau text (e.g., analysis, solution details)
            if (currentTableau.length > 0 && headers.length > 0) {
                renderTableau(currentSection, headers, currentTableau, container, currentPivotRow, currentPivotCol);
                currentTableau = [];
                headers = [];
                isTableau = false;
            }
            if (line) {
                const pre = document.createElement('pre');
                pre.textContent = line;
                container.appendChild(pre);
            }
        });

        // Render the last tableau, if any
        if (currentTableau.length > 0 && headers.length > 0) {
            renderTableau(currentSection, headers, currentTableau, container, currentPivotRow, currentPivotCol);
        }
    }

    function renderTableau(section, headers, data, container, pivotRow = null, pivotCol = null) {
        if (headers.length === 0 || data.length === 0 || data.some(row => row.length !== headers.length)) {
            console.warn(`Skipping tableau "${section}": invalid headers or data`, { headers, data });
            return;
        }

        container.appendChild(document.createElement('br'));

        const h2 = document.createElement('h2');
        h2.textContent = section;
        container.appendChild(h2);

        const table = document.createElement('table');
        table.style.borderCollapse = 'collapse'; // Ensure inline for priority
        table.style.border = '2px solid black'; // Stronger table border
        const thead = document.createElement('thead');
        const tr = document.createElement('tr');
        headers.forEach((header, index) => {
            const th = document.createElement('th');
            th.textContent = header;
            th.style.border = '2px solid black'; // Stronger cell borders
            th.style.padding = '8px';
            // th.style.backgroundColor = '#f2f2f2';
            th.style.backgroundColor = '#CC3300';
            th.style.textAlign = 'center'; // Center headers for clarity
            if (index === pivotCol) {
                // th.style.backgroundColor = 'lightgreen';
                th.style.backgroundColor = '#ff8800ff';
            }
            tr.appendChild(th);
        });
        thead.appendChild(tr);
        table.appendChild(thead);

        const tbody = document.createElement('tbody');
        data.forEach((row, rowIndex) => {
            const tr = document.createElement('tr');
            row.forEach((cell, cellIndex) => {
                const td = document.createElement('td');
                td.textContent = cell;
                td.style.border = '2px solid black'; // Stronger cell borders
                td.style.padding = '8px';
                td.style.textAlign = 'right'; // Right-align numbers
                if (rowIndex === pivotRow || cellIndex === pivotCol) {
                    // td.style.backgroundColor = 'lightgreen';
                    td.style.backgroundColor = '#ff8800ff';
                }
                tr.appendChild(td);
            });
            tbody.appendChild(tr);
        });
        table.appendChild(tbody);
        container.appendChild(table);
        container.appendChild(document.createElement('br'));
    }

    document.getElementById("solveButton").onclick = () => {
        try {
            // objFunc = [13, 8];

            // constraints = [
            //     [1, 2, 10, 0],
            //     [5, 2, 20, 0],
            // ];

            let isMin = (problemType === "Min");

            let result = Module.runCuttingPlane(objFunc, constraints, isMin);


            resultsContainer.innerHTML = "";
            parseAndRenderTableaus(result.solution, resultsContainer);

            // resultsContainer.innerHTML = "";
            // const preElement = document.createElement("pre");
            // preElement.textContent = result.solution;
            // resultsContainer.appendChild(preElement);

        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };


    // ===== INITIAL RENDER =====
    updateObjectiveFunction();
    updateConstraints();
    updateProblemType();
}
