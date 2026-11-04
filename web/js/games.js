/* ==========================================================================
   games.js — Interactive DSA Simulations & Games
   1. Tower of Hanoi (Recursion)
   2. Sorting Race (Bubble Sort vs Quick Sort)
   3. Binary Search Tree (BST Visualizer & Traversals)
   4. Infix to Postfix Calculator & Evaluator (Stack)
   5. Queue Visualizer (Simple, Circular & Priority Queues)
   SE-213 Data Structures & Algorithms Project
   ========================================================================== */

/* ==========================================================================
   1. TOWER OF HANOI SIMULATION
   ========================================================================== */
let hanoiDisks = 3;
let hanoiSpeed = 3;
let hanoiPegs = [[], [], []];
let hanoiMoves = [];
let hanoiMoveIdx = 0;
let hanoiTimer = null;
let hanoiIsRunning = false;
let hanoiTotalMoves = 0;

function setupHanoiGame() {
  const canvas = document.getElementById('hanoiCanvas');
  if (!canvas) return;

  if (!canvas.dataset.initialized) {
    bindHanoiEvents();
    canvas.dataset.initialized = 'true';
  }

  resetHanoi();
}

function bindHanoiEvents() {
  const diskSlider = document.getElementById('diskCount');
  const diskLabel = document.getElementById('diskCountLabel');
  const speedSlider = document.getElementById('hanoiSpeed');
  const speedLabel = document.getElementById('speedLabel');
  const autoBtn = document.getElementById('hanoiAutoSolve');
  const resetBtn = document.getElementById('hanoiReset');

  if (diskSlider) {
    diskSlider.addEventListener('input', (e) => {
      hanoiDisks = parseInt(e.target.value, 10);
      if (diskLabel) diskLabel.textContent = hanoiDisks;
      resetHanoi();
    });
  }

  if (speedSlider) {
    speedSlider.addEventListener('input', (e) => {
      hanoiSpeed = parseInt(e.target.value, 10);
      const speeds = ['Very Slow', 'Slow', 'Medium', 'Fast', 'Ultra'];
      if (speedLabel) speedLabel.textContent = speeds[hanoiSpeed - 1] || 'Medium';
    });
  }

  if (autoBtn) {
    autoBtn.addEventListener('click', () => {
      if (hanoiIsRunning) return;
      startHanoiAutoSolve();
    });
  }

  if (resetBtn) {
    resetBtn.addEventListener('click', resetHanoi);
  }
}

function resetHanoi() {
  if (hanoiTimer) clearTimeout(hanoiTimer);
  hanoiIsRunning = false;
  hanoiTotalMoves = 0;
  hanoiMoveIdx = 0;

  // Peg 0 has disks [hanoiDisks, ..., 1]
  hanoiPegs = [[], [], []];
  for (let i = hanoiDisks; i >= 1; i--) {
    hanoiPegs[0].push(i);
  }

  const optMoves = Math.pow(2, hanoiDisks) - 1;
  const optEl = document.getElementById('optMoves');
  const moveEl = document.getElementById('moveCount');
  const recurEl = document.getElementById('recurCalls');
  const formEl = document.getElementById('formulaVal');
  const statusEl = document.getElementById('hanoiStatus');
  const histEl = document.getElementById('hanoiHistory');

  if (optEl) optEl.textContent = optMoves;
  if (moveEl) moveEl.textContent = '0';
  if (recurEl) recurEl.textContent = optMoves;
  if (formEl) formEl.textContent = `= 2^${hanoiDisks} - 1 = ${optMoves}`;
  if (statusEl) statusEl.textContent = 'Set disk count and click Auto Solve!';
  if (histEl) histEl.innerHTML = '';

  drawHanoi();
}

function generateHanoiMoves(n, from, to, aux, moves) {
  if (n === 1) {
    moves.push({ disk: 1, from, to });
    return;
  }
  generateHanoiMoves(n - 1, from, aux, to, moves);
  moves.push({ disk: n, from, to });
  generateHanoiMoves(n - 1, aux, to, from, moves);
}

function startHanoiAutoSolve() {
  hanoiIsRunning = true;
  hanoiMoves = [];
  generateHanoiMoves(hanoiDisks, 0, 2, 1, hanoiMoves);
  hanoiMoveIdx = 0;

  const statusEl = document.getElementById('hanoiStatus');
  if (statusEl) statusEl.textContent = 'Solving Tower of Hanoi recursively...';

  stepHanoi();
}

