/* ════════════════════════════════════════════════════════
   app.js — SPA Router, Home, Shop, App Init
   DSA Store Frontend
   ════════════════════════════════════════════════════════ */

/* ─── TOAST NOTIFICATION SYSTEM ─── */
const Toast = {
  show(msg, type = 'info', duration = 3500) {
    const icons = { success: '✅', error: '❌', info: 'ℹ️', warning: '⚠️' };
    const container = document.getElementById('toastContainer');
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerHTML = `
      <span class="toast-icon">${icons[type] || 'ℹ️'}</span>
      <span class="toast-msg">${msg}</span>
    `;
    container.appendChild(toast);
    setTimeout(() => {
      toast.classList.add('hiding');
      setTimeout(() => toast.remove(), 350);
    }, duration);
  },
  success: (m, d) => Toast.show(m, 'success', d),
  error:   (m, d) => Toast.show(m, 'error',   d),
  info:    (m, d) => Toast.show(m, 'info',     d),
  warning: (m, d) => Toast.show(m, 'warning',  d),
};

/* ─── SPA ROUTER ─── */
const Router = {
  pages: {
    home:     { title: 'Home',            icon: '🏠', init: initHomePage },
    shop:     { title: 'Shop',            icon: '🛒', init: initShopPage },
    cart:     { title: 'Cart',            icon: '📦', init: initCartPage },
    orders:   { title: 'Orders',          icon: '📋', init: initOrdersPage },
    delivery: { title: 'Delivery Map',    icon: '🌐', init: initDeliveryPage },
    hanoi:    { title: 'Tower of Hanoi',  icon: '🗼', init: initHanoiGame },
    sorting:  { title: 'Sorting Race',    icon: '🏎️', init: initSortingRace },
    bst:      { title: 'BST Visualizer',  icon: '🌳', init: initBSTVisualizer },
    calc:     { title: 'Calculator',      icon: '🧮', init: initCalculator },
    queue:    { title: 'Queue Visualizer',icon: '📊', init: initQueueVisualizer },
  },

  navigate(pageId, pushState = true) {
    const page = this.pages[pageId];
    if (!page) return;

    // Hide all pages
    document.querySelectorAll('.page').forEach(p => p.classList.remove('active'));
    // Show target
    const target = document.getElementById(`page-${pageId}`);
    if (target) target.classList.add('active');

    // Update nav links
    document.querySelectorAll('.nav-link').forEach(link => {
      link.classList.toggle('active', link.dataset.page === pageId);
    });

    // Update title
    const titleEl = document.getElementById('topnavTitle');
    if (titleEl) titleEl.textContent = `${page.icon} ${page.title}`;

    // Push state
    if (pushState) history.pushState({ page: pageId }, '', `#${pageId}`);

    AppState.currentPage = pageId;

    // Close sidebar on mobile
    if (window.innerWidth <= 768) closeSidebar();

    // Call page init
    if (typeof page.init === 'function') page.init();
  },
};

/* ─── SIDEBAR CONTROLS ─── */
function openSidebar() {
  document.getElementById('sidebar').classList.add('open');
  document.getElementById('sidebarOverlay').classList.add('open');
  document.body.style.overflow = 'hidden';
}

function closeSidebar() {
  document.getElementById('sidebar').classList.remove('open');
  document.getElementById('sidebarOverlay').classList.remove('open');
  document.body.style.overflow = '';
}

/* ─── CART COUNT SYNC ─── */
function syncCartCount() {
  const count = AppState.cart.length;
  document.querySelectorAll('#cartBadge, #cartCount').forEach(el => {
    el.textContent = count;
    el.style.display = count > 0 ? '' : '';
  });
}

/* ─── HOME PAGE ─── */
async function initHomePage() {
  // Load products if not loaded
  if (!AppState.products.length) {
    AppState.products = await fetchProducts();
  }

  // Stats
  document.getElementById('statProducts').textContent = AppState.products.length;
  document.getElementById('statCart').textContent = AppState.cart.length;
  document.getElementById('statOrders').textContent = AppState.orders.length;

  // Featured products (top 4 by rating)
  const featured = [...AppState.products]
    .sort((a, b) => b.rating - a.rating)
    .slice(0, 4);
  renderProductGrid(featured, 'featuredGrid');
}

/* ─── SHOP PAGE ─── */
let shopSearchDebounced;

async function initShopPage() {
  if (!AppState.products.length) {
    AppState.products = await fetchProducts();
  }
  renderShopGrid(AppState.products);
  bindShopControls();
}

function renderShopGrid(products) {
  const grid  = document.getElementById('shopGrid');
  const empty = document.getElementById('shopEmpty');
  grid.innerHTML = '';

  if (!products.length) {
    grid.style.display = 'none';
    empty.classList.remove('hidden');
    return;
  }

  grid.style.display = '';
  empty.classList.add('hidden');
  products.forEach(p => {
    grid.appendChild(createProductCard(p));
  });
}

