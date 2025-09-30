export function render(formContainer, resultsContainer, Module) {
    // Insert HTML
    formContainer.innerHTML = `
    <h1>Cheapest Insertion Traveling Sales Person</h1>

    <p id="problemTypeText">Problem is: Max</p>

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
      <button id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
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
        // try {

            distanceMatrix = [
                [ 0, 520, 980, 450, 633 ],  // City 1
                [ 520, 0, 204, 888, 557 ],  // City 2
                [ 980, 204, 0, 446, 1020 ], // City 3
                [ 450, 888, 446, 0, 249 ],  // City 4
                [ 633, 557, 1020, 249, 0 ]  // City 5
            ];

            let result = Module.runCheapestInsertion(distanceMatrix, startCity+1);

            resultsContainer.innerHTML = "";
            const preElement = document.createElement("pre");
            preElement.textContent = result.solution;
            resultsContainer.appendChild(preElement);
        // } catch (err) {
        //     resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        // }
    };

    // ===== INITIAL RENDER =====
    updateStartCitySelector();
    updateDistanceMatrix();
}