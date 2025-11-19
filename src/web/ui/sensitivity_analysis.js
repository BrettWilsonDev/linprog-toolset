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

    <h1 style="margin-top: 60px;" class="row">Sensitivity Analysis</h1>

    <div class="row">
        <label><input type="radio" name="problemType" value="Max" checked> Max</label>
        <label><input type="radio" name="problemType" value="Min"> Min</label>
    </div>

    <div class="row">
        <button id="addDecisionVar">decision variables +</button>
        <button id="removeDecisionVar">decision variables -</button>
    </div>

    <div class="row">
        <p>Use + to mark the item for analysis. O is the current selection.</p>
    </div>

    <div id="objectiveFunction" class="row"></div>

    <div id="rowDiv" class="row">
        <button id="addConstraint">Constraint +</button>
        <button id="removeConstraint">Constraint -</button>
    </div>

    <div id="constraintsContainer"></div>

    <div class="row">
        <button id="solveButton">Solve</button>
        <button style="background-color: red;" id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
    </div>
  `;

    let problemType = "Max";
    let amtOfObjVars = 2;
    let amtOfConstraints = 1;
    let objFunc = [0.0, 0.0];
    let constraints = [[0.0, 0.0, 0.0, 0.0]];
    let signItems = ["<=", ">="];
    let signItemsChoices = [0];
    let currentDeltaSelection = "o0";

    function updateProblemType() {
        problemType = document.querySelector('input[name="problemType"]:checked').value;
    }

    function updateObjectiveFunction() {
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
    }

    document.getElementById("resetButton").onclick = () => {
        problemType = "Max";
        amtOfObjVars = 2;
        amtOfConstraints = 1;
        objFunc = [0.0, 0.0];
        constraints = [[0.0, 0.0, 0.0]];
        signItems = ["<=", ">="];
        signItemsChoices = [0];
        currentDeltaSelection = "o0";

        resultsContainer.innerHTML = "";

        updateObjectiveFunction();
        updateConstraints();
        resetRadios();
    };

    function formatOutput(str) {
        // format numbers
        const formatted = str.replace(/-?\d+\.\d+/g, num =>
            parseFloat(num).toString()
        );
        // convert newlines to <br>
        return formatted.replace(/\n/g, "<br>");
    }

    document.getElementById("solveButton").onclick = () => {
        try {
            // let objFunc = [3.0, 2.0];
            // let constraints = [
            //     [2.0, 1.0, 100.0, 0.0],
            //     [1.0, 1.0, 80.0, 0.0],
            //     [1.0, 0.0, 45.0, 0.0]
            // ];

            let isMin = (problemType === "Min");

            const result = Module.runSensitivityAnalysis(
                objFunc,
                constraints,
                isMin,
                currentDeltaSelection,
            );

            // resultsContainer.innerHTML = result.outputString.replace(/\n/g, "<br>");

            const replacedString = result.outputString.replace(/\bd\b/g, "∆");
            resultsContainer.innerHTML = formatOutput(replacedString);

        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Unsolvable</p>`;
        }
    };

    updateObjectiveFunction();
    updateConstraints();
    resetRadios();
}