function bindShopControls() {
  const searchInput = document.getElementById('shopSearch');
  const catFilter   = document.getElementById('categoryFilter');
  const sortFilter  = document.getElementById('sortFilter');
  const refreshBtn  = document.getElementById('refreshProducts');

  const applyFilters = () => {
    const q   = searchInput.value.toLowerCase().trim();
    const cat = catFilter.value;
    const srt = sortFilter.value;

    let products = [...AppState.products];
    if (q)   products = products.filter(p => p.name.toLowerCase().includes(q) || p.category.toLowerCase().includes(q));
    if (cat) products = products.filter(p => p.category === cat);

    switch (srt) {
      case 'price-asc':   products.sort((a,b) => a.price - b.price); break;
      case 'price-desc':  products.sort((a,b) => b.price - a.price); break;
      case 'name-asc':    products.sort((a,b) => a.name.localeCompare(b.name)); break;
      case 'rating-desc': products.sort((a,b) => b.rating - a.rating); break;
    }
    renderShopGrid(products);
  };

  if (!shopSearchDebounced) shopSearchDebounced = debounce(applyFilters, 300);

  // Remove previous listeners by cloning (simple approach for SPA reinit)
  searchInput.oninput = shopSearchDebounced;
  catFilter.onchange  = applyFilters;
  sortFilter.onchange = applyFilters;

  // Category pills
  document.querySelectorAll('#categoryPills .pill').forEach(pill => {
    pill.onclick = () => {
      document.querySelectorAll('#categoryPills .pill').forEach(p => p.classList.remove('active'));
      pill.classList.add('active');
      catFilter.value = pill.dataset.cat;
      applyFilters();
    };
  });

  if (refreshBtn) {
    refreshBtn.onclick = async () => {
      refreshBtn.textContent = '⏳ Refreshing…';
      AppState.products = await fetchProducts();
      applyFilters();
      refreshBtn.textContent = '↺ Refresh';
      Toast.success('Products refreshed!');
    };
  }
}

/* ─── PRODUCT CARD ─── */
function createProductCard(product) {
  const card = document.createElement('div');
  card.className = 'product-card';
  card.innerHTML = `
    <div class="product-img">
      ${product.emoji || productEmoji(product.category)}
      ${product.badge ? `<span class="product-badge ${product.badge}">${product.badge}</span>` : ''}
    </div>
    <div class="product-body">
      <div class="product-cat">${product.category}</div>
      <div class="product-name">${product.name}</div>
      <div class="product-stars">${starsHTML(product.rating)} <span style="color:var(--text-muted);font-size:0.72rem">(${product.reviews?.toLocaleString() || 0})</span></div>
      <div class="product-footer">
        <div>
          <div class="product-price">${formatPrice(product.price)}</div>
          ${product.oldPrice ? `<div class="product-price-old">${formatPrice(product.oldPrice)}</div>` : ''}
        </div>
        <button class="add-to-cart-btn" data-id="${product.id}">+ Cart</button>
      </div>
    </div>
  `;

  card.querySelector('.add-to-cart-btn').addEventListener('click', e => {
    e.stopPropagation();
    addToCart(product);
  });

  card.addEventListener('click', () => openProductModal(product));
  return card;
}

function renderProductGrid(products, containerId) {
  const container = document.getElementById(containerId);
  if (!container) return;
  container.innerHTML = '';
  products.forEach(p => container.appendChild(createProductCard(p)));
}

/* ─── PRODUCT MODAL ─── */
function openProductModal(product) {
  const modal = document.getElementById('productModal');
  const body  = document.getElementById('modalBody');

  const discount = product.oldPrice
    ? Math.round((1 - product.price / product.oldPrice) * 100)
    : null;

  body.innerHTML = `
    <div class="modal-product-img">${product.emoji || '📦'}</div>
    <div class="product-cat">${product.category}</div>
    <div class="modal-product-name">${product.name}</div>
    <div class="product-stars">${starsHTML(product.rating)} (${product.reviews?.toLocaleString() || 0} reviews)</div>
    <p class="modal-product-desc">${product.description || 'No description available.'}</p>
    <div class="modal-product-meta">
      ${product.badge ? `<div class="modal-meta-item"><strong>${product.badge.toUpperCase()}</strong></div>` : ''}
      ${discount ? `<div class="modal-meta-item"><strong>${discount}% OFF</strong></div>` : ''}
      <div class="modal-meta-item">In Stock: <strong>✅ Yes</strong></div>
      <div class="modal-meta-item">Ships: <strong>2-5 days</strong></div>
    </div>
    <div class="modal-price">${formatPrice(product.price)}${product.oldPrice ? ` <span style="font-size:1rem;color:var(--text-muted);text-decoration:line-through">${formatPrice(product.oldPrice)}</span>` : ''}</div>
    <div class="modal-actions">
      <button class="btn btn-primary" id="modalAddBtn">🛒 Add to Cart</button>
      <button class="btn btn-ghost" id="modalCloseBtn2">✕ Close</button>
    </div>
  `;

  document.getElementById('modalAddBtn').onclick = () => {
    addToCart(product);
    modal.classList.remove('open');
  };
  document.getElementById('modalCloseBtn2').onclick = () => modal.classList.remove('open');

  modal.classList.add('open');
}

