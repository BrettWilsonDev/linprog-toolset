export async function render(formContainer, resultsContainer, Module) {
    if (typeof window.d3 === "undefined") {
        await new Promise((resolve, reject) => {
            const script = document.createElement("script");
            // script.src = "https://d3js.org/d3.v7.min.js";
            script.src = "../../../vendor/d3js/d3.v7.min.js";
            script.onload = resolve;
            script.onerror = reject;
            document.head.appendChild(script);
        });
    }

    // Insert HTML with inline CSS for form and D3.js styling
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
        svg { border: 1px solid red; width: 100%; }
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
            flex-wrap: wrap;
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
            flex-wrap: wrap;
        }
        #results {
            background: #1A1A1A;
            border: 1px solid #333333;
            padding: 10px;
            border-radius: 4px;
            text-align: center; /* Center text in results */
        }
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

    <div class="wrapper">
        <h1 style="margin-top: 60px;">Branch and Bound</h1>

        <div class="row">
            <label><input type="radio" name="problemType" value="Max" checked> Max</label>
            <label><input type="radio" name="problemType" value="Min"> Min</label>
        </div>

        <p id="problemTypeText">Problem is: Max</p>

        <div class="row">
            <button id="addDecisionVar">Decision Variables +</button>
            <button id="removeDecisionVar">Decision Variables -</button>
        </div>

        <div id="objectiveFunction" class="row"></div>

        <div class="row">
            <button id="addConstraint">Constraint +</button>
            <button id="removeConstraint">Constraint -</button>
        </div>

        <div id="constraintsContainer"></div>

        <div style="margin-top: 20px;">
            <div  id="toggleContainer" class="row">
                <label><input type="checkbox" id="toggleBtn" checked> Assume Binary</label>
            </div>
        </div>

        <div class="row">
            <button id="solveButton">Solve</button>
            <button style="background-color: red;" id="resetButton">Reset</button>
        </div>

        <h1>Branch and Bound Tree</h1>
        <p style="text-align: center; color: #FFFFFF;">Drag and zoom</p>
        <div class="row">
            <button hidden id="recenterButton">Recenter</button>
        </div>
    </div>
    <div id="tree"></div>
    `;

    // ===== STATE =====
    let problemType = "Max";
    let amtOfObjVars = 2;
    let amtOfConstraints = 1;
    let objFunc = [0.0, 0.0];
    let constraints = [[0.0, 0.0, 0.0, 0.0]];
    let signItems = ["<=", ">="];
    let signItemsChoices = [0];

    function updateProblemType() {
        problemType = document.querySelector('input[name="problemType"]:checked').value;
        document.getElementById("problemTypeText").innerText = "Problem is: " + problemType;
    }

    function updateObjectiveFunction() {
        const objFuncContainer = document.getElementById("objectiveFunction");
        objFuncContainer.innerHTML = "";
        for (let i = 0; i < amtOfObjVars; i++) {
            const input = document.createElement("input");
            input.type = "number";
            input.value = objFunc[i];
            input.oninput = (e) => {
                objFunc[i] = parseFloat(e.target.value);
            };
            const label = document.createElement("span");
            label.innerText = `x${i + 1}`;
            objFuncContainer.appendChild(input);
            objFuncContainer.appendChild(label);
        }
    }

    function updateConstraints() {
        const container = document.getElementById("constraintsContainer");
        container.innerHTML = "";
        for (let i = 0; i < amtOfConstraints; i++) {
            const constraintRow = document.createElement("div");
            constraintRow.className = "constraint";
            for (let j = 0; j < amtOfObjVars; j++) {
                const input = document.createElement("input");
                input.type = "number";
                input.value = constraints[i][j];
                input.oninput = (e) => {
                    constraints[i][j] = parseFloat(e.target.value);
                };
                const label = document.createElement("span");
                label.innerText = `x${j + 1}`;
                constraintRow.appendChild(input);
                constraintRow.appendChild(label);
            }
            const signSelect = document.createElement("select");
            signItems.forEach((sign, index) => {
                const option = document.createElement("option");
                option.value = index;
                option.innerText = sign;
                signSelect.appendChild(option);
            });
            signSelect.value = signItemsChoices[i];
            signSelect.onchange = (e) => {
                signItemsChoices[i] = parseInt(e.target.value);
                constraints[i][amtOfObjVars + 1] = signItemsChoices[i];
            };
            const rhsInput = document.createElement("input");
            rhsInput.type = "number";
            rhsInput.value = constraints[i][amtOfObjVars];
            rhsInput.oninput = (e) => {
                constraints[i][amtOfObjVars] = parseFloat(e.target.value);
            };
            constraintRow.appendChild(signSelect);
            constraintRow.appendChild(rhsInput);
            container.appendChild(constraintRow);
        }
        const toggleContainer = document.getElementById('toggleContainer');
        if (amtOfConstraints == 1) {
            toggleContainer.style.display = 'flex';
            document.getElementById('toggleBtn').checked = true;
        } else {
            toggleContainer.style.display = 'none';
            document.getElementById('toggleBtn').checked = false;
        }
    }

    document.querySelectorAll('input[name="problemType"]').forEach(radio => {
        radio.onchange = updateProblemType;
    });

    document.getElementById("addDecisionVar").onclick = () => {
        amtOfObjVars++;
        objFunc.push(0.0);
        constraints.forEach(constraint => constraint.splice(amtOfObjVars - 1, 0, 0.0));
        updateObjectiveFunction();
        updateConstraints();
    };

    document.getElementById("removeDecisionVar").onclick = () => {
        if (amtOfObjVars > 2) {
            amtOfObjVars--;
            objFunc.pop();
            constraints.forEach(constraint => constraint.splice(amtOfObjVars, 1));
            updateObjectiveFunction();
            updateConstraints();
        }
    };

    document.getElementById("addConstraint").onclick = () => {
        amtOfConstraints++;
        const newConstraint = new Array(amtOfObjVars).fill(0.0);
        newConstraint.push(0.0); // for sign
        newConstraint.push(0.0); // for rhs
        constraints.push(newConstraint);
        signItemsChoices.push(0);
        updateConstraints();
    };

    document.getElementById("removeConstraint").onclick = () => {
        if (amtOfConstraints > 1) {
            amtOfConstraints--;
            constraints.pop();
            signItemsChoices.pop();
            updateConstraints();
        }
    };

    function resetRadios() {
        document.querySelector('input[value="Max"]').checked = true;
        document.getElementById('toggleBtn').checked = true;
    }

    document.getElementById("resetButton").onclick = () => {
        problemType = "Max";
        amtOfObjVars = 2;
        amtOfConstraints = 1;
        objFunc = [0.0, 0.0];
        constraints = [[0.0, 0.0, 0.0, 0.0]];
        signItems = ["<=", ">="];
        signItemsChoices = [0];
        resultsContainer.innerHTML = "";

        document.getElementById("recenterButton").hidden = true;

        const svg = document.getElementById("tree");
        svg.innerHTML = "";

        updateObjectiveFunction();
        updateConstraints();
        resetRadios();
    };

    function fmt(num, decimals = 6) {
        return parseFloat(num.toFixed(decimals));
    }

    document.getElementById("solveButton").onclick = () => {

        try {
            document.getElementById("recenterButton").removeAttribute("hidden");

            // objFunc = [2, 3, 3, 5, 2, 4];
            // constraints = [[11, 8, 6, 14, 10, 10, 40, 0]];

            const toggleBtn = document.getElementById('toggleBtn');
            if (toggleBtn.checked) {
                const numConstraints = objFunc.length;
                const vectorLength = objFunc.length + 3;
                for (let i = 0; i < numConstraints; i++) {
                    const row = Array(vectorLength).fill(0);
                    row[i] = 1;
                    row[vectorLength - 2] = 1;
                    constraints.push(row);
                }
            }
            let isMin = (problemType === "Min");
            const result = Module.runBranchAndBound(objFunc, constraints, isMin);

            function parseTableau(tableauStr) {
                if (!tableauStr) return null;
                const lines = tableauStr.trim().split('\n').slice(1);
                if (lines.length < 1) return null;
                const headers = lines[0].trim().split(/\s+/);
                const matrix = lines.slice(1).map(row => row.trim().split(/\s+/).map(val => parseFloat(val) || val));
                return { headers, matrix };
            }

            function generateTableauHtml(tableauStr, title, pivotCol, pivotRow) {
                if (!tableauStr) return '';
                const tableau = parseTableau(tableauStr);
                if (!tableau) return '';
                const { headers, matrix } = tableau;
                let html = `<h4 class="text-xs font-semibold mb-1 text-center">${title}</h4><table class="tableau"><thead><tr>`;
                headers.forEach((h, colIndex) => {
                    const isPivotCol = pivotCol === colIndex;
                    html += `<th${isPivotCol && pivotRow !== -1 ? ' class="pivot-row-col"' : ''}>${h}</th>`;
                });
                html += '</tr></thead><tbody>';
                matrix.forEach((row, rowIndex) => {
                    html += '<tr>';
                    row.forEach((val, colIndex) => {
                        const isPivot = pivotRow === rowIndex && pivotCol === colIndex;
                        const isInPivotRow = pivotRow === rowIndex;
                        const isInPivotCol = pivotCol === colIndex;
                        const cellClass = isPivot ? 'pivot-cell' : (isInPivotRow || isInPivotCol) ? 'pivot-row-col' : '';
                        html += `<td${cellClass ? ` class="${cellClass}"` : ''}>${typeof val === 'number' ? val.toFixed(6) : val}</td>`;
                    });
                    html += '</tr>';
                });
                html += '</tbody></table>';
                return html;
            }

            let GOLBAL_MAX_HEIGHT = 0;
            function calculateMaxHeight(hierarchy) {
                let maxHeight = 0;
                hierarchy.each(d => {
                    let h = 80;
                    if (d.data.constraintsPath && d.data.constraintsPath.length > 0) {
                        const constraintsText = d.data.constraintsPath.join('; ');
                        const charsPerLine = 60;
                        const approxLines = Math.ceil(constraintsText.length / charsPerLine) || 1;
                        h += 20 * approxLines;
                    }
                    const tableaus = [
                        { str: d.data.unfixedTab },
                        { str: d.data.fixedTab },
                        ...d.data.intermediateTableaus.map(t => ({ str: t })),
                        { str: d.data.finalTableau }
                    ];
                    tableaus.forEach(t => {
                        if (t.str) {
                            const tab = parseTableau(t.str);
                            if (tab) h += 30 + 20 * tab.matrix.length;
                        }
                    });
                    maxHeight = Math.max(maxHeight, h);
                });
                GOLBAL_MAX_HEIGHT = maxHeight * 1.1;
                return maxHeight * 1.1;
            }

            function calculateMaxWidth(hierarchy) {
                let maxWidth = 0;
                hierarchy.each(d => {
                    let w = 200;
                    if (d.data.constraintsPath && d.data.constraintsPath.length > 0) {
                        const constraintsText = d.data.constraintsPath.join('; ');
                        const maxLineWidth = Math.max(...constraintsText.split('; ').map(line => line.length * 6));
                        w = Math.max(w, maxLineWidth);
                    }
                    const tableaus = [
                        d.data.unfixedTab,
                        d.data.fixedTab,
                        ...d.data.intermediateTableaus,
                        d.data.finalTableau
                    ].filter(Boolean);
                    tableaus.forEach(t => {
                        const tab = parseTableau(t);
                        if (tab) {
                            const colCount = tab.headers.length;
                            const estColWidth = 50;
                            let estWidth = colCount * estColWidth;
                            tab.headers.forEach(h => estWidth = Math.max(estWidth, h.length * 8));
                            tab.matrix.forEach(row => {
                                row.forEach(val => {
                                    const valStr = typeof val === 'number' ? val.toFixed(6) : val.toString();
                                    estWidth = Math.max(estWidth, valStr.length * 8);
                                });
                            });
                            w = Math.max(w, estWidth);
                        }
                    });
                    maxWidth = Math.max(maxWidth, w);
                });
                return Math.max(maxWidth + 60, 450);
            }

            function renderTree(rootData) {
                d3.select('#tree').selectAll('*').remove();
                const hierarchy = d3.hierarchy(rootData);
                const maxBoxHeight = calculateMaxHeight(hierarchy);
                const maxBoxWidth = calculateMaxWidth(hierarchy);
                const nodeSizeWidth = maxBoxWidth + 100;
                const nodeSizeHeight = maxBoxHeight + 100;
                const treeLayout = d3.tree().nodeSize([nodeSizeWidth, nodeSizeHeight]);
                const treeData = treeLayout(hierarchy);
                const allRight = treeData.descendants().map(d => d.x + maxBoxWidth / 2);
                const allLeft = treeData.descendants().map(d => d.x - maxBoxWidth / 2);
                const minLeft = Math.min(...allLeft);
                const maxRight = Math.max(...allRight);
                const allBottom = treeData.descendants().map(d => d.y + maxBoxHeight / 2);
                const allTop = treeData.descendants().map(d => d.y - maxBoxHeight / 2);
                const minTop = Math.min(...allTop);
                const maxBottom = Math.max(...allBottom);
                const horizontalMargin = 50;
                const verticalMargin = 50;
                const contentWidth = Math.max(1, (maxRight - minLeft) + horizontalMargin * 2);
                const contentHeight = Math.max(1, (maxBottom - minTop) + verticalMargin * 2);
                const svgWidth = window.innerWidth;
                const svgHeight = window.innerHeight;
                const svg = d3.select('#tree').append('svg')
                    .attr('width', svgWidth)
                    .attr('height', svgHeight);
                const zoomG = svg.append('g');
                const minScale = 0.1;
                const maxScale = 6;
                const zoom = d3.zoom()
                    .scaleExtent([minScale, maxScale])
                    .on('zoom', (event) => {
                        zoomG.attr('transform', event.transform);
                    });
                svg.call(zoom);
                const g = zoomG.append('g');
                g.selectAll('.link')
                    .data(treeData.links())
                    .enter()
                    .append('path')
                    .attr('class', 'link')
                    .attr('d', d => {
                        const sourceX = d.source.x, sourceY = d.source.y + maxBoxHeight / 2;
                        const targetX = d.target.x, targetY = d.target.y - maxBoxHeight / 2;
                        return `M${sourceX},${sourceY} L${targetX},${targetY}`;
                    });
                const nodes = g.selectAll('.node')
                    .data(treeData.descendants())
                    .enter()
                    .append('g')
                    .attr('class', 'node')
                    .attr('transform', d => `translate(${d.x},${d.y})`);
                nodes.append('foreignObject')
                    .attr('width', maxBoxWidth)
                    .attr('height', maxBoxHeight)
                    .attr('x', -maxBoxWidth / 2)
                    .attr('y', -maxBoxHeight / 2)
                    .html(d => {
                        let html = `<div class="node-box" style="width:${maxBoxWidth}px; height:${maxBoxHeight}px; display:flex; flex-direction:column; justify-content:flex-start; align-items:center;">`;
                        html += `<div class="text-sm font-bold text-center">Node ${d.data.name}</div>`;
                        html += `<div class="text-xs text-center">Objective: ${d.data.objective !== null ? d.data.objective.toFixed(2) : 'INFEASBLE'}</div>`;
                        html += `<div class="text-xs text-center">Solution: (${d.data.solution ? d.data.solution.map(s => s.toFixed(2)).join(', ') : 'INFEASBLE'})</div>`;
                        if (d.data.constraintsPath && d.data.constraintsPath.length > 0) {
                            html += `<div class="text-xs mt-1 break-words text-center">${d.data.constraintsPath.join('; ')}</div>`;
                        }
                        if (d.data.unfixedTab) html += generateTableauHtml(d.data.unfixedTab, 'Unfixed Tableau', -1, -1);
                        if (d.data.fixedTab) html += generateTableauHtml(d.data.fixedTab, 'Fixed Tableau', -1, -1);
                        d.data.intermediateTableaus.forEach((t, i) => {
                            html += generateTableauHtml(t, `Tableau ${i + 1}`, d.data.pivotCols ? d.data.pivotCols[i] : -1, d.data.pivotRows ? d.data.pivotRows[i] : -1);
                        });
                        if (d.data.finalTableau) html += generateTableauHtml(d.data.finalTableau, 'Final Tableau', -1, -1);
                        html += '</div>';
                        return html;
                    });

                function centerOnNodeIndex(nodeIndex = 0, desiredScale = null) {
                    const descendants = treeData.descendants();
                    if (!descendants || descendants.length === 0) return;
                    const idx = Math.max(0, Math.min(nodeIndex, descendants.length - 1));
                    const node = descendants[idx];
                    const nodeX = node.x;
                    const nodeY = node.y;
                    let k = desiredScale;
                    if (k == null) {
                        k = 1.0;
                    }
                    k = Math.max(minScale, Math.min(maxScale, k));
                    const tx = svgWidth / 2 - k * nodeX;
                    const ty = GOLBAL_MAX_HEIGHT / 2;
                    svg.transition().duration(500).call(
                        zoom.transform,
                        d3.zoomIdentity.translate(tx, ty).scale(k)
                    );
                }

                centerOnNodeIndex(0);

                document.getElementById("recenterButton").onclick = () => {
                    centerOnNodeIndex(0); // Center on root node with default scale
                };
            }

            let jsonOut = JSON.parse(result.json);
            renderTree(jsonOut);
            resultsContainer.innerHTML = "";
            const preElement = document.createElement("pre");
            preElement.textContent = result.solution;
            resultsContainer.appendChild(preElement);
        } catch (err) {
            document.getElementById("recenterButton").hidden = true;
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };

    // ===== INITIAL RENDER =====
    updateObjectiveFunction();
    updateConstraints();
    updateProblemType();
}