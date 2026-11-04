// =============================================================================
// main.cpp — DSA E-Commerce Server
// =============================================================================
// This file wires together:
//   • MinimalHTTPServer (include/httplib.h)
//   • ProductCatalog    (src/ecommerce/catalog.h)
//   • ShoppingCart      (src/ecommerce/cart.h)
//   • OrderManager      (src/ecommerce/orders.h)
//   • BST               (src/ds/tree_ds.h)
//   • Graph             (src/ds/graph_ds.h)
//   • StackDS           (src/ds/stack_ds.h)
//   • Tower of Hanoi    (src/algorithms/hanoi.h)
// =============================================================================

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

// Project headers
#include "../include/httplib.h"
#include "ds/array_ds.h"
#include "ds/stack_ds.h"
#include "ds/queue_ds.h"
#include "ds/tree_ds.h"
#include "ds/graph_ds.h"
#include "algorithms/hanoi.h"
#include "ecommerce/product.h"
#include "ecommerce/catalog.h"
#include "ecommerce/cart.h"
#include "ecommerce/orders.h"

// =============================================================================
// Tiny JSON helpers (no external library)
// =============================================================================
namespace json {

inline std::string escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;
        }
    }
    return out;
}

inline std::string str(const std::string& v) {
    return "\"" + escape(v) + "\"";
}

inline std::string kv(const std::string& key, const std::string& val_json) {
    return str(key) + ":" + val_json;
}

inline std::string kv_s(const std::string& key, const std::string& val) {
    return kv(key, str(val));
}

inline std::string kv_n(const std::string& key, double val) {
    std::ostringstream ss;
    ss << val;
    return kv(key, ss.str());
}

inline std::string kv_b(const std::string& key, bool val) {
    return kv(key, val ? "true" : "false");
}

inline std::string obj(const std::vector<std::string>& fields) {
    std::string out = "{";
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i) out += ",";
        out += fields[i];
    }
    out += "}";
    return out;
}

inline std::string arr(const std::vector<std::string>& items) {
    std::string out = "[";
    for (size_t i = 0; i < items.size(); ++i) {
        if (i) out += ",";
        out += items[i];
    }
    out += "]";
    return out;
}

// Very small JSON value extractor (works for flat integer fields)
inline std::string get_str_field(const std::string& body, const std::string& key) {
    // Looks for "key":"value"
    std::string needle = "\"" + key + "\"";
    size_t pos = body.find(needle);
    if (pos == std::string::npos) return "";
    pos = body.find(':', pos + needle.size());
    if (pos == std::string::npos) return "";
    ++pos;
    while (pos < body.size() && (body[pos] == ' ' || body[pos] == '\t')) ++pos;
    if (pos >= body.size()) return "";
    if (body[pos] == '"') {
        size_t start = pos + 1;
        size_t end   = body.find('"', start);
        if (end == std::string::npos) return "";
        return body.substr(start, end - start);
    }
    // Numeric / boolean
    size_t start = pos;
    size_t end   = start;
    while (end < body.size() && body[end] != ',' && body[end] != '}' && body[end] != ']')
        ++end;
    std::string val = body.substr(start, end - start);
    // trim
    while (!val.empty() && (val.back() == ' ' || val.back() == '\n' || val.back() == '\r'))
        val.pop_back();
    return val;
}

inline int get_int_field(const std::string& body, const std::string& key, int def = 0) {
    std::string v = get_str_field(body, key);
    if (v.empty()) return def;
    try { return std::stoi(v); } catch (...) { return def; }
}

inline bool get_bool_field(const std::string& body, const std::string& key, bool def = false) {
    std::string v = get_str_field(body, key);
    if (v.empty()) return def;
    return (v == "true" || v == "1");
}

} // namespace json

// =============================================================================
// Product JSON serialiser
// =============================================================================
static std::string product_to_json(const Product& p) {
    return p.toJSON();
}

