/* ==========================================================================
   visualizer.js — Graph Delivery Network Visualizer
   Algorithms: BFS, DFS, Warshall's Transitive Closure, Dijkstra's Shortest Path
   SE-213 Data Structures & Algorithms Project
   ========================================================================== */

const CITIES = [
  { id: 0, name: 'Karachi',    x: 180, y: 410, hub: 'Mega Hub (South)' },
  { id: 1, name: 'Lahore',     x: 590, y: 220, hub: 'Central Hub' },
  { id: 2, name: 'Islamabad',  x: 520, y: 110, hub: 'Capital Hub (North)' },
  { id: 3, name: 'Peshawar',   x: 410, y: 90,  hub: 'Northwest Terminal' },
  { id: 4, name: 'Quetta',     x: 230, y: 240, hub: 'Western Terminal' },
  { id: 5, name: 'Multan',     x: 430, y: 280, hub: 'South-Punjab Hub' },
  { id: 6, name: 'Faisalabad', x: 500, y: 210, hub: 'Industrial Transit' },
  { id: 7, name: 'Hyderabad',  x: 230, y: 380, hub: 'Sindh Transit' }
];

const GRAPH_EDGES = [
  { u: 0, v: 7, weight: 160 },
  { u: 7, v: 4, weight: 720 },
  { u: 7, v: 5, weight: 680 },
  { u: 0, v: 4, weight: 690 },
  { u: 4, v: 5, weight: 450 },
  { u: 4, v: 3, weight: 780 },
  { u: 5, v: 6, weight: 240 },
  { u: 5, v: 1, weight: 340 },
  { u: 6, v: 1, weight: 130 },
  { u: 6, v: 2, weight: 320 },
  { u: 1, v: 2, weight: 370 },
  { u: 2, v: 3, weight: 180 }
];

let selectedAlgo = 'bfs';
let isGraphRunning = false;
let graphAnimId = null;

// Node status: 0 = unvisited, 1 = visiting, 2 = visited, 3 = path
let nodeStates = new Array(CITIES.length).fill(0);
let activeEdge = null;
let pathEdges = [];

function initDeliveryMap() {
  const canvas = document.getElementById('deliveryCanvas');
  if (!canvas) return;

  // Bind controls once
  if (!canvas.dataset.initialized) {
    bindDeliveryControls();
    canvas.dataset.initialized = 'true';
  }

  resetGraphState();
  drawGraph();
}

function bindDeliveryControls() {
  const algoTabs = document.querySelectorAll('.algo-tab');
  const runBtn = document.getElementById('runAlgoBtn');
  const resetBtn = document.getElementById('resetMapBtn');
  const endCityGroup = document.getElementById('endCityGroup');

  algoTabs.forEach(tab => {
    tab.addEventListener('click', () => {
      algoTabs.forEach(t => t.classList.remove('active'));
      tab.classList.add('active');
      selectedAlgo = tab.dataset.algo;
      updateAlgoInfo(selectedAlgo);
      
      if (endCityGroup) {
        endCityGroup.style.display = (selectedAlgo === 'dijkstra' || selectedAlgo === 'bfs') ? 'block' : 'none';
      }
      
      const warshallMat = document.getElementById('warshallMatrix');
      if (warshallMat) {
        warshallMat.classList.toggle('hidden', selectedAlgo !== 'warshall');
      }

      resetGraphState();
      drawGraph();
    });
  });

  if (runBtn) {
    runBtn.addEventListener('click', () => {
      if (isGraphRunning) return;
      runSelectedGraphAlgo();
    });
  }

  if (resetBtn) {
    resetBtn.addEventListener('click', () => {
      resetGraphState();
      drawGraph();
      logGraphStep('Graph reset to initial topology.');
    });
  }
}

