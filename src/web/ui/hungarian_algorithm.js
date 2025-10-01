export function render(formContainer, resultsContainer, Module) {
    // Insert HTML
    formContainer.innerHTML = `
    <h1>Hungarian Algorithm</h1>

    <div class="row">
      <span>Matrix Size: <span id="matrixSizeDisplay">3 by 3</span></span>
      <button id="addRow">Rows +</button>
      <button id="removeRow">Rows -</button>
      <button id="addCol" style="margin-left: 20px;">Cols +</button>
      <button id="removeCol">Cols -</button>
      <span style="margin-left: 20px;">Blank Value: 
        <input type="number" id="blankValueInput" value="-999" style="width: 80px;">
      </span>
    </div>

    <div class="row">
      <h3>Problem Type:</h3>
      <label style="margin-left: 20px;"><input type="radio" name="problemType" value="Max" checked> Maximize</label>
      <label><input type="radio" name="problemType" value="Min"> Minimize</label>
    </div>

    <div class="row">
      <h3>Cost/Profit Matrix (Agents → Tasks)</h3>
      <div id="matrixContainer"></div>
    </div>

    <div class="row">
      <button id="solveButton">Solve</button>
      <button id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
    </div>
  `;

    // ===== STATE =====
    let BLANK_VALUE = -999;
    let problemType = "Max"; // Default to minimize
    let numRows = 3;
    let numCols = 3;
    let costMatrix = [
        [0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0]
    ];
    let blankMatrix = [
        [false, false, false],
        [false, false, false],
        [false, false, false]
    ];

    // Update BLANK_VALUE when input changes
    document.getElementById("blankValueInput").oninput = (e) => {
        const newValue = parseFloat(e.target.value);
        if (!isNaN(newValue)) {
            BLANK_VALUE = newValue;
            // Update existing blank cells in costMatrix
            for (let i = 0; i < numRows; i++) {
                for (let j = 0; j < numCols; j++) {
                    if (blankMatrix[i][j]) {
                        costMatrix[i][j] = BLANK_VALUE;
                    }
                }
            }
            updateMatrix();
        }
    };

    function updateProblemType() {
        const selected = document.querySelector('input[name="problemType"]:checked');
        if (selected) {
            problemType = selected.value;
        }
    }

    function updateMatrix() {
        const container = document.getElementById("matrixContainer");
        container.innerHTML = "";

        // Create table for matrix
        const table = document.createElement("table");
        table.style.borderCollapse = "collapse";

        // Header row
        const headerRow = document.createElement("tr");
        headerRow.appendChild(document.createElement("th")); // Empty top-left cell
        for (let j = 0; j < numCols; j++) {
            const th = document.createElement("th");
            th.innerText = `Task ${j + 1}`;
            th.style.border = "1px solid #ccc";
            th.style.padding = "5px";
            headerRow.appendChild(th);
        }
        table.appendChild(headerRow);

        // Data rows
        for (let i = 0; i < numRows; i++) {
            const row = document.createElement("tr");
            const labelCell = document.createElement("td");
            labelCell.innerText = `Agent ${i + 1}`;
            labelCell.style.border = "1px solid #ccc";
            labelCell.style.padding = "5px";
            row.appendChild(labelCell);

            for (let j = 0; j < numCols; j++) {
                const cell = document.createElement("td");
                cell.style.border = "1px solid #ccc";
                cell.style.padding = "2px";
                const inputContainer = document.createElement("div");
                inputContainer.style.display = "flex";
                inputContainer.style.alignItems = "center";
                inputContainer.style.gap = "5px";

                const input = document.createElement("input");
                input.type = "number";
                input.step = "0.01";
                input.value = blankMatrix[i][j] ? BLANK_VALUE : costMatrix[i][j];
                input.style.width = "60px";
                input.disabled = blankMatrix[i][j];

                const checkbox = document.createElement("input");
                checkbox.type = "checkbox";
                checkbox.checked = blankMatrix[i][j];
                checkbox.title = "Mark as forbidden/blank";

                checkbox.onchange = (e) => {
                    blankMatrix[i][j] = e.target.checked;
                    input.disabled = e.target.checked;
                    if (e.target.checked) {
                        input.value = BLANK_VALUE;
                        costMatrix[i][j] = BLANK_VALUE;
                    } else {
                        input.value = 0.0;
                        costMatrix[i][j] = 0.0;
                    }
                };

                input.oninput = (e) => {
                    if (!blankMatrix[i][j]) {
                        costMatrix[i][j] = parseFloat(e.target.value) || 0.0;
                    }
                };

                inputContainer.appendChild(input);
                inputContainer.appendChild(checkbox);
                cell.appendChild(inputContainer);
                row.appendChild(cell);
            }
            table.appendChild(row);
        }
        container.appendChild(table);
    }

    // Set up problem type radios
    document.querySelectorAll('input[name="problemType"]').forEach(radio => {
        radio.onchange = updateProblemType;
    });

    document.getElementById("addRow").onclick = () => {
        numRows++;
        // Resize matrices
        const newMatrix = Array(numRows).fill().map(() => Array(numCols).fill(0.0));
        const newBlankMatrix = Array(numRows).fill().map(() => Array(numCols).fill(false));
        for (let i = 0; i < Math.min(costMatrix.length, numRows); i++) {
            for (let j = 0; j < numCols; j++) {
                newMatrix[i][j] = costMatrix[i][j];
                newBlankMatrix[i][j] = blankMatrix[i][j];
            }
        }
        costMatrix = newMatrix;
        blankMatrix = newBlankMatrix;
        document.getElementById("matrixSizeDisplay").innerText = `${numRows} × ${numCols}`;
        updateMatrix();
    };

    document.getElementById("removeRow").onclick = () => {
        if (numRows > 1) {
            numRows--;
            // Resize matrices
            const newMatrix = Array(numRows).fill().map(() => Array(numCols).fill(0.0));
            const newBlankMatrix = Array(numRows).fill().map(() => Array(numCols).fill(false));
            for (let i = 0; i < numRows; i++) {
                for (let j = 0; j < numCols; j++) {
                    newMatrix[i][j] = costMatrix[i][j];
                    newBlankMatrix[i][j] = blankMatrix[i][j];
                }
            }
            costMatrix = newMatrix;
            blankMatrix = newBlankMatrix;
            document.getElementById("matrixSizeDisplay").innerText = `${numRows} × ${numCols}`;
            updateMatrix();
        }
    };

    document.getElementById("addCol").onclick = () => {
        numCols++;
        // Resize matrices
        const newMatrix = Array(numRows).fill().map(() => Array(numCols).fill(0.0));
        const newBlankMatrix = Array(numRows).fill().map(() => Array(numCols).fill(false));
        for (let i = 0; i < numRows; i++) {
            for (let j = 0; j < Math.min(costMatrix[i].length, numCols); j++) {
                newMatrix[i][j] = costMatrix[i][j];
                newBlankMatrix[i][j] = blankMatrix[i][j];
            }
        }
        costMatrix = newMatrix;
        blankMatrix = newBlankMatrix;
        document.getElementById("matrixSizeDisplay").innerText = `${numRows} × ${numCols}`;
        updateMatrix();
    };

    document.getElementById("removeCol").onclick = () => {
        if (numCols > 1) {
            numCols--;
            // Resize matrices
            const newMatrix = Array(numRows).fill().map(() => Array(numCols).fill(0.0));
            const newBlankMatrix = Array(numRows).fill().map(() => Array(numCols).fill(false));
            for (let i = 0; i < numRows; i++) {
                for (let j = 0; j < numCols; j++) {
                    newMatrix[i][j] = costMatrix[i][j];
                    newBlankMatrix[i][j] = blankMatrix[i][j];
                }
            }
            costMatrix = newMatrix;
            blankMatrix = newBlankMatrix;
            document.getElementById("matrixSizeDisplay").innerText = `${numRows} × ${numCols}`;
            updateMatrix();
        }
    };

    document.getElementById("resetButton").onclick = () => {
        problemType = "Min";
        numRows = 3;
        numCols = 3;
        costMatrix = [
            [0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0]
        ];
        blankMatrix = [
            [false, false, false],
            [false, false, false],
            [false, false, false]
        ];
        BLANK_VALUE = -999;
        document.getElementById("blankValueInput").value = BLANK_VALUE;
        resultsContainer.innerHTML = "";
        document.getElementById("matrixSizeDisplay").innerText = `${numRows} × ${numCols}`;
        document.querySelector('input[name="problemType"][value="Min"]').checked = true;
        updateProblemType();
        updateMatrix();
    };

    function fmt(num, decimals = 2) {
        return parseFloat(num.toFixed(decimals));
    }

    document.getElementById("solveButton").onclick = () => {
        try {
            costMatrix = [
                [22, 18, 30, 18],
                [18, BLANK_VALUE, 27, 22],
                [26, 20, 28, 28],
                [16, 22, BLANK_VALUE, 14],
                [21, BLANK_VALUE, 25, 28]
            ];

            // Prepare data for Emscripten
            const maximize = problemType === "Max";
            const hasBlanks = blankMatrix.some(row => row.some(cell => cell));

            const result = Module.runHungarianAlgorithm(
                costMatrix,  // Pass the cost matrix with BLANK_VALUE for blanks
                maximize,    // Pass whether to maximize (true for Max, false for Min)
                BLANK_VALUE, // Pass the blank value
                hasBlanks    // Indicate if blank values are present in the matrix
            );

            resultsContainer.innerHTML = "";
            const preElement = document.createElement("pre");
            preElement.textContent = result.solution;
            resultsContainer.appendChild(preElement);
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err.message || err}</p>`;
        }
    };

    // ===== INITIAL RENDER =====
    updateProblemType();
    updateMatrix();
}