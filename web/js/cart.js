/* ==========================================================================
   cart.js — Shopping Cart with Interactive Stack (LIFO) Visualization
   SE-213 Data Structures & Algorithms Project
   ========================================================================== */

let promoDiscount = 0;
let promoCodeApplied = '';

function logStackOp(op, detail) {
  const log = document.getElementById('opsLog');
  if (!log) return;
  const time = new Date().toLocaleTimeString();
  const entry = document.createElement('div');
  entry.className = 'log-entry';
  entry.innerHTML = `<span style="color: var(--accent, #6c63ff); font-weight: 600;">[${time}]</span> <strong>${op}</strong>: ${detail}`;
  log.prepend(entry);
  while (log.children.length > 20) {
    log.removeChild(log.lastChild);
  }
}

function renderCartStack() {
  const stackItems = document.getElementById('stackItems');
  const stackEmpty = document.getElementById('stackEmpty');
  const stackPointer = document.getElementById('stackPointer');
  const stackSize = document.getElementById('stackSize');
  const subtotalEl = document.getElementById('subtotal');
  const shippingEl = document.getElementById('shipping');
  const taxEl = document.getElementById('tax');
  const cartTotalEl = document.getElementById('cartTotal');

  if (!stackItems) return;

  const count = AppState.cart.length;
  if (stackSize) stackSize.textContent = count;
  syncCartCount();

  if (count === 0) {
    if (stackEmpty) stackEmpty.style.display = 'flex';
    if (stackPointer) stackPointer.style.display = 'none';
    stackItems.innerHTML = '';
    if (subtotalEl) subtotalEl.textContent = '$0.00';
    if (shippingEl) shippingEl.textContent = '$0.00';
    if (taxEl) taxEl.textContent = '$0.00';
    if (cartTotalEl) cartTotalEl.textContent = '$0.00';
    return;
  }

  if (stackEmpty) stackEmpty.style.display = 'none';
  if (stackPointer) stackPointer.style.display = 'block';

  // Render from TOP (highest index) down to BASE (index 0)
  stackItems.innerHTML = '';
  for (let i = count - 1; i >= 0; i--) {
    const item = AppState.cart[i];
    const isTop = (i === count - 1);
    
    const card = document.createElement('div');
    card.className = `stack-item-card ${isTop ? 'is-top' : ''}`;
    card.style.animationDelay = `${(count - 1 - i) * 0.05}s`;
    
    card.innerHTML = `
      <div class="stack-card-left">
        <span class="stack-level-badge">LIFO #${i}</span>
        <span class="stack-emoji">${item.emoji || '📦'}</span>
        <div class="stack-card-details">
          <div class="stack-card-title">${item.name}</div>
          <div class="stack-card-meta">${item.category} • <strong class="text-success">${formatPrice(item.price)}</strong></div>
        </div>
      </div>
      <div class="stack-card-actions">
        ${isTop ? '<span class="top-tag">⚡ TOP (PEEK)</span>' : ''}
        <button class="btn btn-sm btn-danger pop-btn" data-index="${i}" title="${isTop ? 'Pop from Top of Stack' : 'Remove item'}">
          ${isTop ? '⏏ Pop' : '✕'}
        </button>
      </div>
    `;

    stackItems.appendChild(card);
  }

  // Calculate totals
  const subtotal = cartTotal();
  const discountAmt = subtotal * promoDiscount;
  const taxable = Math.max(0, subtotal - discountAmt);
  const tax = taxable * 0.08;
  const shipping = subtotal > 150 || subtotal === 0 ? 0.0 : 9.99;
  const grandTotal = taxable + tax + shipping;

  if (subtotalEl) subtotalEl.textContent = formatPrice(subtotal);
  if (shippingEl) shippingEl.textContent = shipping === 0 ? 'FREE' : formatPrice(shipping);
  if (taxEl) taxEl.textContent = formatPrice(tax);
  if (cartTotalEl) {
    if (promoDiscount > 0) {
      cartTotalEl.innerHTML = `<span style="text-decoration: line-through; opacity: 0.6; font-size: 0.85em;">${formatPrice(subtotal + tax + shipping)}</span> ${formatPrice(grandTotal)} <span class="badge-sm" style="background:#00d4aa; color:#000;">-${Math.round(promoDiscount * 100)}%</span>`;
    } else {
      cartTotalEl.textContent = formatPrice(grandTotal);
    }
  }

  // Attach pop event listeners
  stackItems.querySelectorAll('.pop-btn').forEach(btn => {
    btn.addEventListener('click', (e) => {
      e.stopPropagation();
      const idx = parseInt(btn.dataset.index, 10);
      popCartItem(idx);
    });
  });
}