function stepHanoi() {
  if (!hanoiIsRunning || hanoiMoveIdx >= hanoiMoves.length) {
    hanoiIsRunning = false;
    const statusEl = document.getElementById('hanoiStatus');
    if (statusEl) statusEl.textContent = '🎉 Solved! Minimum moves achieved.';
    Toast.success('Tower of Hanoi solved in optimal moves!');
    return;
  }

  const move = hanoiMoves[hanoiMoveIdx++];
  const disk = hanoiPegs[move.from].pop();
  hanoiPegs[move.to].push(disk);
  hanoiTotalMoves++;

  const moveEl = document.getElementById('moveCount');
  if (moveEl) moveEl.textContent = hanoiTotalMoves;

  const pegNames = ['A (Source)', 'B (Aux)', 'C (Target)'];
  const histEl = document.getElementById('hanoiHistory');
  if (histEl) {
    const entry = document.createElement('div');
    entry.className = 'history-item';
    entry.innerHTML = `#${hanoiTotalMoves}: Disk <strong>${move.disk}</strong> from <code>${pegNames[move.from]}</code> ➔ <code>${pegNames[move.to]}</code>`;
    histEl.prepend(entry);
  }

  drawHanoi();

  // Speed mapping: 1 -> 800ms, 5 -> 100ms
  const delays = [800, 500, 300, 180, 80];
  const delay = delays[hanoiSpeed - 1] || 300;
  hanoiTimer = setTimeout(stepHanoi, delay);
}

function drawHanoi() {
  const canvas = document.getElementById('hanoiCanvas');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  const w = canvas.width;
  const h = canvas.height;

  ctx.clearRect(0, 0, w, h);

  // Base platform
  const baseY = h - 40;
  ctx.fillStyle = '#2d2b55';
  ctx.fillRect(40, baseY, w - 80, 16);

  // 3 Pegs
  const pegX = [w * 0.22, w * 0.5, w * 0.78];
  const pegTopY = 60;
  const pegH = baseY - pegTopY;

  const pegLabels = ['Peg A (Source)', 'Peg B (Auxiliary)', 'Peg C (Destination)'];

  pegX.forEach((px, idx) => {
    // Draw Peg Pillar
    ctx.fillStyle = 'rgba(255, 255, 255, 0.2)';
    ctx.fillRect(px - 5, pegTopY, 10, pegH);

    // Label
    ctx.fillStyle = 'rgba(255, 255, 255, 0.6)';
    ctx.font = '12px Inter, sans-serif';
    ctx.textAlign = 'center';
    ctx.fillText(pegLabels[idx], px, h - 12);
  });

  // Disk Colors palette
  const diskColors = [
    '#ff6b6b', '#feca57', '#48dbfb', '#ff9ff3', 
    '#54a0ff', '#5f27cd', '#1dd1a1', '#ff9f43'
  ];

  // Draw Disks on pegs
  const diskH = Math.min(24, Math.floor((pegH - 20) / hanoiDisks));
  const maxDiskW = 160;
  const minDiskW = 40;

  hanoiPegs.forEach((peg, pIdx) => {
    const px = pegX[pIdx];
    peg.forEach((diskVal, dIdx) => {
      const fraction = diskVal / hanoiDisks;
      const dw = minDiskW + fraction * (maxDiskW - minDiskW);
      const dy = baseY - (dIdx + 1) * diskH - 2;
      const dx = px - dw / 2;

      ctx.fillStyle = diskColors[(diskVal - 1) % diskColors.length];
      // Rounded disk
      ctx.beginPath();
      ctx.roundRect ? ctx.roundRect(dx, dy, dw, diskH - 3, 6) : ctx.fillRect(dx, dy, dw, diskH - 3);
      ctx.fill();

      // Disk number text
      ctx.fillStyle = '#000000';
      ctx.font = 'bold 11px Inter, sans-serif';
      ctx.textAlign = 'center';
      ctx.fillText(diskVal, px, dy + diskH / 2 + 1);
    });
  });
}

/* ==========================================================================
   2. SORTING RACE (Bubble Sort vs Quick Sort)
   ========================================================================== */
let sortArray = [];
let sortSize = 20;
let sortSpeedVal = 5;
let isSortingRunning = false;

function setupSortingRace() {
  const sizeSlider = document.getElementById('arraySize');
  const sizeLabel = document.getElementById('arraySizeLabel');
  const speedSlider = document.getElementById('sortSpeed');
  const speedLabel = document.getElementById('sortSpeedLabel');
  const startBtn = document.getElementById('startRaceBtn');
  const newArrBtn = document.getElementById('newArrayBtn');

  if (sizeSlider && !sizeSlider.dataset.bound) {
    sizeSlider.dataset.bound = 'true';
    sizeSlider.addEventListener('input', (e) => {
      sortSize = parseInt(e.target.value, 10);
      if (sizeLabel) sizeLabel.textContent = sortSize;
      generateNewSortArray();
    });
  }

  if (speedSlider && !speedSlider.dataset.bound) {
    speedSlider.dataset.bound = 'true';
    speedSlider.addEventListener('input', (e) => {
      sortSpeedVal = parseInt(e.target.value, 10);
      const labels = ['1x', '2x', '3x', '4x', '5x', '6x', '7x', '8x', '9x', 'Turbo'];
      if (speedLabel) speedLabel.textContent = labels[sortSpeedVal - 1] || '5x';
    });
  }

  if (startBtn && !startBtn.dataset.bound) {
    startBtn.dataset.bound = 'true';
    startBtn.addEventListener('click', startSortingRace);
  }

  if (newArrBtn && !newArrBtn.dataset.bound) {
    newArrBtn.dataset.bound = 'true';
    newArrBtn.addEventListener('click', generateNewSortArray);
  }

  generateNewSortArray();
}

