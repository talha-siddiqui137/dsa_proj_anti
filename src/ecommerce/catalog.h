#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <functional>
#include "../ds/array_ds.h"
#include "../ds/tree_ds.h"
#include "product.h"

// ─────────────────────────────────────────────────────────────
//  Provide toJSON specialisation for ArrayDS<Product>
//  (injected via a free helper so the template header stays generic)
// ─────────────────────────────────────────────────────────────
inline std::string productArrayToJSON(const ArrayDS<Product>& arr) {
    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < arr.size(); i++) {
        if (i > 0) oss << ",";
        oss << arr.at(i).toJSON();
    }
    oss << "]";
    return oss.str();
}

// ─────────────────────────────────────────────────────────────
//  ProductCatalog
// ─────────────────────────────────────────────────────────────
class ProductCatalog {
private:
    ArrayDS<Product> products;
    BST              priceIndex;   // BST keyed on price
    int              nextId;

    // Internal: rebuild BST from current product array
    void rebuildBST() {
        // We cannot clear BST easily without a reset method, so we reconstruct
        priceIndex.~BST();
        new (&priceIndex) BST();
        for (int i = 0; i < products.size(); i++) {
            const Product& p = products.at(i);
            priceIndex.insert(p.price, p.name, p.id);
        }
    }

    // Custom bubble-sort on a vector<Product> by rating
    static void bubbleSortByRating(std::vector<Product>& vec, bool ascending) {
        int n = (int)vec.size();
        for (int i = 0; i < n - 1; i++) {
            bool swapped = false;
            for (int j = 0; j < n - i - 1; j++) {
                bool cond = ascending
                    ? vec[j].rating > vec[j+1].rating
                    : vec[j].rating < vec[j+1].rating;
                if (cond) { std::swap(vec[j], vec[j+1]); swapped = true; }
            }
            if (!swapped) break;
        }
    }

public:
    ProductCatalog() : products(32), nextId(1) {
        populateSampleProducts();
    }

    // ── CRUD ──────────────────────────────────────────────────
    void addProduct(Product p) {
        if (p.id <= 0) p.id = nextId;
        nextId = std::max(nextId, p.id + 1);
        products.append(p);
        priceIndex.insert(p.price, p.name, p.id);
    }

    bool removeProduct(int id) {
        for (int i = 0; i < products.size(); i++) {
            if (products.at(i).id == id) {
                priceIndex.deleteNode(products.at(i).price);
                products.deleteAt(i);
                return true;
            }
        }
        return false;
    }

    // ── Search ────────────────────────────────────────────────
    // Linear search by name (case-insensitive substring)
    std::vector<Product> searchByName(const std::string& query) const {
        std::vector<Product> results;
        std::string lq = query;
        std::transform(lq.begin(), lq.end(), lq.begin(), ::tolower);
        for (int i = 0; i < products.size(); i++) {
            std::string ln = products.at(i).name;
            std::transform(ln.begin(), ln.end(), ln.begin(), ::tolower);
            if (ln.find(lq) != std::string::npos)
                results.push_back(products.at(i));
        }
        return results;
    }

    // Binary search by exact price (requires sorted array – we sort a copy)
    // Returns first product found at that price, throws if not found
    Product searchByPrice(double price) const {
        // Check BST first (O(log n))
        BSTNode* node = priceIndex.search(price);
        if (node) {
            // Find in array by id
            for (int i = 0; i < products.size(); i++)
                if (products.at(i).id == node->id) return products.at(i);
        }
        throw std::runtime_error("No product found at price: " + std::to_string(price));
    }

    // ── Sorting ───────────────────────────────────────────────
    void sortByPrice(bool ascending = true) {
        // Uses QuickSort (via ArrayDS::quickSort which sorts by operator<)
        // ArrayDS<Product>::operator< uses price, so this works directly
        products.quickSort(ascending);
        rebuildBST();
    }

    void sortByRating(bool ascending = true) {
        // Extract, bubble-sort by rating, re-insert
        std::vector<Product> tmp = products.toVector();
        bubbleSortByRating(tmp, ascending);
        // Reload
        ArrayDS<Product> rebuilt(32);
        for (const auto& p : tmp) rebuilt.append(p);
        products = rebuilt;
    }

    // ── Filter by category ───────────────────────────────────
    std::vector<Product> getCategory(const std::string& cat) const {
        std::vector<Product> results;
        std::string lc = cat;
        std::transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
        for (int i = 0; i < products.size(); i++) {
            std::string lcat = products.at(i).category;
            std::transform(lcat.begin(), lcat.end(), lcat.begin(), ::tolower);
            if (lcat == lc) results.push_back(products.at(i));
        }
        return results;
    }