// =============================================================================
// Populate catalog with 20 products
// =============================================================================
static void populate_catalog(ProductCatalog& cat) {
    struct Seed {
        int id; const char* name; const char* category;
        double price; int stock; const char* desc;
    };
    static const Seed seeds[] = {
        {1,  "Laptop Pro 15",        "Electronics",  1299.99, 10, "High-performance laptop with 16GB RAM"},
        {2,  "Wireless Mouse",       "Electronics",    29.99, 50, "Ergonomic wireless mouse, 2.4GHz"},
        {3,  "Mechanical Keyboard",  "Electronics",    89.99, 30, "RGB backlit, tactile switches"},
        {4,  "4K Monitor",           "Electronics",   399.99, 15, "27-inch IPS display, 144Hz"},
        {5,  "USB-C Hub",            "Electronics",    49.99, 40, "7-in-1 hub with HDMI and USB 3.0"},
        {6,  "DSA Textbook",         "Books",          59.99, 25, "Data Structures and Algorithms, 4th Ed."},
        {7,  "C++ Primer",           "Books",          44.99, 20, "Comprehensive guide to modern C++"},
        {8,  "Algorithm Design",     "Books",          54.99, 18, "Classic algorithm design manual"},
        {9,  "Clean Code",           "Books",          39.99, 22, "A handbook of agile software craftsmanship"},
        {10, "Backpack 30L",         "Accessories",    79.99, 35, "Water-resistant laptop backpack"},
        {11, "Desk Lamp LED",        "Furniture",      34.99, 28, "Adjustable color temperature LED lamp"},
        {12, "Standing Desk",        "Furniture",     499.99,  5, "Electric height-adjustable desk"},
        {13, "Noise-Cancel Headset", "Electronics",   149.99, 20, "Over-ear ANC Bluetooth headset"},
        {14, "Webcam 1080p",         "Electronics",    69.99, 25, "Full HD webcam with built-in mic"},
        {15, "SSD 1TB",              "Electronics",    89.99, 40, "NVMe PCIe Gen4 SSD"},
        {16, "Phone Stand",          "Accessories",    15.99, 60, "Adjustable aluminium phone stand"},
        {17, "Cable Management Kit", "Accessories",    12.99, 75, "Velcro straps and cable clips set"},
        {18, "Whiteboard 90x60",     "Office",         59.99, 12, "Magnetic dry-erase whiteboard"},
        {19, "Sticky Notes Pack",    "Office",          4.99, 100,"Assorted colour sticky notes x 5"},
        {20, "Coffee Mug Warmer",    "Office",         24.99, 45, "USB-powered mug warmer, 55°C"},
    };
    for (const auto& s : seeds) {
        Product p;
        p.id          = s.id;
        p.name        = s.name;
        p.category    = s.category;
        p.price       = s.price;
        p.stock       = s.stock;
        p.description = s.desc;
        cat.addProduct(p);
    }
}

// =============================================================================
// Global data-structure instances
// =============================================================================
static ProductCatalog  g_catalog;
static ShoppingCart    g_cart;
static OrderManager    g_orders;
static BST             g_bst;
static Graph           g_graph;