function popCartItem(idx) {
  if (idx < 0 || idx >= AppState.cart.length) return;
  const isTop = (idx === AppState.cart.length - 1);
  const item = AppState.cart[idx];

  if (isTop) {
    const popped = cartPop();
    logStackOp('POP', `Popped "${popped.name}" ($${popped.price}) from Stack top. Remaining: ${AppState.cart.length}`);
    Toast.info(`Popped "${popped.name}" from Cart Stack (LIFO)`);
  } else {
    // If user removes item below top: simulate unwinding stack or direct splice
    AppState.cart.splice(idx, 1);
    persistCart();
    logStackOp('DELETE_AT', `Removed "${item.name}" from level #${idx}. Remaining: ${AppState.cart.length}`);
    Toast.warning(`Removed "${item.name}" from stack level #${idx}`);
  }

  renderCartStack();
}

function pushToCartWithAnimation(product) {
  cartPush(product);
  logStackOp('PUSH', `Pushed "${product.name}" ($${product.price}) to Stack top. Size: ${AppState.cart.length}`);
  syncCartCount();
  Toast.success(`Added "${product.name}" to Stack (LIFO)`);
  
  if (AppState.currentPage === 'cart') {
    renderCartStack();
  }
}

// Global hook for promo codes
function applyPromoCode() {
  const input = document.getElementById('promoInput');
  if (!input) return;
  const code = input.value.trim().toUpperCase();
  if (code === 'DSA10') {
    promoDiscount = 0.10;
    promoCodeApplied = 'DSA10';
    Toast.success('Promo DSA10 applied! 10% discount added.');
    logStackOp('PROMO', 'Applied coupon DSA10 (10% off)');
  } else if (code === 'BINARY20' || code === 'STACK20') {
    promoDiscount = 0.20;
    promoCodeApplied = code;
    Toast.success(`Promo ${code} applied! 20% discount added.`);
    logStackOp('PROMO', `Applied coupon ${code} (20% off)`);
  } else if (code === 'ALGO50') {
    promoDiscount = 0.50;
    promoCodeApplied = 'ALGO50';
    Toast.success('Mega Promo ALGO50 applied! 50% discount added.');
    logStackOp('PROMO', 'Applied coupon ALGO50 (50% off)');
  } else {
    Toast.error('Invalid promo code. Try DSA10, BINARY20, or ALGO50!');
    return;
  }
  renderCartStack();
}

// Wire Cart page controls on DOMContentLoaded
document.addEventListener('DOMContentLoaded', () => {
  const checkoutBtn = document.getElementById('checkoutBtn');
  const clearCartBtn = document.getElementById('clearCartBtn');
  const applyPromoBtn = document.getElementById('applyPromo');
  const promoInput = document.getElementById('promoInput');

  if (checkoutBtn) {
    checkoutBtn.addEventListener('click', () => {
      if (AppState.cart.length === 0) {
        Toast.warning('Your cart stack is empty! Add products first.');
        return;
      }
      
      const subtotal = cartTotal();
      const discount = subtotal * promoDiscount;
      const total = Math.max(0, subtotal - discount) * 1.08;
      
      const newOrder = {
        id: 'ORD-' + Math.floor(1000 + Math.random() * 9000),
        items: [...AppState.cart],
        itemCount: AppState.cart.length,
        total: total,
        status: 'Processing',
        timestamp: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }),
        priority: Math.random() > 0.6 ? 1 : 3, // VIP vs Standard
        city: 'Karachi'
      };

      // Push into Order Queue (FIFO)
      ordersEnqueue(newOrder);
      logStackOp('CHECKOUT', `Processed ${AppState.cart.length} items -> Enqueued to Order Queue (FIFO) as #${newOrder.id}`);
      
      // Clear Stack
      cartClear();
      promoDiscount = 0;
      if (promoInput) promoInput.value = '';
      
      renderCartStack();
      Toast.success(`Order #${newOrder.id} placed! Enqueued to Order Queue (FIFO).`);
      
      // Navigate to orders page to see queue in action!
      setTimeout(() => {
        Router.navigate('orders');
      }, 700);
    });
  }

  if (clearCartBtn) {
    clearCartBtn.addEventListener('click', () => {
      if (AppState.cart.length === 0) {
        Toast.info('Cart stack is already empty.');
        return;
      }
      const count = AppState.cart.length;
      cartClear();
      logStackOp('CLEAR_STACK', `Popped all ${count} items from stack.`);
      Toast.info(`Cart stack cleared (${count} items removed).`);
      renderCartStack();
    });
  }

  if (applyPromoBtn) {
    applyPromoBtn.addEventListener('click', applyPromoCode);
  }
  if (promoInput) {
    promoInput.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') applyPromoCode();
    });
  }
});
