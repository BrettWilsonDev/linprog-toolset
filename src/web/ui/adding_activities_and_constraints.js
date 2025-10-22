export function render(formContainer, resultsContainer, Module) {
    // Insert HTML with inline CSS for button layout, form, and radio button styling
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
            text-align: center; /* Center paragraphs */
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
            margin-bottom: 15px;
            color: #FFFFFF;
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
            background-color: #ff8800ff; /* Green for pivot highlighting */
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

    <div class="wrapper">
        <h1 style="margin-top: 60px;">Adding Activities and Constraints</h1>

        <div class="row">
            <label><input type="radio" name="problemType" value="Max" checked> Max</label>
            <label><input type="radio" name="problemType" value="Min"> Min</label>
        </div>
        <p id="problemTypeText">Problem is: Max</p>
        <div class="row">
            <label><input type="radio" name="absProblemType" value="On"> Abs On</label>
            <label><input type="radio" name="absProblemType" value="Off" checked> Abs Off</label>
        </div>
        <p id="absProblemTypeText">absolute rule is: Off</p>

        <div class="row">
            <button id="addDecisionVar">Decision Variables +</button>
            <button id="removeDecisionVar">Decision Variables -</button>
        </div>

        <div id="objectiveFunction" class="row"></div>

        <div class="row">
            <button id="addConstraint">Constraint +</button>
            <button id="removeConstraint">Constraint -</button>
        </div>

        <div id="constraintsContainer"></div>

        <div class="row">
            <label><input type="radio" name="choiceProblemType" value="activities" checked> Adding Activities</label>
            <label><input type="radio" name="choiceProblemType" value="constraints"> Adding Constraints</label>
        </div>
        <p id="choiceProblemTypeText">The current problem is adding activities</p>

        <div id="problemTypeContainer"></div>

        <div id="extra"></div>

        <div class="row">
            <button id="solveButton">Solve</button>
            <button style="background-color: red;" id="resetButton">Reset</button>
        </div>
    </div>
    `;

    // ===== STATE =====
    let problemType = "Max";
    let absProblemType = "Off";
    let choiceProblemType = "activities";
    let amtOfObjVars = 2;
    let amtOfConstraints = 1;
    let objFunc = [0.0, 0.0];
    let constraints = [[0.0, 0.0, 0.0, 0.0]];
    let signItems = ["<=", ">="];
    let signItemsChoices = [0];
    let activity = [0.0, 0.0];
    let amtOfAddingConstraints = 1;
    let addingConstraints = [];
    let addingSignItemsChoices = [0];
    let rowsReversed = "Off";
    let negRule = "Off";

    function updateProblemType() {
        problemType = document.querySelector('input[name="problemType"]:checked').value;
        document.getElementById("problemTypeText").innerText = "Problem is: " + problemType;
    }
    function updateAbsProblemType() {
        absProblemType = document.querySelector('input[name="absProblemType"]:checked').value;
        document.getElementById("absProblemTypeText").innerText = "absolute rule is: " + absProblemType;
    }
    function updateChoiceProblemType() {
        choiceProblemType = document.querySelector('input[name="choiceProblemType"]:checked').value;
        document.getElementById("choiceProblemTypeText").innerText = "The current problem is adding " + choiceProblemType;
        updateChoices();
    }
    function updateReverseRows() {
        rowsReversed = document.querySelector('input[name="rowsReversed"]:checked').value;
        document.getElementById("rowsReversedText").innerText = "reverse rows: " + rowsReversed;
    }
    function updateKeepSlack() {
        negRule = document.querySelector('input[name="keepSlack"]:checked').value;
        document.getElementById("keepSlackText").innerText = "keep slack basic: " + negRule;
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
    document.querySelectorAll('input[name="absProblemType"]').forEach(radio => {
        radio.onchange = updateAbsProblemType;
    });
    document.querySelectorAll('input[name="choiceProblemType"]').forEach(radio => {
        radio.onchange = updateChoiceProblemType;
    });

    document.getElementById("addDecisionVar").onclick = () => {
        amtOfObjVars++;
        objFunc.push(0.0);
        constraints.forEach(constraint => constraint.splice(amtOfObjVars - 1, 0, 0.0));
        updateObjectiveFunction();
        updateConstraints();
        updateNewConstraints();
    };

    document.getElementById("removeDecisionVar").onclick = () => {
        if (amtOfObjVars > 2) {
            amtOfObjVars--;
            objFunc.pop();
            constraints.forEach(constraint => constraint.splice(amtOfObjVars, 1));
            updateObjectiveFunction();
            updateConstraints();
            updateNewConstraints();
        }
    };

    document.getElementById("addConstraint").onclick = () => {
        amtOfConstraints++;
        const newConstraint = new Array(amtOfObjVars).fill(0.0);
        newConstraint.push(0.0); // for sign
        newConstraint.push(0.0); // for rhs
        constraints.push(newConstraint);
        signItemsChoices.push(0);
        updateConstraints();
        activity.push(0.0);
        updateChoices();
    };

    document.getElementById("removeConstraint").onclick = () => {
        if (amtOfConstraints > 1) {
            amtOfConstraints--;
            constraints.pop();
            signItemsChoices.pop();
            updateConstraints();
            activity.pop();
            updateChoices();
        }
    };

    function resetRadios() {
        document.querySelector('input[value="Max"]').checked = true;
        document.querySelectorAll('input[value="Off"]').forEach(radioButton => {
            radioButton.checked = true;
        });
        document.querySelector('input[value="activities"]').checked = true;
    }

    document.getElementById("resetButton").onclick = () => {
        amtOfObjVars = 2;
        amtOfConstraints = 1;
        objFunc = [0.0, 0.0];
        constraints = [[0.0, 0.0, 0.0, 0.0]];
        signItems = ["<=", ">="];
        signItemsChoices = [0];
        activity = [0.0, 0.0];
        amtOfAddingConstraints = 1;
        addingConstraints = [];
        addingSignItemsChoices = [0];
        problemType = "Max";
        absProblemType = "Off";
        choiceProblemType = "activities";
        negRule = "Off";
        rowsReversed = "Off";
        resultsContainer.innerHTML = "";
        updateChoices();
        updateObjectiveFunction();
        updateConstraints();
        resetRadios();
    };

    function updateActivityInputs() {
        const container = document.getElementById('problemTypeContainer');
        container.innerHTML = "";
        for (let i = 0; i < activity.length; i++) {
            const row = document.createElement('div');
            row.className = 'constraint';
            const input = document.createElement('input');
            input.type = 'number';
            input.value = activity[i];
            input.addEventListener('input', (event) => {
                activity[i] = parseFloat(event.target.value);
            });
            const label = document.createElement('span');
            label.textContent = i === 0 ? 'x' : `c${i}`;
            row.appendChild(input);
            row.appendChild(label);
            container.appendChild(row);
        }
    }

    function updateNewConstraints() {
        const container = document.getElementById("newConstraintsContainer");
        if (!container) {
            return;
        }
        container.innerHTML = "";
        for (let i = 0; i < amtOfAddingConstraints; i++) {
            if (addingConstraints.length <= i) {
                addingConstraints.push(new Array(amtOfObjVars + 2).fill(0.0));
            }
            const constraintRow = document.createElement("div");
            constraintRow.className = "constraint";
            for (let j = 0; j < amtOfObjVars; j++) {
                const input = document.createElement("input");
                input.type = "number";
                input.value = addingConstraints[i][j];
                input.oninput = (e) => {
                    addingConstraints[i][j] = parseFloat(e.target.value);
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
            signSelect.value = addingSignItemsChoices[i];
            signSelect.onchange = (e) => {
                signItemsChoices[i] = parseInt(e.target.value);
                addingConstraints[i][amtOfObjVars + 1] = addingSignItemsChoices[i];
            };
            const rhsInput = document.createElement("input");
            rhsInput.type = "number";
            rhsInput.value = addingConstraints[i][amtOfObjVars];
            rhsInput.oninput = (e) => {
                addingConstraints[i][amtOfObjVars] = parseFloat(e.target.value);
            };
            constraintRow.appendChild(signSelect);
            constraintRow.appendChild(rhsInput);
            container.appendChild(constraintRow);
        }
    }

    function updateAddingNewConstraints() {
        const container = document.getElementById('problemTypeContainer');
        container.innerHTML = "";
        const row = document.createElement("div");
        row.className = "row";
        const addButton = document.createElement("button");
        addButton.id = "addNewConstraint";
        addButton.textContent = "New Constraint +";
        const removeButton = document.createElement("button");
        removeButton.id = "removeNewConstraint";
        removeButton.textContent = "New Constraint -";
        row.appendChild(addButton);
        row.appendChild(removeButton);
        container.appendChild(row);
        const newConstraint = document.createElement("div");
        newConstraint.id = "newConstraintsContainer";
        container.appendChild(newConstraint);
        document.getElementById("addNewConstraint").onclick = () => {
            amtOfAddingConstraints++;
            const newConstraint = new Array(amtOfObjVars).fill(0.0);
            newConstraint.push(0.0); // for sign
            newConstraint.push(0.0); // for rhs
            addingConstraints.push(newConstraint);
            addingSignItemsChoices.push(0);
            updateNewConstraints();
        };
        document.getElementById("removeNewConstraint").onclick = () => {
            if (amtOfAddingConstraints > 1) {
                amtOfAddingConstraints--;
                addingConstraints.pop();
                addingSignItemsChoices.pop();
                updateNewConstraints();
            }
        };
        updateNewConstraints();
    }

    function updateChoices() {
        if (choiceProblemType == "activities") {
            document.querySelectorAll('.tableauContainer').forEach(elem => elem.remove());
            document.querySelectorAll('.tableauContainer2').forEach(elem => elem.remove());
            resultsContainer.innerHTML = "";
            updateActivityInputs();
        } else {
            document.querySelectorAll('.tableauContainer').forEach(elem => elem.remove());
            document.querySelectorAll('.tableauContainer2').forEach(elem => elem.remove());
            resultsContainer.innerHTML = "";
            updateAddingNewConstraints();
        }
    }

    document.getElementById("solveButton").onclick = () => {
        // let objFunc = [60, 30, 20];
        // let constraints = [
        //     [8, 6, 1, 48, 0],
        //     [4, 2, 1.5, 20, 0],
        //     [2, 1.5, 0.5, 8, 0]
        // ];
        // let newAct = [40, -1, -1, -1];
        // let newCons = [[0, 0, 1, 5, 0]];
        let isMin = (problemType === "Min");
        let result = "something";
        if (choiceProblemType == "activities") {
            result = Module.runAddActivity(objFunc, constraints, isMin, activity);
        } else {
            result = Module.runAddConstraints(objFunc, constraints, isMin, newCons);
        }
        const formatCell = (cell) => {
            if (typeof cell === "number") {
                if (Math.abs(cell) < 1e-10) return "0";
                return Number(cell.toFixed(4));
            }
            return String(cell).replace(/\bd\b/g, "Δ");
        };
        const renderRow = (arr, rowLabel = "") => {
            return `<tr>${rowLabel ? `<th>${rowLabel}</th>` : ""}${arr.map(c => `<td>${formatCell(c)}</td>`).join("")}</tr>`;
        };
        const renderTable2D = (mat) => {
            if (!Array.isArray(mat) || !mat.length) return "";
            const numCols = mat[0].length;
            const headers = [];
            for (let i = 0; i < numCols; i++) {
                if (i < objFunc.length) {
                    // headers.push(`<th>x${i + 1}</th>`);
                } else if (i === objFunc.length) {
                    headers.push(`<th>rhs</th>`);
                } else {
                    const index = i - objFunc.length - 1;
                    headers.push(`<th>s/e${index + 1}</th>`);
                }
            }
            const header = `<tr><th></th>${headers.join("")}</tr>`;
            return `<table border="1" cellspacing="0" cellpadding="4">
            ${header}
            ${mat.map((row, i) => {
                const rowLabel = i === 0 ? "Z" : `c${i}`;
                return renderRow(row, rowLabel);
            }).join("")}
            </table>`;
        };
        const renderTables3D = (tensor, pivotRowsArr = [], pivotColsArr = []) => {
            if (!Array.isArray(tensor)) return "";
            return tensor.map((mat, i) => {
                const pivotRow = pivotRowsArr[i] ?? -1;
                const pivotCol = pivotColsArr[i] ?? -1;
                const tableHTML = `<table border="1" cellspacing="0" cellpadding="4">
            <tr>
                <th></th>
                ${Object.values(mat[0]).map((_, colIndex) => {
                    if (choiceProblemType == "activities") {
                        if (colIndex < objFunc.length + 1) {
                            return `<th>x${colIndex + 1}</th>`;
                        } else {
                            const index = colIndex - objFunc.length;
                            return index === mat[0].length - objFunc.length - 1 ? `<th>rhs</th>` : `<th>s/e${index + 1}</th>`;
                        }
                    } else {
                        if (colIndex < objFunc.length) {
                            return `<th>x${colIndex + 1}</th>`;
                        } else {
                            const index = colIndex - objFunc.length;
                            return index === mat[0].length - objFunc.length - 1 ? `<th>rhs</th>` : `<th>s/e${index + 1}</th>`;
                        }
                    }
                }).join('')}
            </tr>
            ${mat.map((row, rowIndex) => {
                    const rowLabel = rowIndex === 0 ? "Z" : `c${rowIndex}`;
                    return `<tr>${rowLabel ? `<th>${rowLabel}</th>` : ""}${row.map((cell, colIndex) => {
                        let classes = "";
                        if (rowIndex === pivotRow || colIndex === pivotCol) classes = "pivot";
                        if (typeof cell === "number") {
                            if (Math.abs(cell) < 1e-10) cell = 0;
                            cell = Number(cell.toFixed(4));
                        } else {
                            cell = String(cell).replace(/\bd\b/g, "Δ");
                        }
                        return `<td class="${classes}">${cell}</td>`;
                    }).join("")}</tr>`;
                }).join("")}
                </table>`;
                return tensor.length === 1 ? `<h4></h4>${tableHTML}` : `<h4>${i == tensor.length - 1 ? "Optimal Table" : `Table ${i + 1}`}</h4>${tableHTML}`;
            }).join("<br>");
        };
        const style = document.createElement("style");
        // style.innerHTML = `.pivot { background-color: #4CAF50; }`;
        style.innerHTML = `.pivot { background-color: #ff8800ff; }`;
        document.head.appendChild(style);
        const fmt = (num) => {
            if (typeof num === "number") {
                if (Math.abs(num) < 1e-10) return "0";
                return Math.round(num * 10000) / 10000;
            }
            return String(num).replace(/\bd\b/g, "Δ");
        };
        const renderVector = (vec) => {
            if (!vec || !vec.length) return "";
            const is2D = Array.isArray(vec[0]);
            const tableRows = vec.map((row) => {
                if (is2D) {
                    const cells = row.map(cell => `<td>${fmt(cell)}</td>`).join("");
                    return `<tr>${cells}</tr>`;
                } else {
                    return `<tr><td>${fmt(row)}</td></tr>`;
                }
            });
            return `<table border="1" cellspacing="0" cellpadding="4">${tableRows.join("")}</table>`;
        };
        if (result.newOptTabs.length != 1) {
            if (choiceProblemType == "activities") {
                resultsContainer.innerHTML = `
                <h3>Math Preliminaries Results</h3>
                <h4>New Activity Column</h4>
                ${renderVector(result.actCol)}
                <h4>Changing Table</h4>
                ${renderTables3D(result.OptTabs)}
                <h4>New Optimal Tables</h4>
                ${renderTables3D(result.newOptTabs, result.pivotRows, result.pivotCols)}
            `;
            } else {
                resultsContainer.innerHTML = `
                <h3>Math Preliminaries Results</h3>
                <h4>Changing Table</h4>
                ${renderTables3D(result.OptTabs)}
                <h4>New Optimal Tables</h4>
                ${renderTables3D(result.newOptTabs, result.pivotRows, result.pivotCols)}
            `;
            }
        } else {
            if (choiceProblemType == "activities") {
                resultsContainer.innerHTML = `
                <h3>Math Preliminaries Results</h3>
                <h4>New Activity Column</h4>
                ${renderVector(result.actCol)}
                <h4>Changing Table</h4>
                ${renderTables3D(result.OptTabs)}
            `;
            } else {
                resultsContainer.innerHTML = `
                <h3>Math Preliminaries Results</h3>
                <h4>Changing Table</h4>
                ${renderTables3D(result.OptTabs)}
            `;
            }
        }
    };

    // Initial render
    updateChoices();
    updateObjectiveFunction();
    updateConstraints();
    resetRadios();
}