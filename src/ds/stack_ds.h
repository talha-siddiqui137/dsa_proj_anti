#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <cmath>
#include <vector>
#include <map>

template <typename T>
class StackDS {
private:
    T* data;
    int topIndex;
    int capacity;

    void resize() {
        capacity *= 2;
        T* newData = new T[capacity];
        for (int i = 0; i <= topIndex; i++) newData[i] = data[i];
        delete[] data;
        data = newData;
    }

public:
    explicit StackDS(int initialCapacity = 64)
        : topIndex(-1), capacity(initialCapacity) {
        data = new T[capacity];
    }

    StackDS(const StackDS& other) : topIndex(other.topIndex), capacity(other.capacity) {
        data = new T[capacity];
        for (int i = 0; i <= topIndex; i++) data[i] = other.data[i];
    }

    StackDS& operator=(const StackDS& other) {
        if (this == &other) return *this;
        delete[] data;
        topIndex = other.topIndex;
        capacity = other.capacity;
        data = new T[capacity];
        for (int i = 0; i <= topIndex; i++) data[i] = other.data[i];
        return *this;
    }

    ~StackDS() { delete[] data; }

    void push(const T& val) {
        if (topIndex + 1 == capacity) resize();
        data[++topIndex] = val;
    }

    T pop() {
        if (isEmpty()) throw std::underflow_error("Stack underflow: pop on empty stack");
        return data[topIndex--];
    }

    T& peek() {
        if (isEmpty()) throw std::underflow_error("Stack underflow: peek on empty stack");
        return data[topIndex];
    }

    const T& peek() const {
        if (isEmpty()) throw std::underflow_error("Stack underflow: peek on empty stack");
        return data[topIndex];
    }

    bool isEmpty() const { return topIndex == -1; }
    bool isFull()  const { return false; } // dynamic – never truly full
    int  size()    const { return topIndex + 1; }

    // Returns all items bottom→top as a vector (non-destructive)
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(topIndex + 1);
        for (int i = 0; i <= topIndex; i++) result.push_back(data[i]);
        return result;
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << "Stack (top→bottom): [ ";
        for (int i = topIndex; i >= 0; i--) {
            oss << data[i];
            if (i > 0) oss << ", ";
        }
        oss << " ]";
        return oss.str();
    }

    std::string toJSON() const {
        std::ostringstream oss;
        oss << "{\"stack\":[";
        for (int i = topIndex; i >= 0; i--) {
            if (i < topIndex) oss << ",";
            oss << data[i];
        }
        oss << "],\"size\":" << size() << "}";
        return oss.str();
    }
};

// ─────────────────────────────────────────────────────────────
// Infix → Postfix + Postfix Evaluator (string-specialised, but
// kept as free functions so StackDS<char/double> can be used)
// ─────────────────────────────────────────────────────────────

inline int operatorPrecedence(char op) {
    if (op == '^') return 3;
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

inline bool isRightAssociative(char op) { return op == '^'; }

// Converts an infix expression string to postfix (Shunting-Yard)
// Supports: +  -  *  /  ^  (  )   and multi-digit / decimal numbers
inline std::string infixToPostfix(const std::string& expr) {
    StackDS<char> opStack(64);
    std::ostringstream output;
    bool lastWasOperandOrClose = false;

    for (size_t i = 0; i < expr.size(); i++) {
        char ch = expr[i];
        if (std::isspace(ch)) continue;

        // Number (integer or decimal, handles unary minus as 0-x)
        if (std::isdigit(ch) || ch == '.') {
            if (lastWasOperandOrClose) output << ' ';
            while (i < expr.size() && (std::isdigit(expr[i]) || expr[i] == '.'))
                output << expr[i++];
            i--;
            lastWasOperandOrClose = true;
        }
        else if (ch == '(') {
            opStack.push(ch);
            lastWasOperandOrClose = false;
        }
        else if (ch == ')') {
            while (!opStack.isEmpty() && opStack.peek() != '(') {
                output << ' ' << opStack.pop();
            }
            if (!opStack.isEmpty()) opStack.pop(); // discard '('
            lastWasOperandOrClose = true;
        }
        else {
            // Handle unary minus: treat as "0 -"
            if (ch == '-' && !lastWasOperandOrClose) {
                output << (lastWasOperandOrClose ? ' ' : '\0');
                output << "0 ";
            }
            while (!opStack.isEmpty() && opStack.peek() != '(' &&
                   (operatorPrecedence(opStack.peek()) > operatorPrecedence(ch) ||
                    (operatorPrecedence(opStack.peek()) == operatorPrecedence(ch) &&
                     !isRightAssociative(ch)))) {
                output << ' ' << opStack.pop();
            }
            opStack.push(ch);
            lastWasOperandOrClose = false;
        }
    }
    while (!opStack.isEmpty()) output << ' ' << opStack.pop();
    return output.str();
}

// Evaluates a postfix expression string, returns double result
inline double evaluatePostfix(const std::string& expr) {
    StackDS<double> valStack(64);
    std::istringstream iss(expr);
    std::string token;
    while (iss >> token) {
        if (token.size() == 1 && !std::isdigit(token[0]) && token[0] != '.') {
            char op = token[0];
            if (valStack.size() < 2)
                throw std::runtime_error("Invalid postfix expression");
            double b = valStack.pop();
            double a = valStack.pop();
            switch (op) {
                case '+': valStack.push(a + b); break;
                case '-': valStack.push(a - b); break;
                case '*': valStack.push(a * b); break;
                case '/':
                    if (b == 0.0) throw std::runtime_error("Division by zero");
                    valStack.push(a / b); break;
                case '^': valStack.push(std::pow(a, b)); break;
                default:  throw std::runtime_error(std::string("Unknown operator: ") + op);
            }
        } else {
            try { valStack.push(std::stod(token)); }
            catch (...) { throw std::runtime_error("Invalid token in postfix: " + token); }
        }
    }
    if (valStack.size() != 1)
        throw std::runtime_error("Invalid postfix expression");
    return valStack.pop();
}
