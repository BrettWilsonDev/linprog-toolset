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
        body {
            font-family: Arial, sans-serif;
            margin: 0;
        }

        svg {
            background: #fafafa;
            cursor: grab;
        }

        .node rect {
            fill: #fefefe;
            stroke: #333;
            stroke-width: 1.2px;
            rx: 6;
        }

        .link {
            fill: none;
            stroke: #aaa;
            stroke-width: 1.2px;
        }

        .tooltip {
            position: absolute;
            background: #fff;
            border: 1px solid #999;
            padding: 6px;
            font-size: 12px;
            font-family: monospace;
            pointer-events: none;
            white-space: pre-wrap;
            max-width: 400px;
            box-shadow: 2px 2px 6px rgba(0, 0, 0, 0.2);
            display: none;
        }
    </style>

    <h1>Branch and Bound Knapsack</h1>

    <p id="problemTypeText">Problem is: Max</p>

    <div class="row">
      <button id="addDecisionVar">decision variables +</button>
      <button id="removeDecisionVar">decision variables -</button>
    </div>

    <div id="objectiveFunction" class="row"></div>

    <br>

    <div id="constraintsContainer"></div>

    <div class="row">
      <button id="solveButton">Solve</button>
      <button id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
    </div>

    <h2 style="padding:10px;">Branch and Bound Knapsack Tree</h2>
    <svg id="svgID" width="1600" height="1000"></svg>
    <div class="tooltip"></div>
  `;


    // ===== STATE =====
    let amtOfObjVars = 2;
    let amtOfConstraints = 1;
    let objFunc = [0.0, 0.0];
    let constraints = [[0.0, 0.0, 0.0, 0.0]];
    let signItems = ["<="];
    let signItemsChoices = [0];

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

    document.getElementById("resetButton").onclick = () => {
        amtOfObjVars = 2;
        amtOfConstraints = 1;
        objFunc = [0.0, 0.0];
        constraints = [[0.0, 0.0, 0.0, 0.0]];
        signItems = ["<="];
        signItemsChoices = [0];
        resultsContainer.innerHTML = "";

        const svg = document.getElementById("svgID");
        svg.innerHTML = "";

        updateObjectiveFunction();
        updateConstraints();
    };

    function fmt(num, decimals = 6) {
        return parseFloat(num.toFixed(decimals));
    }

    document.getElementById("solveButton").onclick = () => {
        try {
            objFunc = [300, 840, 160, 520];

            constraints = [
                [7, 15, 3, 13, 23, 0],
            ]

            const result = Module.runKnapsack(objFunc, constraints);

            const svgClear = document.getElementById("svgID");
            svgClear.innerHTML = "";

            const svg = d3.select("svg"),
                width = +svg.attr("width"),
                height = +svg.attr("height");

            const g = svg.append("g");

            // Enable zoom & drag
            const zoom = d3.zoom()
                .scaleExtent([0.5, 2])
                .on("zoom", (event) => {
                    g.attr("transform", event.transform);
                });

            svg.call(zoom);

            const tooltip = d3.select(".tooltip");

            const boxWidth = 200;
            const boxHeight = 80; // base min height
            const lineHeight = 12;

            let jsonOut = JSON.parse(result.json)

            renderTree(jsonOut);

            function renderTree(data) {
                g.selectAll("*").remove();

                const root = d3.hierarchy(data, d => d.children);

                // Compute the maximum number of lines across all nodes
                const maxLines = d3.max(root.descendants(), d => {
                    const content = d.data.consoleOutput || "";
                    const lines = content.replace(/<br\s*\/?>/gi, "\n").split("\n");
                    return lines.length;
                });
                const maxBoxHeight = Math.max(boxHeight, maxLines * lineHeight + 10); // add padding

                // Tree layout
                const treeLayout = d3.tree()
                    .size([width - 200, height - 200])
                    .separation(() => 1);

                treeLayout(root);

                // Links (custom path from bottom of parent to top of child)
                g.selectAll(".link")
                    .data(root.links())
                    .enter().append("path")
                    .attr("class", "link")
                    .attr("d", d => {
                        const sourceX = d.source.x;
                        const sourceY = d.source.y + maxBoxHeight; // Bottom of parent node
                        const targetX = d.target.x;
                        const targetY = d.target.y; // Top of child node
                        return `M${sourceX},${sourceY} L${targetX},${targetY}`;
                    });

                // Nodes
                const node = g.selectAll(".node")
                    .data(root.descendants())
                    .enter().append("g")
                    .attr("class", "node")
                    .attr("transform", d => `translate(${d.x - boxWidth / 2},${d.y})`);

                // Rectangles
                node.append("rect")
                    .attr("width", boxWidth)
                    .attr("height", maxBoxHeight);

                // ForeignObject for formatted console output
                node.each(function (d) {
                    const content = d.data.consoleOutput || "";

                    d3.select(this).selectAll("foreignObject").remove();

                    d3.select(this).append("foreignObject")
                        .attr("x", 0)
                        .attr("y", 0)
                        .attr("width", boxWidth)
                        .attr("height", maxBoxHeight)
                        .append("xhtml:div")
                        .html(`<pre style="margin:4px; font-family:monospace; font-size:11px; white-space:pre-wrap; overflow:auto; height:${maxBoxHeight}px;">${content}</pre>`);
                });

                // Tooltip
                node.on("mouseover", (event, d) => {
                    tooltip.style("display", "block")
                        .style("left", (event.pageX + 10) + "px")
                        .style("top", (event.pageY + 10) + "px")
                        .text(d.data.consoleOutput || "");
                })
                    .on("mousemove", (event) => {
                        tooltip.style("left", (event.pageX + 10) + "px")
                            .style("top", (event.pageY + 10) + "px");
                    })
                    .on("mouseout", () => tooltip.style("display", "none"));
            }

            resultsContainer.innerHTML = "";
            const preElement = document.createElement("pre");
            preElement.textContent = result.ranking + " \n\n" + result.solution;
            resultsContainer.appendChild(preElement);

        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err}</p>`;
        }
    };


    // ===== INITIAL RENDER =====
    updateObjectiveFunction();
    updateConstraints();
}


