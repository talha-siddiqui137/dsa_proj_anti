/* ==========================================================================
   store.js — Global state management & API layer
   DSA Store Frontend
   ========================================================================== */

const API_BASE = 'http://localhost:8080/api';

/* ─── GLOBAL STATE ─────────────────────────────────────── */
const AppState = {
  products: [],
  cart: [],       // Stack (LIFO)
  orders: [],     // Queue (FIFO)
  currentPage: 'home',
  isOnline: false,
  demoMode: true,
};

/* ─── MOCK DATA (20 products across 5 categories) ──────── */
const MOCK_PRODUCTS = [
  // Electronics
  { id: 1,  name: 'Quantum Wireless Earbuds',    category: 'Electronics', price: 79.99,  oldPrice: 129.99, rating: 4.8, reviews: 2341, emoji: '🎧', badge: 'hot',  description: 'Immersive 3D spatial audio with 36hr battery life, ANC, and crystal-clear mics.' },
  { id: 2,  name: 'UltraSlim Laptop Pro 15"',    category: 'Electronics', price: 1299.99,oldPrice: 1599.99,rating: 4.9, reviews: 892,  emoji: '💻', badge: 'new',  description: 'AMD Ryzen 9, 32GB RAM, 1TB NVMe SSD. The perfect developer companion.' },
  { id: 3,  name: 'Smart LED Gaming Monitor',    category: 'Electronics', price: 349.99, oldPrice: 449.99, rating: 4.7, reviews: 1205, emoji: '🖥️', badge: 'sale', description: '27" 4K 165Hz IPS panel with 1ms response and HDR10+ support.' },
  { id: 4,  name: 'Mechanical Keyboard RGB',     category: 'Electronics', price: 119.99, oldPrice: null,   rating: 4.6, reviews: 3100, emoji: '⌨️', badge: null,   description: 'Cherry MX Red switches, full RGB, PBT keycaps, USB-C detachable cable.' },
  { id: 5,  name: 'Wireless Charging Pad Pro',   category: 'Electronics', price: 39.99,  oldPrice: 59.99,  rating: 4.3, reviews: 4520, emoji: '⚡', badge: 'sale', description: '15W fast wireless charging, compatible with all Qi devices.' },

  // Clothing
  { id: 6,  name: 'Algorithm Hoodie (Dark)',      category: 'Clothing',    price: 54.99,  oldPrice: null,   rating: 4.9, reviews: 1780, emoji: '🧥', badge: 'hot',  description: 'Embroidered binary tree on back. 100% organic cotton, unisex fit.' },
  { id: 7,  name: 'DSA Tee — Stack Overflow',     category: 'Clothing',    price: 24.99,  oldPrice: null,   rating: 4.7, reviews: 3201, emoji: '👕', badge: 'new',  description: 'Soft premium cotton tee with stack-data-structure graphic. True to size.' },
  { id: 8,  name: 'Coder Joggers',                category: 'Clothing',    price: 44.99,  oldPrice: 64.99,  rating: 4.5, reviews: 890,  emoji: '👖', badge: null,   description: 'Slim-fit tech joggers with deep pockets. Perfect for long coding sessions.' },
  { id: 9,  name: 'Circuit Board Snapback',       category: 'Clothing',    price: 29.99,  oldPrice: null,   rating: 4.4, reviews: 512,  emoji: '🧢', badge: null,   description: 'Embroidered PCB pattern, adjustable, one-size-fits-all.' },

  // Books
  { id: 10, name: 'CLRS — Algorithms 4th Ed.',   category: 'Books',       price: 89.99,  oldPrice: 120.00, rating: 5.0, reviews: 8900, emoji: '📚', badge: 'hot',  description: 'The gold standard. Cormen, Leiserson, Rivest, Stein. Hardcover, 1200+ pages.' },
  { id: 11, name: 'Clean Code by R. Martin',      category: 'Books',       price: 34.99,  oldPrice: null,   rating: 4.8, reviews: 12000,emoji: '📖', badge: null,   description: 'A handbook of agile software craftsmanship. Every dev must read this.' },
  { id: 12, name: 'The Pragmatic Programmer',     category: 'Books',       price: 39.99,  oldPrice: 49.99,  rating: 4.9, reviews: 7800, emoji: '📘', badge: 'sale', description: '20th anniversary edition. Tips and tricks from veterans to make you a better dev.' },
  { id: 13, name: 'Cracking the Coding Interview',category: 'Books',       price: 29.99,  oldPrice: null,   rating: 4.6, reviews: 15200,emoji: '📗', badge: 'new',  description: '189 programming questions & solutions. FAANG interview bible.' },

  // Sports
  { id: 14, name: 'Ergonomic Gaming Chair',       category: 'Sports',      price: 299.99, oldPrice: 399.99, rating: 4.6, reviews: 2340, emoji: '💺', badge: 'sale', description: 'Lumbar support, adjustable armrests, breathable mesh. Up to 150kg.' },
  { id: 15, name: 'Resistance Band Set (6-pack)', category: 'Sports',      price: 24.99,  oldPrice: null,   rating: 4.5, reviews: 6700, emoji: '🏋️', badge: null,   description: 'Progressive resistance 10–50 lbs. Latex-free, door anchor included.' },
  { id: 16, name: 'Smart Fitness Tracker Pro',    category: 'Sports',      price: 89.99,  oldPrice: 129.99, rating: 4.7, reviews: 3400, emoji: '⌚', badge: 'sale', description: 'Heart rate, SpO2, sleep tracking, GPS, 7-day battery. IP68 waterproof.' },
  { id: 17, name: 'Under-Desk Treadmill',         category: 'Sports',      price: 449.99, oldPrice: 599.99, rating: 4.4, reviews: 890,  emoji: '🏃', badge: 'hot',  description: 'Walk while you code! Quiet motor, 0–4 km/h, fold-flat design.' },

  // Food
  { id: 18, name: 'Nootropic Focus Blend',        category: 'Food',        price: 49.99,  oldPrice: null,   rating: 4.3, reviews: 2100, emoji: '🧪', badge: 'new',  description: 'L-theanine + Caffeine + Lion\'s Mane. Cognitive boost without the jitters.' },
  { id: 19, name: 'Cold Brew Coffee Pack (12)',   category: 'Food',        price: 29.99,  oldPrice: 39.99,  rating: 4.8, reviews: 5600, emoji: '☕', badge: 'sale', description: 'Smooth Colombian single-origin cold brew, ready-to-drink cans. Stay caffeinated.' },
  { id: 20, name: 'Programmer\'s Snack Box',      category: 'Food',        price: 34.99,  oldPrice: null,   rating: 4.9, reviews: 3200, emoji: '🍱', badge: 'hot',  description: '24 premium snacks curated for long coding sessions. International mix.' },
];

