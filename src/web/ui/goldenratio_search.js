export function render(formContainer, resultsContainer, Module) {
    // Insert HTML with input section using default browser styles
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
            background-color: #1A1A1A;
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

        .btn {
            margin: 5px;
        }
    </style>
    
        <h1 style="margin-top: 60px;" class="row">Golden Ratio Search</h1>
        <div class="row">
            <label>
                <input type="radio" name="problemType" value="Max" checked> Max
            </label>
            <label>
                <input type="radio" name="problemType" value="Min"> Min
            </label>
        </div>
        <div id="InputSection">
            <div>
                <label class="row">Function</label>
                <div class="row">
                    <input type="text" id="funcInput" placeholder="x^3-6*x">
                </div>
            </div>
            <div>
                <div class="row">
                    <label>xLower</label>
                    <input type="number" id="xLowerInput" value="0.0" step="any">
                </div>
                <div class="row">
                    <label>xUpper</label>
                    <input type="number" id="xUpperInput" value="1.0" step="any">
                </div>
                <div class="row">
                    <label>tol</label>
                    <input type="number" id="tolInput" value="0.05" step="any">
                </div>
                <div class="row">
                    <label>Max Iterations</label>
                    <input type="number" id="maxIterationsInput" value="100" step="any">
                </div>
            </div>
        </div>
        <div class="row">
            <button id="solveButton">Solve</button>
            <button style="background-color: red;" id="resetButton">Reset</button>
        </div>
    `;

    // Function to dynamically generate HTML table from output string
    function generateTableFromOutput(rawOutput) {
        // Parse the output string
        const lines = rawOutput.trim().split('\n').filter(line => line.trim() !== '');
        let data = {
            function: '',
            headers: [],
            rows: [],
            optimalX: null,
            optimalValue: null,
            type: ''
        };

        let currentSection = 'header';
        for (const line of lines) {
            if (line.startsWith('Function:')) {
                data.function = line.replace('Function:', '').trim();
            } else if (line.startsWith('Iteration |')) {
                data.headers = line.split('|').map(h => h.trim());
                currentSection = 'rows';
            } else if (currentSection === 'rows' && line.match(/^\d+\s*\|/)) {
                const row = line.split('|').map(v => v.trim());
                data.rows.push(row.map((v, i) => i === 0 ? parseInt(v) : parseFloat(v) || v));
            } else if (line.startsWith('Optimal x:')) {
                data.optimalX = parseFloat(line.replace('Optimal x:', '').trim());
                currentSection = 'footer';
            } else if (line.startsWith('Min') || line.startsWith('Max')) {
                data.type = line.split(' ')[0].trim();
                // data.optimalValue = parseFloat(line.replace(`${data.type} f(x):`, '').trim());
                data.optimalValue = parseFloat(line.replace(`${data.type} z:`, '').trim());
            }
        }

        // Generate HTML table with inline CSS
        return `

            <div style="box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1); border-radius: 8px; padding: 16px;">
                <p style="margin-bottom: 16px;">Function: ${data.function}</p>
                <table style="width: 100%; background-color: white; border: 1px solid black; border-collapse: collapse;">
                    <thead>
                        <tr style="background-color: #e5e7eb;">
                            ${data.headers.map(header => `<th style="padding: 8px 16px; border: 1px solid black; text-align: left;">${header}</th>`).join('')}
                        </tr>
                    </thead>
                    <tbody>
                        ${data.rows.length > 0 ? data.rows.map(row => `
                            <tr style="background-color: ${row[0] % 2 === 0 ? '#f9fafb' : 'white'};">
                                ${row.map((cell, index) => `
                                    <td style="padding: 8px 16px; border: 1px solid black;">
                                        ${index === 0 ? cell : typeof cell === 'number' ? cell.toFixed(6) : cell}
                                    </td>
                                `).join('')}
                            </tr>
                        `).join('') : `
                            <tr style="background-color: white;">
                                <td style="padding: 8px 16px; border: 1px solid black; text-align: center;" colspan="${data.headers.length}">No iteration data available</td>
                            </tr>
                        `}
                    </tbody>
                </table>
                <p style="margin-top: 16px;">Optimal x: ${data.optimalX !== null ? data.optimalX.toFixed(6) : 'N/A'}</p>
                <p>${data.type} Z: ${data.optimalValue !== null ? data.optimalValue.toFixed(6) : 'N/A'} Radians</p>
            </div>
        `;
    }

    // ===== STATE =====
    let problemType = "Max";
    let func = "";
    let params = [0.0, 1.0, 0.05]; // [xLower, xUpper, tol]

    function updateProblemType() {
        problemType = document.querySelector('input[name="problemType"]:checked').value;
    }

    // Radio button event listeners
    document.querySelectorAll('input[name="problemType"]').forEach(radio => {
        radio.onchange = updateProblemType;
    });

    // Input event listeners
    document.getElementById("funcInput").addEventListener("input", (event) => {
        func = event.target.value;
    });

    document.getElementById("xLowerInput").addEventListener("input", (event) => {
        params[0] = parseFloat(event.target.value) || 0.0;
    });

    document.getElementById("xUpperInput").addEventListener("input", (event) => {
        params[1] = parseFloat(event.target.value) || 1.0;
    });

    document.getElementById("tolInput").addEventListener("input", (event) => {
        params[2] = parseFloat(event.target.value) || 0.05;
    });

    document.getElementById("maxIterationsInput").addEventListener("input", (event) => {
        params[3] = parseInt(event.target.value) || 100;
    });


    function resetRadios() {
        document.querySelector('input[value="Max"]').checked = true;
    }

    document.getElementById("resetButton").onclick = () => {
        problemType = "Max";
        func = "";
        params = [0.0, 1.0, 0.05, 100];
        document.getElementById("funcInput").value = "";
        document.getElementById("xLowerInput").value = "0.0";
        document.getElementById("xUpperInput").value = "1.0";
        document.getElementById("tolInput").value = "0.05";
        document.getElementById("maxIterationsInput").value = "100";
        resetRadios();
        resultsContainer.innerHTML = "";
    };

    document.getElementById("solveButton").onclick = () => {
        // func = "x^3-6*x";
        // params = [0, 3, 0.05];

        try {
            if (!func) {
                throw new Error("Function string is empty");
            }
            if (isNaN(params[0]) || isNaN(params[1]) || isNaN(params[2])) {
                throw new Error("Invalid parameters");
            }
            const isMin = (problemType === "Min");
            const result = Module.runGoldenSectionSearch(func, params[0], params[1], params[2], isMin, params[3] + 1);
            resultsContainer.innerHTML = generateTableFromOutput(result.outputString);
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color: red;">Error: ${err.message}</p>`;
        }
    };

    // ===== INITIAL RENDER =====
    resetRadios();
}