export function render(formContainer, resultsContainer, Module) {
    // Insert HTML
    formContainer.innerHTML = `
    <h1>Sensitivity Analysis</h1>

    <div>
        <label><input type="radio" name="problemType" value="Max" checked> Max</label>
        <label><input type="radio" name="problemType" value="Min"> Min</label>
    </div>

    <p id="problemTypeText">Problem is: Max</p>

    <div class="row">
        <button id="addDecisionVar">decision variables +</button>
        <button id="removeDecisionVar">decision variables -</button>
    </div>

    <p>Use + to mark the item for analysis. O is the current selection.</p>

    <div id="objectiveFunction" class="row"></div>

    <div id="rowDiv" class="row">
        <button id="addConstraint">Constraint +</button>
        <button id="removeConstraint">Constraint -</button>
    </div>

    <div id="constraintsContainer"></div>

    <div class="row">
        <button id="solveButton">Solve</button>
        <button id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
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
        document.getElementById("problemTypeText").innerText = "Problem is: " + problemType;
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
            let objFunc = [3.0, 2.0];
            let constraints = [
                [2.0, 1.0, 100.0, 0.0],
                [1.0, 1.0, 80.0, 0.0],
                [1.0, 0.0, 45.0, 0.0]
            ];

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