/* ─── API HELPER ───────────────────────────────────────── */
const api = {
  async request(endpoint, options = {}) {
    try {
      const res = await fetch(`${API_BASE}${endpoint}`, {
        headers: { 'Content-Type': 'application/json', ...options.headers },
        ...options,
      });
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      return await res.json();
    } catch (err) {
      console.warn(`[API] ${endpoint} failed:`, err.message, '— using local state/mock');
      throw err;
    }
  },

  get:    (ep)       => api.request(ep),
  post:   (ep, body) => api.request(ep, { method: 'POST',   body: JSON.stringify(body) }),
  put:    (ep, body) => api.request(ep, { method: 'PUT',    body: JSON.stringify(body) }),
  delete: (ep)       => api.request(ep, { method: 'DELETE' }),
};

/* ─── API FUNCTIONS ────────────────────────────────────── */
async function fetchProducts() {
  try {
    const data = await api.get('/products');
    AppState.isOnline = true;
    AppState.demoMode = false;
    updateServerStatus(true);
    return Array.isArray(data) ? data : (data.products || MOCK_PRODUCTS);
  } catch {
    updateServerStatus(false);
    return MOCK_PRODUCTS;
  }
}

async function fetchCart() {
  try {
    const data = await api.get('/cart');
    return Array.isArray(data) ? data : (data.items || []);
  } catch {
    return AppState.cart;
  }
}