function updateAlgoInfo(algo) {
  const infoEl = document.getElementById('algoInfo');
  if (!infoEl) return;

  const infoMap = {
    bfs: {
      title: 'BFS — Breadth First Search (Queue-based)',
      desc: 'Explores delivery hubs layer by layer. Finds the route with the fewest transit stops.',
      time: 'O(V + E)',
      space: 'O(V)'
    },
    dfs: {
      title: 'DFS — Depth First Search (Stack/Recursion)',
      desc: 'Explores along each delivery branch as deep as possible before backtracking. Used for network connectivity.',
      time: 'O(V + E)',
      space: 'O(V)'
    },
    warshall: {
      title: 'Warshall’s Algorithm (Transitive Closure)',
      desc: 'Computes global delivery reachability matrix between all pairs of hubs. R[i][j] = R[i][j] || (R[i][k] && R[k][j]).',
      time: 'O(V³)',
      space: 'O(V²)'
    },
    dijkstra: {
      title: 'Dijkstra’s Algorithm (Priority Queue / Min-Heap)',
      desc: 'Finds the optimal shortest shipping distance (in kilometers) between warehouse origin and destination.',
      time: 'O((V + E) log V)',
      space: 'O(V)'
    }
  };

  const current = infoMap[algo] || infoMap.bfs;
  infoEl.innerHTML = `
    <h4>${current.title}</h4>
    <p>${current.desc}</p>
    <div class="complexity"><span>Time: ${current.time}</span><span>Space: ${current.space}</span></div>
  `;
}

function resetGraphState() {
  if (graphAnimId) {
    clearTimeout(graphAnimId);
    graphAnimId = null;
  }
  isGraphRunning = false;
  nodeStates = new Array(CITIES.length).fill(0);
  activeEdge = null;
  pathEdges = [];
  
  const stepsEl = document.getElementById('traversalSteps');
  if (stepsEl) stepsEl.innerHTML = '';
}

function logGraphStep(text, type = 'info') {
  const stepsEl = document.getElementById('traversalSteps');
  if (!stepsEl) return;
  const div = document.createElement('div');
  div.className = `step-log-item ${type}`;
  div.innerHTML = `<span class="time-badge">${new Date().toLocaleTimeString().slice(3)}</span> ${text}`;
  stepsEl.appendChild(div);
  stepsEl.scrollTop = stepsEl.scrollHeight;
}

function getAdjList() {
  const adj = Array.from({ length: CITIES.length }, () => []);
  GRAPH_EDGES.forEach(e => {
    adj[e.u].push({ to: e.v, weight: e.weight });
    adj[e.v].push({ to: e.u, weight: e.weight });
  });
  return adj;
}

