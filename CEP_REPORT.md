# Complex Engineering Problem (CEP) Report
## Course: SE-213 Data Structures and Algorithms
### Department of Software Engineering, NED University of Engineering & Technology

---

## Executive Summary
This report documents the design, implementation, analysis, and demonstration of a **Smart Information Management System (SIMS)** contextualized within modern **E-Commerce Shopping & Distribution Platforms (ShopDSA)**. All required data structures and algorithms — Linear Arrays, Binary Search Trees, Stacks, Queues (Simple, Circular, Priority), Graph Traversals (DFS, BFS), Transitive Closure (Warshall's Algorithm), and Recursive Problems (Tower of Hanoi) — are implemented in C++ and visualized via a responsive Single Page Application (SPA).

---

## 1. Problem Understanding Report

### 1.1 Chosen Context: High-Throughput E-Commerce Platform
Modern e-commerce platforms (such as Amazon, Daraz, and Shopify) handle millions of transactions, catalog searches, customer shopping carts, priority order dispatching, and multi-city delivery logistics. Operating at this scale requires algorithmic efficiency:

1. **Inventory Management & Search:** Catalog lookups must operate instantaneously. Linear scan $O(n)$ suffices for unindexed keyword matching, whereas Binary Search $O(\log n)$ and Binary Search Trees (BST) provide sub-linear retrieval for price-range indexing.
2. **Shopping Cart Management:** Shopping carts naturally follow a **Last-In, First-Out (LIFO)** stack structure. Customers expect quick "undo" operations (popping the last added item) and immediate cart subtotal recalculations.
3. **Discount & Coupon Evaluation:** Complex promotional discount expressions (e.g., `(subtotal - discount) * tax + shipping`) require parsing mathematical formulas without ambiguity using **Infix-to-Postfix (Shunting-Yard)** stack algorithms.
4. **Order Processing Queues:** Standard customer orders follow a **First-In, First-Out (FIFO)** simple queue. Flash sale reservations operate in a memory-bounded **Circular Ring Buffer Queue** to avoid memory reallocation overhead. VIP fast-lane orders require a **Priority Queue (Min-Heap)** where high-tier orders preempt lower-urgency tasks.
5. **Distribution Logistics & Routing:** Delivery between city hubs forms a weighted graph. **BFS** determines minimal transit stops, **DFS** checks network connectivity, **Dijkstra** discovers shortest shipping mileage, and **Warshall’s Algorithm** precomputes the complete transitive reachability matrix.

---

## 2. Requirement Identification & DSA Mapping

| DSA Requirement | Implementation in C++ | E-Commerce Application | Interactive Simulation UI |
|---|---|---|---|
| **Array Operations** | `ArrayDS<T>` (dynamic array) | Product catalog inventory | Dynamic product grid, stock updates |
| **Linear Search** | `ArrayDS::linearSearch` | Keyword search in name & description | Real-time search bar |
| **Binary Search** | `ArrayDS::binarySearch` | Exact price match on sorted inventory | Price index lookup |
| **Bubble Sort** | `ArrayDS::bubbleSort` | Sorting products by rating | 🏎️ Sorting Race Game (O(n²)) |
| **Quick Sort** | `ArrayDS::quickSort` | Sorting products by price | 🏎️ Sorting Race Game (O(n log n)) |
| **Stack (LIFO)** | `StackDS<T>` | Shopping Cart push, pop, peek | Visual stacked cards with TOP pointer |
| **Infix → Postfix** | `infixToPostfix` | Promotional coupon formula parser | 🧮 Interactive Shunting-Yard Stack |
| **Postfix Evaluation** | `evaluatePostfix` | Dynamic formula calculator | Live stack evaluation steps |
| **Tower of Hanoi** | `solveTowerOfHanoi` | Recursive algorithmic benchmark | 🗼 Interactive 3-Peg Animated Game |
| **Simple Queue** | `SimpleQueue<T>` | Standard FIFO order processing queue | Enqueue ➔ FIFO track ➔ Dequeue |
| **Circular Queue** | `CircularQueue<T>` | Flash-sale slot management (cap: 5–8) | 8-slot circular ring buffer diagram |
| **Priority Queue** | `PriorityQueue<T>` (Min-Heap) | VIP & Express Delivery fast-lane | Priority-ranked order list |
| **Binary Search Tree** | `BST` class | Price-indexed hierarchical catalog | 🌳 Canvas BST with traversals |
| **Graph Traversal** | `Graph::BFS` & `Graph::DFS` | Distribution hubs network traversal | 🌐 Animated canvas hub routing |
| **Transitive Closure** | `Graph::warshall` | Global reachability between hubs | 8×8 Reachability Matrix Grid |

---

## 3. Algorithm Design Document (Step-by-Step Pseudocode & Flowcharts)

### 3.1 Linear Search & Binary Search
#### Linear Search
```text
Algorithm LinearSearch(A, n, target):
1. For i = 0 to n - 1 do:
2.    If A[i] matches target:
3.        Return i
4. Return -1 (Not Found)
```
#### Binary Search (Requires sorted array)
```text
Algorithm BinarySearch(A, low, high, target):
1. While low <= high do:
2.    mid = low + (high - low) / 2
3.    If A[mid] == target:
4.        Return mid
5.    Else if A[mid] < target:
6.        low = mid + 1
7.    Else:
8.        high = mid - 1
9. Return -1
```

### 3.2 Sorting Algorithms: Bubble Sort vs. Quick Sort
#### Bubble Sort
```text
Algorithm BubbleSort(A, n):
1. For i = 0 to n - 2 do:
2.    swapped = false
3.    For j = 0 to n - i - 2 do:
4.        If A[j] > A[j + 1]:
5.            Swap(A[j], A[j + 1])
6.            swapped = true
7.    If not swapped:
8.        Break (Early exit optimization)
```
#### Quick Sort (Median-of-Three Partitioning)
```text
Algorithm QuickSort(A, low, high):
1. If low < high:
2.    pivotIndex = Partition(A, low, high)
3.    QuickSort(A, low, pivotIndex - 1)
4.    QuickSort(A, pivotIndex + 1, high)

Algorithm Partition(A, low, high):
1. pivot = A[high]
2. i = low - 1
3. For j = low to high - 1 do:
4.    If A[j] <= pivot:
5.        i = i + 1
6.        Swap(A[i], A[j])
7. Swap(A[i + 1], A[high])
8. Return i + 1
```

### 3.3 Stack Algorithms: Infix to Postfix & Evaluation
```mermaid
flowchart TD
    Start([Start]) --> Tokenize[Read next token from Infix]
    CheckType{Token Type?}
    Tokenize --> CheckType
    CheckType -- Operand --> AppendOut[Append to Output String]
    CheckType -- Left Paren '(' --> PushStack[Push '(' to Operator Stack]
    CheckType -- Right Paren ')' --> PopUntil[Pop Stack to Output until '(' is found]
    PopUntil --> DiscardLeft[Discard '(']
    CheckType -- Operator --> CheckPrec{Stack top has >= precedence?}
    CheckPrec -- Yes --> PopOp[Pop Stack top to Output]
    PopOp --> CheckPrec
    CheckPrec -- No --> PushOp[Push Operator to Stack]
    AppendOut --> MoreTokens{More tokens?}
    DiscardLeft --> MoreTokens
    PushOp --> MoreTokens
    MoreTokens -- Yes --> Tokenize
    MoreTokens -- No --> EmptyStack[Pop remaining operators to Output]
    EmptyStack --> Finish([Output Postfix Expression])
```

### 3.4 Tower of Hanoi (Recursive Algorithm)
```text
Algorithm SolveHanoi(n, source, destination, auxiliary):
1. If n == 1:
2.    Print "Move disk 1 from " + source + " to " + destination
3.    Return
4. SolveHanoi(n - 1, source, auxiliary, destination)
5. Print "Move disk " + n + " from " + source + " to " + destination
6. SolveHanoi(n - 1, auxiliary, destination, source)
```
**Recursive Recurrence Relation:**
$$T(n) = 2T(n-1) + 1$$
Solving by repeated substitution:
$$T(n) = 2^n - 1$$
Number of recursive moves for $n=3$ disks is $2^3 - 1 = 7$.

### 3.5 Queue Algorithms
#### Circular Queue (Ring Buffer)
```text
Algorithm CircularEnqueue(Q, val):
1. If (rear + 1) % CAPACITY == front:
2.    Throw QueueOverflowError
3. If front == -1:
4.    front = 0
5. rear = (rear + 1) % CAPACITY
6. Q[rear] = val

Algorithm CircularDequeue(Q):
1. If front == -1:
2.    Throw QueueUnderflowError
3. item = Q[front]
4. If front == rear:
5.    front = -1, rear = -1
6. Else:
7.    front = (front + 1) % CAPACITY
8. Return item
```

### 3.6 Binary Search Tree (BST)
- **Insert:** Recursively navigate left if `key < root.key`, right if `key > root.key`. Attach node at leaf.
- **Delete:**
  1. *Leaf node:* Simply delete node.
  2. *Single child:* Replace node with its child.
  3. *Two children:* Locate in-order successor (minimum node in right subtree), copy its key, recursively delete successor.
- **Traversals:**
  - In-order: Left $\rightarrow$ Root $\rightarrow$ Right (yields monotonically sorted keys)
  - Pre-order: Root $\rightarrow$ Left $\rightarrow$ Right (useful for serialization)
  - Post-order: Left $\rightarrow$ Right $\rightarrow$ Root (useful for bottom-up memory deallocation)

### 3.7 Warshall’s Algorithm (Transitive Closure)
```text
Algorithm Warshall(Matrix, V):
1. R = Copy(Matrix)
2. For i = 0 to V - 1 do: R[i][i] = 1
3. For k = 0 to V - 1 do:
4.    For i = 0 to V - 1 do:
5.        For j = 0 to V - 1 do:
6.            R[i][j] = R[i][j] OR (R[i][k] AND R[k][j])
7. Return R
```

---

## 4. Complexity Analysis Report

| Algorithm / Data Structure | Best Time | Average Time | Worst Time | Space Complexity | E-Commerce Scenario |
|---|---|---|---|---|---|
| **Linear Search** | $O(1)$ | $O(n)$ | $O(n)$ | $O(1)$ | Keyword search across unindexed product descriptions |
| **Binary Search** | $O(1)$ | $O(\log n)$ | $O(\log n)$ | $O(1)$ | Price lookup on sorted product table |
| **Bubble Sort** | $O(n)$ | $O(n^2)$ | $O(n^2)$ | $O(1)$ | Small array sorting by user rating |
| **Quick Sort** | $O(n \log n)$ | $O(n \log n)$ | $O(n^2)$ | $O(\log n)$ | High-performance catalog price sorting |
| **Stack Push / Pop** | $O(1)$ | $O(1)$ | $O(1)$ | $O(n)$ | Adding/popping items to shopping cart |
| **Infix to Postfix** | $O(n)$ | $O(n)$ | $O(n)$ | $O(n)$ | Parsing promo code formula |
| **Postfix Evaluation** | $O(n)$ | $O(n)$ | $O(n)$ | $O(n)$ | Computing discount total |
| **Tower of Hanoi** | $O(2^n)$ | $O(2^n)$ | $O(2^n)$ | $O(n)$ | Simulation game with optimal moves |
| **Simple Queue Enqueue/Dequeue** | $O(1)$ | $O(1)$ | $O(1)$ | $O(n)$ | Regular FIFO customer order queue |
| **Circular Queue Operations** | $O(1)$ | $O(1)$ | $O(1)$ | $O(k)$ | Fixed-capacity flash sale order buffer |
| **Priority Queue (Min-Heap)** | $O(1)$ peek | $O(\log n)$ | $O(\log n)$ | $O(n)$ | VIP order fast-lane dispatch |
| **BST Insert / Search** | $O(\log n)$ | $O(\log n)$ | $O(n)$ | $O(n)$ | Price tree indexing (balanced vs degenerate) |
| **BST Traversals** | $O(n)$ | $O(n)$ | $O(n)$ | $O(h)$ | Generating sorted product price catalogues |
| **Graph BFS / DFS** | $O(V + E)$ | $O(V + E)$ | $O(V + E)$ | $O(V)$ | Finding valid shipping hubs |
| **Warshall Transitive Closure** | $O(V^3)$ | $O(V^3)$ | $O(V^3)$ | $O(V^2)$ | Complete city-to-city reachability matrix |

---

## 5. Walkthrough & Demonstration

### 5.1 Dataset Context: 8 Major Distribution Hubs
- **Cities ($V=8$):** Karachi (0), Lahore (1), Islamabad (2), Peshawar (3), Quetta (4), Multan (5), Faisalabad (6), Hyderabad (7)
- **Edges ($E=12$):** Direct transit highways connecting South, Central, and North Pakistan.

### 5.2 Test Walkthrough Output
- **BFS Traversal starting at Karachi (0):**
  `Karachi ➔ Hyderabad ➔ Quetta ➔ Multan ➔ Peshawar ➔ Faisalabad ➔ Lahore ➔ Islamabad`
- **Dijkstra Shortest Path:**
  Karachi to Islamabad: `Karachi ➔ Hyderabad (160km) ➔ Multan (680km) ➔ Faisalabad (240km) ➔ Islamabad (320km) = 1400 km`
- **Tower of Hanoi ($n=3$ disks):**
  Moves: 7 optimal transitions $[A \rightarrow C, A \rightarrow B, C \rightarrow B, A \rightarrow C, B \rightarrow A, B \rightarrow C, A \rightarrow C]$.
- **Infix to Postfix:**
  Input: `(3+5)*2-4/2` $\rightarrow$ Postfix: `3 5 + 2 * 4 2 / -` $\rightarrow$ Evaluated Result: `14.00`.
- **Order Queue Processing:**
  Orders dispatched strictly respecting Priority Queue (VIP urgency 1 preempts Standard urgency 5).

---

## 6. How to Run the Demonstration

### Starting the Live C++ Server
```powershell
cd d:\NED\dsa\dsa_proj_anti
.\ShopDSA.exe
```
The server will bind to `http://localhost:8080`, register all REST API routes, serve the static web files, and open the default web browser automatically.

### Running in Standalone Frontend Demo Mode
You can also open `d:\NED\dsa\dsa_proj_anti\web\index.html` directly in any web browser. The application includes a self-contained offline demo mode with mock data and real-time JavaScript implementations for all algorithms.