// =============================================================================
// Register all API routes
// =============================================================================
static void register_routes(MinimalHTTPServer& srv) {

    // =========================================================================
    // PRODUCTS
    // =========================================================================

    // GET /api/products — all products
    srv.Get("/api/products", [](const HTTPRequest&, HTTPResponse& res) {
        auto products = g_catalog.getAllProducts();
        std::vector<std::string> arr;
        for (const auto& p : products)
            arr.push_back(product_to_json(p));
        res.set_json(json::arr(arr));
    });

    // GET /api/products/search?q=…
    srv.Get("/api/products/search", [](const HTTPRequest& req, HTTPResponse& res) {
        std::string q = req.get_param("q");
        if (q.empty()) {
            res.set_json("{\"error\":\"query param 'q' required\"}", 400);
            return;
        }
        auto results = g_catalog.searchByName(q);
        std::vector<std::string> arr;
        for (const auto& p : results)
            arr.push_back(product_to_json(p));
        res.set_json(json::obj({
            json::kv_s("query", q),
            json::kv("results", json::arr(arr)),
            json::kv_n("count", (double)results.size()),
            json::kv_s("algorithm", "Linear Search O(n)")
        }));
    });

    // GET /api/products/sort?by=price|name&order=asc|desc
    srv.Get("/api/products/sort", [](const HTTPRequest& req, HTTPResponse& res) {
        std::string by    = req.get_param("by",    "price");
        std::string order = req.get_param("order", "asc");

        auto products = g_catalog.getAllProducts();

        if (by == "name") {
            std::sort(products.begin(), products.end(),
                      [&](const Product& a, const Product& b) {
                          return order == "asc" ? a.name < b.name : a.name > b.name;
                      });
        } else {
            std::sort(products.begin(), products.end(),
                      [&](const Product& a, const Product& b) {
                          return order == "asc" ? a.price < b.price : a.price > b.price;
                      });
        }

        std::vector<std::string> arr;
        for (const auto& p : products)
            arr.push_back(product_to_json(p));

        res.set_json(json::obj({
            json::kv_s("sorted_by", by),
            json::kv_s("order",     order),
            json::kv_s("algorithm", "std::sort — Introsort O(n log n)"),
            json::kv("products", json::arr(arr))
        }));
    });

    // GET /api/products/categories
    srv.Get("/api/products/categories", [](const HTTPRequest&, HTTPResponse& res) {
        auto cats = g_catalog.getCategories();
        std::vector<std::string> arr;
        for (const auto& c : cats) arr.push_back(json::str(c));
        res.set_json(json::arr(arr));
    });

    // GET /api/products/category?cat=…
    srv.Get("/api/products/category", [](const HTTPRequest& req, HTTPResponse& res) {
        std::string cat = req.get_param("cat");
        if (cat.empty()) {
            res.set_json("{\"error\":\"query param 'cat' required\"}", 400);
            return;
        }
        auto results = g_catalog.getByCategory(cat);
        std::vector<std::string> arr;
        for (const auto& p : results)
            arr.push_back(product_to_json(p));
        res.set_json(json::obj({
            json::kv_s("category", cat),
            json::kv("products",  json::arr(arr)),
            json::kv_n("count",   (double)results.size())
        }));
    });

    // =========================================================================
    // CART  (backed by StackDS so push/pop semantics apply)
    // =========================================================================

    // GET /api/cart
    srv.Get("/api/cart", [](const HTTPRequest&, HTTPResponse& res) {
        auto items = g_cart.getItems();
        std::vector<std::string> arr;
        for (const auto& p : items)
            arr.push_back(product_to_json(p));
        res.set_json(json::obj({
            json::kv("items",      json::arr(arr)),
            json::kv_n("item_count", (double)items.size()),
            json::kv_n("total",    g_cart.getTotal()),
            json::kv_s("ds_used",  "Stack (LIFO) — push/pop")
        }));
    });

    // POST /api/cart/add   body: {"product_id": 3}
    srv.Post("/api/cart/add", [](const HTTPRequest& req, HTTPResponse& res) {
        int pid = json::get_int_field(req.body, "product_id");
        if (pid <= 0) {
            res.set_json("{\"error\":\"product_id required\"}", 400);
            return;
        }
        Product* p = g_catalog.findById(pid);
        if (!p) {
            res.set_json("{\"error\":\"product not found\"}", 404);
            return;
        }
        if (p->stock <= 0) {
            res.set_json("{\"error\":\"out of stock\"}", 400);
            return;
        }
        g_cart.addToCart(*p);
        res.set_json(json::obj({
            json::kv_s("message", "Item pushed onto cart stack"),
            json::kv("product", product_to_json(*p)),
            json::kv_n("cart_size", (double)g_cart.size()),
            json::kv_n("cart_total", g_cart.getTotal())
        }));
    });

    // POST /api/cart/remove
    srv.Post("/api/cart/remove", [](const HTTPRequest&, HTTPResponse& res) {
        if (g_cart.isEmpty()) {
            res.set_json("{\"error\":\"cart is empty\"}", 400);
            return;
        }
        Product removed = g_cart.removeLastItem();
        res.set_json(json::obj({
            json::kv_s("message",  "Top item popped from cart stack"),
            json::kv("removed",    product_to_json(removed)),
            json::kv_n("cart_size",  (double)g_cart.size()),
            json::kv_n("cart_total", g_cart.getTotal())
        }));
    });

    // GET /api/cart/total
    srv.Get("/api/cart/total", [](const HTTPRequest&, HTTPResponse& res) {
        std::ostringstream ss;
        ss << g_cart.getTotal();
        res.set_json(json::obj({
            json::kv_n("total",      g_cart.getTotal()),
            json::kv_n("item_count", (double)g_cart.size())
        }));
    });

    // =========================================================================
    // ORDERS  (backed by PriorityQueue / SimpleQueue)
    // =========================================================================

    // POST /api/orders/place   body: {"customer":"Ali","priority":2,"flash_sale":true}
    srv.Post("/api/orders/place", [](const HTTPRequest& req, HTTPResponse& res) {
        std::string customer  = json::get_str_field(req.body, "customer");
        int         priority  = json::get_int_field(req.body, "priority", 1);
        bool        flash_sale= json::get_bool_field(req.body, "flash_sale", false);

        if (customer.empty()) {
            res.set_json("{\"error\":\"customer name required\"}", 400);
            return;
        }

        // Build an order from the current cart
        auto cart_items = g_cart.getItems();
        if (cart_items.empty()) {
            res.set_json("{\"error\":\"cart is empty — add products before placing order\"}", 400);
            return;
        }

        Order ord;
        ord.customerName = customer;
        ord.priority     = priority;
        ord.isFlashSale  = flash_sale;
        ord.isVIP        = (priority == 1);
        ord.items        = cart_items;
        ord.total        = g_cart.getTotal();

        g_orders.placeOrder(ord, ord.isVIP, ord.isFlashSale);
        g_cart.clear();

        res.set_json(json::obj({
            json::kv_s("message",   "Order placed in priority queue"),
            json::kv_s("customer",  customer),
            json::kv_n("priority",  (double)priority),
            json::kv_b("flash_sale",flash_sale),
            json::kv_n("total",     ord.total),
            json::kv_s("ds_used",   "PriorityQueue (min-heap)")
        }));
    });

    // GET /api/orders/next — dequeue next order
    srv.Get("/api/orders/next", [](const HTTPRequest&, HTTPResponse& res) {
        if (!g_orders.hasOrders()) {
            res.set_json("{\"error\":\"no pending orders\"}", 404);
            return;
        }
        Order ord = g_orders.processNextOrder();
        std::vector<std::string> items_json;
        for (const auto& p : ord.items)
            items_json.push_back(product_to_json(p));

        res.set_json(json::obj({
            json::kv_s("message",   "Order dequeued and processed"),
            json::kv_n("order_id",  (double)ord.id),
            json::kv_s("customer",  ord.customerName),
            json::kv_n("priority",  (double)ord.priority),
            json::kv_b("flash_sale",ord.isFlashSale),
            json::kv_n("total",     ord.total),
            json::kv("items",       json::arr(items_json))
        }));
    });

    // GET /api/orders/all
    srv.Get("/api/orders/all", [](const HTTPRequest&, HTTPResponse& res) {
        res.set_json(g_orders.getOrdersJSON());
    });

    // =========================================================================
    // BST
    // =========================================================================

    // GET /api/bst/tree
    srv.Get("/api/bst/tree", [](const HTTPRequest&, HTTPResponse& res) {
        res.set_json(json::obj({
            json::kv("inorder",   g_bst.inorderJSON()),
            json::kv("preorder",  g_bst.preorderJSON()),
            json::kv("postorder", g_bst.postorderJSON()),
            json::kv_n("height",  (double)g_bst.height()),
            json::kv_n("count",   (double)g_bst.countNodes()),
            json::kv_s("ds_used", "Binary Search Tree")
        }));
    });

    // POST /api/bst/insert   body: {"key":15,"value":"Headset"}
    srv.Post("/api/bst/insert", [](const HTTPRequest& req, HTTPResponse& res) {
        int         key = json::get_int_field(req.body, "key");
        std::string val = json::get_str_field(req.body, "value");
        g_bst.insert(key, val);
        res.set_json(json::obj({
            json::kv_s("message", "Node inserted into BST"),
            json::kv_n("key",     (double)key),
            json::kv_s("value",   val)
        }));
    });

    // POST /api/bst/delete   body: {"key":15}
    srv.Post("/api/bst/delete", [](const HTTPRequest& req, HTTPResponse& res) {
        int key = json::get_int_field(req.body, "key");
        g_bst.deleteNode(key);
        res.set_json(json::obj({
            json::kv_b("success", true),
            json::kv_s("message", "Node deleted from BST"),
            json::kv_n("key",     (double)key)
        }));
    });

    // GET /api/bst/traversal?type=inorder|preorder|postorder
    srv.Get("/api/bst/traversal", [](const HTTPRequest& req, HTTPResponse& res) {
        std::string type = req.get_param("type", "inorder");
        std::vector<BSTNode*> nodes;

        if (type == "preorder")       nodes = g_bst.preorder();
        else if (type == "postorder") nodes = g_bst.postorder();
        else                          nodes = g_bst.inorder();

        std::vector<std::string> arr;
        for (BSTNode* n : nodes) {
            std::ostringstream ss; ss << n->key;
            arr.push_back(ss.str());
        }
        res.set_json(json::obj({
            json::kv_s("type",   type),
            json::kv("result",  json::arr(arr)),
            json::kv_n("size",  (double)nodes.size())
        }));
    });

    // =========================================================================
    // GRAPH
    // =========================================================================

    // GET /api/graph
    srv.Get("/api/graph", [](const HTTPRequest&, HTTPResponse& res) {
        res.set_json(g_graph.toJSON());
    });

    // POST /api/graph/dfs?start=0
    srv.Post("/api/graph/dfs", [](const HTTPRequest& req, HTTPResponse& res) {
        int start = std::stoi(req.get_param("start", "0"));
        auto path = g_graph.dfs(start);
        std::vector<std::string> arr;
        for (int v : path) {
            std::ostringstream ss; ss << v;
            arr.push_back(ss.str());
        }
        res.set_json(json::obj({
            json::kv_n("start",     (double)start),
            json::kv("path",        json::arr(arr)),
            json::kv_s("algorithm", "DFS — Depth-First Search O(V+E)")
        }));
    });

    // POST /api/graph/bfs?start=0
    srv.Post("/api/graph/bfs", [](const HTTPRequest& req, HTTPResponse& res) {
        int start = std::stoi(req.get_param("start", "0"));
        auto path = g_graph.bfs(start);
        std::vector<std::string> arr;
        for (int v : path) {
            std::ostringstream ss; ss << v;
            arr.push_back(ss.str());
        }
        res.set_json(json::obj({
            json::kv_n("start",     (double)start),
            json::kv("path",        json::arr(arr)),
            json::kv_s("algorithm", "BFS — Breadth-First Search O(V+E)")
        }));
    });

    // GET /api/graph/warshall
    srv.Get("/api/graph/warshall", [](const HTTPRequest&, HTTPResponse& res) {
        auto matrix = g_graph.warshall();
        std::vector<std::string> rows;
        for (const auto& row : matrix) {
            std::vector<std::string> cells;
            for (int v : row) {
                std::ostringstream ss; ss << v;
                cells.push_back(ss.str());
            }
            rows.push_back(json::arr(cells));
        }
        res.set_json(json::obj({
            json::kv("matrix",     json::arr(rows)),
            json::kv_s("algorithm","Warshall's Transitive Closure O(V^3)")
        }));
    });

    // =========================================================================
    // ALGORITHMS
    // =========================================================================

    // GET /api/hanoi?n=3
    srv.Get("/api/hanoi", [](const HTTPRequest& req, HTTPResponse& res) {
        int n = std::stoi(req.get_param("n", "3"));
        if (n < 1 || n > 20) {
            res.set_json("{\"error\":\"n must be between 1 and 20\"}", 400);
            return;
        }
        res.set_json(getHanoiJSON(n));
    });

    // GET /api/sort/demo — run bubble, selection, insertion on the same array
    srv.Get("/api/sort/demo", [](const HTTPRequest&, HTTPResponse& res) {
        // Generate a reproducible random array of 15 elements
        std::vector<int> base = {64,34,25,12,22,11,90,45,67,3,88,77,55,23,41};

        auto to_arr_str = [](const std::vector<int>& v) {
            std::vector<std::string> strs;
            for (int x : v) { std::ostringstream ss; ss << x; strs.push_back(ss.str()); }
            return json::arr(strs);
        };

        // Bubble sort (counting passes)
        auto bubble = base;
        int  bubble_ops = 0;
        for (size_t i = 0; i < bubble.size(); ++i)
            for (size_t j = 0; j + 1 < bubble.size() - i; ++j) {
                ++bubble_ops;
                if (bubble[j] > bubble[j+1]) std::swap(bubble[j], bubble[j+1]);
            }

        // Selection sort
        auto sel = base;
        int  sel_ops = 0;
        for (size_t i = 0; i < sel.size(); ++i) {
            size_t min_idx = i;
            for (size_t j = i+1; j < sel.size(); ++j) { ++sel_ops; if (sel[j] < sel[min_idx]) min_idx = j; }
            std::swap(sel[i], sel[min_idx]);
        }

        // Insertion sort
        auto ins = base;
        int  ins_ops = 0;
        for (size_t i = 1; i < ins.size(); ++i) {
            int key = ins[i]; int j = (int)i - 1;
            while (j >= 0 && ins[j] > key) { ++ins_ops; ins[j+1] = ins[j]; --j; }
            ins[j+1] = key;
        }

        res.set_json(json::obj({
            json::kv("original", to_arr_str(base)),
            json::kv("bubble_sort", json::obj({
                json::kv("sorted",     to_arr_str(bubble)),
                json::kv_n("comparisons", (double)bubble_ops),
                json::kv_s("complexity", "O(n^2)")
            })),
            json::kv("selection_sort", json::obj({
                json::kv("sorted",     to_arr_str(sel)),
                json::kv_n("comparisons", (double)sel_ops),
                json::kv_s("complexity", "O(n^2)")
            })),
            json::kv("insertion_sort", json::obj({
                json::kv("sorted",     to_arr_str(ins)),
                json::kv_n("comparisons", (double)ins_ops),
                json::kv_s("complexity", "O(n^2) worst, O(n) best")
            }))
        }));
    });

    // GET /api/complexity — algorithm complexity reference table
    srv.Get("/api/complexity", [](const HTTPRequest&, HTTPResponse& res) {
        auto entry = [](const char* name, const char* best,
                        const char* avg,  const char* worst,
                        const char* space) {
            return json::obj({
                json::kv_s("algorithm", name),
                json::kv_s("best",      best),
                json::kv_s("average",   avg),
                json::kv_s("worst",     worst),
                json::kv_s("space",     space)
            });
        };
        std::vector<std::string> table = {
            entry("Bubble Sort",    "O(n)",       "O(n^2)",     "O(n^2)",     "O(1)"),
            entry("Selection Sort", "O(n^2)",     "O(n^2)",     "O(n^2)",     "O(1)"),
            entry("Insertion Sort", "O(n)",       "O(n^2)",     "O(n^2)",     "O(1)"),
            entry("Merge Sort",     "O(n log n)", "O(n log n)", "O(n log n)", "O(n)"),
            entry("Quick Sort",     "O(n log n)", "O(n log n)", "O(n^2)",     "O(log n)"),
            entry("Heap Sort",      "O(n log n)", "O(n log n)", "O(n log n)", "O(1)"),
            entry("Linear Search",  "O(1)",       "O(n)",       "O(n)",       "O(1)"),
            entry("Binary Search",  "O(1)",       "O(log n)",   "O(log n)",   "O(1)"),
            entry("BST Insert",     "O(log n)",   "O(log n)",   "O(n)",       "O(n)"),
            entry("BFS",            "O(V+E)",     "O(V+E)",     "O(V+E)",     "O(V)"),
            entry("DFS",            "O(V+E)",     "O(V+E)",     "O(V+E)",     "O(V)"),
            entry("Warshall",       "O(V^3)",     "O(V^3)",     "O(V^3)",     "O(V^2)"),
            entry("Tower of Hanoi", "O(2^n)",     "O(2^n)",     "O(2^n)",     "O(n)"),
        };
        res.set_json(json::arr(table));
    });

    // =========================================================================
    // STACK / CALCULATOR
    // =========================================================================

    // POST /api/calculator/convert   body: {"infix":"a+b*c"}
    srv.Post("/api/calculator/convert", [](const HTTPRequest& req, HTTPResponse& res) {
        std::string infix = json::get_str_field(req.body, "infix");
        if (infix.empty()) {
            res.set_json("{\"error\":\"'infix' field required\"}", 400);
            return;
        }

        // Infix → Postfix (Shunting-Yard)
        auto precedence = [](char c) -> int {
            if (c == '+' || c == '-') return 1;
            if (c == '*' || c == '/') return 2;
            if (c == '^')             return 3;
            return 0;
        };

        StackDS<char> ops;
        std::string   postfix;
        std::vector<std::string> steps;

        for (char ch : infix) {
            if (ch == ' ') continue;
            if (std::isalnum(ch)) {
                postfix += ch;
                postfix += ' ';
                steps.push_back(std::string("Operand '") + ch + "' → output");
            } else if (ch == '(') {
                ops.push(ch);
                steps.push_back("'(' → operator stack");
            } else if (ch == ')') {
                while (!ops.isEmpty() && ops.peek() != '(') {
                    postfix += ops.pop();
                    postfix += ' ';
                }
                if (!ops.isEmpty()) ops.pop(); // discard '('
                steps.push_back("')' → pop until '('");
            } else {
                while (!ops.isEmpty() && precedence(ops.peek()) >= precedence(ch)) {
                    postfix += ops.pop();
                    postfix += ' ';
                }
                ops.push(ch);
                steps.push_back(std::string("Operator '") + ch + "' → operator stack");
            }
        }
        while (!ops.isEmpty()) {
            postfix += ops.pop();
            postfix += ' ';
        }
        if (!postfix.empty() && postfix.back() == ' ') postfix.pop_back();

        std::vector<std::string> step_arr;
        for (const auto& s : steps) step_arr.push_back(json::str(s));

        res.set_json(json::obj({
            json::kv_s("infix",     infix),
            json::kv_s("postfix",   postfix),
            json::kv_s("algorithm", "Shunting-Yard (Stack-based)"),
            json::kv("steps",       json::arr(step_arr))
        }));
    });

    // POST /api/calculator/evaluate   body: {"postfix":"2 3 4 * +"}
    srv.Post("/api/calculator/evaluate", [](const HTTPRequest& req, HTTPResponse& res) {
        std::string postfix = json::get_str_field(req.body, "postfix");
        if (postfix.empty()) {
            res.set_json("{\"error\":\"'postfix' field required\"}", 400);
            return;
        }

        StackDS<double> st;
        std::istringstream ss(postfix);
        std::string token;
        std::vector<std::string> steps;
        bool error = false;
        std::string err_msg;

        while (ss >> token) {
            if (token == "+" || token == "-" || token == "*" || token == "/") {
                if (st.size() < 2) { error = true; err_msg = "insufficient operands"; break; }
                double b = st.pop(), a = st.pop(), r = 0;
                if (token == "+")       r = a + b;
                else if (token == "-")  r = a - b;
                else if (token == "*")  r = a * b;
                else {
                    if (b == 0) { error = true; err_msg = "division by zero"; break; }
                    r = a / b;
                }
                st.push(r);
                std::ostringstream msg;
                msg << a << " " << token << " " << b << " = " << r;
                steps.push_back(msg.str());
            } else {
                try {
                    double val = std::stod(token);
                    st.push(val);
                    steps.push_back("Push " + token);
                } catch (...) {
                    error = true; err_msg = "invalid token: " + token; break;
                }
            }
        }

        if (error) {
            res.set_json(json::obj({json::kv_s("error", err_msg)}), 400);
            return;
        }

        double result = st.isEmpty() ? 0.0 : st.pop();
        std::vector<std::string> step_arr;
        for (const auto& s : steps) step_arr.push_back(json::str(s));

        res.set_json(json::obj({
            json::kv_s("postfix",   postfix),
            json::kv_n("result",    result),
            json::kv_s("algorithm", "Postfix evaluation via Stack"),
            json::kv("steps",       json::arr(step_arr))
        }));
    });
}

