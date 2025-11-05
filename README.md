# Linear Programming Toolset

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![WASM Powered](https://img.shields.io/badge/Powered%20by-WebAssembly-blue)](https://webassembly.org/)
[![Live Demo](https://img.shields.io/badge/Live%20Demo-linprog-brightgreen)](https://brettwilsondev.github.io/linprog-toolset)

A **free, interactive, browser-based toolset** for solving **linear programming**, **integer programming**, and **combinatorial optimization** problems. Built with **WebAssembly (WASM)** for high performance.

---

## Features

- **Dual Simplex Algorithm** – Efficient for sensitivity and modified constraints
- **Two-Phase Simplex Solver** – Handles artificial variables automatically
- **Graphical LP Solver** – Visualize 2-variable problems interactively
- **Goal Programming** – Penalties & Preemptive methods
- **Branch and Bound** – For integer and mixed-integer problems
- **Cutting Plane Optimization** – Gomory cuts for tighter bounds
- **Knapsack Solver** – 0/1 and bounded variants
- **Traveling Salesman Problem (TSP)**
  - Cheapest Insertion
  - Nearest Neighbor
- **Hungarian Algorithm** – Optimal assignment problems
- **Data Envelopment Analysis (DEA)** – Efficiency measurement
- **Machine Scheduling** – Job sequencing optimization
- **Golden Ratio Search** – Unimodal function optimization
- **Steepest Descent** – Gradient-based optimization
- **Sensitivity Analysis** – Shadow prices & allowable ranges
- **Duality Explorer** – Primal ↔ Dual relationships
- **Dynamic Model Builder** – Add activities & constraints on-the-fly
- **Math Preliminaries for Optimization** Learn foundational optimization basics for understanding linear programming concepts.
---

## Live Demo

[linprog](https://brettwilsondev.github.io/linprog-toolset)

---

## Tech Stack

- **Frontend**: HTML5, CSS3, Vanilla JS (ES6 Modules)
- **Backend Logic**: **C++ → WebAssembly** (via `Emscripten`)
- **UI Framework**: Custom lightweight, responsive design
- **No dependencies** – Pure client-side execution

---
