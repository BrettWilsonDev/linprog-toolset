export function render(formContainer, resultsContainer, Module) {
    // Insert HTML
    formContainer.innerHTML = `
    <style>
            /* Form container and elements */
            .row {
            margin-bottom: 1rem;
            display: flex;
            align-items: center;
            justify-content: center; /* Center items horizontally */
            gap: 1rem;
            flex-wrap: wrap;
            }

            .row span {
            color: #FFFFFF;
            font-size: 1rem;
            }

            #numCitiesDisplay {
            color: #CC3300; /* Amber for emphasis */
            font-weight: bold;
            }

            #addCity, #removeCity, #solveButton, #resetButton {
            background: #333333; /* Dark gray background */
            color: #FFFFFF;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            cursor: pointer;
            font-size: 1rem;
            transition: background 0.2s ease;
            }

            #addCity:hover, #removeCity:hover, #solveButton:hover {
            background: #CC3300; /* Amber on hover */
            color: #000000;
            }

            #resetButton {
            background: #B30000; /* Darker red for reset button */
            }

            #resetButton:hover {
            background: #FF3333; /* Brighter red on hover */
            }

            #startCity {
            background: #1A1A1A; /* Dark gray background */
            color: #FFFFFF;
            border: 1px solid #333333;
            padding: 8px;
            border-radius: 4px;
            font-size: 1rem;
            cursor: pointer;
            min-width: 150px; /* Ensure consistent width for alignment */
            text-align: center; /* Center text within select */
            }

            #startCity:focus {
            border-color: #CC3300; /* Amber border on focus */
            outline: none;
            }

            #distanceMatrixContainer {
            margin-top: 0.5rem;
            padding: 1rem;
            background: #1A1A1A; /* Dark gray background */
            border: 1px solid #333333;
            border-radius: 4px;
            color: #FFFFFF;
            display: flex; /* Use Flexbox to center contents */
            justify-content: center; /* Center horizontally */
            align-items: center; /* Center vertically */
            width: 100%; /* Span the container */
            flex-wrap: wrap; /* Allow wrapping for responsiveness */
            }

            #distanceMatrixContainer * {
            text-align: center; /* Center text for all child elements */
            }

            #distanceMatrixContainer input,
            #distanceMatrixContainer select {
            background: #1A1A1A; /* Match dark theme */
            color: #FFFFFF;
            border: 1px solid #333333;
            padding: 6px;
            border-radius: 4px;
            margin: 2px; /* Small margin for spacing */
            font-size: 0.9rem;
            width: 60px; /* Fixed width for consistent input sizing */
            text-align: center; /* Center text in inputs */
            }

            #distanceMatrixContainer input:focus,
            #distanceMatrixContainer select:focus {
            border-color: #CC3300; /* Amber border on focus */
            outline: none;
            }

            #distanceMatrixContainer table {
            margin: 0 auto; /* Center table if used */
            border-collapse: collapse;
            }

            #distanceMatrixContainer th,
            #distanceMatrixContainer td {
            padding: 6px;
            border: 1px solid #333333;
            color: #FFFFFF;
            text-align: center; /* Center text in table cells */
            }

            h3 {
            color: #CC3300; /* Amber for subheadings */
            font-size: 1.1rem;
            margin-bottom: 0.5rem;
            text-align: center; /* Center subheading */
            }

            /* Responsive adjustments */
            @media (max-width: 600px) {
            .row {
                flex-direction: column;
                align-items: center; /* Center items in column layout */
                gap: 0.5rem;
            }

            #addCity, #removeCity, #solveButton, #resetButton {
                width: 100%;
                max-width: 200px; /* Limit width for better centering */
                padding: 10px;
            }

            #startCity {
                width: 100%;
                max-width: 200px; /* Consistent width for centering */
            }

            #distanceMatrixContainer {
                width: 100%;
            }

            #distanceMatrixContainer input,
            #distanceMatrixContainer select {
                width: 50px; /* Slightly smaller inputs for mobile */
                margin: 2px;
            }
            }
    </style>

    <h1 style="margin-top: 60px;" class="row">Cheapest Insertion Traveling Sales Person</h1>

    <div class="row">
      <span>Number of Cities: <span id="numCitiesDisplay">3</span></span>
      <button id="addCity">Cities +</button>
      <button id="removeCity">Cities -</button>
    </div>

    <div class="row">
      <label for="startCity">Starting City:</label>
      <select id="startCity"></select>
    </div>

    <div class="row">
      <h3>Distance Matrix</h3>
      <div id="distanceMatrixContainer"></div>
    </div>

    <div class="row">
      <button id="solveButton">Solve</button>
      <button style="background-color: red;" id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
    </div>
  `;

    // ===== STATE =====
    let problemType = "Max";
    let numCities = 3;
    let startCity = 0;
    let distanceMatrix = [
        [0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0]
    ];

    function updateStartCitySelector() {
        const startCitySelect = document.getElementById("startCity");
        startCitySelect.innerHTML = "";
        // Add Unknown option
        const unknownOption = document.createElement("option");
        unknownOption.value = -1;
        unknownOption.innerText = "Unknown";
        startCitySelect.appendChild(unknownOption);
        // Add city options
        for (let i = 0; i < numCities; i++) {
            const option = document.createElement("option");
            option.value = i;
            option.innerText = `City ${i + 1}`;
            startCitySelect.appendChild(option);
        }
        startCitySelect.value = startCity;
        startCitySelect.onchange = (e) => {
            startCity = parseInt(e.target.value);
        };
    }

    function updateDistanceMatrix() {
        const container = document.getElementById("distanceMatrixContainer");
        container.innerHTML = "";

        // Create table for distance matrix
        const table = document.createElement("table");
        table.style.borderCollapse = "collapse";

        // Header row
        const headerRow = document.createElement("tr");
        headerRow.appendChild(document.createElement("th")); // Empty top-left cell
        for (let j = 0; j < numCities; j++) {
            const th = document.createElement("th");
            th.innerText = `City ${j + 1}`;
            headerRow.appendChild(th);
        }
        table.appendChild(headerRow);

        // Data rows
        for (let i = 0; i < numCities; i++) {
            const row = document.createElement("tr");
            const labelCell = document.createElement("td");
            labelCell.innerText = `City ${i + 1}`;
            row.appendChild(labelCell);

            for (let j = 0; j < numCities; j++) {
                const cell = document.createElement("td");
                const input = document.createElement("input");
                input.type = "number";
                input.value = distanceMatrix[i][j];
                input.style.width = "60px";
                input.oninput = (e) => {
                    distanceMatrix[i][j] = parseFloat(e.target.value) || 0.0;
                };
                cell.appendChild(input);
                row.appendChild(cell);
            }
            table.appendChild(row);
        }
        container.appendChild(table);
    }

    document.querySelectorAll('input[name="problemType"]').forEach(radio => {
        radio.onchange = updateProblemType;
    });

    document.getElementById("addCity").onclick = () => {
        numCities++;
        // Resize distance matrix
        const newMatrix = Array(numCities).fill().map(() => Array(numCities).fill(0.0));
        for (let i = 0; i < Math.min(distanceMatrix.length, numCities); i++) {
            for (let j = 0; j < Math.min(distanceMatrix[i].length, numCities); j++) {
                newMatrix[i][j] = distanceMatrix[i][j];
            }
        }
        distanceMatrix = newMatrix;
        document.getElementById("numCitiesDisplay").innerText = numCities;
        updateStartCitySelector();
        updateDistanceMatrix();
    };

    document.getElementById("removeCity").onclick = () => {
        if (numCities > 2) {
            numCities--;
            startCity = Math.min(startCity, numCities - 1);
            // Resize distance matrix
            const newMatrix = Array(numCities).fill().map(() => Array(numCities).fill(0.0));
            for (let i = 0; i < numCities; i++) {
                for (let j = 0; j < numCities; j++) {
                    newMatrix[i][j] = distanceMatrix[i][j];
                }
            }
            distanceMatrix = newMatrix;
            document.getElementById("numCitiesDisplay").innerText = numCities;
            updateStartCitySelector();
            updateDistanceMatrix();
        }
    };

    document.getElementById("resetButton").onclick = () => {
        problemType = "Max";
        numCities = 3;
        startCity = -1; // Default to Unknown
        distanceMatrix = [
            [0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0]
        ];
        resultsContainer.innerHTML = "";
        document.getElementById("numCitiesDisplay").innerText = 3;
        updateStartCitySelector();
        updateDistanceMatrix();
    };

    function fmt(num, decimals = 6) {
        return parseFloat(num.toFixed(decimals));
    }

    document.getElementById("solveButton").onclick = () => {
        try {

            // distanceMatrix = [
            //     [ 0, 520, 980, 450, 633 ],  // City 1
            //     [ 520, 0, 204, 888, 557 ],  // City 2
            //     [ 980, 204, 0, 446, 1020 ], // City 3
            //     [ 450, 888, 446, 0, 249 ],  // City 4
            //     [ 633, 557, 1020, 249, 0 ]  // City 5
            // ];

            let result = Module.runCheapestInsertion(distanceMatrix, startCity+1);

            resultsContainer.innerHTML = "";
            const preElement = document.createElement("pre");
            preElement.textContent = result.solution;
            resultsContainer.appendChild(preElement);
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };

    // ===== INITIAL RENDER =====
    updateStartCitySelector();
    updateDistanceMatrix();
}