function generateNewSortArray() {
  if (isSortingRunning) return;
  sortArray = [];
  for (let i = 0; i < sortSize; i++) {
    sortArray.push(Math.floor(10 + Math.random() * 90));
  }
  renderBars('bubbleBars', sortArray);
  renderBars('quickBars', sortArray);

  // Reset counters
  resetSortStats();
}

function resetSortStats() {
  ['bubbleCmp', 'bubbleSwap', 'quickCmp', 'quickSwap'].forEach(id => {
    const el = document.getElementById(id);
    if (el) el.textContent = '0';
  });
  const bBar = document.getElementById('bubbleBar');
  const qBar = document.getElementById('quickBar');
  const winnerEl = document.getElementById('sortWinner');
  if (bBar) bBar.style.width = '0%';
  if (qBar) qBar.style.width = '0%';
  if (winnerEl) {
    winnerEl.classList.add('hidden');
    winnerEl.innerHTML = '';
  }
}

function renderBars(containerId, arr, activeIdxs = [], swapIdxs = []) {
  const container = document.getElementById(containerId);
  if (!container) return;
  container.innerHTML = '';

  const maxVal = Math.max(...arr, 100);

  arr.forEach((val, idx) => {
    const bar = document.createElement('div');
    bar.className = 'sort-bar';
    const heightPercent = (val / maxVal) * 100;
    bar.style.height = `${heightPercent}%`;

    if (swapIdxs.includes(idx)) {
      bar.classList.add('swap');
    } else if (activeIdxs.includes(idx)) {
      bar.classList.add('active');
    }

    container.appendChild(bar);
  });
}

async function startSortingRace() {
  if (isSortingRunning) return;
  isSortingRunning = true;
  resetSortStats();

  const bubbleData = [...sortArray];
  const quickData = [...sortArray];

  let bubbleFinished = false;
  let quickFinished = false;

  const delay = Math.max(8, 220 - sortSpeedVal * 20);

  // Run both algorithms concurrently
  const bubblePromise = runVisualBubbleSort(bubbleData, delay, () => { bubbleFinished = true; });
  const quickPromise = runVisualQuickSort(quickData, delay, () => { quickFinished = true; });

  await Promise.all([bubblePromise, quickPromise]);

  isSortingRunning = false;
  const winnerEl = document.getElementById('sortWinner');
  if (winnerEl) {
    winnerEl.classList.remove('hidden');
    winnerEl.innerHTML = `
      <div class="winner-badge">🏆 Winner: Quick Sort O(N log N)</div>
      <p style="font-size: 0.9em; opacity: 0.85; margin-top: 4px;">Quick Sort significantly outperforms Bubble Sort O(N²) on partitions and comparisons!</p>
    `;
  }
  Toast.success('Sorting Race finished!');
}

async function runVisualBubbleSort(arr, delay, onDone) {
  let cmp = 0;
  let swaps = 0;
  const n = arr.length;
  const cmpEl = document.getElementById('bubbleCmp');
  const swapEl = document.getElementById('bubbleSwap');
  const barEl = document.getElementById('bubbleBar');

  for (let i = 0; i < n - 1; i++) {
    for (let j = 0; j < n - i - 1; j++) {
      cmp++;
      if (cmpEl) cmpEl.textContent = cmp;

      if (arr[j] > arr[j + 1]) {
        // Swap
        const temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
        swaps++;
        if (swapEl) swapEl.textContent = swaps;

        renderBars('bubbleBars', arr, [], [j, j + 1]);
      } else {
        renderBars('bubbleBars', arr, [j, j + 1], []);
      }

      if (barEl) barEl.style.width = `${((i * n + j) / ((n * (n - 1)) / 2)) * 100}%`;
      await new Promise(r => setTimeout(r, delay));
    }
  }

  if (barEl) barEl.style.width = '100%';
  renderBars('bubbleBars', arr);
  onDone();
}

async function runVisualQuickSort(arr, delay, onDone) {
  let cmp = 0;
  let swaps = 0;
  const cmpEl = document.getElementById('quickCmp');
  const swapEl = document.getElementById('quickSwap');
  const barEl = document.getElementById('quickBar');

  async function quickSortHelper(low, high) {
    if (low < high) {
      const pi = await partition(low, high);
      await quickSortHelper(low, pi - 1);
      await quickSortHelper(pi + 1, high);
    }
  }

  async function partition(low, high) {
    const pivot = arr[high];
    let i = low - 1;

    for (let j = low; j < high; j++) {
      cmp++;
      if (cmpEl) cmpEl.textContent = cmp;

      if (arr[j] < pivot) {
        i++;
        const temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
        swaps++;
        if (swapEl) swapEl.textContent = swaps;
        renderBars('quickBars', arr, [j, high], [i, j]);
      } else {
        renderBars('quickBars', arr, [j, high], []);
      }
      await new Promise(r => setTimeout(r, delay));
    }

    const temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    swaps++;
    if (swapEl) swapEl.textContent = swaps;

    renderBars('quickBars', arr, [], [i + 1, high]);
    await new Promise(r => setTimeout(r, delay));

    return i + 1;
  }

  await quickSortHelper(0, arr.length - 1);
  if (barEl) barEl.style.width = '100%';
  renderBars('quickBars', arr);
  onDone();
}

