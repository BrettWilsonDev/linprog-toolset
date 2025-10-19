export function render(formContainer, resultsContainer, Module) {
    // Insert HTML with input section using default browser styles
    formContainer.innerHTML = `
        <h1>Golden Ratio Search</h1>
        <div>
            <label>
                <input type="radio" name="problemType" value="Max" checked> Max
            </label>
            <label>
                <input type="radio" name="problemType" value="Min"> Min
            </label>
        </div>
        <div id="problemTypeText">Problem is: Max</div>
        <div id="InputSection">
            <div>
                <label>Function</label>
                <input type="text" id="funcInput" placeholder="x^3-6*x">
            </div>
            <div>
                <div>
                    <label>xLower</label>
                    <input type="number" id="xLowerInput" value="0.0" step="any">
                </div>
                <div>
                    <label>xUpper</label>
                    <input type="number" id="xUpperInput" value="1.0" step="any">
                </div>
                <div>
                    <label>tol</label>
                    <input type="number" id="tolInput" value="0.05" step="any">
                </div>
            </div>
        </div>
        <div>
            <button id="solveButton">Solve</button>
            <button id="resetButton">Reset</button>
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
            <div style="background-color: white; box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1); border-radius: 8px; padding: 16px;">
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
        document.getElementById("problemTypeText").innerText = "Problem is: " + problemType;
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

    function resetRadios() {
        document.querySelector('input[value="Max"]').checked = true;
        document.getElementById("problemTypeText").innerText = "Problem is: Max";
    }

    document.getElementById("resetButton").onclick = () => {
        problemType = "Max";
        func = "";
        params = [0.0, 1.0, 0.05];
        document.getElementById("funcInput").value = "";
        document.getElementById("xLowerInput").value = "0.0";
        document.getElementById("xUpperInput").value = "1.0";
        document.getElementById("tolInput").value = "0.05";
        resetRadios();
        resultsContainer.innerHTML = "";
    };

    document.getElementById("solveButton").onclick = () => {
        func = "x^3-6*x";
        params = [0, 3, 0.05];

        try {
            if (!func) {
                throw new Error("Function string is empty");
            }
            if (isNaN(params[0]) || isNaN(params[1]) || isNaN(params[2])) {
                throw new Error("Invalid parameters");
            }
            const isMin = (problemType === "Min");
            const result = Module.runGoldenSectionSearch(func, params[0], params[1], params[2], isMin);
            resultsContainer.innerHTML = generateTableFromOutput(result.outputString);

            console.log(result.outputString);
            
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color: red;">Error: ${err.message}</p>`;
        }
    };

    // ===== INITIAL RENDER =====
    resetRadios();
}