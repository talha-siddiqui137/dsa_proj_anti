#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "../ds/stack_ds.h"
#include "product.h"

// ─────────────────────────────────────────────────────────────
//  ShoppingCart – stack-based, LIFO item management
// ─────────────────────────────────────────────────────────────
class ShoppingCart {
private:
    StackDS<Product> stack;

public:
    ShoppingCart() : stack(64) {}

    // Push a product onto the cart stack
    void addToCart(const Product& p) {
        stack.push(p);
    }

    // Pop (remove last added item)
    Product removeLastItem() {
        if (stack.isEmpty())
            throw std::underflow_error("Cart is empty – nothing to remove");
        return stack.pop();
    }

    // Peek at the most recently added item
    const Product& viewTop() const {
        return stack.peek();
    }

    // Non-destructive snapshot of all cart items (bottom→top = oldest→newest)
    std::vector<Product> getItems() const {
        return stack.toVector();
    }

    // Sum of all item prices in the cart
    double getTotal() const {
        double total = 0.0;
        std::vector<Product> items = stack.toVector();
        for (const auto& p : items) total += p.price;
        return total;
    }

    bool isEmpty() const { return stack.isEmpty(); }
    int  size()    const { return stack.size(); }

    void clear() {
        while (!stack.isEmpty()) stack.pop();
    }

    // ── JSON ──────────────────────────────────────────────────
    std::string getCartJSON() const {
        std::ostringstream oss;
        std::vector<Product> items = stack.toVector();

        oss << "{";
        oss << "\"itemCount\":" << items.size() << ",";
        oss << "\"total\":"     << getTotal()    << ",";
        oss << "\"items\":[";

        // Output newest first (top of stack first)
        for (int i = (int)items.size() - 1; i >= 0; i--) {
            if (i < (int)items.size() - 1) oss << ",";
            oss << items[i].toJSON();
        }

        oss << "]}";
        return oss.str();
    }
};