/* ==========================================================================
   3. BINARY SEARCH TREE (BST) VISUALIZER
   ========================================================================== */
class BSTNode {
  constructor(key) {
    this.key = key;
    this.left = null;
    this.right = null;
    this.x = 0;
    this.y = 0;
    this.highlight = false;
  }
}

let bstRoot = null;
let selectedTrav = 'inorder';

function setupBSTVisualizer() {
  const canvas = document.getElementById('bstCanvas');
  if (!canvas) return;

  if (!canvas.dataset.initialized) {
    bindBSTEvents();
    canvas.dataset.initialized = 'true';
    // Seed with balanced default
    [50, 30, 70, 20, 40, 60, 80].forEach(val => bstInsert(val));
  }

  drawBST();
}

function bindBSTEvents() {
  const insBtn = document.getElementById('bstInsertBtn');
  const insInput = document.getElementById('bstInsertVal');
  const delBtn = document.getElementById('bstDeleteBtn');
  const delInput = document.getElementById('bstDeleteVal');
  const searchBtn = document.getElementById('bstSearchBtn');
  const searchInput = document.getElementById('bstSearchVal');
  const travBtn = document.getElementById('bstTraverseBtn');
  const clearBtn = document.getElementById('bstClearBtn');
  const preset1 = document.getElementById('bstPreset1');
  const preset2 = document.getElementById('bstPreset2');

  const travTabs = document.querySelectorAll('.trav-tab');
  travTabs.forEach(tab => {
    tab.addEventListener('click', () => {
      travTabs.forEach(t => t.classList.remove('active'));
      tab.classList.add('active');
      selectedTrav = tab.dataset.trav;
    });
  });

  if (insBtn && insInput) {
    insBtn.addEventListener('click', () => {
      const val = parseInt(insInput.value, 10);
      if (isNaN(val)) return;
      bstInsert(val);
      insInput.value = '';
      drawBST();
      setBSTStatus(`Inserted ${val} into BST.`);
    });
  }

  if (delBtn && delInput) {
    delBtn.addEventListener('click', () => {
      const val = parseInt(delInput.value, 10);
      if (isNaN(val)) return;
      bstRoot = bstDelete(bstRoot, val);
      delInput.value = '';
      drawBST();
      setBSTStatus(`Deleted ${val} from BST.`);
    });
  }

  if (searchBtn && searchInput) {
    searchBtn.addEventListener('click', () => {
      const val = parseInt(searchInput.value, 10);
      if (isNaN(val)) return;
      bstSearchAnimate(val);
    });
  }

  if (travBtn) {
    travBtn.addEventListener('click', () => {
      runBSTTraversal(selectedTrav);
    });
  }

  if (clearBtn) {
    clearBtn.addEventListener('click', () => {
      bstRoot = null;
      drawBST();
      setBSTStatus('BST cleared.');
      const resEl = document.getElementById('bstTraversalResult');
      if (resEl) resEl.innerHTML = '';
    });
  }

  if (preset1) {
    preset1.addEventListener('click', () => {
      bstRoot = null;
      const count = 7;
      for (let i = 0; i < count; i++) {
        bstInsert(Math.floor(10 + Math.random() * 80));
      }
      drawBST();
      setBSTStatus('Generated Random BST.');
    });
  }

  if (preset2) {
    preset2.addEventListener('click', () => {
      bstRoot = null;
      [40, 20, 60, 10, 30, 50, 70].forEach(val => bstInsert(val));
      drawBST();
      setBSTStatus('Loaded Balanced BST preset.');
    });
  }
}

function setBSTStatus(msg) {
  const statusEl = document.getElementById('bstStatus');
  if (statusEl) statusEl.textContent = msg;
}

function bstInsert(key) {
  const newNode = new BSTNode(key);
  if (!bstRoot) {
    bstRoot = newNode;
    return;
  }
  let curr = bstRoot;
  while (true) {
    if (key < curr.key) {
      if (!curr.left) {
        curr.left = newNode;
        break;
      }
      curr = curr.left;
    } else if (key > curr.key) {
      if (!curr.right) {
        curr.right = newNode;
        break;
      }
      curr = curr.right;
    } else {
      break; // Duplicate
    }
  }
}

function bstDelete(root, key) {
  if (!root) return null;
  if (key < root.key) {
    root.left = bstDelete(root.left, key);
  } else if (key > root.key) {
    root.right = bstDelete(root.right, key);
  } else {
    // Node to delete found
    if (!root.left) return root.right;
    if (!root.right) return root.left;
    // 2 children: find inorder successor (min in right subtree)
    let minNode = root.right;
    while (minNode.left) minNode = minNode.left;
    root.key = minNode.key;
    root.right = bstDelete(root.right, minNode.key);
  }
  return root;
}

