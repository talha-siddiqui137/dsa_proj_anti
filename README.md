# ShopDSA — DSA E-Commerce Server

> A fully functional single-page e-commerce application that demonstrates every major **Data Structure and Algorithm** taught in a DSA course. Built with C++17, WinSock2, and a vanilla HTML/CSS/JS front-end.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Algorithm & DS Mapping](#algorithm--ds-mapping)
3. [Project Structure](#project-structure)
4. [Build Instructions](#build-instructions)
5. [Running the Server](#running-the-server)
6. [API Documentation](#api-documentation)
7. [Team](#team)

---

## Project Overview

ShopDSA is an e-commerce platform built from scratch without any external libraries. Every feature maps to a concrete data structure or algorithm:

| Feature | Data Structure / Algorithm |
|---|---|
| Product list display | Dynamic Array (`ArrayDS<T>`) |
| Shopping cart | Stack (`StackDS<T>`) |
| Order queue | Priority Queue (max-heap) / Circular Queue |
| Product search | Linear Search |
| Product sorting | Bubble / Selection / Insertion Sort |
| BST Visualisation | Binary Search Tree |
| Network / Delivery | Graph — DFS, BFS, Warshall |
| Infix→Postfix | Shunting-Yard algorithm (Stack) |
| Postfix evaluation | Stack-based evaluator |
| Divide & Conquer demo | Tower of Hanoi (recursion) |

---

## Algorithm & DS Mapping

| Module | File | DS / Algorithm | Complexity |
|---|---|---|---|
| Array DS | `src/ds/array_ds.h` | Dynamic array | O(1) amortised push |
| Stack DS | `src/ds/stack_ds.h` | LIFO stack | O(1) push/pop |
| Queue DS | `src/ds/queue_ds.h` | SimpleQueue, CircularQueue, PriorityQueue | O(1) / O(log n) |
| Tree DS | `src/ds/tree_ds.h` | Binary Search Tree | O(log n) avg |
| Graph DS | `src/ds/graph_ds.h` | Adjacency matrix + list | O(V+E) |
| Hanoi | `src/algorithms/hanoi.h` | Recursive divide-and-conquer | O(2^n) |
| Product Catalog | `src/ecommerce/catalog.h` | Linear search, std::sort | O(n) / O(n log n) |
| Cart | `src/ecommerce/cart.h` | Stack | O(1) |
| Orders | `src/ecommerce/orders.h` | Priority queue | O(log n) |
| Calculator | `src/main.cpp` | Shunting-Yard + Stack evaluator | O(n) |

---

## Project Structure

```
dsa_proj_anti/
├── CMakeLists.txt          ← Build configuration
├── README.md
├── include/
│   └── httplib.h           ← Minimal single-threaded HTTP server (WinSock2)
├── src/
│   ├── main.cpp            ← Server entry point + all API routes
│   ├── ds/
│   │   ├── array_ds.h      ← ArrayDS<T>
│   │   ├── stack_ds.h      ← StackDS<T>
│   │   ├── queue_ds.h      ← SimpleQueue / CircularQueue / PriorityQueue
│   │   ├── tree_ds.h       ← BST
│   │   └── graph_ds.h      ← Graph (adjacency matrix + DFS/BFS/Warshall)
│   ├── algorithms/
│   │   └── hanoi.h         ← Tower of Hanoi solver
│   └── ecommerce/
│       ├── product.h       ← Product struct
│       ├── catalog.h       ← ProductCatalog
│       ├── cart.h          ← ShoppingCart (Stack-backed)
│       └── orders.h        ← OrderManager (PriorityQueue-backed)
├── web/
│   ├── index.html          ← Single-page application
│   ├── css/
│   │   └── style.css
│   └── js/
│       └── app.js
└── bin/                    ← Generated executables + web/ copy
```

---

## Build Instructions

### Prerequisites

| Tool | Version | Notes |
|---|---|---|
| CMake | ≥ 3.16 | [cmake.org](https://cmake.org/download/) |
| MSVC | Visual Studio 2019+ | Windows SDK required |
| **or** MinGW-w64 | GCC ≥ 9 | [winlibs.com](https://winlibs.com) |

### Option A — Visual Studio (MSVC)

```powershell
# From the project root
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

The executable is placed at `bin\ShopDSA.exe`.

### Option B — MinGW (GCC)

```powershell
# From the project root
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Option C — VS Code (CMake Tools extension)

1. Open the folder in VS Code.
2. Install the **CMake Tools** extension.
3. Press `Ctrl+Shift+P` → **CMake: Configure** → select your kit.
4. Press `F7` to build.

---

## Running the Server

```powershell
# From project root
.\bin\ShopDSA.exe
```

The server will:
1. Load the 20-product catalog into memory.
2. Seed the BST with 7 nodes.
3. Build a 6-vertex delivery graph.
4. Start listening on **http://localhost:8080**.
5. Open the browser automatically.

Press `Ctrl+C` to stop.

---

## API Documentation

All endpoints return `application/json`. CORS is open (`*`).

### Products

| Method | Endpoint | Description |
|---|---|---|
| GET | `/api/products` | All 20 products |
| GET | `/api/products/search?q=laptop` | Linear search by name |
| GET | `/api/products/sort?by=price&order=asc` | Sorted products (`by`: price/name) |
| GET | `/api/products/categories` | Unique category list |
| GET | `/api/products/category?cat=Electronics` | Filter by category |

### Cart

| Method | Endpoint | Body | Description |
|---|---|---|---|
| GET | `/api/cart` | — | Current cart (Stack) |
| POST | `/api/cart/add` | `{"product_id":3}` | Push item onto stack |
| POST | `/api/cart/remove` | — | Pop top item from stack |
| GET | `/api/cart/total` | — | Running total |

### Orders

| Method | Endpoint | Body | Description |
|---|---|---|---|
| POST | `/api/orders/place` | `{"customer":"Ali","priority":2,"flash_sale":true}` | Enqueue order |
| GET | `/api/orders/next` | — | Dequeue highest-priority order |
| GET | `/api/orders/all` | — | Peek all pending orders |

### BST

| Method | Endpoint | Body | Description |
|---|---|---|---|
| GET | `/api/bst/tree` | — | Full BST as nested JSON |
| POST | `/api/bst/insert` | `{"key":42,"value":"Item"}` | Insert node |
| POST | `/api/bst/delete` | `{"key":42}` | Delete node |
| GET | `/api/bst/traversal?type=inorder` | — | inorder / preorder / postorder |

### Graph

| Method | Endpoint | Description |
|---|---|---|
| GET | `/api/graph` | Adjacency matrix + list |
| POST | `/api/graph/dfs?start=0` | DFS traversal |
| POST | `/api/graph/bfs?start=0` | BFS traversal |
| GET | `/api/graph/warshall` | Transitive closure matrix |

### Algorithms

| Method | Endpoint | Description |
|---|---|---|
| GET | `/api/hanoi?n=4` | Tower of Hanoi moves (n ≤ 20) |
| GET | `/api/sort/demo` | Bubble / Selection / Insertion on same array |
| GET | `/api/complexity` | Big-O reference table |

### Calculator

| Method | Endpoint | Body | Description |
|---|---|---|---|
| POST | `/api/calculator/convert` | `{"infix":"a+b*c"}` | Infix → Postfix (Shunting-Yard) |
| POST | `/api/calculator/evaluate` | `{"postfix":"2 3 4 * +"}` | Evaluate postfix expression |

---

## Team

| Name | Role |
|---|---|
| *(Team Member 1)* | Backend — Data Structures |
| *(Team Member 2)* | Backend — Algorithms |
| *(Team Member 3)* | Frontend — SPA / API integration |
| *(Team Member 4)* | Documentation & Testing |

---

*Built for the NED University DSA course project — Fall 2024.*
