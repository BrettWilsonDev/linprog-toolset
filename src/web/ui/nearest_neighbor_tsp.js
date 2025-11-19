export function render(formContainer, resultsContainer, Module) {
    formContainer.innerHTML = `
    <style>
        /* BULLETPROOF SCROLL FIX - NEVER OVERFLOWS */
        #distanceMatrixContainer {
            margin-top: 0.5rem;
            padding: 1rem;
            background: #1A1A1A;
            border: 1px solid #333333;
            border-radius: 4px;
            color: #FFFFFF;
            width: 100%;
            box-sizing: border-box;
            overflow: hidden;
        }
        .matrix-scroll-wrapper {
            width: 100%;
            overflow-x: auto;
            overflow-y: hidden;
            -webkit-overflow-scrolling: touch;
            padding: 8px 4px;
            box-sizing: border-box;
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
            min-width: 600px;
            margin: 0 auto;
        }

        /* Your original styles - unchanged */
        .row {
            margin-bottom: 1rem;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 1rem;
            flex-wrap: wrap;
        }
        .row span { color: #FFFFFF; font-size: 1rem; }
        #numCitiesDisplay { color: #CC3300; font-weight: bold; }
        #addCity, #removeCity, #solveButton, #resetButton {
            background: #333333;
            color: #FFFFFF;
            border: none;
            padding: 8px 16px;
            border-radius: 4px;
            cursor: pointer;
            font-size: 1rem;
            transition: background 0.2s ease;
        }
        #addCity:hover, #removeCity:hover, #solveButton:hover {
            background: #CC3300;
            color: #000000;
        }
        #resetButton { background: #B30000; }
        #resetButton:hover { background: #FF3333; }
        #startCity {
            background: #1A1A1A;
            color: #FFFFFF;
            border: 1px solid #333333;
            padding: 8px;
            border-radius: 4px;
            font-size: 1rem;
            cursor: pointer;
            min-width: 150px;
            text-align: center;
        }
        #startCity:focus { border-color: #CC3300; outline: none; }
        #distanceMatrixContainer input,
        #distanceMatrixContainer select {
            background: #1A1A1A;
            color: #FFFFFF;
            border: 1px solid #333333;
            padding: 6px;
            border-radius: 4px;
            margin: 2px;
            font-size: 0.9rem;
            width: 60px;
            text-align: center;
        }
        #distanceMatrixContainer input:focus,
        #distanceMatrixContainer select:focus {
            border-color: #CC3300;
            outline: none;
        }
        #distanceMatrixContainer table {
            margin: 0 auto;
            border-collapse: collapse;
        }
        #distanceMatrixContainer th,
        #distanceMatrixContainer td {
            padding: 6px;
            border: 1px solid #333333;
            color: #FFFFFF;
            text-align: center;
        }
        h3 {
            color: #CC3300;
            font-size: 1.1rem;
            margin-bottom: 0.5rem;
            text-align: center;
        }
        @media (max-width: 600px) {
            .row { flex-direction: column; align-items: center; gap: 0.5rem; }
            #addCity, #removeCity, #solveButton, #resetButton {
                width: 100%; max-width: 200px; padding: 10px;
            }
            #startCity { width: 100%; max-width: 200px; }
            #distanceMatrixContainer input,
            #distanceMatrixContainer select { width: 50px; }
        }
    </style>

    <h1 style="margin-top: 60px;" class="row">Nearest Neighbor Traveling Sales Person</h1>
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
    let numCities = 3;
    let startCity = 0; // 0-based index
    let distanceMatrix = [
        [0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0]
    ];

    function updateStartCitySelector() {
        const select = document.getElementById("startCity");
        select.innerHTML = "";
        for (let i = 0; i < numCities; i++) {
            const opt = document.createElement("option");
            opt.value = i;
            opt.textContent = `City ${i + 1}`;
            select.appendChild(opt);
        }
        select.value = startCity;
        select.onchange = () => { startCity = parseInt(select.value); };
    }

    function updateDistanceMatrix() {
        const container = document.getElementById("distanceMatrixContainer");
        container.innerHTML = "";

        const table = document.createElement("table");
        table.style.borderCollapse = "collapse";

        // Header
        const header = document.createElement("tr");
        header.appendChild(document.createElement("th"));
        for (let j = 0; j < numCities; j++) {
            const th = document.createElement("th");
            th.textContent = `City ${j + 1}`;
            header.appendChild(th);
        }
        table.appendChild(header);

        // Rows
        for (let i = 0; i < numCities; i++) {
            const row = document.createElement("tr");
            const label = document.createElement("td");
            label.textContent = `City ${i + 1}`;
            row.appendChild(label);

            for (let j = 0; j < numCities; j++) {
                const cell = document.createElement("td");
                const input = document.createElement("input");
                input.type = "number";
                input.value = distanceMatrix[i][j];
                input.style.width = "60px";
                input.oninput = () => {
                    distanceMatrix[i][j] = parseFloat(input.value) || 0.0;
                };
                cell.appendChild(input);
                row.appendChild(cell);
            }
            table.appendChild(row);
        }

        // ONLY CHANGE: wrap in scrollable div
        const wrapper = document.createElement("div");
        wrapper.className = "matrix-scroll-wrapper";
        wrapper.appendChild(table);
        container.appendChild(wrapper);

        setTimeout(() => { wrapper.scrollLeft = 0; }, 0);
    }

    document.getElementById("addCity").onclick = () => {
        numCities++;
        const newMatrix = Array(numCities).fill().map(() => Array(numCities).fill(0.0));
        for (let i = 0; i < Math.min(distanceMatrix.length, numCities); i++) {
            for (let j = 0; j < Math.min(distanceMatrix[i].length, numCities); j++) {
                newMatrix[i][j] = distanceMatrix[i][j];
            }
        }
        distanceMatrix = newMatrix;
        document.getElementById("numCitiesDisplay").textContent = numCities;
        updateStartCitySelector();
        updateDistanceMatrix();
    };

    document.getElementById("removeCity").onclick = () => {
        if (numCities > 2) {
            numCities--;
            if (startCity >= numCities) startCity = 0;
            const newMatrix = Array(numCities).fill().map(() => Array(numCities).fill(0.0));
            for (let i = 0; i < numCities; i++) {
                for (let j = 0; j < numCities; j++) {
                    newMatrix[i][j] = distanceMatrix[i][j];
                }
            }
            distanceMatrix = newMatrix;
            document.getElementById("numCitiesDisplay").textContent = numCities;
            updateStartCitySelector();
            updateDistanceMatrix();
        }
    };

    document.getElementById("resetButton").onclick = () => {
        numCities = 3;
        startCity = 0;
        distanceMatrix = [
            [0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0],
            [0.0, 0.0, 0.0]
        ];
        resultsContainer.innerHTML = "";
        document.getElementById("numCitiesDisplay").textContent = "3";
        updateStartCitySelector();
        updateDistanceMatrix();
    };

    document.getElementById("solveButton").onclick = () => {
        try {
            const result = Module.runNearestNeighbor(distanceMatrix, startCity + 1);
            resultsContainer.innerHTML = "";
            const pre = document.createElement("pre");
            pre.textContent = result.solution;
            resultsContainer.appendChild(pre);
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };

    // Initial render
    updateStartCitySelector();
    updateDistanceMatrix();
}