// -------------------------------------------------------------
// Drawing Logic
// -------------------------------------------------------------
function drawGraph() {
  const canvas = document.getElementById('deliveryCanvas');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  const w = canvas.width;
  const h = canvas.height;

  ctx.clearRect(0, 0, w, h);

  // Background map grid pattern
  ctx.strokeStyle = 'rgba(255, 255, 255, 0.03)';
  ctx.lineWidth = 1;
  for (let x = 0; x < w; x += 40) {
    ctx.beginPath();
    ctx.moveTo(x, 0);
    ctx.lineTo(x, h);
    ctx.stroke();
  }
  for (let y = 0; y < h; y += 40) {
    ctx.beginPath();
    ctx.moveTo(0, y);
    ctx.lineTo(w, y);
    ctx.stroke();
  }

  // Draw Edges
  GRAPH_EDGES.forEach(e => {
    const uCity = CITIES[e.u];
    const vCity = CITIES[e.v];
    const isPath = pathEdges.some(pe => (pe.u === e.u && pe.v === e.v) || (pe.u === e.v && pe.v === e.u));
    const isActive = activeEdge && ((activeEdge.u === e.u && activeEdge.v === e.v) || (activeEdge.u === e.v && activeEdge.v === e.u));

    ctx.beginPath();
    ctx.moveTo(uCity.x, uCity.y);
    ctx.lineTo(vCity.x, vCity.y);

    if (isPath) {
      ctx.strokeStyle = '#00d4aa';
      ctx.lineWidth = 4;
      ctx.shadowColor = '#00d4aa';
      ctx.shadowBlur = 12;
    } else if (isActive) {
      ctx.strokeStyle = '#ffd166';
      ctx.lineWidth = 3;
      ctx.shadowColor = '#ffd166';
      ctx.shadowBlur = 10;
    } else {
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.16)';
      ctx.lineWidth = 2;
      ctx.shadowBlur = 0;
    }
    ctx.stroke();
    ctx.shadowBlur = 0;

    // Draw edge weight label (km)
    const midX = (uCity.x + vCity.x) / 2;
    const midY = (uCity.y + vCity.y) / 2;
    ctx.fillStyle = isPath ? '#00d4aa' : 'rgba(255, 255, 255, 0.55)';
    ctx.font = '10px Inter, sans-serif';
    ctx.fillText(`${e.weight} km`, midX + 4, midY - 4);
  });

  // Draw Nodes (Cities)
  CITIES.forEach(city => {
    const state = nodeStates[city.id];
    let fill = '#1c1b2f';
    let stroke = '#6c63ff';
    let glow = 'rgba(108, 99, 255, 0.4)';
    let radius = 18;

    if (state === 1) { // visiting
      fill = '#ffd166';
      stroke = '#fff';
      glow = '#ffd166';
      radius = 22;
    } else if (state === 2) { // visited
      fill = '#6c63ff';
      stroke = '#8c7ae6';
      glow = '#6c63ff';
    } else if (state === 3) { // in shortest path
      fill = '#00d4aa';
      stroke = '#fff';
      glow = '#00d4aa';
      radius = 22;
    }

    // Outer glow
    ctx.beginPath();
    ctx.arc(city.x, city.y, radius + 4, 0, Math.PI * 2);
    ctx.fillStyle = glow;
    ctx.fill();

    // Node body
    ctx.beginPath();
    ctx.arc(city.x, city.y, radius, 0, Math.PI * 2);
    ctx.fillStyle = fill;
    ctx.fill();
    ctx.lineWidth = 2.5;
    ctx.strokeStyle = stroke;
    ctx.stroke();

    // Node label
    ctx.fillStyle = '#ffffff';
    ctx.font = 'bold 12px Inter, sans-serif';
    ctx.textAlign = 'center';
    ctx.fillText(city.name, city.x, city.y - radius - 8);

    // City code inside node
    ctx.font = '10px Inter, sans-serif';
    ctx.fillStyle = state === 1 ? '#000000' : '#ffffff';
    ctx.fillText(city.name.slice(0, 3).toUpperCase(), city.x, city.y + 3);
  });
}

// -------------------------------------------------------------
// Algorithm Execution Runners
// -------------------------------------------------------------
function runSelectedGraphAlgo() {
  const startSelect = document.getElementById('startCity');
  const endSelect = document.getElementById('endCity');
  const startId = parseInt(startSelect ? startSelect.value : 0, 10);
  const endId = parseInt(endSelect ? endSelect.value : 2, 10);

  resetGraphState();
  isGraphRunning = true;

  if (selectedAlgo === 'bfs') {
    runBFS(startId, endId);
  } else if (selectedAlgo === 'dfs') {
    runDFS(startId);
  } else if (selectedAlgo === 'warshall') {
    runWarshall();
  } else if (selectedAlgo === 'dijkstra') {
    runDijkstra(startId, endId);
  }
}