/* ─── ADD TO CART ─── */
function addToCart(product) {
  const cartProduct = { ...product, qty: 1, addedAt: Date.now() };
  cartPush(cartProduct);
  syncCartCount();
  apiAddToCart(product.id);
  Toast.success(`🛒 ${product.name} added to cart!`);

  // Animate cart button
  const cartBtn = document.querySelector('.cart-btn');
  if (cartBtn) {
    cartBtn.style.transform = 'scale(1.2)';
    setTimeout(() => { cartBtn.style.transform = ''; }, 200);
  }

  // Refresh cart page if open
  if (AppState.currentPage === 'cart') initCartPage();
}

/* ─── ORDERS PAGE ─── */
async function initOrdersPage() {
  const serverOrders = await fetchOrders();
  if (serverOrders.length && AppState.orders.length === 0) {
    AppState.orders = serverOrders;
  }
  renderOrdersList();
  renderOrdersQueue();
}

function renderOrdersList() {
  const list = document.getElementById('ordersList');
  if (!list) return;

  if (!AppState.orders.length) {
    list.innerHTML = `<div class="order-empty"><div class="empty-icon">📦</div><h3>No orders yet</h3><p>Place your first order from the cart!</p></div>`;
    return;
  }

  list.innerHTML = '';
  [...AppState.orders].reverse().forEach(order => {
    const card = document.createElement('div');
    card.className = 'order-card';
    card.innerHTML = `
      <div class="order-header">
        <span class="order-id">🧾 ${order.id}</span>
        <span class="order-status ${order.status}">${order.status}</span>
      </div>
      <div class="order-items">${order.items?.map(i => i.name).join(', ').substring(0, 80) || 'Items'}</div>
      <div class="order-total">${formatPrice(order.total)}</div>
      <div style="font-size:0.72rem;color:var(--text-muted);margin-top:4px">${new Date(order.timestamp).toLocaleString()}</div>
    `;
    list.appendChild(card);
  });
}

function renderOrdersQueue() {
  const track = document.getElementById('ordersTrack');
  const size  = document.getElementById('queueSize');
  if (!track) return;

  if (size) size.textContent = AppState.orders.length;

  if (!AppState.orders.length) {
    track.innerHTML = '<div class="queue-empty-msg" style="color:var(--text-muted);font-size:0.82rem;padding:20px">No orders in queue</div>';
    return;
  }

  track.innerHTML = '';
  AppState.orders.forEach((order, i) => {
    const item = document.createElement('div');
    item.className = 'queue-item';
    item.innerHTML = `
      <div class="q-id">#${i + 1}</div>
      <div style="font-size:0.72rem;color:var(--text-muted)">${order.id?.slice(-6) || '—'}</div>
      <div class="q-items">${formatPrice(order.total)}</div>
    `;
    track.appendChild(item);
  });
}

/* ─── GLOBAL SEARCH ─── */
function initGlobalSearch() {
  const input = document.getElementById('globalSearch');
  if (!input) return;
  input.addEventListener('keydown', e => {
    if (e.key === 'Enter') {
      const q = input.value.trim();
      if (q) {
        Router.navigate('shop');
        setTimeout(() => {
          const shopSearch = document.getElementById('shopSearch');
          if (shopSearch) { shopSearch.value = q; shopSearch.dispatchEvent(new Event('input')); }
        }, 100);
        input.value = '';
      }
    }
  });
}

/* ─── DELIVERY PAGE STUB ─── */
function initDeliveryPage() {
  if (typeof initDeliveryMap === 'function') initDeliveryMap();
}

/* ─── STUB INIT FUNCTIONS (other files handle them) ─── */
function initCartPage()        { if (typeof renderCartStack   === 'function') renderCartStack(); }
function initHanoiGame()       { if (typeof setupHanoiGame    === 'function') setupHanoiGame(); }
function initSortingRace()     { if (typeof setupSortingRace  === 'function') setupSortingRace(); }
function initBSTVisualizer()   { if (typeof setupBSTVisualizer=== 'function') setupBSTVisualizer(); }
function initCalculator()      { if (typeof setupCalculator   === 'function') setupCalculator(); }
function initQueueVisualizer() { if (typeof setupQueueViz     === 'function') setupQueueViz(); }

