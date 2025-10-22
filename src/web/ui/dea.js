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
        #tree { width: 100%; height: 100vh; overflow: hidden; }
        svg { display: block; }
        .link { fill: none; stroke: #ff0000ff; stroke-width: 5px; }
        .node-box { 
            border-radius: 8px; 
            box-shadow: 0 4px 6px rgba(0,0,0,0.1); 
            padding: 8px; 
            box-sizing: border-box; 
            overflow: auto; 
            border: 20px solid red;
        }
        .node-box.integer { background-color: #d1fae5; }
        .node-box.infeasible { background-color: #fee2e2; }
        .node-box.pruned { background-color: #e5e7eb; }
        .tableau table { border-collapse: collapse; margin-top: 6px; margin-left: auto; margin-right: auto; max-width: 100%; }
        .tableau th, .tableau td { border: 1px solid #d1d5db; padding: 3px; text-align: right; font-size: 9px; word-break: break-all; }
        .tableau .pivot-cell { background-color: #CC3300; font-weight: bold; }
        .tableau .pivot-row-col { background-color: #CC3300; }

        /* Form and button styling */
        .row { 
            display: flex; 
            gap: 10px; 
            margin-bottom: 15px; 
            align-items: center;
            justify-content: center; /* Center items in the row */
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
        #form input[type="number"], #form select {
            background: #1A1A1A;
            color: #FFFFFF;
            border: 1px solid #333333;
            padding: 6px;
            border-radius: 4px;
            margin: 5px 0;
        }
        #form input[type="number"] {
            width: 50px;
            -webkit-appearance: none !important;
            -moz-appearance: textfield !important;
            appearance: none !important;
        }
        #form input[type="number"]::-webkit-inner-spin-button,
        #form input[type="number"]::-webkit-outer-spin-button {
            -webkit-appearance: none !important;
            display: none !important;
            margin: 0 !important;
            opacity: 0 !important;
        }
        #form input[type="number"] {
            -moz-appearance: textfield !important;
        }
        #form select {
            width: auto;
            min-width: 60px;
        }
        #form input[type="number"]:focus, #form select:focus {
            border-color: #CC3300;
            outline: none;
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
        #form input[type="radio"]:hover {
            border-color: #CC3300;
        }
        #form input[type="radio"]:focus {
            outline: none;
            box-shadow: 0 0 0 2px #CC3300;
        }
        #form input[type="checkbox"] {
            -webkit-appearance: none;
            -moz-appearance: none;
            appearance: none;
            width: 16px;
            height: 16px;
            border: 2px solid #333333;
            border-radius: 4px;
            position: relative;
            cursor: pointer;
            transition: all 0.2s ease;
            margin-right: 8px;
        }
        #form input[type="checkbox"]:checked {
            border-color: #CC3300;
            background: #CC3300;
        }
        #form input[type="checkbox"]:checked::after {
            content: '✔';
            color: #FFFFFF;
            font-size: 12px;
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
        }
        #form input[type="checkbox"]:hover {
            border-color: #CC3300;
        }
        #form input[type="checkbox"]:focus {
            outline: none;
            box-shadow: 0 0 0 2px #CC3300;
        }
        label {
            margin-right: 15px;
            display: flex;
            align-items: center;
        }
        p {
            color: #000000;
            margin-bottom: 10px;
        }
        h1, h3, h4 {
            color: #CC3300;
            margin-bottom: 10px;
            text-align: center; /* Center headings */
        }
        .constraint {
            display: flex;
            gap: 10px;
            margin-bottom: 10px;
            align-items: center;
            justify-content: center; /* Center constraint rows */
        }
        // #results {
        //     background: #1A1A1A;
        //     border: 1px solid #333333;
        //     padding: 10px;
        //     border-radius: 4px;
        //     text-align: center; /* Center text in results */
        // }
        @media (max-width: 600px) {
            .row {
                flex-wrap: wrap;
                gap: 8px;
            }
            button {
                padding: 6px 12px;
                font-size: 0.85rem;
            }
            #form input[type="number"] {
                width: 40px;
            }
            #form input[type="radio"], #form input[type="checkbox"] {
                width: 14px;
                height: 14px;
            }
            #form input[type="radio"]:checked::after, #form input[type="checkbox"]:checked::after {
                width: 7px;
                height: 7px;
                font-size: 10px;
            }
        }
    </style>

    <h1 style="margin-top: 60px;">Data Envelopment Analysis</h1>

    <div id="problem-type" class="row">
        <input type="radio" id="max" name="problemType" value="Max" checked>
        <label for="max">Max</label>
        <input type="radio" id="min" name="problemType" value="Min">
        <label for="min">Min</label>
    </div>
    <br>

    <div class="button-group row">
        <button class="button" id="itemRowPlus">Item Row +</button>
        <button class="button" id="itemRowMinus">Item Row -</button>
    </div>

    <div class="button-group row">
        <button class="button" id="inputsPlus">Inputs +</button>
        <button class="button" id="inputsMinus">Inputs -</button>
        <button class="button" id="outputsPlus">Outputs +</button>
        <button class="button" id="outputsMinus">Outputs -</button>
    </div>

    <div id="InputSection"></div>

    <br>
    <div class="row">
        <button id="solveButton">Solve</button>
        <button style="background-color: red;" id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
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
            // LpInputs = [
            //     [5, 14],
            //     [8, 15],
            //     [7, 12]
            // ];

            // LpOutputs = [
            //     [9, 4, 16],
            //     [5, 7, 10],
            //     [4, 9, 13]
            // ];

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
