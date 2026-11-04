#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include "../ds/queue_ds.h"
#include "product.h"

// ─────────────────────────────────────────────────────────────
//  Order struct
// ─────────────────────────────────────────────────────────────
struct Order {
    int                  id;
    std::string          customerName;
    std::vector<Product> items;
    double               total;
    int                  priority;   // lower = higher urgency
    std::string          status;     // "pending", "processing", "completed"
    bool                 isVIP;
    bool                 isFlashSale;

    Order()
        : id(0), total(0.0), priority(10), status("pending"),
          isVIP(false), isFlashSale(false) {}

    Order(int id_, const std::string& customer, const std::vector<Product>& items_,
          int priority_ = 10, bool vip = false, bool flash = false)
        : id(id_), customerName(customer), items(items_),
          priority(priority_), status("pending"), isVIP(vip), isFlashSale(flash) {
        total = 0.0;
        for (const auto& p : items_) total += p.price;
    }

    // Escape helper
    static std::string jsonEscape(const std::string& s) {
        std::string out; out.reserve(s.size());
        for (char c : s) {
            if      (c == '"')  out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else                out += c;
        }
        return out;
    }

    std::string toJSON() const {
        std::ostringstream oss;
        oss << "{"
            << "\"id\":"           << id              << ","
            << "\"customerName\":\"" << jsonEscape(customerName) << "\","
            << "\"total\":"        << total           << ","
            << "\"priority\":"     << priority        << ","
            << "\"status\":\""     << jsonEscape(status) << "\","
            << "\"isVIP\":"        << (isVIP       ? "true" : "false") << ","
            << "\"isFlashSale\":"  << (isFlashSale ? "true" : "false") << ","
            << "\"itemCount\":"    << items.size()    << ","
            << "\"items\":[";
        for (size_t i = 0; i < items.size(); i++) {
            if (i > 0) oss << ",";
            oss << items[i].toJSON();
        }
        oss << "]}";
        return oss.str();
    }

    // Allow comparison by priority for PriorityQueue heapification
    bool operator<(const Order& o) const { return priority < o.priority; }
    bool operator>(const Order& o) const { return priority > o.priority; }
    bool operator==(const Order& o) const { return id == o.id; }
};

// ─────────────────────────────────────────────────────────────
//  OrderManager
//  • VIP orders  → PriorityQueue<Order>  (min-heap on priority)
//  • Normal orders → SimpleQueue<Order>
//  • Flash-sale slots → CircularQueue<Order> (cap = 5)
// ─────────────────────────────────────────────────────────────
class OrderManager {
private:
    PriorityQueue<Order> vipQueue;       // VIP / high-priority orders
    SimpleQueue<Order>   normalQueue;    // Standard orders
    CircularQueue<Order> flashQueue;     // Flash-sale slot queue (fixed 5)
    int                  nextOrderId;

public:
    OrderManager() : flashQueue(5), nextOrderId(1) {}

    // ── Place order ──────────────────────────────────────────
    void placeOrder(Order order, bool isVIP = false, bool isFlashSale = false) {
        order.id         = nextOrderId++;
        order.isVIP      = isVIP;
        order.isFlashSale = isFlashSale;
        order.status     = "pending";

        if (isFlashSale) {
            if (flashQueue.isFull())
                throw std::overflow_error("Flash sale queue is full (max 5 slots)");
            flashQueue.enqueue(order);
        } else if (isVIP) {
            // VIP priority 1 (highest urgency); normal starts at 5
            int prio = (order.priority > 0) ? order.priority : 1;
            vipQueue.insert(order, prio);
        } else {
            int prio = (order.priority > 0) ? order.priority : 5;
            normalQueue.enqueue(order);
            (void)prio; // SimpleQueue is FIFO; priority stored in Order struct
        }
    }

    // ── Process next order ───────────────────────────────────
    // Priority: flash sale > VIP > normal
    Order processNextOrder() {
        if (!flashQueue.isEmpty()) {
            Order o = flashQueue.dequeue();
            o.status = "processing";
            return o;
        }
        if (!vipQueue.isEmpty()) {
            Order o = vipQueue.extractMin();
            o.status = "processing";
            return o;
        }
        if (!normalQueue.isEmpty()) {
            Order o = normalQueue.dequeue();
            o.status = "processing";
            return o;
        }
        throw std::runtime_error("No orders to process");
    }

    // ── Inspection helpers ───────────────────────────────────
    bool hasOrders() const {
        return !flashQueue.isEmpty() || !vipQueue.isEmpty() || !normalQueue.isEmpty();
    }

    int totalPending() const {
        return flashQueue.size() + vipQueue.size() + normalQueue.size();
    }

    // ── JSON ──────────────────────────────────────────────────
    std::string getOrdersJSON() const {
        std::ostringstream oss;
        oss << "{";

        // Flash queue snapshot
        oss << "\"flashQueue\":{";
        oss << "\"capacity\":5,\"size\":" << flashQueue.size() << ",\"orders\":[";
        std::vector<Order> fv = flashQueue.toVector();
        for (size_t i = 0; i < fv.size(); i++) {
            if (i > 0) oss << ",";
            oss << fv[i].toJSON();
        }
        oss << "]}";

        // VIP queue snapshot (heap order – not necessarily sorted)
        oss << ",\"vipQueue\":{";
        oss << "\"size\":" << vipQueue.size() << ",\"orders\":[";
        // PriorityQueue::toJSON already formats items
        // We need to traverse without modifying – make a copy
        PriorityQueue<Order> vipCopy = vipQueue;
        bool firstVip = true;
        while (!vipCopy.isEmpty()) {
            if (!firstVip) oss << ",";
            oss << vipCopy.extractMin().toJSON();
            firstVip = false;
        }
        oss << "]}";

        // Normal queue snapshot
        oss << ",\"normalQueue\":{";
        oss << "\"size\":" << normalQueue.size() << ",\"orders\":[";
        std::vector<Order> nv = normalQueue.toVector();
        for (size_t i = 0; i < nv.size(); i++) {
            if (i > 0) oss << ",";
            oss << nv[i].toJSON();
        }
        oss << "]}";

        oss << ",\"totalPending\":" << totalPending();
        oss << "}";
        return oss.str();
    }
};