async function bstSearchAnimate(val) {
  clearBSTHighlights(bstRoot);
  let curr = bstRoot;
  let path = [];

  while (curr) {
    path.push(curr);
    if (val === curr.key) break;
    if (val < curr.key) curr = curr.left;
    else curr = curr.right;
  }

  for (const node of path) {
    node.highlight = true;
    drawBST();
    await new Promise(r => setTimeout(r, 450));
  }

  const found = path.length > 0 && path[path.length - 1].key === val;
  setBSTStatus(found ? `✅ Found ${val} in BST!` : `❌ Value ${val} not found in BST.`);
  Toast[found ? 'success' : 'warning'](found ? `Key ${val} found!` : `Key ${val} not found`);
}

function clearBSTHighlights(node) {
  if (!node) return;
  node.highlight = false;
  clearBSTHighlights(node.left);
  clearBSTHighlights(node.right);
}

function runBSTTraversal(type) {
  const res = [];
  const nodes = [];

  function inorder(n) {
    if (!n) return;
    inorder(n.left);
    res.push(n.key);
    nodes.push(n);
    inorder(n.right);
  }

  function preorder(n) {
    if (!n) return;
    res.push(n.key);
    nodes.push(n);
    preorder(n.left);
    preorder(n.right);
  }

  function postorder(n) {
    if (!n) return;
    postorder(n.left);
    postorder(n.right);
    res.push(n.key);
    nodes.push(n);
  }

  if (type === 'inorder') inorder(bstRoot);
  else if (type === 'preorder') preorder(bstRoot);
  else if (type === 'postorder') postorder(bstRoot);

  const resEl = document.getElementById('bstTraversalResult');
  if (resEl) {
    resEl.innerHTML = `<strong>${type.toUpperCase()} Traversal:</strong> [ ${res.join(', ')} ]`;
  }

  // Animate nodes in sequence
  (async () => {
    for (const node of nodes) {
      clearBSTHighlights(bstRoot);
      node.highlight = true;
      drawBST();
      await new Promise(r => setTimeout(r, 380));
    }
    clearBSTHighlights(bstRoot);
    drawBST();
  })();
}

function drawBST() {
  const canvas = document.getElementById('bstCanvas');
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  const w = canvas.width;
  const h = canvas.height;

  ctx.clearRect(0, 0, w, h);

  if (!bstRoot) {
    ctx.fillStyle = 'rgba(255, 255, 255, 0.3)';
    ctx.font = '14px Inter, sans-serif';
    ctx.textAlign = 'center';
    ctx.fillText('BST is empty. Insert values to build the tree.', w / 2, h / 2);
    return;
  }

  // Calculate layout
  function layout(node, depth, left, right) {
    if (!node) return;
    node.x = (left + right) / 2;
    node.y = 50 + depth * 70;
    layout(node.left, depth + 1, left, node.x);
    layout(node.right, depth + 1, node.x, right);
  }

  layout(bstRoot, 0, 0, w);

  // Draw Edges first
  function drawEdges(node) {
    if (!node) return;
    if (node.left) {
      ctx.beginPath();
      ctx.moveTo(node.x, node.y);
      ctx.lineTo(node.left.x, node.left.y);
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.2)';
      ctx.lineWidth = 2;
      ctx.stroke();
      drawEdges(node.left);
    }
    if (node.right) {
      ctx.beginPath();
      ctx.moveTo(node.x, node.y);
      ctx.lineTo(node.right.x, node.right.y);
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.2)';
      ctx.lineWidth = 2;
      ctx.stroke();
      drawEdges(node.right);
    }
  }

  drawEdges(bstRoot);

  // Draw Nodes
  function drawNodes(node) {
    if (!node) return;
    const r = 20;

    ctx.beginPath();
    ctx.arc(node.x, node.y, r, 0, Math.PI * 2);
    ctx.fillStyle = node.highlight ? '#ffd166' : '#6c63ff';
    ctx.fill();
    ctx.lineWidth = 2.5;
    ctx.strokeStyle = node.highlight ? '#fff' : '#8c7ae6';
    ctx.stroke();

    ctx.fillStyle = node.highlight ? '#000000' : '#ffffff';
    ctx.font = 'bold 12px Inter, sans-serif';
    ctx.textAlign = 'center';
    ctx.fillText(node.key, node.x, node.y + 4);

    drawNodes(node.left);
    drawNodes(node.right);
  }

  drawNodes(bstRoot);
}

/* ==========================================================================
   4. INFIX TO POSTFIX CALCULATOR & EVALUATOR (Stack)
   ========================================================================== */
