export function render(formContainer, resultsContainer, Module) {
    if (typeof Plotly === "undefined") {
        const script = document.createElement("script");
        script.src = "https://cdn.plot.ly/plotly-latest.min.js";
        script.onload = () => console.log("Plotly loaded!");
        document.head.appendChild(script);
    }

    // Insert HTML
    formContainer.innerHTML = `
    <h1>Graphical Solver</h1>

    <div>
        <div>
            <label><input type="radio" name="problemType" value="Max" checked> Max</label>
            <label><input type="radio" name="problemType" value="Min"> Min</label>
        </div>

        <p id="problemTypeText">Problem is: Max</p>

        <div id="objectiveFunction" class="row"></div>

        <div class="row">
            <button id="addConstraint">Constraint +</button>
            <button id="removeConstraint">Constraint -</button>
        </div>

        <div id="constraintsContainer"></div>


        <div class="row">
            <button id="solveButton">Solve</button>
            <button id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
        </div>

        <div id="plotContainer" style="width:600px; height:400px; margin-top:20px;"></div>
  `;

    // ===== STATE =====
    let problemType = "Max";
    let amtOfObjVars = 2;
    let amtOfConstraints = 1;
    let objFunc = [0.0, 0.0];
    let constraints = [[0.0, 0.0, 0.0, 0.0]];
    let signItems = ["<=", ">=", "="];
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
        signItems = ["<=", ">=", "="];
        signItemsChoices = [0];

        resultsContainer.innerHTML = "";
        plotContainer.innerHTML = "";

        updateObjectiveFunction();
        updateConstraints();
        resetRadios();
    };

    function fmt(num, decimals = 6) {
        return parseFloat(num.toFixed(decimals));
    }

    document.getElementById("solveButton").onclick = () => {
        try {
            objFunc = [100, 30];
            constraints = [
                [0, 1, 3, 1],
                [1, 1, 7, 0],
                [10, 4, 40, 0]
            ];

            const result = Module.runGraphicalSolver(objFunc, constraints, problemType);

            const fmt = (num, decimals = 6) => parseFloat(num.toFixed(decimals));
            let html = `<h3>Graphical Solver Result</h3>`;
            html += `<p>Optimal Value: ${fmt(result.optimalSolution)}</p>`;
            html += `<p>Optimal Variables: ${result.changingVars.map(v => fmt(v)).join(", ")}</p>`;
            resultsContainer.innerHTML = html;

            if (typeof Plotly !== "undefined") {
                // Feasible region polygon
                const hullX = result.feasibleHull.map(p => p.x);
                const hullY = result.feasibleHull.map(p => p.y);
                hullX.push(result.feasibleHull[0].x);
                hullY.push(result.feasibleHull[0].y);

                const feasibleRegion = {
                    x: hullX,
                    y: hullY,
                    mode: "lines",
                    fill: "toself",
                    name: "Feasible Region",
                    line: { color: "blue" },
                    opacity: 0.3
                };

                // Feasible points scatter with labels
                const feasiblePoints = {
                    x: result.feasiblePoints.map(p => p.x),
                    y: result.feasiblePoints.map(p => p.y),
                    mode: "markers+text",
                    name: "Feasible Points",
                    marker: { color: "green", size: 6 },
                    text: result.feasiblePoints.map(p => `(${p.x.toFixed(2)}, ${p.y.toFixed(2)})`),
                    textposition: "top center"
                };

                // Optimal solution with label offset to avoid overlap
                const optimalX = result.changingVars[0];
                const optimalY = result.changingVars[1];

                // Check if optimal point coincides with a feasible point
                let labelOffset = "top right";
                for (const p of result.feasiblePoints) {
                    if (Math.abs(p.x - optimalX) < 1e-6 && Math.abs(p.y - optimalY) < 1e-6) {
                        labelOffset = "bottom right"; // move label away
                        break;
                    }
                }

                const optimalPoint = {
                    x: [optimalX],
                    y: [optimalY],
                    mode: "markers+text",
                    name: "Optimal Solution",
                    marker: { color: "red", size: 12, symbol: "star" },
                    text: [`(${optimalX.toFixed(2)}, ${optimalY.toFixed(2)})`],
                    textposition: labelOffset
                };

                Plotly.newPlot("plotContainer", [feasibleRegion, feasiblePoints, optimalPoint], {
                    title: "Graphical Solution",
                    xaxis: { title: "x1", zeroline: true },
                    yaxis: { title: "x2", zeroline: true }
                });
            }

        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };


    // ===== INITIAL RENDER =====
    updateObjectiveFunction();
    updateConstraints();
    updateProblemType();
}
