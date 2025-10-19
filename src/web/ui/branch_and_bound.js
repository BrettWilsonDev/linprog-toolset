export async function render(formContainer, resultsContainer, Module) {

    if (typeof window.d3 === "undefined") {
        await new Promise((resolve, reject) => {
            const script = document.createElement("script");
            script.src = "https://d3js.org/d3.v7.min.js";
            script.onload = resolve;
            script.onerror = reject;
            document.head.appendChild(script);
        });
    }

    // Insert HTML
    formContainer.innerHTML = `
    <style>
        body { font-family: Arial, sans-serif; margin: 0; background: #f9fafb; }
        #tree { width: 100%; height: 100vh; overflow: hidden; }
        svg { display: block; }
        .link { fill: none; stroke: #6b7280; stroke-width: 2px; }
        .node-box { 
            background-color: #f3f4f6; 
            border-radius: 8px; 
            box-shadow: 0 4px 6px rgba(0,0,0,0.1); 
            padding: 8px; 
            box-sizing: border-box; 
            overflow: auto; 
        }
        .node-box.integer { background-color: #d1fae5; }
        .node-box.infeasible { background-color: #fee2e2; }
        .node-box.pruned { background-color: #e5e7eb; }
        .tableau table { border-collapse: collapse; margin-top: 6px; margin-left: auto; margin-right: auto; max-width: 100%; }
        .tableau th, .tableau td { border: 1px solid #d1d5db; padding: 3px; text-align: right; font-size: 9px; word-break: break-all; }
        .tableau th { background-color: #e5e7eb; }
        .tableau .pivot-cell { background-color: hsl(103, 90%, 62%); font-weight: bold; }
        .tableau .pivot-row-col { background-color: hsl(103, 90%, 62%); }
        </style>

        <h1>Branch and Bound</h1>

        <div>
        <label><input type="radio" name="problemType" value="Max" checked> Max</label>
        <label><input type="radio" name="problemType" value="Min"> Min</label>
        </div>

        <p id="problemTypeText">Problem is: Max</p>

        <div class="row">
        <button id="addDecisionVar">decision variables +</button>
        <button id="removeDecisionVar">decision variables -</button>
        </div>

        <div id="objectiveFunction" class="row"></div>

        <div class="row">
        <button id="addConstraint">Constraint +</button>
        <button id="removeConstraint">Constraint -</button>
        </div>

        <div id="constraintsContainer"></div>

        <div id="toggleContainer" style="display: none;">
        <br>
        <input type="checkbox" id="toggleBtn" />
        <label for="toggleBtn">Assume Binary</label>
        <br>
        <br>
        </div>


        <div class="row">
        <button id="solveButton">Solve</button>
        <button id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
        </div>

        <h1 class="text-2xl font-bold p-4">Branch and Bound Tree</h1>
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
        const toggleBtn = document.getElementById('toggleBtn');

        if (amtOfConstraints == 1) {
            toggleBtn.checked = false;
            toggleContainer.style.display = 'block';
        }
        else {
            toggleBtn.checked = false;
            toggleContainer.style.display = 'none';
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
        newConstraint.push(0.0);  // for sign
        newConstraint.push(0.0);  // for rhs
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

        updateObjectiveFunction();
        updateConstraints();
        resetRadios();
    };

    function fmt(num, decimals = 6) {
        return parseFloat(num.toFixed(decimals));
    }

    document.getElementById("solveButton").onclick = () => {
        try {
            objFunc = [2, 3, 3, 5, 2, 4];

            // constraints = [
            //     [11, 8, 6, 14, 10, 10, 40, 0],
            //     [1, 0, 0, 0, 0, 0, 0, 1, 0],
            //     [0, 1, 0, 0, 0, 0, 0, 1, 0],
            //     [0, 0, 1, 0, 0, 0, 0, 1, 0],
            //     [0, 0, 0, 1, 0, 0, 0, 1, 0],
            //     [0, 0, 0, 0, 1, 0, 0, 1, 0],
            //     [0, 0, 0, 0, 0, 1, 0, 1, 0],
            // ]

            constraints = [[11, 8, 6, 14, 10, 10, 40, 0]]

            // objFunc = [300, 840, 160, 520];

            // constraints = [
            //     [7, 15, 3, 13, 23, 0],
            // ]

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
                    // let h = 80;
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


                GOLBAL_MAX_HEIGHT = maxHeight * 1.1
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

            /* ---------- main rendering ---------- */
            function renderTree(rootData) {
                d3.select('#tree').selectAll('*').remove();

                const hierarchy = d3.hierarchy(rootData);
                const maxBoxHeight = calculateMaxHeight(hierarchy);
                const maxBoxWidth = calculateMaxWidth(hierarchy);

                const nodeSizeWidth = maxBoxWidth + 100;
                const nodeSizeHeight = maxBoxHeight + 100;

                const treeLayout = d3.tree().nodeSize([nodeSizeWidth, nodeSizeHeight]);
                const treeData = treeLayout(hierarchy);

                // bounding box of content
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
                        //   let html = `<div class="node-box" style="width:${maxBoxWidth}px; height:${maxBoxHeight}px; display:flex; flex-direction:column; justify-content:center; align-items:center;">`;
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

                // --- helper to center on a node by index ---
                function centerOnNodeIndex(nodeIndex = 0, desiredScale = null) {
                    const descendants = treeData.descendants();
                    if (!descendants || descendants.length === 0) return;
                    const idx = Math.max(0, Math.min(nodeIndex, descendants.length - 1));
                    const node = descendants[idx];
                    const nodeX = node.x;
                    const nodeY = node.y;

                    let k = desiredScale;
                    if (k == null) {
                        //   k = Math.min(svgWidth / contentWidth, svgHeight / contentHeight) * 1.2; // zoom in a bit
                        k = 1.0
                    }
                    k = Math.max(minScale, Math.min(maxScale, k));

                    const tx = svgWidth / 2 - k * nodeX;
                    // const ty = svgHeight / 2 - k * nodeY + 500;
                    const ty = GOLBAL_MAX_HEIGHT / 2;

                    svg.transition().duration(500).call(
                        zoom.transform,
                        d3.zoomIdentity.translate(tx, ty).scale(k)
                    );
                }

                // ---- Center on the first node (root) ----
                centerOnNodeIndex(0);
            }

            let jsonOut = JSON.parse(result.json)

            renderTree(jsonOut);

            resultsContainer.innerHTML = "";
            const preElement = document.createElement("pre");
            preElement.textContent = result.solution;
            resultsContainer.appendChild(preElement);

        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };

    // ===== INITIAL RENDER =====
    updateObjectiveFunction();
    updateConstraints();
    updateProblemType();
}