function setupCalculator() {
  const input = document.getElementById('infixInput');
  const convertBtn = document.getElementById('convertBtn');
  const resetBtn = document.getElementById('resetCalcBtn');
  const evalBtn = document.getElementById('evalBtn');
  const helperInput = document.getElementById('evalHelperInput');
  const evalRow = document.getElementById('evalRow');

  // Example pills
  document.querySelectorAll('.example-exprs .pill-sm[data-expr]').forEach(pill => {
    pill.addEventListener('click', () => {
      if (input) input.value = pill.dataset.expr;
    });
  });

  if (convertBtn && !convertBtn.dataset.bound) {
    convertBtn.dataset.bound = 'true';
    convertBtn.addEventListener('click', () => {
      const expr = input ? input.value.trim() : '';
      if (!expr) {
        Toast.warning('Enter an infix expression first!');
        return;
      }
      runInfixToPostfix(expr);
      if (evalRow) evalRow.classList.remove('hidden');
    });
  }

  if (resetBtn && !resetBtn.dataset.bound) {
    resetBtn.dataset.bound = 'true';
    resetBtn.addEventListener('click', () => {
      if (input) input.value = '';
      const res = document.getElementById('postfixResult');
      const evalRes = document.getElementById('evalResult');
      const stackVis = document.getElementById('calcStackVis');
      const outQ = document.getElementById('outputQueue');
      const log = document.getElementById('stepLog');

      if (res) res.textContent = '—';
      if (evalRes) evalRes.textContent = '';
      if (stackVis) stackVis.innerHTML = '<div class="stack-empty-msg">Stack empty</div>';
      if (outQ) outQ.innerHTML = '';
      if (log) log.innerHTML = '';
      if (evalRow) evalRow.classList.add('hidden');
    });
  }

  if (evalBtn && !evalBtn.dataset.bound) {
    evalBtn.dataset.bound = 'true';
    evalBtn.addEventListener('click', () => {
      const postEl = document.getElementById('postfixResult');
      if (!postEl || postEl.textContent === '—') return;
      evaluatePostfixExpr(postEl.textContent, helperInput ? helperInput.value : '');
    });
  }
}

function precedence(op) {
  if (op === '^') return 3;
  if (op === '*' || op === '/') return 2;
  if (op === '+' || op === '-') return 1;
  return 0;
}

function isRightAssociative(op) {
  return op === '^';
}

function isOperator(ch) {
  return ['+', '-', '*', '/', '^'].includes(ch);
}

function runInfixToPostfix(expr) {
  const stack = [];
  const output = [];
  const logEl = document.getElementById('stepLog');
  const stackVis = document.getElementById('calcStackVis');
  const outQ = document.getElementById('outputQueue');
  const resEl = document.getElementById('postfixResult');

  if (logEl) logEl.innerHTML = '';

  function logStep(msg) {
    if (!logEl) return;
    const item = document.createElement('div');
    item.className = 'log-item';
    item.textContent = msg;
    logEl.appendChild(item);
  }

  const tokens = expr.match(/[a-zA-Z0-9]+|[\+\-\*\/\^\(\)]/g) || [];

  for (const token of tokens) {
    if (/^[a-zA-Z0-9]+$/.test(token)) {
      output.push(token);
      logStep(`Operand "${token}": Appended to Output Queue.`);
    } else if (token === '(') {
      stack.push(token);
      logStep(`Left Parenthesis "(": Pushed to Stack.`);
    } else if (token === ')') {
      logStep(`Right Parenthesis ")": Unwinding stack until "("...`);
      while (stack.length > 0 && stack[stack.length - 1] !== '(') {
        output.push(stack.pop());
      }
      stack.pop(); // Pop '('
    } else if (isOperator(token)) {
      while (
        stack.length > 0 &&
        stack[stack.length - 1] !== '(' &&
        (precedence(stack[stack.length - 1]) > precedence(token) ||
         (precedence(stack[stack.length - 1]) === precedence(token) && !isRightAssociative(token)))
      ) {
        const popped = stack.pop();
        output.push(popped);
        logStep(`Higher/equal precedence: Popped "${popped}" to Output.`);
      }
      stack.push(token);
      logStep(`Operator "${token}": Pushed to Stack.`);
    }
  }

  while (stack.length > 0) {
    output.push(stack.pop());
  }

  const postfixStr = output.join(' ');
  if (resEl) resEl.textContent = postfixStr;

  // Render Visual Stack
  if (stackVis) {
    stackVis.innerHTML = stack.length === 0 ? '<div class="stack-empty-msg">Stack completely emptied ✓</div>' : '';
    stack.forEach(op => {
      const b = document.createElement('div');
      b.className = 'stack-block';
      b.textContent = op;
      stackVis.prepend(b);
    });
  }

  // Render Output Queue
  if (outQ) {
    outQ.innerHTML = '';
    output.forEach(item => {
      const b = document.createElement('div');
      b.className = 'queue-token';
      b.textContent = item;
      outQ.appendChild(b);
    });
  }

  Toast.success('Converted to Postfix successfully!');
}

