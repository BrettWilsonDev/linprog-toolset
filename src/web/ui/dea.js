export function render(formContainer, resultsContainer, Module) {
    // Insert HTML
    formContainer.innerHTML = `
    <h1>Data Envelopment Analysis</h1>

    <div id="problem-type">
        <input type="radio" id="max" name="problemType" value="Max" checked>
        <label for="max">Max</label>
        <input type="radio" id="min" name="problemType" value="Min">
        <label for="min">Min</label>
    </div>
    <br>

    <div class="button-group">
        <button class="button" id="itemRowPlus">Item Row +</button>
        <button class="button" id="itemRowMinus">Item Row -</button>
    </div>

    <div class="button-group">
        <button class="button" id="inputsPlus">Inputs +</button>
        <button class="button" id="inputsMinus">Inputs -</button>
        <button class="button" id="outputsPlus">Outputs +</button>
        <button class="button" id="outputsMinus">Outputs -</button>
    </div>

    <div id="InputSection"></div>

    <br>
    <div class="row">
        <button id="solveButton">Solve</button>
        <button id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
    </div>
  `;

    // ===== STATE =====
    let problemType = "Max";
    let amtOfItems = 1;
    let amtOfInputs = 1;
    let amtOfOutputs = 1;
    let LpInputs = [[0.0]];
    let LpOutputs = [[0.0]];

    document.querySelectorAll('input[name="problemType"]').forEach((elem) => {
        elem.addEventListener("change", function (event) {
            problemType = event.target.value;
        });
    });

    document.getElementById("itemRowPlus").addEventListener("click", function () {
        amtOfItems += 1;
        LpInputs.push(Array(amtOfInputs).fill(0.0));
        LpOutputs.push(Array(amtOfOutputs).fill(0.0));
        updateTable();
    });

    document.getElementById("itemRowMinus").addEventListener("click", function () {
        if (amtOfItems > 1) {
            amtOfItems -= 1;
            LpInputs.pop();
            LpOutputs.pop();
            updateTable();
        }
    });

    document.getElementById("inputsPlus").addEventListener("click", function () {
        amtOfInputs += 1;
        LpInputs.forEach(row => row.push(0.0));
        updateTable();
    });

    document.getElementById("inputsMinus").addEventListener("click", function () {
        if (amtOfInputs > 1) {
            amtOfInputs -= 1;
            LpInputs.forEach(row => row.pop());
            updateTable();
        }
    });

    document.getElementById("outputsPlus").addEventListener("click", function () {
        amtOfOutputs += 1;
        LpOutputs.forEach(row => row.push(0.0));
        updateTable();
    });

    document.getElementById("outputsMinus").addEventListener("click", function () {
        if (amtOfOutputs > 1) {
            amtOfOutputs -= 1;
            LpOutputs.forEach(row => row.pop());
            updateTable();
        }
    });

    function updateTable() {
        const tableContainer = document.getElementById("InputSection");
        tableContainer.innerHTML = ""; // Clear previous table

        for (let i = 0; i < amtOfItems; i++) {
            const rowDiv = document.createElement("div");
            rowDiv.className = "row";

            for (let j = 0; j < amtOfInputs; j++) {
                const inputField = document.createElement("input");
                inputField.type = "number";
                inputField.className = "input-field";
                inputField.value = LpInputs[i][j];
                inputField.addEventListener("input", (event) => {
                    LpInputs[i][j] = parseFloat(event.target.value);
                });
                rowDiv.appendChild(inputField);
                const label = document.createElement("label");
                label.textContent = `i${j + 1}`;
                rowDiv.appendChild(label);
            }

            for (let j = 0; j < amtOfOutputs; j++) {
                const outputField = document.createElement("input");
                outputField.type = "number";
                outputField.className = "input-field";
                outputField.value = LpOutputs[i][j];
                outputField.addEventListener("input", (event) => {
                    LpOutputs[i][j] = parseFloat(event.target.value);
                });
                rowDiv.appendChild(outputField);
                const label = document.createElement("label");
                label.textContent = `o${j + 1}`;
                rowDiv.appendChild(label);
            }

            tableContainer.appendChild(rowDiv);
        }
    }

    function resetRadios() {
        document.querySelector('input[value="Max"]').checked = true;
    }

    document.getElementById("resetButton").onclick = () => {
        problemType = "Max";
        amtOfItems = 1;
        amtOfInputs = 1;
        amtOfOutputs = 1;
        LpInputs = [[0.0]];
        LpOutputs = [[0.0]];

        updateTable();
        resetRadios();
    };

    function fmt(num, decimals = 6) {
        return parseFloat(num.toFixed(decimals));
    }

    document.getElementById("solveButton").onclick = () => {
        try {
            LpInputs = [
                [5, 14],
                [8, 15],
                [7, 12]
            ];

            LpOutputs = [
                [9, 4, 16],
                [5, 7, 10],
                [4, 9, 13]
            ];

            let isMin = (problemType === "Min");

            const result = Module.runDEA(LpInputs, LpOutputs, isMin);

            resultsContainer.innerHTML = '<pre>' + result.outputString + '</pre>';

            // resultsContainer.innerHTML = html;
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };


    // ===== INITIAL RENDER =====
    updateTable();
    resetRadios();
}