/* ─── APP INITIALIZATION ─── */
document.addEventListener('DOMContentLoaded', async () => {
  // Restore persisted state
  loadPersistedState();
  syncCartCount();

  // Sidebar controls
  document.getElementById('hamburgerBtn')?.addEventListener('click', openSidebar);
  document.getElementById('sidebarClose')?.addEventListener('click', closeSidebar);
  document.getElementById('sidebarOverlay')?.addEventListener('click', closeSidebar);

  // Nav links
  document.querySelectorAll('.nav-link[data-page]').forEach(link => {
    link.addEventListener('click', e => {
      e.preventDefault();
      Router.navigate(link.dataset.page);
    });
  });

  // Buttons with data-page
  document.addEventListener('click', e => {
    const btn = e.target.closest('[data-page]:not(.nav-link)');
    if (btn) Router.navigate(btn.dataset.page);
  });

  // Cart button in topnav
  document.querySelector('.cart-btn')?.addEventListener('click', () => Router.navigate('cart'));

  // Modal close
  document.getElementById('modalClose')?.addEventListener('click', () => {
    document.getElementById('productModal')?.classList.remove('open');
  });
  document.getElementById('productModal')?.addEventListener('click', e => {
    if (e.target === e.currentTarget) e.currentTarget.classList.remove('open');
  });

  // Hash-based routing
  const hash = location.hash.slice(1);
  if (hash && Router.pages[hash]) {
    Router.navigate(hash, false);
  } else {
    Router.navigate('home', false);
  }

  // Browser back/forward
  window.addEventListener('popstate', e => {
    const page = e.state?.page || 'home';
    Router.navigate(page, false);
  });

  // Theme toggle
  const themeToggle = document.getElementById('themeToggle');
  let darkMode = true;
  themeToggle?.addEventListener('click', () => {
    darkMode = !darkMode;
    themeToggle.textContent = darkMode ? '🌙' : '☀️';
    // Light mode stub (full light mode would need CSS vars override)
    Toast.info(darkMode ? 'Dark mode activated' : 'Light mode coming soon!');
  });

  // Global search
  initGlobalSearch();

  // Checkout button
  document.getElementById('checkoutBtn')?.addEventListener('click', handleCheckout);
  document.getElementById('clearCartBtn')?.addEventListener('click', handleClearCart);
  document.getElementById('applyPromo')?.addEventListener('click', handlePromo);

  // Ping server
  pingServer();
});

async function pingServer() {
  try {
    await api.get('/health');
    updateServerStatus(true);
    AppState.isOnline = true;
    AppState.demoMode = false;
    Toast.success('🟢 Connected to live server!', 4000);
  } catch {
    updateServerStatus(false);
    Toast.info('📴 Running in Demo Mode (mock data)', 4000);
  }
}

/* ─── CHECKOUT ─── */
async function handleCheckout() {
  if (!AppState.cart.length) {
    Toast.warning('Your cart is empty!');
    return;
  }

  const total = cartTotal();
  const orderData = {
    items: [...AppState.cart],
    total,
    shipping: total > 50 ? 0 : 4.99,
    tax: total * 0.08,
  };

  try {
    const btn = document.getElementById('checkoutBtn');
    btn.textContent = '⏳ Processing…';
    btn.disabled = true;

    const order = await apiPlaceOrder(orderData);
    const finalOrder = {
      id: order.id || 'ORD-' + Date.now().toString(36).toUpperCase(),
      items: [...AppState.cart],
      total,
      status: 'pending',
      timestamp: new Date().toISOString(),
    };

    ordersEnqueue(finalOrder);
    cartClear();
    syncCartCount();
    apiClearCart();

    Toast.success(`✅ Order ${finalOrder.id} placed! Redirecting…`, 3000);
    setTimeout(() => Router.navigate('orders'), 1500);
  } catch (err) {
    Toast.error('Checkout failed. Please try again.');
  } finally {
    const btn = document.getElementById('checkoutBtn');
    if (btn) { btn.textContent = '🚀 Checkout'; btn.disabled = false; }
  }
}

function handleClearCart() {
  if (!AppState.cart.length) { Toast.warning('Cart is already empty!'); return; }
  cartClear();
  syncCartCount();
  apiClearCart();
  Toast.info('Cart cleared');
  if (AppState.currentPage === 'cart') initCartPage();
}

function handlePromo() {
  const code = document.getElementById('promoInput')?.value.trim().toUpperCase();
  const PROMOS = { 'DSA10': 10, 'ALGO20': 20, 'STACK15': 15 };
  if (PROMOS[code]) {
    Toast.success(`Promo "${code}" applied: ${PROMOS[code]}% off!`);
  } else {
    Toast.error('Invalid promo code');
  }
}