function evaluatePostfixExpr(postfix, varAssigns) {
  const evalResEl = document.getElementById('evalResult');
  const tokens = postfix.split(/\s+/).filter(Boolean);
  const stack = [];

  // Parse variables e.g. "a=2, b=3"
  const vars = {};
  if (varAssigns) {
    varAssigns.split(',').forEach(pair => {
      const [k, v] = pair.split('=').map(s => s.trim());
      if (k && !isNaN(parseFloat(v))) vars[k] = parseFloat(v);
    });
  }

  for (const t of tokens) {
    if (!isOperator(t)) {
      let val = parseFloat(t);
      if (isNaN(val) && vars[t] !== undefined) val = vars[t];
      if (isNaN(val)) val = 1; // Default fallback
      stack.push(val);
    } else {
      const b = stack.pop();
      const a = stack.pop();
      let res = 0;
      if (t === '+') res = a + b;
      else if (t === '-') res = a - b;
      else if (t === '*') res = a * b;
      else if (t === '/') res = b !== 0 ? a / b : 0;
      else if (t === '^') res = Math.pow(a, b);
      stack.push(res);
    }
  }

  const finalVal = stack.length > 0 ? stack[0].toFixed(2) : 'Error';
  if (evalResEl) {
    evalResEl.innerHTML = `<strong>Result:</strong> <span class="text-success">${finalVal}</span>`;
  }
}

/* ==========================================================================
   5. QUEUE VISUALIZER (Simple, Circular & Priority Queues)
   ========================================================================== */
let activeQueueType = 'simple';
let simpleQ = ['Order #101', 'Order #102', 'Order #103'];
let circularQ = new Array(8).fill(null);
let circHead = 0;
let circTail = 3;
let priorityQ = [
  { item: 'VIP Drone Delivery', priority: 1 },
  { item: 'Express Courier', priority: 2 },
  { item: 'Standard Shipping', priority: 5 }
];

// Pre-fill circular queue
circularQ[0] = 'Slot #1';
circularQ[1] = 'Slot #2';
circularQ[2] = 'Slot #3';

function setupQueueViz() {
  const tabs = document.querySelectorAll('.queue-tab');
  tabs.forEach(tab => {
    tab.addEventListener('click', () => {
      tabs.forEach(t => t.classList.remove('active'));
      tab.classList.add('active');
      activeQueueType = tab.dataset.qtype;
      renderActiveQueueUI();
    });
  });

  renderActiveQueueUI();
}

function renderActiveQueueUI() {
  const main = document.getElementById('queueVizMain');
  if (!main) return;

  if (activeQueueType === 'simple') {
    renderSimpleQueue(main);
  } else if (activeQueueType === 'circular') {
    renderCircularQueue(main);
  } else if (activeQueueType === 'priority') {
    renderPriorityQueue(main);
  }
}

function renderSimpleQueue(container) {
  container.innerHTML = `
    <div class="q-control-card">
      <h3>Simple Queue (First-In, First-Out)</h3>
      <p class="panel-sub">Front element is dequeued first. Enqueue adds to rear.</p>
      <div class="input-row" style="margin-top: 15px;">
        <input type="text" id="simpleQInput" placeholder="Enter package or order name..." />
        <button class="btn btn-primary" id="simpleEnqueueBtn">Enqueue</button>
        <button class="btn btn-danger" id="simpleDequeueBtn">Dequeue</button>
      </div>
    </div>
    <div class="simple-q-track" id="simpleQTrack"></div>
  `;

  const track = document.getElementById('simpleQTrack');
  renderSimpleQItems(track);

  document.getElementById('simpleEnqueueBtn')?.addEventListener('click', () => {
    const input = document.getElementById('simpleQInput');
    const val = input ? input.value.trim() : '';
    if (!val) return;
    simpleQ.push(val);
    input.value = '';
    renderSimpleQItems(track);
    Toast.success(`Enqueued "${val}" to Rear.`);
  });

  document.getElementById('simpleDequeueBtn')?.addEventListener('click', () => {
    if (simpleQ.length === 0) {
      Toast.warning('Queue Underflow: Simple queue is empty!');
      return;
    }
    const popped = simpleQ.shift();
    renderSimpleQItems(track);
    Toast.info(`Dequeued "${popped}" from Front.`);
  });
}