// BFS with step-by-step queue visual animation
function runBFS(startId, targetId) {
  const adj = getAdjList();
  const queue = [startId];
  const visited = new Array(CITIES.length).fill(false);
  const parent = new Array(CITIES.length).fill(-1);
  const traversalOrder = [];

  visited[startId] = true;
  logGraphStep(`<b>BFS Started</b> from Hub: <strong>${CITIES[startId].name}</strong> (Queue: [${CITIES[startId].name}])`);

  function step() {
    if (queue.length === 0) {
      finishTraversal('BFS Traversal Complete', traversalOrder);
      if (parent[targetId] !== -1 || startId === targetId) {
        highlightPath(parent, startId, targetId);
      }
      return;
    }

    const curr = queue.shift();
    traversalOrder.push(curr);
    nodeStates[curr] = 1; // visiting
    drawGraph();

    logGraphStep(`Dequeued <strong>${CITIES[curr].name}</strong>. Exploring adjacent distribution routes...`);

    graphAnimId = setTimeout(() => {
      nodeStates[curr] = 2; // visited

      adj[curr].forEach(neighbor => {
        if (!visited[neighbor.to]) {
          visited[neighbor.to] = true;
          parent[neighbor.to] = curr;
          queue.push(neighbor.to);
          logGraphStep(`&nbsp;&nbsp;↳ Enqueued unvisited hub <strong>${CITIES[neighbor.to].name}</strong>`);
        }
      });

      drawGraph();
      graphAnimId = setTimeout(step, 650);
    }, 450);
  }

  step();
}

// DFS with stack animation
function runDFS(startId) {
  const adj = getAdjList();
  const visited = new Array(CITIES.length).fill(false);
  const traversalOrder = [];

  logGraphStep(`<b>DFS Started</b> from Origin: <strong>${CITIES[startId].name}</strong>`);

  const stack = [startId];

  function step() {
    if (stack.length === 0) {
      finishTraversal('DFS Traversal Complete', traversalOrder);
      return;
    }

    const curr = stack.pop();
    if (!visited[curr]) {
      visited[curr] = true;
      traversalOrder.push(curr);
      nodeStates[curr] = 1;
      drawGraph();

      logGraphStep(`Visited <strong>${CITIES[curr].name}</strong>. Pushing neighbors to Call Stack.`);

      graphAnimId = setTimeout(() => {
        nodeStates[curr] = 2;
        // Push unvisited neighbors in reverse for standard order
        for (let i = adj[curr].length - 1; i >= 0; i--) {
          const neighbor = adj[curr][i].to;
          if (!visited[neighbor]) {
            stack.push(neighbor);
          }
        }
        drawGraph();
        graphAnimId = setTimeout(step, 650);
      }, 450);
    } else {
      step();
    }
  }

  step();
}

// Dijkstra Shortest Path Algorithm
function runDijkstra(startId, targetId) {
  const adj = getAdjList();
  const dist = new Array(CITIES.length).fill(Infinity);
  const parent = new Array(CITIES.length).fill(-1);
  const visited = new Array(CITIES.length).fill(false);

  dist[startId] = 0;
  logGraphStep(`<b>Dijkstra Started:</b> Origin = <strong>${CITIES[startId].name}</strong>, Destination = <strong>${CITIES[targetId].name}</strong>`);

  function step() {
    // Find unvisited vertex with min dist
    let minD = Infinity;
    let u = -1;
    for (let i = 0; i < CITIES.length; i++) {
      if (!visited[i] && dist[i] < minD) {
        minD = dist[i];
        u = i;
      }
    }

    if (u === -1 || dist[u] === Infinity || u === targetId) {
      // Reached destination or done
      if (dist[targetId] < Infinity) {
        logGraphStep(`🎯 <b>Optimal Route Found!</b> Distance: <strong>${dist[targetId]} km</strong>`, 'success');
        highlightPath(parent, startId, targetId);
      } else {
        logGraphStep(`No route exists to ${CITIES[targetId].name}`, 'error');
      }
      isGraphRunning = false;
      return;
    }

    visited[u] = true;
    nodeStates[u] = 1;
    drawGraph();

    logGraphStep(`Current Min-Distance Hub: <strong>${CITIES[u].name}</strong> (${dist[u]} km)`);

    graphAnimId = setTimeout(() => {
      nodeStates[u] = 2;

      adj[u].forEach(edge => {
        const v = edge.to;
        const w = edge.weight;
        if (!visited[v] && dist[u] + w < dist[v]) {
          dist[v] = dist[u] + w;
          parent[v] = u;
          logGraphStep(`&nbsp;&nbsp;↳ Relaxed edge to <strong>${CITIES[v].name}</strong>: new distance = ${dist[v]} km`);
        }
      });

      drawGraph();
      graphAnimId = setTimeout(step, 600);
    }, 400);
  }

  step();
}

