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
    </style>

    <h1 style="margin-top: 60px;" class="row">Dual Simplex</h1>

    <div class="row">
      <label><input type="radio" name="problemType" value="Max" checked> Max</label>
      <label><input type="radio" name="problemType" value="Min"> Min</label>
    </div>

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
        // document.getElementById("problemTypeText").innerText = "Problem is: " + problemType;
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

    document.getElementById("solveButton").onclick = () => {
        try {
            // objFunc = [100, 30];

            // constraints = [
            //     [
            //         0,
            //         1,
            //         3,
            //         1
            //     ],
            //     [
            //         1,
            //         1,
            //         7,
            //         0
            //     ],
            //     [
            //         10,
            //         4,
            //         40,
            //         0
            //     ]
            // ]

            const result = Module.runDualSimplex(objFunc, constraints, problemType);

            const fmt = (num, decimals = 6) => parseFloat(num.toFixed(decimals));

            // Render the tableau iterations
            let html = `<h3>Dual Simplex Result</h3>`;
            html += `<p>Optimal Value: ${fmt(result.optimalSolution)}</p>`;
            // html += `<p>Optimal Variables: [${result.changingVars.map(v => fmt(v)).join(", ")}]</p>`;
            html += `<p>Optimal Variables: ${result.changingVars.map(v => fmt(v)).join(", ")}</p>`;

            result.headerRow.unshift(`t -`)
            result.headerRow.push(``);

            result.tableaus.forEach((tbl, iter) => {
                html += `<h4>Tableau ${iter + 1}</h4>`;
                if (result.phases[iter] == 0) {
                    html += `<h4>Dual Phase</h4>`;
                    result.headerRow[result.headerRow.length - 1] = ("Largest negative");
                }
                else if (result.phases[iter] == 1) {
                    html += `<h4>Primal Phase</h4>`;
                    result.headerRow[result.headerRow.length - 1] = ("θ");
                }
                else {
                    html += `<h4>Optimal Tableau</h4>`;
                    result.headerRow.pop();
                }

                html += `<table border="1" cellpadding="4"><tr>`;

                // Header row
                result.headerRow[0] = (`t - ${iter + 1}`)
                result.headerRow.forEach(h => html += `<th>${h}</th>`);
                html += `</tr>`;

                tbl.forEach((row, rIdx) => {
                    html += `<tr>`;

                    if (rIdx == 0) {
                        html += `<td>Z</td>`;
                    }
                    else {
                        html += `<td>c ${rIdx}</td>`;
                    }


                    row.forEach((v, cIdx) => {
                        let style = "";
                        // Highlight pivot cells
                        if (result.pivotRows[iter] === rIdx || result.pivotCols[iter] === cIdx) {
                            // style = 'style="background-color: lightgreen;"';
                            style = 'style="background-color: #ff8800ff;"';
                        }

                        html += `<td ${style}>${fmt(v)}</td>`;
                    });

                    if (result.phases[iter] == 0) {
                        // html += `<td>${result.pivotCols[iter]}</td>`;
                        if (result.pivotRows[iter] == rIdx) {
                            // html += `<td>${result.pivotRows[iter]}</td>`;
                            html += `<td>${row[row.length - 1]}</td>`;
                        }
                        else {
                            html += `<td></td>`;
                        }
                    }
                    else if (result.phases[iter] == 1) {
                        if (rIdx != 0) {
                            let theta = row[row.length - 1] / row[result.pivotCols[iter]];
                            if (theta == Infinity) {
                                theta = "inv";
                            }
                            else if (theta == -Infinity) {
                                theta = "inv";
                            }
                            else if (isNaN(theta)) {
                                theta = "inv";
                            }
                            else if (theta == 0) {
                                theta = "inv";
                            }
                            html += `<td>${theta}</td>`;
                            // html += `<td>${row[row.length - 1] / row[result.pivotCols[iter]]}</td>`;
                        }
                        else {
                            html += `<td></td>`;
                        }
                    }
                    else {
                        // html += `<td></td>`;
                    }

                    html += `</tr>`;
                });

                if (result.phases[iter] == 0) {
                    html += `<tr>`;
                    html += `<td>θ</td>`;
                    for (let i = 0; i < tbl[0].length; i++) {
                        // html += `<td>${result.pivotCols[iter] / tbl[0][i]}</td>`;
                        let rowCalc = Math.abs(tbl[0][i] / tbl[result.pivotRows[iter]][i])

                        if (rowCalc == Infinity) {
                            rowCalc = "";
                        }
                        else if (rowCalc == -Infinity) {
                            rowCalc = "";
                        }
                        else if (isNaN(rowCalc)) {
                            rowCalc = "";
                        }
                        else if (rowCalc == 0) {
                            rowCalc = "";
                        }

                        html += `<td>${rowCalc}</td>`;
                    }
                    html += `<td></td>`;
                    html += `</tr>`;
                }

                html += `</table>`;
            });

            resultsContainer.innerHTML = html;
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };


    // ===== INITIAL RENDER =====
    updateObjectiveFunction();
    updateConstraints();
    updateProblemType();
}