function renderSimpleQItems(track) {
  if (!track) return;
  track.innerHTML = '';
  if (simpleQ.length === 0) {
    track.innerHTML = '<div class="queue-empty-msg">Queue is empty</div>';
    return;
  }
  simpleQ.forEach((item, idx) => {
    const node = document.createElement('div');
    node.className = `q-node ${idx === 0 ? 'is-front' : ''} ${idx === simpleQ.length - 1 ? 'is-rear' : ''}`;
    node.innerHTML = `
      <div class="q-node-label">${idx === 0 ? 'FRONT ➔' : idx === simpleQ.length - 1 ? 'REAR ⬅' : `#${idx}`}</div>
      <div class="q-node-val">${item}</div>
    `;
    track.appendChild(node);
  });
}

function renderCircularQueue(container) {
  container.innerHTML = `
    <div class="q-control-card">
      <h3>Circular Queue (Ring Buffer, Cap: 8)</h3>
      <p class="panel-sub">Eliminates memory waste of simple queues by wrapping around: <code>(rear + 1) % size</code></p>
      <div class="input-row" style="margin-top: 15px;">
        <input type="text" id="circQInput" placeholder="Enter flash sale reservation..." />
        <button class="btn btn-primary" id="circEnqueueBtn">Enqueue</button>
        <button class="btn btn-danger" id="circDequeueBtn">Dequeue</button>
      </div>
      <div class="circ-pointers" style="margin-top: 10px; font-size: 0.9em; opacity: 0.85;">
        <span>HEAD: [${circHead}]</span> • <span>TAIL: [${circTail}]</span>
      </div>
    </div>
    <div class="circular-grid" id="circGrid"></div>
  `;

  renderCircularQItems();

  document.getElementById('circEnqueueBtn')?.addEventListener('click', () => {
    const input = document.getElementById('circQInput');
    const val = input ? input.value.trim() : '';
    if (!val) return;

    if (circularQ[circTail] !== null) {
      Toast.error('Circular Queue Overflow! Ring buffer full.');
      return;
    }

    circularQ[circTail] = val;
    circTail = (circTail + 1) % 8;
    input.value = '';
    renderCircularQueue(container);
    Toast.success(`Enqueued "${val}" to circular slot.`);
  });

  document.getElementById('circDequeueBtn')?.addEventListener('click', () => {
    if (circularQ[circHead] === null) {
      Toast.warning('Circular Queue Underflow: Ring buffer is empty!');
      return;
    }
    const val = circularQ[circHead];
    circularQ[circHead] = null;
    circHead = (circHead + 1) % 8;
    renderCircularQueue(container);
    Toast.info(`Dequeued "${val}" from circular slot.`);
  });
}

function renderCircularQItems() {
  const grid = document.getElementById('circGrid');
  if (!grid) return;
  grid.innerHTML = '';

  circularQ.forEach((slot, idx) => {
    const isHead = idx === circHead;
    const isTail = idx === circTail;
    const cell = document.createElement('div');
    cell.className = `circ-cell ${slot ? 'occupied' : 'empty'} ${isHead ? 'is-head' : ''} ${isTail ? 'is-tail' : ''}`;
    cell.innerHTML = `
      <div class="cell-idx">Slot #${idx} ${isHead ? '👑 HEAD' : ''} ${isTail ? '📍 TAIL' : ''}</div>
      <div class="cell-val">${slot ? slot : '— Empty —'}</div>
    `;
    grid.appendChild(cell);
  });
}

function renderPriorityQueue(container) {
  container.innerHTML = `
    <div class="q-control-card">
      <h3>Priority Queue (VIP Express Orders)</h3>
      <p class="panel-sub">Lowest numerical priority value gets dequeued first (Min-Heap / VIP 1 &gt; Standard 5).</p>
      <div class="input-row" style="margin-top: 15px;">
        <input type="text" id="pqItem" placeholder="Order / Package..." style="flex:2;" />
        <select id="pqPriority" style="flex:1;">
          <option value="1">Priority 1 (VIP Urgent)</option>
          <option value="2">Priority 2 (Express)</option>
          <option value="3">Priority 3 (Standard)</option>
          <option value="5">Priority 5 (Economy)</option>
        </select>
        <button class="btn btn-primary" id="pqInsertBtn">Insert</button>
        <button class="btn btn-danger" id="pqExtractMinBtn">Extract Min</button>
      </div>
    </div>
    <div class="pq-list" id="pqList"></div>
  `;

  renderPQItems();

  document.getElementById('pqInsertBtn')?.addEventListener('click', () => {
    const itemInput = document.getElementById('pqItem');
    const prioSelect = document.getElementById('pqPriority');
    const item = itemInput ? itemInput.value.trim() : '';
    const prio = parseInt(prioSelect ? prioSelect.value : 3, 10);
    if (!item) return;

    priorityQ.push({ item, priority: prio });
    priorityQ.sort((a, b) => a.priority - b.priority); // Min-heap ordering
    itemInput.value = '';
    renderPQItems();
    Toast.success(`Inserted "${item}" with Priority ${prio}`);
  });

  document.getElementById('pqExtractMinBtn')?.addEventListener('click', () => {
    if (priorityQ.length === 0) {
      Toast.warning('Priority Queue Underflow!');
      return;
    }
    const highest = priorityQ.shift();
    renderPQItems();
    Toast.success(`Extracted Min-Priority Order: "${highest.item}" (Priority: ${highest.priority})`);
  });
}

function renderPQItems() {
  const list = document.getElementById('pqList');
  if (!list) return;
  list.innerHTML = '';

  if (priorityQ.length === 0) {
    list.innerHTML = '<div class="queue-empty-msg">Priority queue is empty</div>';
    return;
  }

  priorityQ.forEach((order, idx) => {
    const row = document.createElement('div');
    row.className = `pq-row ${idx === 0 ? 'is-highest' : ''}`;
    row.innerHTML = `
      <div class="pq-badge ${order.priority === 1 ? 'prio-vip' : 'prio-std'}">
        ${order.priority === 1 ? '👑 VIP (1)' : `Prio (${order.priority})`}
      </div>
      <div class="pq-title">${order.item}</div>
      ${idx === 0 ? '<span class="next-tag">NEXT TO PROCESS</span>' : ''}
    `;
    list.appendChild(row);
  });
}
