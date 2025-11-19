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
        .row {
            display: flex;
            gap: 10px; /* Space between buttons */
            margin-bottom: 15px; /* Space between rows */
            align-items: center;
            justify-content: center; /* Center items in the row */
            flex-wrap: wrap;
        }
        button {
            background: #333333; /* Dark gray base */
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
            background: #CC3300; /* Deep orange on hover */
            color: #000000;
        }
        button#resetButton {
            background: #1A1A1A; /* Darker gray for distinction */
        }
        button#resetButton:hover {
            background: #992600; /* Darker orange for reset hover */
        }
        #form input[type="radio"], #form input[type="number"], #form select {
            background: #1A1A1A; /* Dark input background */
            color: #FFFFFF;
            border: 1px solid #333333;
            padding: 6px;
            border-radius: 4px;
            margin: 5px 0;
        }
        /* Custom radio button styling */
        #form input[type="radio"] {
            -webkit-appearance: none;
            -moz-appearance: none;
            appearance: none;
            width: 16px;
            height: 16px;
            border: 2px solid #333333; /* Border for unchecked state */
            border-radius: 50%;
            position: relative;
            cursor: pointer;
            transition: all 0.2s ease;
            margin-right: 8px;
        }
        #form input[type="radio"]:checked {
            border-color: #CC3300; /* Orange border for checked */
            background: #CC3300; /* Orange fill */
        }
        #form input[type="radio"]:checked::after {
            content: '';
            width: 8px;
            height: 8px;
            background: #FFFFFF; /* White inner dot */
            border-radius: 50%;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
        }
        #form input[type="radio"]:hover {
            border-color: #CC3300; /* Orange border on hover */
        }
        #form input[type="radio"]:focus {
            outline: none;
            box-shadow: 0 0 0 2px #CC3300; /* Orange focus ring */
        }
        #form input[type="number"] {
            width: 50px; /* Reasonable width for number inputs */
            -webkit-appearance: none !important; /* Force remove arrows */
            -moz-appearance: textfield !important; /* Force remove arrows for Firefox */
            appearance: none !important; /* Force remove arrows */
        }
        #form input[type="number"]::-webkit-inner-spin-button,
        #form input[type="number"]::-webkit-outer-spin-button {
            -webkit-appearance: none !important; /* Force hide arrows in Chrome/Safari */
            display: none !important;
            margin: 0 !important;
            opacity: 0 !important;
        }
        #form input[type="number"] {
            -moz-appearance: textfield !important; /* Reinforce for Firefox */
        }
        #form select {
            width: auto;
            min-width: 60px;
        }
        #form input[type="number"]:focus, #form select:focus {
            border-color: #CC3300; /* Deep orange border on focus */
            outline: none;
        }
        label {
            color: #CCCCCC; /* Light gray for labels */
            margin-right: 15px;
            display: flex;
            align-items: center;
        }
        p {
            color: #FFFFFF;
            margin-bottom: 10px;
            // text-align: center; /* Center paragraphs */
        }
        h1, h3, h4 {
            color: #CC3300; /* Deep orange headings */
            margin-bottom: 10px;
            // text-align: center; /* Center headings */
        }
        .constraint {
            display: flex;
            gap: 10px;
            margin-bottom: 10px;
            align-items: center;
            justify-content: center; /* Center constraint rows */
            flex-wrap: wrap;
        }
        #output, #results {
            background: #1A1A1A;
            border: 1px solid #333333;
            padding: 10px;
            border-radius: 4px;
            color: #FFFFFF;
            // text-align: center; /* Center text in output/results */
        }
        table {
            border-collapse: collapse;
            // border: none;
            margin-bottom: 15px;
            // color: #FFFFFF;
            // margin-left: auto; /* Center table */
            margin-right: auto;
        }
        th, td {
            border: 1px solid #333333;
            padding: 6px;
            text-align: center;
        }
        th {
            background: #1A1A1A;
            color: #CC3300; /* Deep orange table headers */
        }
        .pivot {
            // background-color: #4CAF50; /* Green for pivot highlighting */
            background-color: #CC3300; /* Green for pivot highlighting */
        }
        @media (max-width: 600px) {
            .row {
                flex-wrap: wrap; /* Allow buttons to wrap on small screens */
                gap: 8px;
            }
            button {
                padding: 6px 12px;
                font-size: 0.85rem;
            }
            #form input[type="number"] {
                width: 40px; /* Reasonable width for mobile */
            }
            #form input[type="radio"] {
                width: 14px;
                height: 14px;
            }
            #form input[type="radio"]:checked::after {
                width: 7px;
                height: 7px;
            }
        }

        /* Checkbox styling */
        input[type="checkbox"] {
        accent-color: #CC3300; /* Amber for checkbox */
        width: 16px;
        height: 16px;
        cursor: pointer;
        background: #1A1A1A; /* Dark gray background */
        border: 1px solid #333333; /* Match theme border */
        border-radius: 3px; /* Slightly rounded for aesthetics */
        appearance: none; /* Remove default browser styling */
        -webkit-appearance: none;
        -moz-appearance: none;
        position: relative;
        }

        input[type="checkbox"]:checked {
        background: #CC3300; /* Amber when checked */
        border-color: #CC3300;
        }

        input[type="checkbox"]:checked::after {
        content: "✔"; /* Checkmark symbol */
        color: #000000; /* Black for contrast */
        font-size: 12px;
        position: absolute;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        }

        input[type="checkbox"]:focus {
        outline: 2px solid #CC3300; /* Amber outline on focus */
        outline-offset: 2px;
        }

        input[type="checkbox"]:hover {
        border-color: #FF9800; /* Lighter amber on hover */
        }

        /* Checkbox labels */
        label[for] {
        color: #FFFFFF;
        font-size: 1rem;
        display: flex;
        align-items: center;
        gap: 0.5rem;
        cursor: pointer;
        }

        /* Specific checkbox styling for dynamic containers */
        #distanceMatrixContainer input[type="checkbox"],
        #objectiveFunction input[type="checkbox"],
        #constraintsContainer input[type="checkbox"] {
        margin: 2px; /* Consistent spacing with other inputs */
        }

    </style>

    <h1 style="margin-top: 60px;" class="row">Mathematical Preliminaries</h1>

    <div class="row">
        <label><input type="radio" name="problemType" value="Max" checked> Max</label>
        <label><input type="radio" name="problemType" value="Min"> Min</label>
    </div>
    <br>
    <br>
    <div class="row">
        <button id="addDecisionVar">decision variables +</button>
        <button id="removeDecisionVar">decision variables -</button>
    </div>
    
    <div class="row">
        <p>Use + to mark the item for delta analysis. O is the current selection.</p>
    </div>
    <div class="row">
        <div id="dStoreContainer" style="display: inline-flex; align-items: center;">
            <p style="margin-right: 5px;">For no selection select:</p>
            <div id="dStore"></div>
        </div>
    </div>

    <div id="objectiveFunction" class="row"></div>

    <div id="rowDiv" class="row">
        <button id="addConstraint">Constraint +</button>
        <button id="removeConstraint">Constraint -</button>
    </div>

    <div id="constraintsContainer"></div>
    <br>
    <div class="row">
        <label><input type="radio" name="lockOptTab" value="On"> Range Analyzer On</label>
        <label><input type="radio" name="lockOptTab" value="Off" checked> Range Analyzer Off</label>
    </div>
    <br>
    <div class="row">
        <input type="number" id="valueRangeInput" step="any" placeholder="0" style="display: none;">
        <p id="valueRangeText"></p>
    </div>
    <br>
    <div class="row">
        <label><input type="radio" name="solveDelta" value="On"> solve Delta On</label>
        <label><input type="radio" name="solveDelta" value="Off" checked> Solve Delta Off</label>
    </div>
    <br>
    <div class="row">
        <button id="solveButton">Solve</button>
        <button style="background-color: red;" id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
    </div>
  `;

    let problemType = "Max";
    let absProblemType = "Off";
    let lockOptimalTab = "Off";
    let solveDelta = "Off";
    let amtOfObjVars = 2;
    let amtOfConstraints = 1;
    let objFunc = [0.0, 0.0];
    let constraints = [[0.0, 0.0, 0.0, 0.0]];
    let signItems = ["<=", ">="];
    let signItemsChoices = [0];
    let currentDeltaSelection = "dStore0";

    function updateProblemType() {
        problemType = document.querySelector('input[name="problemType"]:checked').value;
    }

    function updateAbsProblemType() {
        absProblemType = document.querySelector('input[name="absProblemType"]:checked').value;
    }

    function updateLockOptTab() {
        lockOptimalTab = document.querySelector('input[name="lockOptTab"]:checked').value;
    }

    function updateSolveDelta() {
        solveDelta = document.querySelector('input[name="solveDelta"]:checked').value;
    }

    function updateObjectiveFunction() {

        document.getElementById("dStore").innerHTML = "";
        const button = document.createElement("button");
        button.innerText = currentDeltaSelection === `dStore0` ? "O" : "+";
        button.addEventListener("click", () => {
            currentDeltaSelection = `dStore0`;
            updateObjectiveFunction();
            updateConstraints();
        });
        document.getElementById("dStore").appendChild(button);


        const objectiveFunctionContainer = document.getElementById("objectiveFunction");
        objectiveFunctionContainer.innerHTML = "";

        const rowDiv = document.createElement("div");
        rowDiv.className = "row";

        for (let i = 0; i < amtOfObjVars; i++) {
            const value = objFunc[i];

            const button = document.createElement("button");
            button.innerText = currentDeltaSelection === `o${i}` ? "O" : "+";
            button.addEventListener("click", () => {
                currentDeltaSelection = `o${i}`;
                updateObjectiveFunction();
                updateConstraints();
            });
            rowDiv.appendChild(button);

            const input = document.createElement("input");
            input.type = "number";
            input.value = value;
            input.addEventListener("input", (e) => {
                objFunc[i] = parseFloat(e.target.value);
            });
            rowDiv.appendChild(input);

            const label = document.createElement("span");
            label.innerText = `x${i + 1}`;
            rowDiv.appendChild(label);
        }

        objectiveFunctionContainer.appendChild(rowDiv);
    }

    function updateConstraints() {
        const constraintsContainer = document.getElementById("constraintsContainer");
        constraintsContainer.innerHTML = "";

        for (let i = 0; i < amtOfConstraints; i++) {
            const rowDiv = document.createElement("div");
            rowDiv.className = "row";

            const constraintRow = constraints[i];

            for (let j = 0; j < amtOfObjVars; j++) {
                const value = constraintRow[j];

                const button = document.createElement("button");
                button.innerText = currentDeltaSelection === `c${i}${j}` ? "O" : "+";
                button.addEventListener("click", () => {
                    currentDeltaSelection = `c${i}${j}`;
                    updateObjectiveFunction();
                    updateConstraints();
                });
                rowDiv.appendChild(button);

                const input = document.createElement("input");
                input.type = "number";
                input.value = value;
                input.addEventListener("input", (e) => {
                    constraints[i][j] = parseFloat(e.target.value);
                });
                rowDiv.appendChild(input);

                const label = document.createElement("span");
                label.innerText = `x${j + 1}`;
                rowDiv.appendChild(label);
            }

            const signSelect = document.createElement("select");
            signItems.forEach((sign, index) => {
                const option = document.createElement("option");
                option.value = index;
                option.text = sign;
                signSelect.appendChild(option);
            });
            signSelect.value = signItemsChoices[i];
            signSelect.addEventListener("change", (e) => {
                signItemsChoices[i] = parseInt(e.target.value);
                constraints[i][amtOfObjVars + 1] = signItemsChoices[i];
            });
            rowDiv.appendChild(signSelect);

            // RHS button for delta selection
            const rhsButton = document.createElement("button");
            rhsButton.innerText = currentDeltaSelection === `cRhs${i}` ? "O" : "+";
            rhsButton.addEventListener("click", () => {
                currentDeltaSelection = `cRhs${i}`;
                updateObjectiveFunction();
                updateConstraints();
            });
            rowDiv.appendChild(rhsButton);

            const rhsInput = document.createElement("input");
            rhsInput.type = "number";
            rhsInput.value = constraintRow[amtOfObjVars];
            rhsInput.addEventListener("input", (e) => {
                constraints[i][amtOfObjVars] = parseFloat(e.target.value);
            });
            rowDiv.appendChild(rhsInput);

            constraintsContainer.appendChild(rowDiv);
        }
    }

    document.querySelectorAll('input[name="problemType"]').forEach(radio => {
        radio.onchange = updateProblemType;
    });

    document.querySelectorAll('input[name="absProblemType"]').forEach(radio => {
        radio.onchange = updateAbsProblemType;
    });


    document.querySelectorAll('input[name="lockOptTab"]').forEach(radio => {
        radio.onchange = updateLockOptTab;
    });

    document.querySelectorAll('input[name="solveDelta"]').forEach(radio => {
        radio.onchange = updateSolveDelta;
    });

    document.getElementById("addDecisionVar").addEventListener("click", () => {
        amtOfObjVars++;
        objFunc.push(0.0);
        constraints.forEach(c => c.push(0.0));
        updateObjectiveFunction();
        updateConstraints();
    });

    document.getElementById("removeDecisionVar").addEventListener("click", () => {
        if (amtOfObjVars > 2) {
            amtOfObjVars--;
            objFunc.pop();
            constraints.forEach(c => c.pop());
            updateObjectiveFunction();
            updateConstraints();
        }
    });

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
        const radioButtons = document.querySelectorAll('input[value="Off"]');
        radioButtons.forEach(radioButton => {
            radioButton.checked = true;
        });
    }

    document.querySelectorAll('input[name="lockOptTab"]').forEach(radio => {
        radio.addEventListener('change', () => {
            const valueRangeInput = document.getElementById("valueRangeInput");
            if (radio.value === "On") {
                valueRangeInput.style.display = "inline-block"; // show the input
            } else {
                valueRangeInput.style.display = "none"; // hide the input
            }

            if (currentDeltaSelection === "dStore0" && lockOptimalTab === "On") {
                valueRangeText.style.display = "inline-block";
                valueRangeText.innerHTML = "Please pick a selection";
            }
            else {
                valueRangeText.style.display = "none";
            }
        });
    });

    document.getElementById("resetButton").onclick = () => {
        problemType = "Max";
        absProblemType = "Off";
        lockOptimalTab = "Off";
        solveDelta = "Off";
        amtOfObjVars = 2;
        amtOfConstraints = 1;
        objFunc = [0.0, 0.0];
        constraints = [[0.0, 0.0, 0.0, 0.0]];
        signItems = ["<=", ">="];
        signItemsChoices = [0];
        currentDeltaSelection = "dStore0";

        resultsContainer.innerHTML = "";

        valueRangeInput.value = "0";

        updateObjectiveFunction();
        updateConstraints();
        resetRadios();
    };

    document.querySelectorAll('input[name="lockOptTab"]').forEach(radio => {
        radio.addEventListener('change', () => {
            const value = radio.value;
            // if (value === "On") {
            //     document.getElementById("reoptimizeBtn").removeAttribute("hidden");
            // } else {
            //     document.getElementById("reoptimizeBtn").hidden = true;
            // }
        });
    });

    function renderSimplex(result) {
        let html = "";

        // Clone header row and add an extra column for last element if needed
        let headerRow = [...result.headerRow];
        headerRow.unshift("t -"); // first column for tableau number

        result.reOptTableaus.forEach((tbl, iter) => {
            html += `<h4>Re-optimized Tableau ${iter + 1}</h4>`;
            html += `<table border="1" cellpadding="4"><tr>`;

            // Header row
            headerRow[0] = `t - ${iter + 1}`;
            headerRow.forEach(h => html += `<th>${h}</th>`);
            html += `</tr>`;

            const pivotRow = result.pivotRows[iter];
            const pivotCol = result.pivotCols[iter];

            tbl.forEach((row, rIdx) => {
                html += `<tr>`;
                html += `<td>${rIdx === 0 ? "Z" : `c ${rIdx}`}</td>`;

                // Table cells
                row.forEach((v, cIdx) => {
                    let style = "";
                    if ((pivotRow !== undefined && pivotRow === rIdx) ||
                        (pivotCol !== undefined && pivotCol === cIdx)) {
                        style = 'style="background-color: lightgreen;"';
                    }
                    html += `<td ${style}>${fmt(v)}</td>`;
                });

                html += `</tr>`;
            });

            html += `</table>`;
        });

        return html;
    }

    // Safe formatter
    const fmt = (num, decimals = 6) => {
        if (num === undefined || num === null || isNaN(num)) return "";
        if (!isFinite(num)) return "∞";
        return parseFloat(Number(num).toFixed(decimals));
    };

    document.getElementById("solveButton").onclick = () => {
        try {
            // let objFunc = [3.0, 2.0];
            // let constraints = [
            //     [2.0, 1.0, 100.0, 0.0],
            //     [1.0, 1.0, 80.0, 0.0],
            //     [1.0, 0.0, 45.0, 0.0]
            // ];

            let isMin = (problemType === "Min");
            let absRule = (absProblemType === "On");
            let lockOptimalTabState = (lockOptimalTab === "On");
            let solveDeltaState = (solveDelta === "On");

            if (currentDeltaSelection === "dStore0" && lockOptimalTabState === true) {
                valueRangeText.style.display = "inline-block";
                valueRangeText.innerHTML = "Please pick a selection";
            }
            else {
                valueRangeText.style.display = "none";
            }

            const result = Module.runMathPreliminaries(
                objFunc,
                constraints,
                isMin,
                currentDeltaSelection,
                absRule,
                lockOptimalTabState,
                solveDeltaState,
                valueRangeInput.value
            );

            // Convert emscripten::val array -> JS array
            const toJSArray = (val) => {
                if (!val || !val.length) return [];
                return Array.from({ length: val.length }, (_, i) => val[i]);
            };

            // Render 1D array as a single row table, replacing "d" with Δ
            const renderVector = (vec) => {
                const arr = toJSArray(vec);
                return `<table border="1" cellspacing="0" cellpadding="4">
                    <tr>${arr.map(cell => `<td>${String(cell).replace(/d/g, 'Δ')}</td>`).join('')}</tr>
                </table>`;
            };

            // Render 2D array as a table, replacing "d" with Δ
            const renderMatrix = (mat, header = null) => {
                const rows = toJSArray(mat);
                return `<table border="1" cellspacing="0" cellpadding="4">
                    ${header ? `<tr>${header.map(cell => `<th>${cell}</th>`).join('')}</tr>` : ''}
                    ${rows.map(row => {
                    const cells = toJSArray(row);
                    return `<tr>${cells.map(cell => `<td>${String(cell).replace(/d/g, 'Δ')}</td>`).join('')}</tr>`;
                }).join('')}
                </table>`;
            };

            const renderTables = (mat, header = null) => {
                const rows = toJSArray(mat);
                return `<table border="1" cellspacing="0" cellpadding="4">
                    ${header ? `<tr><th>t-</th>${header.map(cell => `<th>${cell}</th>`).join('')}</tr>` : ''}
                    ${rows.map((row, i) => {
                    const cells = toJSArray(row);

                    // Declare rowLabel once
                    let rowLabel;
                    if (i === 0) {
                        rowLabel = 'Z';
                    } else {
                        rowLabel = `c${i}`;  // Automatic row label
                    }

                    return `<tr>
                                    <th>${rowLabel}</th>
                                    ${cells.map(cell => `<td>${String(cell).replace(/d/g, 'Δ')}</td>`).join('')}
                                </tr>`;
                }).join('')}
                </table>`;
            };

            resultsContainer.innerHTML = `
                <h3>Math Preliminaries Results</h3>

                <h4>cbv</h4>
                ${renderVector(result.cbv)}

                <h4>B</h4>
                ${renderMatrix(result.B)}

                <h4>B^-1</h4>
                ${renderMatrix(result.BInv)}

                <h4>cbvB^-1</h4>
                ${renderVector(result.cbvBInv)}

                <h4>Initial Table</h4>
                ${renderTables(result.initialTable, ['x1', 'x2', 's1', 's2', 's3', 'rhs'])}

                <h4>Optimal Table</h4>
                ${renderTables(result.optimalTable, ['x1', 'x2', 's1', 's2', 's3', 'rhs'])}

                <h4>Changing Table</h4>
                ${renderTables(result.changingTable, ['x1', 'x2', 's1', 's2', 's3', 'rhs'])}
            `;

            if (result.shouldReOptimize) {
                resultsContainer.innerHTML += renderSimplex(result);
            }
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };

    updateObjectiveFunction();
    updateConstraints();
    resetRadios();
}