    // ── Accessors ────────────────────────────────────────────
    int count() const { return products.size(); }
    int size()  const { return products.size(); }

    std::vector<Product> getAllProducts() const {
        return products.toVector();
    }

    std::vector<std::string> getCategories() const {
        std::vector<std::string> cats;
        for (int i = 0; i < products.size(); i++) {
            const std::string& c = products.at(i).category;
            if (std::find(cats.begin(), cats.end(), c) == cats.end())
                cats.push_back(c);
        }
        return cats;
    }

    std::vector<Product> getByCategory(const std::string& cat) const {
        return getCategory(cat);
    }

    Product& getProduct(int index) { return products.at(index); }
    const Product& getProduct(int index) const { return products.at(index); }

    Product* findById(int id) {
        for (int i = 0; i < products.size(); i++)
            if (products.at(i).id == id) return &products.at(i);
        return nullptr;
    }

    ArrayDS<Product>& getArray() { return products; }
    const ArrayDS<Product>& getArray() const { return products; }

    BST& getBST() { return priceIndex; }

    std::string toJSON() const {
        std::ostringstream oss;
        oss << "{\"count\":" << products.size()
            << ",\"products\":" << productArrayToJSON(products)
            << "}";
        return oss.str();
    }

    // ── Pre-populate 20 sample products ──────────────────────
    void populateSampleProducts() {
        // Electronics (6)
        addProduct({1,  "iPhone 15 Pro",       "Electronics",  999.99, 4.8, 50,  "📱", "Latest Apple flagship with A17 chip"});
        addProduct({2,  "Samsung Galaxy S24",  "Electronics",  849.99, 4.6, 75,  "📱", "Android powerhouse with AI features"});
        addProduct({3,  "MacBook Air M3",       "Electronics", 1299.99, 4.9, 30,  "💻", "Ultra-thin laptop with M3 chip"});
        addProduct({4,  "Sony WH-1000XM5",     "Electronics",  349.99, 4.7, 100, "🎧", "Industry-leading noise cancelling headphones"});
        addProduct({5,  "iPad Pro 12.9",        "Electronics",  1099.99, 4.7, 40, "📟", "Pro tablet with M2 chip"});
        addProduct({6,  "LG OLED 55\" TV",     "Electronics", 1499.99, 4.8, 20,  "📺", "55-inch OLED 4K smart TV"});
        // Clothing (4)
        addProduct({7,  "Nike Air Max 270",    "Clothing",      120.00, 4.5, 200, "👟", "Classic Nike running shoes"});
        addProduct({8,  "Levi's 501 Jeans",    "Clothing",       59.99, 4.4, 300, "👖", "Classic straight-fit jeans"});
        addProduct({9,  "Adidas Hoodie",        "Clothing",       79.99, 4.3, 150, "🧥", "Comfortable fleece hoodie"});
        addProduct({10, "Ray-Ban Sunglasses",   "Clothing",      150.00, 4.6, 80,  "🕶️", "Iconic Wayfarer sunglasses"});
        // Books (4)
        addProduct({11, "Clean Code",           "Books",          34.99, 4.7, 500, "📖", "A handbook of Agile software craftsmanship"});
        addProduct({12, "The Pragmatic Programmer","Books",        45.00, 4.8, 400, "📚", "From journeyman to master"});
        addProduct({13, "Introduction to Algorithms","Books",      79.99, 4.6, 250, "📕", "CLRS – comprehensive algorithms textbook"});
        addProduct({14, "Design Patterns",      "Books",          44.99, 4.5, 350, "📗", "Gang of Four design patterns"});
        // Sports (3)
        addProduct({15, "Wilson Tennis Racket", "Sports",         89.99, 4.4, 60,  "🎾", "Intermediate-level tennis racket"});
        addProduct({16, "Yoga Mat Pro",         "Sports",         29.99, 4.6, 200, "🧘", "6mm thick non-slip yoga mat"});
        addProduct({17, "Dumbbell Set 20kg",    "Sports",        149.99, 4.5, 45,  "🏋️", "Adjustable dumbbell set"});
        // Food (3)
        addProduct({18, "Whey Protein 2kg",     "Food",           59.99, 4.5, 120, "🥛", "Vanilla flavoured whey protein"});
        addProduct({19, "Organic Green Tea",    "Food",           19.99, 4.7, 300, "🍵", "100% organic green tea leaves"});
        addProduct({20, "Dark Chocolate Box",   "Food",           24.99, 4.6, 180, "🍫", "Premium 72% cacao dark chocolate"});
        nextId = 21;
    }
};
