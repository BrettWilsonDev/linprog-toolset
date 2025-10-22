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

    <h3 class="row">Cost/Profit Matrix (Agents → Tasks)</h3>
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
        // if (selected) {
        //     problemType = selected.value;
        // }
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
            // th.style.border = "1px solid #ccc";
            // th.style.border = "1px solid #ccc";
            th.style.padding = "5px";
            headerRow.appendChild(th);
        }
        table.appendChild(headerRow);

        // Data rows
        for (let i = 0; i < numRows; i++) {
            const row = document.createElement("tr");
            const labelCell = document.createElement("td");
            labelCell.innerText = `Agent ${i + 1}`;
            // labelCell.style.border = "1px solid #ccc";
            labelCell.style.padding = "5px";
            row.appendChild(labelCell);

            for (let j = 0; j < numCols; j++) {
                const cell = document.createElement("td");
                // cell.style.border = "1px solid #ccc";
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
            // costMatrix = [
            //     [22, 18, 30, 18],
            //     [18, BLANK_VALUE, 27, 22],
            //     [26, 20, 28, 28],
            //     [16, 22, BLANK_VALUE, 14],
            //     [21, BLANK_VALUE, 25, 28]
            // ];

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