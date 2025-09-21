export function render(formContainer, resultsContainer, Module) {
    // Insert HTML
    formContainer.innerHTML = `
    <h1>Mathematical Preliminaries</h1>

    <div>
        <label><input type="radio" name="problemType" value="Max" checked> Max</label>
        <label><input type="radio" name="problemType" value="Min"> Min</label>
    </div>
    <br>
    <div>
        <!-- <label><input type="radio" name="absProblemType" value="On"> Abs On</label> -->
        <!-- <label><input type="radio" name="absProblemType" value="Off" checked> Abs Off</label> -->
    </div>
    <br>
    <div class="row">
        <button id="addDecisionVar">decision variables +</button>
        <button id="removeDecisionVar">decision variables -</button>
    </div>
    
    <p>Use + to mark the item for delta analysis. O is the current selection.</p>
    <div id="dStoreContainer" style="display: inline-flex; align-items: center;">
        <p style="margin-right: 5px;">For no selection select:</p>
        <div id="dStore"></div>
    </div>

    <div id="objectiveFunction" class="row"></div>

    <div id="rowDiv" class="row">
        <button id="addConstraint">Constraint +</button>
        <button id="removeConstraint">Constraint -</button>
    </div>

    <div id="constraintsContainer"></div>
    <br>
    <div>
        <label><input type="radio" name="lockOptTab" value="On"> Range Analyzer On</label>
        <label><input type="radio" name="lockOptTab" value="Off" checked> Range Analyzer Off</label>
    </div>
    <br>
    <input type="number" id="valueRangeInput" step="any" placeholder="0" style="display: none;">
    <p id="valueRangeText"></p>
    <br>
    <div>
        <label><input type="radio" name="solveDelta" value="On"> solve Delta On</label>
        <label><input type="radio" name="solveDelta" value="Off" checked> Solve Delta Off</label>
    </div>
    <br>
    <div class="row">
        <button id="solveButton">Solve</button>
        <button id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
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
            let objFunc = [3.0, 2.0];
            let constraints = [
                [2.0, 1.0, 100.0, 0.0],
                [1.0, 1.0, 80.0, 0.0],
                [1.0, 0.0, 45.0, 0.0]
            ];

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
