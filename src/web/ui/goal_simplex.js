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
        }
        h1, h3, h4 {
            color: #CC3300; /* Deep orange headings */
            margin-bottom: 10px;
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
        }
        table {
            border-collapse: collapse;
            margin-bottom: 15px;
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
            background-color: #CC3300;
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
        .btn {
            margin: 5px;
        }
        </style>
        <h1 style="margin-top: 60px;" class="row">Goal Simplex Preemptive / Penalties</h1>
        <div class="row">
            <button class="btn" id="addDecisionVarBtn">Decision Variable +</button>
            <button class="btn" id="removeDecisionVarBtn">Decision Variable -</button>
        </div>
        <h3 class="row">Goal Constraints</h3>
        <div class="row">
            <button style="margin-bottom: 20px;" class="btn" id="addGoalConstraintBtn">Goal Constraint +</button>
            <button style="margin-bottom: 20px;" class="btn" id="removeGoalConstraintBtn">Goal Constraint -</button>
        </div>
        <br>
        <div id="goalConstraintsContainer"></div>
        <h3 class="row">Normal Constraints</h3>
        <div class="row">
            <button class="btn" id="addConstraintBtn">Constraint +</button>
            <button class="btn" id="removeConstraintBtn">Constraint -</button>
        </div>
        <div id="constraintsContainer"></div>

        <!-- ONLY ADDED PART START -->
        <div class="row" style="margin: 30px 0; justify-content: center;">
            <label style="color: #CC3300; font-size: 1.2rem; font-weight: bold; cursor: pointer;">
                <input type="checkbox" id="penaltiesToggle" style="transform: scale(1.4); margin-right: 12px;">
                Enable Penalties
            </label>
        </div>

        <div id="penaltiesWrapper" style="display: none;">
            <h3 class="row">Penalties</h3>
            <div class="row" style="margin-bottom: 20px;" id="penaltiesContainer"></div>
        </div>
        <!-- ONLY ADDED PART END -->

        <div class="row">
            <button style="margin-bottom: 20px;" class="btn" id="toggleGoalOrderBtn">Show Goal Order</button>
        </div>
        <div class="row">
            <div class="row" id="goalOrderContainer" style="display: none;"></div>
        </div>
        <div class="row">
            <button id="solveButton">Solve</button>
            <button style="background-color: red;" id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
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
    let signItemsC = ["<=", ">="];
    let goals = ["Goal 1"];
    let goalOrder = [0];
    let penalties = [0.0];
    let constraints = [];
    let signItemsChoicesC = [];
    let toggle = false;

    // YOUR REQUESTED BOOL — OFF BY DEFAULT
    let penaltiesEnabled = false;

    const goalConstraintsContainer = document.getElementById('goalConstraintsContainer');
    const constraintsContainer = document.getElementById('constraintsContainer');
    const penaltiesContainer = document.getElementById('penaltiesContainer');
    const penaltiesWrapper = document.getElementById('penaltiesWrapper');
    const penaltiesToggle = document.getElementById('penaltiesToggle');
    const goalOrderContainer = document.getElementById('goalOrderContainer');
    const toggleGoalOrderBtn = document.getElementById('toggleGoalOrderBtn');

    // TOGGLE LOGIC — ONLY THING ADDED
    penaltiesToggle.onchange = () => {
        penaltiesEnabled = penaltiesToggle.checked;
        penaltiesWrapper.style.display = penaltiesEnabled ? 'block' : 'none';
    };

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
            signItemsC.forEach((item, index) => {
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
        updateGoalOrder();
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
        penaltiesEnabled = false;
        penaltiesToggle.checked = false;
        penaltiesWrapper.style.display = 'none';
        resultsContainer.innerHTML = "";
        updateGoalConstraints();
        updatePenalties();
        updateConstraints();
        updateGoalOrder();
    };

    function fmt(num, decimals = 6) {
        if (num === undefined || num === null || isNaN(num)) return "";
        return parseFloat(num.toFixed(decimals));
    }

    document.getElementById("solveButton").onclick = () => {

        let rowHeaders = [];
        for (let i = 0; i < goalConstraints.length; i++) {
            if (goalConstraints[i][goalConstraints[i].length - 1] != 2) {
                rowHeaders.push(`z`);
            }
            else {
                rowHeaders.push(`z'`);
                rowHeaders.push(`z''`);
            }
        }

        for (let i = 0; i < goalConstraints.length + constraints.length; i++) {
            rowHeaders.push(`c ${i + 1}`);
        }

        for (let i = 0; i < rowHeaders.length; i++) {
            if (rowHeaders[i] === "z''") {
                rowHeaders[i] = `z'' ${i + 1}`;
            }
            else if (rowHeaders[i] === "z'") {
                rowHeaders[i] = `z' ${i + 1}`;
            }
            else if (rowHeaders[i] === "z") {
                rowHeaders[i] = `z ${i + 1}`;
            }
        }

        if (penaltiesEnabled) {
            try {
                // Example input
                // const goals = [
                //     [40, 30, 20, 100, 0],
                //     [2, 4, 3, 10, 2],
                //     [5, 8, 4, 30, 1]
                // ];

                // // const constraints = [[1, 2, 3, 4, 2], [5, 6, 7, 8, 2]];
                // const constraints = [];
                // const penalties = [5, 8, 12, 15];
                // const goalOrder = [2, 1, 0];
                // const goalConstraints = goals;



                // Call the C++ solver
                const result = Module.runGoalPenaltiesSimplex(goalConstraints, constraints, penalties, goalOrder);

                result.pivotCols.unshift(-1);
                result.pivotRows.unshift(-1);

                result.headerRow.unshift(`t -`)

                // Render the tableau iterations
                let html = `<h3>Goal Simplex Result</h3>`;

                result.tableaus.forEach((tbl, iter) => {
                    if (iter === 0) {
                        html += `<h4>Setup Tableau</h4>`;
                    } else if (iter === result.opTable) {
                        // html += `<h4 style="color: blue;">Optimal Tableau ${iter}</h4>`;
                        html += `<h4 style="color: YELLOW;">Optimal Tableau ${iter}</h4>`;
                    } else {
                        html += `<h4>Tableau ${iter}</h4>`;
                    }

                    if (iter !== 0) {
                        html += `<h4>Penalties: ${fmt(result.penaltiesTotals[iter])}</h4>`;
                    }

                    if (result.goalMetStrings[iter] != undefined) {
                        // Render goal status
                        result.goalMetStrings[iter].forEach((metString, idx) => {
                            if (metString !== " ") {
                                html += `<h5>Goal ${idx + 1}: ${metString}</h5>`;
                                html += `<br>`;
                            }
                        });
                    }

                    // Render tableau table
                    html += `<table border="1" cellpadding="4">`;

                    result.headerRow[0] = (`t - ${iter + 1}`)
                    result.headerRow.forEach(h => html += `<th>${h}</th>`);

                    tbl.forEach((row, rIdx) => {
                        html += `<tr><td>${rowHeaders[rIdx]}</td>`;

                        row.forEach((v, cIdx) => {
                            let style = "";
                            // Highlight pivot cells
                            if ((result.pivotRows[iter] === rIdx || result.pivotCols[iter] === cIdx) && iter !== 0) {
                                // style = 'style="background-color: lightgreen;"';
                                style = 'style="background-color: #ff8800ff;"';
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

        }
        else {
            try {
                // Example input
                // const goals = [
                //     [40, 30, 20, 100, 0],
                //     [2, 4, 3, 10, 2],
                //     [5, 8, 4, 30, 1]
                // ];

                // const constraints = [[7, 2, 3, 69, 0],];
                // const goalOrder = [2, 1, 0];
                // const goalConstraints = goals;

                // Call the C++ solver
                const result = Module.runGoalPreemptiveSimplex(goalConstraints, constraints, goalOrder);

                result.pivotCols.unshift(-1);
                result.pivotRows.unshift(-1);

                result.headerRow.unshift(`t -`)

                // Render the tableau iterations
                let html = `<h3>Goal Penalties Simplex Result</h3>`;

                result.tableaus.forEach((tbl, iter) => {
                    if (iter === 0) {
                        html += `<h4>Setup Tableau</h4>`;
                    } else if (iter === result.opTable) {
                        // html += `<h4 style="color: blue;">Optimal Tableau ${iter}</h4>`;
                        html += `<h4 style="color: yellow;">Optimal Tableau ${iter}</h4>`;
                    } else {
                        html += `<h4>Tableau ${iter}</h4>`;
                    }

                    if (result.goalMetStrings[iter] != undefined) {
                        // Render goal status
                        result.goalMetStrings[iter].forEach((metString, idx) => {
                            if (metString !== " ") {
                                html += `<h5>Goal ${idx + 1}: ${metString}</h5>`;
                                html += `<br>`;
                            }
                        });
                    }

                    // Render tableau table
                    html += `<table border="1" cellpadding="4">`;

                    result.headerRow[0] = (`t - ${iter + 1}`)
                    result.headerRow.forEach(h => html += `<th>${h}</th>`);

                    tbl.forEach((row, rIdx) => {
                        html += `<tr><td>${rowHeaders[rIdx]}</td>`;


                        row.forEach((v, cIdx) => {
                            let style = "";
                            // Highlight pivot cells
                            if ((result.pivotRows[iter] === rIdx || result.pivotCols[iter] === cIdx) && iter !== 0) {
                                // style = 'style="background-color: lightgreen;"';
                                style = 'style="background-color: #ff8800ff;"';
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
        }
    };

    // ===== INITIAL RENDER =====
    updateGoalConstraints();
    updatePenalties();
    updateConstraints();
}