// Highlight the reconstructed shortest path
function highlightPath(parent, startId, targetId) {
  let curr = targetId;
  const path = [];
  while (curr !== -1) {
    path.push(curr);
    curr = parent[curr];
  }
  path.reverse();

  if (path[0] !== startId) return;

  pathEdges = [];
  for (let i = 0; i < path.length; i++) {
    nodeStates[path[i]] = 3; // Path color
    if (i < path.length - 1) {
      pathEdges.push({ u: path[i], v: path[i + 1] });
    }
  }

  drawGraph();
  const routeNames = path.map(id => CITIES[id].name).join(' ➔ ');
  logGraphStep(`🚚 <b>Delivery Route:</b> ${routeNames}`, 'success');
  Toast.success(`Shortest delivery route highlighted: ${routeNames}`);
}

// Warshall's Algorithm for Transitive Closure
function runWarshall() {
  const n = CITIES.length;
  // Initialize adjacency reachability matrix
  const matrix = Array.from({ length: n }, () => new Array(n).fill(false));

  // Reflexive (each node reaches itself)
  for (let i = 0; i < n; i++) matrix[i][i] = true;

  // Direct edges
  GRAPH_EDGES.forEach(e => {
    matrix[e.u][e.v] = true;
    matrix[e.v][e.u] = true;
  });

  const matrixGrid = document.getElementById('matrixGrid');
  const warshallDiv = document.getElementById('warshallMatrix');
  if (warshallDiv) warshallDiv.classList.remove('hidden');

  logGraphStep(`<b>Warshall’s Transitive Closure</b> running across ${n} distribution vertices...`);

  // Compute transitive closure: matrix[i][j] = matrix[i][j] || (matrix[i][k] && matrix[k][j])
  for (let k = 0; k < n; k++) {
    for (let i = 0; i < n; i++) {
      for (let j = 0; j < n; j++) {
        matrix[i][j] = matrix[i][j] || (matrix[i][k] && matrix[k][j]);
      }
    }
  }

  // Render HTML table
  if (matrixGrid) {
    let html = '<table class="warshall-table"><thead><tr><th>Hub</th>';
    CITIES.forEach(c => { html += `<th>${c.name.slice(0, 3)}</th>`; });
    html += '</tr></thead><tbody>';

    for (let i = 0; i < n; i++) {
      html += `<tr><td><strong>${CITIES[i].name}</strong></td>`;
      for (let j = 0; j < n; j++) {
        const val = matrix[i][j] ? '1' : '0';
        const cls = matrix[i][j] ? 'reachable' : 'unreachable';
        html += `<td class="${cls}">${val}</td>`;
      }
      html += '</tr>';
    }
    html += '</tbody></table>';
    matrixGrid.innerHTML = html;
  }

  nodeStates.fill(2);
  drawGraph();
  logGraphStep('Transitive closure computed! All hubs are connected.', 'success');
  isGraphRunning = false;
}

function finishTraversal(title, order) {
  isGraphRunning = false;
  const names = order.map(id => CITIES[id].name).join(' ➔ ');
  logGraphStep(`<b>${title}:</b> ${names}`, 'success');
  Toast.info(`${title}`);
}