// =============================================================================
// main
// =============================================================================
int main() {
    std::cout << "==============================================\n";
    std::cout << "  ShopDSA — DSA E-Commerce Server\n";
    std::cout << "==============================================\n\n";

    // ------------------------------------------------------------------
    // 1. Populate catalog
    // ------------------------------------------------------------------
    populate_catalog(g_catalog);
    std::cout << "[INIT] Catalog loaded with " << g_catalog.size() << " products\n";

    // ------------------------------------------------------------------
    // 2. Seed BST with some product IDs
    // ------------------------------------------------------------------
    g_bst.insert(10, "SSD 1TB");
    g_bst.insert(5,  "DSA Textbook");
    g_bst.insert(15, "4K Monitor");
    g_bst.insert(3,  "Wireless Mouse");
    g_bst.insert(7,  "C++ Primer");
    g_bst.insert(13, "Noise-Cancel Headset");
    g_bst.insert(18, "Whiteboard 90x60");
    std::cout << "[INIT] BST seeded with 7 nodes\n";

    // ------------------------------------------------------------------
    // 3. Seed Graph (6 nodes — city delivery network)
    // ------------------------------------------------------------------
    g_graph.setVertices(6);
    g_graph.addEdge(0, 1); g_graph.addEdge(0, 2);
    g_graph.addEdge(1, 3); g_graph.addEdge(2, 3);
    g_graph.addEdge(3, 4); g_graph.addEdge(4, 5);
    g_graph.addEdge(2, 5);
    std::cout << "[INIT] Graph initialised (6 vertices, 7 edges)\n";

    // ------------------------------------------------------------------
    // 4. HTTP Server
    // ------------------------------------------------------------------
    constexpr int PORT = 8080;
    MinimalHTTPServer srv(PORT, "web");

    register_routes(srv);
    std::cout << "[INIT] " << "API routes registered\n\n";

    // ------------------------------------------------------------------
    // 5. Open browser
    // ------------------------------------------------------------------
    std::string url = "http://localhost:" + std::to_string(PORT);
    std::cout << "  Server : " << url << "\n";
    std::cout << "  Press Ctrl+C to stop\n\n";
    std::string cmd = "start " + url;
    std::system(cmd.c_str());

    // ------------------------------------------------------------------
    // 6. Block-listen
    // ------------------------------------------------------------------
    if (!srv.listen()) {
        std::cerr << "[ERROR] Failed to start HTTP server\n";
        return 1;
    }

    return 0;
}
