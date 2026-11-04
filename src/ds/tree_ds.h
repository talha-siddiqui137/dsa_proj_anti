#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <algorithm>

// ─────────────────────────────────────────────────────────────
//  BST Node
// ─────────────────────────────────────────────────────────────
struct BSTNode {
    double   key;      // price (search key)
    std::string value; // product name
    int      id;       // product id
    BSTNode* left;
    BSTNode* right;

    BSTNode(double k, const std::string& v, int i)
        : key(k), value(v), id(i), left(nullptr), right(nullptr) {}
};

// ─────────────────────────────────────────────────────────────
//  BST class
// ─────────────────────────────────────────────────────────────
class BST {
private:
    BSTNode* root;

    // ── Internal helpers ──────────────────────────────────────
    BSTNode* insertHelper(BSTNode* node, double key,
                          const std::string& value, int id) {
        if (!node) return new BSTNode(key, value, id);
        if (key < node->key)       node->left  = insertHelper(node->left,  key, value, id);
        else if (key > node->key)  node->right = insertHelper(node->right, key, value, id);
        else {
            // Duplicate key: store in right subtree (allow multiple products at same price)
            node->right = insertHelper(node->right, key, value, id);
        }
        return node;
    }

    BSTNode* minNode(BSTNode* node) const {
        while (node->left) node = node->left;
        return node;
    }

    BSTNode* deleteHelper(BSTNode* node, double key) {
        if (!node) return nullptr;
        if (key < node->key)      node->left  = deleteHelper(node->left,  key);
        else if (key > node->key) node->right = deleteHelper(node->right, key);
        else {
            if (!node->left) {
                BSTNode* tmp = node->right;
                delete node;
                return tmp;
            } else if (!node->right) {
                BSTNode* tmp = node->left;
                delete node;
                return tmp;
            }
            BSTNode* successor = minNode(node->right);
            node->key   = successor->key;
            node->value = successor->value;
            node->id    = successor->id;
            node->right = deleteHelper(node->right, successor->key);
        }
        return node;
    }

    void inorderHelper(BSTNode* node, std::vector<BSTNode*>& result) const {
        if (!node) return;
        inorderHelper(node->left,  result);
        result.push_back(node);
        inorderHelper(node->right, result);
    }

    void preorderHelper(BSTNode* node, std::vector<BSTNode*>& result) const {
        if (!node) return;
        result.push_back(node);
        preorderHelper(node->left,  result);
        preorderHelper(node->right, result);
    }

    void postorderHelper(BSTNode* node, std::vector<BSTNode*>& result) const {
        if (!node) return;
        postorderHelper(node->left,  result);
        postorderHelper(node->right, result);
        result.push_back(node);
    }

    int heightHelper(BSTNode* node) const {
        if (!node) return 0;
        return 1 + std::max(heightHelper(node->left), heightHelper(node->right));
    }

    int countHelper(BSTNode* node) const {
        if (!node) return 0;
        return 1 + countHelper(node->left) + countHelper(node->right);
    }

    void destroyHelper(BSTNode* node) {
        if (!node) return;
        destroyHelper(node->left);
        destroyHelper(node->right);
        delete node;
    }

    std::string nodeToJSON(const BSTNode* node) const {
        std::ostringstream oss;
        oss << "{\"key\":" << node->key
            << ",\"value\":\"" << node->value << "\""
            << ",\"id\":" << node->id << "}";
        return oss.str();
    }

    std::string vectorToJSON(const std::vector<BSTNode*>& nodes) const {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < nodes.size(); i++) {
            if (i > 0) oss << ",";
            oss << nodeToJSON(nodes[i]);
        }
        oss << "]";
        return oss.str();
    }

public:
    BST() : root(nullptr) {}

    BST(const BST&) = delete;            // deep-copy not needed here
    BST& operator=(const BST&) = delete;

    ~BST() { destroyHelper(root); }

    // ── Public Interface ──────────────────────────────────────

    void insert(double key, const std::string& value, int id = 0) {
        root = insertHelper(root, key, value, id);
    }

    BSTNode* getRoot() const { return root; }

    void deleteNode(double key) {
        root = deleteHelper(root, key);
    }

    // Search: returns pointer to node (nullptr if not found)
    BSTNode* search(double key) const {
        BSTNode* cur = root;
        while (cur) {
            if (key == cur->key)       return cur;
            else if (key < cur->key)   cur = cur->left;
            else                       cur = cur->right;
        }
        return nullptr;
    }

    // Traversals (return vectors for use in UI / JSON)
    std::vector<BSTNode*> inorder()   const {
        std::vector<BSTNode*> res; inorderHelper(root,   res); return res;
    }
    std::vector<BSTNode*> preorder()  const {
        std::vector<BSTNode*> res; preorderHelper(root,  res); return res;
    }
    std::vector<BSTNode*> postorder() const {
        std::vector<BSTNode*> res; postorderHelper(root, res); return res;
    }

    // JSON traversals
    std::string inorderJSON()   const { return vectorToJSON(inorder());   }
    std::string preorderJSON()  const { return vectorToJSON(preorder());  }
    std::string postorderJSON() const { return vectorToJSON(postorder()); }

    int height()     const { return heightHelper(root); }
    int countNodes() const { return countHelper(root);  }

    bool isEmpty() const { return root == nullptr; }
};