async function apiAddToCart(productId, quantity = 1) {
  try {
    await api.post('/cart/add', { product_id: productId });
  } catch { /* local state fallback */ }
}

async function apiRemoveFromCart() {
  try {
    await api.post('/cart/remove', {});
  } catch { /* local state fallback */ }
}

async function fetchOrders() {
  try {
    const data = await api.get('/orders/all');
    return Array.isArray(data) ? data : (data.orders || []);
  } catch {
    return AppState.orders;
  }
}

async function apiPlaceOrder(orderData) {
  try {
    return await api.post('/orders/place', orderData);
  } catch {
    const newOrder = {
      id: 'ORD-' + Date.now().toString(36).toUpperCase(),
      items: [...AppState.cart],
      total: orderData.total,
      status: 'pending',
      timestamp: new Date().toISOString(),
    };
    return newOrder;
  }
}

/* ─── SERVER STATUS UI ─────────────────────────────────── */
function updateServerStatus(online) {
  const dot  = document.querySelector('.status-dot');
  const text = document.querySelector('.status-text');
  if (!dot || !text) return;
  if (online) {
    dot.className  = 'status-dot online';
    text.textContent = 'Live Server (Port 8080)';
  } else {
    dot.className  = 'status-dot offline';
    text.textContent = 'Demo Mode (Offline)';
  }
}

/* ─── CART STATE HELPERS (Stack operations) ────────────── */
function cartPush(product) {
  AppState.cart.push(product);
  persistCart();
}

function cartPop() {
  const item = AppState.cart.pop();
  persistCart();
  return item;
}

function cartPeek() {
  return AppState.cart[AppState.cart.length - 1] || null;
}

function cartRemoveById(id) {
  const idx = AppState.cart.findIndex(p => p.id === id);
  if (idx !== -1) {
    AppState.cart.splice(idx, 1);
    persistCart();
    return true;
  }
  return false;
}

function cartClear() {
  AppState.cart = [];
  persistCart();
}

function cartTotal() {
  return AppState.cart.reduce((sum, p) => sum + p.price * (p.qty || 1), 0);
}

/* ─── ORDERS STATE HELPERS (Queue operations) ──────────── */
function ordersEnqueue(order) {
  AppState.orders.push(order);
  persistOrders();
}

function ordersDequeue() {
  const order = AppState.orders.shift();
  persistOrders();
  return order;
}

/* ─── LOCAL STORAGE PERSISTENCE ────────────────────────── */
function persistCart() {
  try { localStorage.setItem('dsa_cart', JSON.stringify(AppState.cart)); } catch {}
}

function persistOrders() {
  try { localStorage.setItem('dsa_orders', JSON.stringify(AppState.orders)); } catch {}
}

function loadPersistedState() {
  try {
    const cart = localStorage.getItem('dsa_cart');
    if (cart) AppState.cart = JSON.parse(cart);
    const orders = localStorage.getItem('dsa_orders');
    if (orders) AppState.orders = JSON.parse(orders);
  } catch {}
}

/* ─── UTILITY HELPERS ──────────────────────────────────── */
function formatPrice(p) { return '$' + Number(p).toFixed(2); }

function starsHTML(rating) {
  const full  = Math.floor(rating);
  const half  = rating % 1 >= 0.5 ? 1 : 0;
  const empty = 5 - full - half;
  return '★'.repeat(full) + (half ? '½' : '') + '☆'.repeat(empty) + ` (${rating})`;
}

function productEmoji(category) {
  const map = { Electronics:'📱', Clothing:'👕', Books:'📚', Sports:'🏃', Food:'☕' };
  return map[category] || '📦';
}

function debounce(fn, ms) {
  let t;
  return (...args) => { clearTimeout(t); t = setTimeout(() => fn(...args), ms); };
}

// Initialize on load
loadPersistedState();
