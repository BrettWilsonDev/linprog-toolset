export function render(formContainer, resultsContainer, Module) {
    // Insert HTML
    formContainer.innerHTML = `
    <h1>Goal Simplex Penalties</h1>

    <button class="btn" id="addDecisionVarBtn">Decision Variable +</button>
    <button class="btn" id="removeDecisionVarBtn">Decision Variable -</button>

    <h3>Goal Constraints</h3>
    <button style="margin-bottom: 20px;" class="btn" id="addGoalConstraintBtn">Goal Constraint +</button>
    <button style="margin-bottom: 20px;" class="btn" id="removeGoalConstraintBtn">Goal Constraint -</button>

    <br>
    <div id="goalConstraintsContainer"></div>

    <h3>Normal Constraints</h3>
    <button class="btn" id="addConstraintBtn">Constraint +</button>
    <button class="btn" id="removeConstraintBtn">Constraint -</button>

    <div id="constraintsContainer"></div>

    <h3>Penalties</h3>
    <div style="margin-bottom: 20px;" id="penaltiesContainer"></div>

    <button style="margin-bottom: 20px;" class="btn" id="toggleGoalOrderBtn">Show Goal Order</button>

    <div id="goalOrderContainer" style="display: none;"></div>

    <div class="row">
        <button id="solveButton">Solve</button>
        <button id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
    </div>

    <div id="tableauContainer"></div>
  `;

    // ===== STATE =====
    let amtOfObjVars = 2;
    let amtOfGoalConstraints = 1;
    let amtOfConstraints = 0;
    let goalConstraints = [[0.0, 0.0, 0.0, 0.0]];
    let signItemsChoices = [0];
    let signItems = ["<=", ">=", "="];
    let goals = ["Goal 1"];
    let goalOrder = [0];
    let penalties = [0.0];
    let constraints = [];
    let signItemsChoicesC = [];
    let toggle = false;

    const goalConstraintsContainer = document.getElementById('goalConstraintsContainer');
    const constraintsContainer = document.getElementById('constraintsContainer');
    const penaltiesContainer = document.getElementById('penaltiesContainer');
    const goalOrderContainer = document.getElementById('goalOrderContainer');
    const toggleGoalOrderBtn = document.getElementById('toggleGoalOrderBtn');

    function updateGoalConstraints() {
        goalConstraintsContainer.innerHTML = '';
        for (let i = 0; i < amtOfGoalConstraints; i++) {
            const div = document.createElement('div');
            div.className = 'row';
            for (let j = 0; j < amtOfObjVars; j++) {
                const input = document.createElement('input');
                input.className = 'input-field';
                input.type = 'number';
                input.value = goalConstraints[i][j];
                input.oninput = (e) => goalConstraints[i][j] = parseFloat(e.target.value);
                div.appendChild(input);

                const label = document.createElement('label');
                label.textContent = `x${j + 1}`;
                div.appendChild(label);
            }

            const signSelect = document.createElement('select');
            signSelect.className = 'input-field';
            signItems.forEach((item, index) => {
                const option = document.createElement('option');
                option.value = index;
                option.textContent = item;
                signSelect.appendChild(option);
            });
            signSelect.value = signItemsChoices[i];
            signSelect.onchange = (e) => {
                signItemsChoices[i] = parseInt(e.target.value);
                goalConstraints[i][amtOfObjVars + 1] = signItemsChoices[i];
                if (goalConstraints[i][goalConstraints[i].length - 1] === 2) {
                    penalties.push(0.0);
                    updatePenalties();
                }

                let equalCount = 0;
                let notEqualCount = 0;
                for (let eqCtr = 0; eqCtr < goalConstraints.length; eqCtr++) {
                    if ((goalConstraints[eqCtr][amtOfObjVars + 1] == 2)) {
                        equalCount += 2;
                    }
                    else {
                        notEqualCount += 1;
                    }
                }

                if ((equalCount + notEqualCount) != penalties.length) {
                    penalties.pop();
                    updatePenalties();
                }

            };
            div.appendChild(signSelect);

            const rhsInput = document.createElement('input');
            rhsInput.className = 'input-field';
            rhsInput.type = 'number';
            rhsInput.value = goalConstraints[i][amtOfObjVars];
            rhsInput.oninput = (e) => goalConstraints[i][amtOfObjVars] = parseFloat(e.target.value);
            div.appendChild(rhsInput);

            goalConstraintsContainer.appendChild(div);
        }
    }

    function updateConstraints() {
        constraintsContainer.innerHTML = '';
        for (let i = 0; i < amtOfConstraints; i++) {
            const div = document.createElement('div');
            div.className = 'row';
            for (let j = 0; j < amtOfObjVars; j++) {
                const input = document.createElement('input');
                input.className = 'input-field';
                input.type = 'number';
                input.value = constraints[i][j];
                input.oninput = (e) => constraints[i][j] = parseFloat(e.target.value);
                div.appendChild(input);

                const label = document.createElement('label');
                label.textContent = `x${j + 1}`;
                div.appendChild(label);
            }

            const signSelect = document.createElement('select');
            signSelect.className = 'input-field';
            signItems.forEach((item, index) => {
                const option = document.createElement('option');
                option.value = index;
                option.textContent = item;
                signSelect.appendChild(option);
            });
            signSelect.value = signItemsChoicesC[i];
            signSelect.onchange = (e) => {
                signItemsChoicesC[i] = parseInt(e.target.value);
                constraints[i][amtOfObjVars + 1] = signItemsChoicesC[i];
            };
            div.appendChild(signSelect);

            const rhsInput = document.createElement('input');
            rhsInput.className = 'input-field';
            rhsInput.type = 'number';
            rhsInput.value = constraints[i][amtOfObjVars];
            rhsInput.oninput = (e) => constraints[i][amtOfObjVars] = parseFloat(e.target.value);
            div.appendChild(rhsInput);

            constraintsContainer.appendChild(div);
        }
    }

    function updatePenalties() {
        penaltiesContainer.innerHTML = '';
        penalties.forEach((penalty, i) => {
            const div = document.createElement('div');
            div.className = 'penaltyItem';

            const label = document.createElement('label');
            label.textContent = `Penalty ${i + 1}`;
            div.appendChild(label);

            const input = document.createElement('input');
            input.className = 'input-field';
            input.type = 'number';
            input.value = penalty;
            input.oninput = (e) => penalties[i] = parseFloat(e.target.value);
            div.appendChild(input);

            penaltiesContainer.appendChild(div);
        });
    }

    function updateGoalOrder() {
        goalOrderContainer.innerHTML = '';
        for (let i = 0; i < goals.length; i++) {
            const div = document.createElement('div');
            div.style = "margin-bottom: 10px;";
            div.textContent = goals[i];

            const upBtn = document.createElement('button');
            upBtn.style = 'margin-left: 10px;';
            upBtn.className = 'btn';
            upBtn.textContent = 'Up';
            upBtn.onclick = () => {
                if (i > 0) {
                    [goals[i], goals[i - 1]] = [goals[i - 1], goals[i]];
                    [goalOrder[i], goalOrder[i - 1]] = [goalOrder[i - 1], goalOrder[i]];
                    updateGoalOrder();
                }
            };
            div.appendChild(upBtn);

            const downBtn = document.createElement('button');
            downBtn.className = 'btn';
            downBtn.textContent = 'Down';
            downBtn.onclick = () => {
                if (i < goals.length - 1) {
                    [goals[i], goals[i + 1]] = [goals[i + 1], goals[i]];
                    [goalOrder[i], goalOrder[i + 1]] = [goalOrder[i + 1], goalOrder[i]];
                    updateGoalOrder();
                }
            };
            div.appendChild(downBtn);

            goalOrderContainer.appendChild(div);
        }
    }

    document.getElementById('addDecisionVarBtn').onclick = function () {
        amtOfObjVars++;
        goalConstraints.forEach(gc => gc.splice(gc.length - 2, 0, 0.0));
        constraints.forEach(c => c.splice(c.length - 2, 0, 0.0));
        updateGoalConstraints();
        updateConstraints();
    };

    document.getElementById('removeDecisionVarBtn').onclick = function () {
        if (amtOfObjVars > 2) {
            amtOfObjVars--;
            goalConstraints.forEach(gc => gc.splice(gc.length - 3, 1));
            constraints.forEach(c => c.splice(c.length - 3, 1));
            updateGoalConstraints();
            updateConstraints();
        }
    };

    document.getElementById('addGoalConstraintBtn').onclick = function () {
        amtOfGoalConstraints++;
        goalConstraints.push(Array(amtOfObjVars).fill(0.0).concat([0.0, 0.0]));
        signItemsChoices.push(0);
        goals.push(`Goal ${goals.length + 1}`);
        goalOrder.push(goals.length - 1);
        penalties.push(0.0);
        updateGoalConstraints();
        updatePenalties();
        updateGoalOrder();
    };

    document.getElementById('removeGoalConstraintBtn').onclick = function () {
        if (amtOfGoalConstraints > 1) {
            amtOfGoalConstraints--;
            goalConstraints.pop();
            signItemsChoices.pop();
            goals.pop();
            goalOrder.pop();
            penalties.pop();
            updateGoalConstraints();
            updatePenalties();
            updateGoalOrder();
        }
    };

    document.getElementById('addConstraintBtn').onclick = function () {
        amtOfConstraints++;
        constraints.push(Array(amtOfObjVars).fill(0.0).concat([0.0, 0.0]));
        signItemsChoicesC.push(0);
        updateConstraints();
    };

    document.getElementById('removeConstraintBtn').onclick = function () {
        if (amtOfConstraints > 0) {
            amtOfConstraints--;
            constraints.pop();
            signItemsChoicesC.pop();
            updateConstraints();
        }
    };

    toggleGoalOrderBtn.onclick = function () {
        toggle = !toggle;
        goalOrderContainer.style.display = toggle ? 'block' : 'none';
        toggleGoalOrderBtn.textContent = toggle ? 'Hide Goal Order' : 'Show Goal Order';
    };

    document.getElementById("resetButton").onclick = () => {
        amtOfObjVars = 2;
        amtOfGoalConstraints = 1;
        amtOfConstraints = 0;
        goalConstraints = [[0.0, 0.0, 0.0, 0.0]];
        signItemsChoices = [0];
        signItems = ["<=", ">=", "="];
        goals = ["Goal 1"];
        goalOrder = [0];
        penalties = [0.0];
        constraints = [];
        signItemsChoicesC = [];
        toggle = false;

        updateGoalConstraints();
        updatePenalties();
        updateConstraints();
    };

    function fmt(num, decimals = 6) {
        return parseFloat(num.toFixed(decimals));
    }

    document.getElementById("solveButton").onclick = () => {
        try {
            // Example input
            const goals = [
                [40, 30, 20, 100, 0],
                [2, 4, 3, 10, 2],
                [5, 8, 4, 30, 1]
            ];

            // const constraints = [[1, 2, 3, 4, 2], [5, 6, 7, 8, 2]];
            const constraints = [];
            const penalties = [5, 8, 12, 15];
            const goalOrder = [2, 1, 0];
            const goalConstraints = goals;

            // Call the C++ solver
            const result = Module.runGoalPenaltiesSimplex(goalConstraints, constraints, penalties, goalOrder);

            result.pivotCols.unshift(-1);
            result.pivotRows.unshift(-1);

            result.headerRow.unshift(`t -`)

            // Render the tableau iterations
            let html = `<h3>Goal Penalties Simplex Result</h3>`;

            result.tableaus.forEach((tbl, iter) => {
                if (iter === 0) {
                    html += `<h4>Setup Tableau</h4>`;
                } else if (iter === result.opTable) {
                    html += `<h4 style="color: blue;">Optimal Tableau ${iter}</h4>`;
                } else {
                    html += `<h4>Tableau ${iter}</h4>`;
                }

                if (iter !== 0) {
                    html += `<h4>Penalties: ${fmt(result.penaltiesTotals[iter])}</h4>`;
                }

                // Render goal status
                result.goalMetStrings[iter].forEach((metString, idx) => {
                    if (metString !== " ") {
                        html += `<h5>Goal ${idx + 1}: ${metString}</h5>`;
                    }
                });

                // Render tableau table
                html += `<table border="1" cellpadding="4">`;

                result.headerRow[0] = (`t - ${iter + 1}`)
                result.headerRow.forEach(h => html += `<th>${h}</th>`);

                tbl.forEach((row, rIdx) => {
                    if (rIdx <= goalConstraints.length) {
                        html += `<tr><td>z ${rIdx + 1}</td>`;
                    }
                    else
                    {
                        html += `<tr><td>c ${(rIdx - goalConstraints.length)}</td>`;
                    }

                    row.forEach((v, cIdx) => {
                        let style = "";
                        // Highlight pivot cells
                        if ((result.pivotRows[iter] === rIdx || result.pivotCols[iter] === cIdx) && iter !== 0) {
                            style = 'style="background-color: lightgreen;"';
                        }

                        html += `<td ${style}>${fmt(v)}</td>`;
                    });
                    html += `</tr>`;
                });
                html += `</table>`;
            });

            resultsContainer.innerHTML = html;
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };



    // ===== INITIAL RENDER =====
    updateGoalConstraints();
    updatePenalties();
    updateConstraints();
}
