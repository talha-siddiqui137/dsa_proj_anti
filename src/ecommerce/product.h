#pragma once
#include <string>
#include <sstream>

// ─────────────────────────────────────────────────────────────
//  Product – core e-commerce entity
// ─────────────────────────────────────────────────────────────
struct Product {
    int         id;
    std::string name;
    std::string category;
    double      price;
    double      rating;    // 0.0 – 5.0
    int         stock;
    std::string imageEmoji; // e.g. "📱", "👟", "💻"
    std::string description;

    Product()
        : id(0), price(0.0), rating(0.0), stock(0) {}

    Product(int id_, const std::string& name_, const std::string& category_,
            double price_, double rating_, int stock_,
            const std::string& emoji_, const std::string& desc_)
        : id(id_), name(name_), category(category_), price(price_),
          rating(rating_), stock(stock_), imageEmoji(emoji_), description(desc_) {}

    // Escape a string for JSON (handles quotes and backslashes)
    static std::string jsonEscape(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if      (c == '"')  out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else if (c == '\t') out += "\\t";
            else                out += c;
        }
        return out;
    }

    std::string toJSON() const {
        std::ostringstream oss;
        oss << "{"
            << "\"id\":"          << id            << ","
            << "\"name\":\""      << jsonEscape(name)        << "\","
            << "\"category\":\"" << jsonEscape(category)    << "\","
            << "\"price\":"      << price          << ","
            << "\"rating\":"     << rating         << ","
            << "\"stock\":"      << stock          << ","
            << "\"imageEmoji\":\"" << jsonEscape(imageEmoji) << "\","
            << "\"description\":\"" << jsonEscape(description) << "\""
            << "}";
        return oss.str();
    }

    // Comparison operators (used by ArrayDS sorting)
    bool operator<(const Product& other) const { return price < other.price; }
    bool operator>(const Product& other) const { return price > other.price; }
    bool operator==(const Product& other) const { return id == other.id; }
    bool operator!=(const Product& other) const { return id != other.id; }
    bool operator<=(const Product& other) const { return price <= other.price; }
    bool operator>=(const Product& other) const { return price >= other.price; }
};
