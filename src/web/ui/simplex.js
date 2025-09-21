// Import WASM module (built with emcc)
import Module from "../wasm/linprog-toolset.js";

export function render(formContainer, resultsContainer) {
  formContainer.innerHTML = `
    <label>Matrix (comma-separated rows):</label>
    <textarea id="matrixInput">1,2,3\n4,5,6</textarea>
    <button id="runBtn">Run Simplex</button>
  `;

  document.getElementById("runBtn").onclick = () => run(resultsContainer);
}

function run(resultsContainer) {
  const matrixStr = document.getElementById("matrixInput").value;
  const matrix = parseMatrix(matrixStr);

  // Call into WASM
  const result = Module.ccall(
    "run_simplex",   // function name in C++
    "number",        // return type
    ["array", "number", "number"], // arg types
    [matrix, 2, 3]   // arg values
  );

  resultsContainer.innerHTML = `<p>Optimal value: ${result}</p>`;
}

function parseMatrix(str) {
  return str
    .split("\n")
    .map(row => row.split(",").map(Number))
    .flat();
}
