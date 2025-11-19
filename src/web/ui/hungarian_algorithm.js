export function render(formContainer, resultsContainer, Module) {
    // Insert HTML
    formContainer.innerHTML = `
    <style>
        /* BULLETPROOF FIX - NEVER GOES OFF SCREEN */
        #matrixContainer {
            width: 100% !important;
            max-width: 100vw !important;
            overflow: hidden !important;
            box-sizing: border-box !important;
            padding: 0 4px;
        }
        .matrix-scroll-wrapper {
            width: 100%;
            overflow-x: auto;
            overflow-y: hidden;
            -webkit-overflow-scrolling: touch;
            padding: 12px 8px;
            box-sizing: border-box;
            border: 1px solid #333333;
            border-radius: 6px;
            background: #1A1A1A;
        }
        .matrix-scroll-wrapper::-webkit-scrollbar {
            height: 10px;
        }
        .matrix-scroll-wrapper::-webkit-scrollbar-track {
            background: #111;
            border-radius: 5px;
        }
        .matrix-scroll-wrapper::-webkit-scrollbar-thumb {
            background: #CC3300;
            border-radius: 5px;
        }
        .matrix-scroll-wrapper table {
            min-width: 620px;
            margin: 0 auto;
        }

        /* Your original styles - 100% untouched */
        .wrapper {
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        .row {
            display: flex;
            gap: 10px;
            margin-bottom: 15px;
            align-items: center;
            justify-content: center;
        }
        button {
            background: #333333;
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
            background: #CC3300;
            color: #000000;
        }
        button#resetButton {
            background: #1A1A1A;
        }
        button#resetButton:hover {
            background: #992600;
        }
        #form input[type="radio"], #form input[type="number"], #form select {
            background: #1A1A1A;
            color: #FFFFFF;
            border: 1px solid #333333;
            padding: 6px;
            border-radius: 4px;
            margin: 5px 0;
        }
        #form input[type="radio"] {
            -webkit-appearance: none;
            -moz-appearance: none;
            appearance: none;
            width: 16px;
            height: 16px;
            border: 2px solid #333333;
            border-radius: 50%;
            position: relative;
            cursor: pointer;
            transition: all 0.2s ease;
            margin-right: 8px;
        }
        #form input[type="radio"]:checked {
            border-color: #CC3300;
            background: #CC3300;
        }
        #form input[type="radio"]:checked::after {
            content: '';
            width: 8px;
            height: 8px;
            background: #FFFFFF;
            border-radius: 50%;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
        }
        #form input[type="radio"]:hover { border-color: #CC3300; }
        #form input[type="radio"]:focus {
            outline: none;
            box-shadow: 0 0 0 2px #CC3300;
        }
        #form input[type="number"] {
            width: 50px;
            -moz-appearance: textfield !important;
        }
        #form input[type="number"]::-webkit-inner-spin-button,
        #form input[type="number"]::-webkit-outer-spin-button {
            -webkit-appearance: none !important;
            display: none !important;
        }
        #form input[type="number"]:focus, #form select:focus {
            border-color: #CC3300;
            outline: none;
        }
        label {
            color: #CCCCCC;
            margin-right: 15px;
            display: flex;
            align-items: center;
        }
        h1, h3, h4 {
            color: #CC3300;
            margin-bottom: 10px;
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
            color: #CC3300;
        }
        input[type="checkbox"] {
            accent-color: #CC3300;
            width: 16px;
            height: 16px;
            cursor: pointer;
            background: #1A1A1A;
            border: 1px solid #333333;
            border-radius: 3px;
            appearance: none;
            -webkit-appearance: none;
            -moz-appearance: none;
            position: relative;
        }
        input[type="checkbox"]:checked {
            background: #CC3300;
            border-color: #CC3300;
        }
        input[type="checkbox"]:checked::after {
            content: "✔";
            color: #000000;
            font-size: 12px;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
        }
        @media (max-width: 600px) {
            .row { flex-wrap: wrap; gap: 8px; }
            button { padding: 6px 12px; font-size: 0.85rem; }
            #form input[type="number"] { width: 40px; }
        }
    </style>

    <h1 style="margin-top: 60px;" class="row">Hungarian Algorithm</h1>
    <div>
      <span class="row">Matrix Size: <span id="matrixSizeDisplay">3 by 3</span></span>
      <div class="row">
      <button id="addRow">Rows +</button>
      <button id="removeRow">Rows -</button>
      <button id="addCol" style="margin-left: 20px;">Cols +</button>
      <button id="removeCol">Cols -</button>
      </div>
      <br>
      <span class="row" style="margin-left: 20px;">Blank Value:
        <input type="number" id="blankValueInput" value="-999" style="width: 80px;">
      </span>
    </div>
    <div class="row">
      <h3>Problem Type:</h3>
      <label style="margin-left: 20px;"><input type="radio" name="problemType" value="Max" checked> Maximize</label>
      <label><input type="radio" name="problemType" value="Min"> Minimize</label>
    </div>
    <h3 class="row">Cost/Profit Matrix (Agents to Tasks)</h3>
    <div class="row">
      <div id="matrixContainer"></div>
    </div>
    <div class="row">
      <button id="solveButton">Solve</button>
      <button style="background-color: red;" id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
    </div>
    `;

    // ===== STATE =====
    let BLANK_VALUE = -999;
    let problemType = "Max";
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

    function updateProblemType() {
        const selected = document.querySelector('input[name="problemType"]:checked');
        if (selected) problemType = selected.value;
    }

    function updateMatrix() {
        const container = document.getElementById("matrixContainer");
        container.innerHTML = "";

        const table = document.createElement("table");
        table.style.borderCollapse = "collapse";

        // Header row
        const headerRow = document.createElement("tr");
        headerRow.appendChild(document.createElement("th"));
        for (let j = 0; j < numCols; j++) {
            const th = document.createElement("th");
            th.innerText = `Task ${j + 1}`;
            th.style.padding = "5px";
            headerRow.appendChild(th);
        }
        table.appendChild(headerRow);

        // Data rows
        for (let i = 0; i < numRows; i++) {
            const row = document.createElement("tr");
            const labelCell = document.createElement("td");
            labelCell.innerText = `Agent ${i + 1}`;
            labelCell.style.padding = "5px";
            row.appendChild(labelCell);

            for (let j = 0; j < numCols; j++) {
                const cell = document.createElement("td");
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

                checkbox.onchange = () => {
                    blankMatrix[i][j] = checkbox.checked;
                    input.disabled = checkbox.checked;
                    if (checkbox.checked) {
                        input.value = BLANK_VALUE;
                        costMatrix[i][j] = BLANK_VALUE;
                    } else {
                        input.value = 0.0;
                        costMatrix[i][j] = 0.0;
                    }
                };

                input.oninput = () => {
                    if (!blankMatrix[i][j]) {
                        costMatrix[i][j] = parseFloat(input.value) || 0.0;
                    }
                };

                inputContainer.appendChild(input);
                inputContainer.appendChild(checkbox);
                cell.appendChild(inputContainer);
                row.appendChild(cell);
            }
            table.appendChild(row);
        }

        // THE ONLY CHANGE: wrap in scrollable wrapper
        const scrollWrapper = document.createElement("div");
        scrollWrapper.className = "matrix-scroll-wrapper";
        scrollWrapper.appendChild(table);
        container.appendChild(scrollWrapper);

        // Fix iOS Safari bug
        setTimeout(() => { scrollWrapper.scrollLeft = 0; }, 0);
    }

    // All your original handlers - unchanged
    document.getElementById("blankValueInput").oninput = (e) => {
        const newValue = parseFloat(e.target.value);
        if (!isNaN(newValue)) {
            BLANK_VALUE = newValue;
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

    document.querySelectorAll('input[name="problemType"]').forEach(radio => {
        radio.onchange = updateProblemType;
    });

    document.getElementById("addRow").onclick = () => {
        numRows++;
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
        numRows = 3;
        numCols = 3;
        costMatrix = [[0.0,0.0,0.0],[0.0,0.0,0.0],[0.0,0.0,0.0]];
        blankMatrix = [[false,false,false],[false,false,false],[false,false,false]];
        BLANK_VALUE = -999;
        document.getElementById("blankValueInput").value = BLANK_VALUE;
        resultsContainer.innerHTML = "";
        document.getElementById("matrixSizeDisplay").innerText = "3 × 3";
        document.querySelector('input[name="problemType"][value="Max"]').checked = true;
        problemType = "Max";
        updateMatrix();
    };

    document.getElementById("solveButton").onclick = () => {
        try {
            const maximize = problemType === "Max";
            const hasBlanks = blankMatrix.some(row => row.some(cell => cell));
            const result = Module.runHungarianAlgorithm(
                costMatrix,
                maximize,
                BLANK_VALUE,
                hasBlanks
            );
            resultsContainer.innerHTML = "";
            const preElement = document.createElement("pre");
            preElement.textContent = result.solution;
            resultsContainer.appendChild(preElement);
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err.message || err}</p>`;
        }
    };

    // Initial render
    updateProblemType();
    updateMatrix();
}