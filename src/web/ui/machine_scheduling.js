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

    // Insert HTML
    formContainer.innerHTML = `
    <style>
        .node rect {
            stroke: #333;
            stroke-width: 1.5px;
        }

        .node text {
            font: 12px sans-serif;
        }

        .link {
            fill: none;
            stroke: #ccc;
            stroke-width: 1.5px;
        }

        svg {
            // border: 1px solid #ccc;
        }
    </style>

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
            flex-wrap: wrap;
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
            flex-wrap: wrap;
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

        // #tree {border: 1px solid red; width: 100%;}
        svg { border: 1px solid red; width: 100%; }
    </style>

    <h1 style="margin-top: 60px;" class="row">Machine Scheduling</h1>

    <div class="row">
      <span>Number of Jobs: <span id="numJobsDisplay">1</span></span>
      <button id="addJob">Add Job</button>
      <button id="removeJob">Remove Job</button>
      <button id="togglePenalties">Enable Penalties</button>
    </div>

    <h3 class="row">Job Data (ID | Processing Time | Due Date | Penalty)</h3>
    <div class="row">
      <div id="jobsContainer"></div>
    </div>

    <br>

    <div class="row">
      <button id="solveButton">Solve</button>
      <button style="background-color: red;" id="resetButton" style="margin-left: 25px; background-color: red">Reset</button>
    </div>

    <h1 class="row" class="text-2xl font-bold p-4">Branch and Bound Tree</h1>
    <div id="tree"></div>
  `;

    // ===== STATE =====
    let numJobs = 1;
    let processingTimes = [0];
    let dueDates = [0];
    let penalties = [0];
    let penaltiesEnabled = false;

    function updateJobs() {
        const container = document.getElementById("jobsContainer");
        container.innerHTML = "";

        // Create table for jobs
        const table = document.createElement("table");
        table.style.borderCollapse = "collapse";

        // Header row
        const headerRow = document.createElement("tr");
        const headers = ["Job ID", "Processing Time", "Due Date"];
        if (penaltiesEnabled) headers.push("Penalty");
        for (let header of headers) {
            const th = document.createElement("th");
            th.innerText = header;
            th.style.border = "1px solid #ccc";
            th.style.padding = "5px";
            headerRow.appendChild(th);
        }
        table.appendChild(headerRow);

        // Data rows
        for (let i = 0; i < numJobs; i++) {
            const row = document.createElement("tr");

            const idCell = document.createElement("td");
            idCell.innerText = `${i + 1}`;
            idCell.style.border = "1px solid #ccc";
            idCell.style.padding = "5px";
            row.appendChild(idCell);

            const pCell = document.createElement("td");
            pCell.style.border = "1px solid #ccc";
            pCell.style.padding = "2px";
            const pInput = document.createElement("input");
            pInput.type = "number";
            pInput.step = "1";
            pInput.value = processingTimes[i];
            pInput.style.width = "80px";
            pInput.oninput = (e) => {
                processingTimes[i] = parseInt(e.target.value) || 0;
            };
            pCell.appendChild(pInput);
            row.appendChild(pCell);

            const dCell = document.createElement("td");
            dCell.style.border = "1px solid #ccc";
            dCell.style.padding = "2px";
            const dInput = document.createElement("input");
            dInput.type = "number";
            dInput.step = "1";
            dInput.value = dueDates[i];
            dInput.style.width = "80px";
            dInput.oninput = (e) => {
                dueDates[i] = parseInt(e.target.value) || 0;
            };
            dCell.appendChild(dInput);
            row.appendChild(dCell);

            if (penaltiesEnabled) {
                const penCell = document.createElement("td");
                penCell.style.border = "1px solid #ccc";
                penCell.style.padding = "2px";
                const penInput = document.createElement("input");
                penInput.type = "number";
                penInput.step = "1";
                penInput.value = penalties[i];
                penInput.style.width = "80px";
                penInput.oninput = (e) => {
                    penalties[i] = parseInt(e.target.value) || 0;
                };
                penCell.appendChild(penInput);
                row.appendChild(penCell);
            }

            table.appendChild(row);
        }
        container.appendChild(table);
    }

    document.getElementById("addJob").onclick = () => {
        numJobs++;
        processingTimes.push(0);
        dueDates.push(0);
        penalties.push(0);
        document.getElementById("numJobsDisplay").innerText = `${numJobs}`;
        updateJobs();
    };

    document.getElementById("removeJob").onclick = () => {
        if (numJobs > 1) {
            numJobs--;
            processingTimes.pop();
            dueDates.pop();
            penalties.pop();
            document.getElementById("numJobsDisplay").innerText = `${numJobs}`;
            updateJobs();
        }
    };

    document.getElementById("togglePenalties").onclick = () => {
        penaltiesEnabled = !penaltiesEnabled;
        document.getElementById("togglePenalties").innerText = penaltiesEnabled ? "Disable Penalties" : "Enable Penalties";
        updateJobs();
    };

    document.getElementById("resetButton").onclick = () => {
        numJobs = 1;
        processingTimes = [0, 0, 0, 0];
        dueDates = [0, 0, 0, 0];
        penalties = [0, 0, 0, 0];
        penaltiesEnabled = false;
        document.getElementById("togglePenalties").innerText = "Enable Penalties";
        resultsContainer.innerHTML = "";
        document.getElementById("numJobsDisplay").innerText = `${numJobs}`;

        let tree = document.getElementById("tree");
        tree.innerHTML = "";

        updateJobs();
    };


    function MachineSchedulingTardinessSchedulerD3(data) {
        // const data = JSON.parse(result.json)

        const width = 1400;
        const height = 1000;
        const margin = { top: 40, right: 120, bottom: 40, left: 120 };

        let treeDiv = document.getElementById("tree");
        treeDiv.innerHTML = "";

        const svg = d3.select("#tree")
            .append("svg")
            .attr("width", width)
            .attr("height", height);

        const g = svg.append("g")
            .attr("transform", `translate(${margin.left},${margin.top})`);

        // Add zoom and pan behavior
        const zoom = d3.zoom()
            .scaleExtent([0.5, 3])
            .on("zoom", function (event) {
                g.attr("transform", event.transform.translate(margin.left, margin.top));
            });
        svg.call(zoom);

        // Function to get node text content in the desired format
        function getNodeText(d) {
            let text = `Problem ${d.data.name}\n`;
            if (d.data.variables) text += `${d.data.variables}\n`;
            if (d.data.time_required) text += `Time required = ${d.data.time_required}\n`;
            if (d.data.overdue) text += `Overdue = ${d.data.overdue}\n`;
            if (d.data.total_overdue) text += `Total overdue = ${d.data.total_overdue}\n`;
            if (d.data.status === "branching" && d.data.branch_on) {
                text += `Branching on ${d.data.branch_on}`;
            } else if (d.data.status === "eliminated" && d.data.eliminated_by) {
                text += `Eliminated by ${d.data.eliminated_by}`;
            }
            // Add root-specific fields
            if (d.data.initial_tardiness || d.data.initial_sequence || d.data.best_tardiness || d.data.best_sequence) {
                text = `Problem ${d.data.name}\n`;
                if (d.data.initial_tardiness) text += `Initial tardiness = ${d.data.initial_tardiness}\n`;
                if (d.data.initial_sequence) text += `Initial sequence = ${d.data.initial_sequence}\n`;
                if (d.data.best_tardiness) text += `Best tardiness = ${d.data.best_tardiness}\n`;
                if (d.data.best_sequence) text += `Best sequence = ${d.data.best_sequence}`;
            }
            return text.trim();
        }

        // Calculate maximum box size based on text content
        const tempSvg = d3.select("body").append("svg");
        const root = d3.hierarchy(data);
        const allNodes = root.descendants();
        let maxWidth = 0;
        let maxHeight = 0;
        const fontSize = 12;
        const padding = 10;
        const topGap = 5; // Additional gap at the top of the box

        allNodes.forEach(d => {
            const text = tempSvg.append("text").style("font", `${fontSize}px sans-serif`);
            const lines = getNodeText(d).split('\n');
            lines.forEach((line, i) => {
                text.append("tspan")
                    .attr("x", 0)
                    .attr("dy", i === 0 ? 0 : fontSize * 1.2)
                    .text(line);
            });
            const bbox = text.node().getBBox();
            maxWidth = Math.max(maxWidth, bbox.width);
            maxHeight = Math.max(maxHeight, bbox.height);
            text.remove();
        });
        tempSvg.remove();

        const boxWidth = maxWidth + padding * 2;
        const boxHeight = maxHeight + padding * 2 + topGap;

        // Adjust tree size for top-down layout with increased vertical gap
        const tree = d3.tree().nodeSize([boxWidth + 50, boxHeight + 60]);
        tree(root);

        // Links (straight lines from bottom center to top center)
        g.selectAll(".link")
            .data(root.links())
            .enter()
            .append("path")
            .attr("class", "link")
            .attr("d", d => {
                // Source: bottom center of parent box
                const sourceX = d.source.x;
                const sourceY = d.source.y + boxHeight / 2;
                // Target: top center of child box
                const targetX = d.target.x;
                const targetY = d.target.y - boxHeight / 2;
                return `M${sourceX},${sourceY} L${targetX},${targetY}`;
            });

        // Nodes
        const node = g.selectAll(".node")
            .data(root.descendants())
            .enter()
            .append("g")
            .attr("class", "node")
            .attr("transform", d => `translate(${d.x},${d.y})`);

        // Draw rectangles
        node.append("rect")
            .attr("x", -boxWidth / 2)
            .attr("y", -boxHeight / 2)
            .attr("width", boxWidth)
            .attr("height", boxHeight)
            .style("fill", d => d.data.status === "eliminated" ? "#ff9999" : "#99ff99");

        // Add text inside boxes with additional top gap
        node.each(function (d) {
            const text = d3.select(this).append("text")
                .style("text-anchor", "middle")
                .attr("y", -boxHeight / 2 + padding + topGap);
            const lines = getNodeText(d).split('\n');
            lines.forEach((line, i) => {
                text.append("tspan")
                    .attr("x", 0)
                    .attr("dy", i === 0 ? 0 : fontSize * 1.2)
                    .text(line);
            });
        });

        // Initial zoom and pan to center on root node
        const rootNode = root;
        const initialScale = 1;
        const initialTranslateX = (width / 2) - (rootNode.x * initialScale) - margin.left;
        const initialTranslateY = (height / 2) - (rootNode.y * initialScale) - margin.top;

        svg.call(zoom.transform, d3.zoomIdentity
            .translate(initialTranslateX, initialTranslateY)
            .scale(initialScale));
    }

    function MachineSchedulingPenaltySchedulerD3(data) {
        // const data = JSON.parse(result.json)

        const width = 1400;
        const height = 1000;
        const margin = { top: 40, right: 120, bottom: 40, left: 120 };

        let treeDiv = document.getElementById("tree");
        treeDiv.innerHTML = "";

        const svg = d3.select("#tree")
            .append("svg")
            .attr("width", width)
            .attr("height", height);

        const g = svg.append("g")
            .attr("transform", `translate(${margin.left},${margin.top})`);

        // Add zoom and pan behavior
        const zoom = d3.zoom()
            .scaleExtent([0.5, 3])
            .on("zoom", function (event) {
                g.attr("transform", event.transform.translate(margin.left, margin.top));
            });
        svg.call(zoom);

        // Function to get node text content in the desired format
        function getNodeText(d) {
            let text = `Problem ${d.data.name}\n`;
            if (d.data.variables) text += `${d.data.variables}\n`;
            if (d.data.time_required) text += `Time required = ${d.data.time_required}\n`;
            if (d.data.overdue) text += `Overdue = ${d.data.overdue}\n`;
            if (d.data.penalty) text += `Penalty = ${d.data.penalty}\n`;
            if (d.data.total_overdue) text += `Total overdue = ${d.data.total_overdue}\n`;
            if (d.data.total_penalty) text += `Total penalty = ${d.data.total_penalty}\n`;
            if (d.data.status === "branching" && d.data.branch_on) {
                text += `Branching on ${d.data.branch_on}`;
            } else if (d.data.status === "eliminated" && d.data.eliminated_by) {
                text += `Eliminated by ${d.data.eliminated_by}`;
            }
            // Add root-specific fields
            if (d.data.initial_tardiness || d.data.initial_penalty || d.data.initial_sequence || d.data.best_tardiness || d.data.best_penalty || d.data.best_sequence) {
                text = `Problem ${d.data.name}\n`;
                if (d.data.initial_tardiness) text += `Initial tardiness = ${d.data.initial_tardiness}\n`;
                if (d.data.initial_penalty) text += `Initial penalty = ${d.data.initial_penalty}\n`;
                if (d.data.initial_sequence) text += `Initial sequence = ${d.data.initial_sequence}\n`;
                if (d.data.best_tardiness) text += `Best tardiness = ${d.data.best_tardiness}\n`;
                if (d.data.best_penalty) text += `Best penalty = ${d.data.best_penalty}\n`;
                if (d.data.best_sequence) text += `Best sequence = ${d.data.best_sequence}`;
            }
            return text.trim();
        }

        // Calculate maximum box size based on text content
        const tempSvg = d3.select("body").append("svg");
        const root = d3.hierarchy(data);
        const allNodes = root.descendants();
        let maxWidth = 0;
        let maxHeight = 0;
        const fontSize = 12;
        const padding = 10;
        const topGap = 5; // Additional gap at the top of the box

        allNodes.forEach(d => {
            const text = tempSvg.append("text").style("font", `${fontSize}px sans-serif`);
            const lines = getNodeText(d).split('\n');
            lines.forEach((line, i) => {
                text.append("tspan")
                    .attr("x", 0)
                    .attr("dy", i === 0 ? 0 : fontSize * 1.2)
                    .text(line);
            });
            const bbox = text.node().getBBox();
            maxWidth = Math.max(maxWidth, bbox.width);
            maxHeight = Math.max(maxHeight, bbox.height);
            text.remove();
        });
        tempSvg.remove();

        const boxWidth = maxWidth + padding * 2;
        const boxHeight = maxHeight + padding * 2 + topGap;

        // Adjust tree size for top-down layout with increased vertical gap
        const tree = d3.tree().nodeSize([boxWidth + 50, boxHeight + 60]);
        tree(root);

        // Links (straight lines from bottom center to top center)
        g.selectAll(".link")
            .data(root.links())
            .enter()
            .append("path")
            .attr("class", "link")
            .attr("d", d => {
                // Source: bottom center of parent box
                const sourceX = d.source.x;
                const sourceY = d.source.y + boxHeight / 2;
                // Target: top center of child box
                const targetX = d.target.x;
                const targetY = d.target.y - boxHeight / 2;
                return `M${sourceX},${sourceY} L${targetX},${targetY}`;
            });

        // Nodes
        const node = g.selectAll(".node")
            .data(root.descendants())
            .enter()
            .append("g")
            .attr("class", "node")
            .attr("transform", d => `translate(${d.x},${d.y})`);

        // Draw rectangles
        node.append("rect")
            .attr("x", -boxWidth / 2)
            .attr("y", -boxHeight / 2)
            .attr("width", boxWidth)
            .attr("height", boxHeight)
            .style("fill", d => d.data.status === "eliminated" ? "#ff9999" : "#99ff99");

        // Add text inside boxes with additional top gap
        node.each(function (d) {
            const text = d3.select(this).append("text")
                .style("text-anchor", "middle")
                .attr("y", -boxHeight / 2 + padding + topGap);
            const lines = getNodeText(d).split('\n');
            lines.forEach((line, i) => {
                text.append("tspan")
                    .attr("x", 0)
                    .attr("dy", i === 0 ? 0 : fontSize * 1.2)
                    .text(line);
            });
        });

        // Initial zoom and pan to center on root node
        const rootNode = root;
        const initialScale = 1;
        const initialTranslateX = (width / 2) - (rootNode.x * initialScale) - margin.left;
        const initialTranslateY = (height / 2) - (rootNode.y * initialScale) - margin.top;

        svg.call(zoom.transform, d3.zoomIdentity
            .translate(initialTranslateX, initialTranslateY)
            .scale(initialScale));
    }

    document.getElementById("solveButton").onclick = () => {
        try {
            // let numJobs = 4;
            // let processingTimes = [6, 4, 5, 8];
            // let dueDates = [8, 4, 12, 16];
            // let penalties = [5, 6, 5, 6];

            let jobData = [];
            for (let i = 0; i < numJobs; i++) {
                jobData.push([i + 1, processingTimes[i], dueDates[i]]);
            }

            if (penaltiesEnabled) {
                const result = Module.runMachineSchedulingPenaltyScheduler(jobData, penalties);
                const data = JSON.parse(result.json);
                console.log(data);
                
                MachineSchedulingPenaltySchedulerD3(data);
            }
            else {
                const result = Module.runMachineSchedulingTardinessScheduler(jobData);
                const data = JSON.parse(result.json);

                console.log(data);
                MachineSchedulingTardinessSchedulerD3(data);
            }
        } catch (err) {
            resultsContainer.innerHTML = `<p style="color:red">Error: ${err.message || err}</p>`;
        }
    };

    // ===== INITIAL RENDER =====
    updateJobs();
}