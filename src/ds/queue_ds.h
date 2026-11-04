#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <functional>

// ═══════════════════════════════════════════════════════════════
//  1. SimpleQueue<T>  –  singly-linked-list based, unbounded
// ═══════════════════════════════════════════════════════════════
template <typename T>
class SimpleQueue {
private:
    struct Node {
        T data;
        Node* next;
        explicit Node(const T& d) : data(d), next(nullptr) {}
    };
    Node* headNode;
    Node* tailNode;
    int count;

public:
    SimpleQueue() : headNode(nullptr), tailNode(nullptr), count(0) {}

    SimpleQueue(const SimpleQueue& other) : headNode(nullptr), tailNode(nullptr), count(0) {
        Node* cur = other.headNode;
        while (cur) { enqueue(cur->data); cur = cur->next; }
    }

    SimpleQueue& operator=(const SimpleQueue& other) {
        if (this == &other) return *this;
        clear();
        Node* cur = other.headNode;
        while (cur) { enqueue(cur->data); cur = cur->next; }
        return *this;
    }

    ~SimpleQueue() { clear(); }

    void enqueue(const T& val) {
        Node* node = new Node(val);
        if (!tailNode) { headNode = tailNode = node; }
        else { tailNode->next = node; tailNode = node; }
        count++;
    }

    T dequeue() {
        if (isEmpty()) throw std::underflow_error("SimpleQueue: dequeue on empty queue");
        Node* tmp = headNode;
        T val = tmp->data;
        headNode = headNode->next;
        if (!headNode) tailNode = nullptr;
        delete tmp;
        count--;
        return val;
    }

    T& front() {
        if (isEmpty()) throw std::underflow_error("SimpleQueue: front on empty queue");
        return headNode->data;
    }

    const T& front() const {
        if (isEmpty()) throw std::underflow_error("SimpleQueue: front on empty queue");
        return headNode->data;
    }

    bool isEmpty() const { return count == 0; }
    int  size()    const { return count; }

    void clear() {
        while (!isEmpty()) dequeue();
    }

    // Returns items front→back as vector (non-destructive)
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count);
        Node* cur = headNode;
        while (cur) { result.push_back(cur->data); cur = cur->next; }
        return result;
    }

    std::string toJSON() const {
        std::ostringstream oss;
        oss << "{\"type\":\"SimpleQueue\",\"size\":" << count << ",\"items\":[";
        Node* cur = headNode;
        bool first = true;
        while (cur) {
            if (!first) oss << ",";
            oss << cur->data;
            first = false;
            cur = cur->next;
        }
        oss << "]}";
        return oss.str();
    }
};

// ═══════════════════════════════════════════════════════════════
//  2. CircularQueue<T>  –  fixed-size ring buffer
// ═══════════════════════════════════════════════════════════════
template <typename T>
class CircularQueue {
private:
    T*   data;
    int  frontIdx;
    int  rearIdx;
    int  count;
    int  capacity;

public:
    explicit CircularQueue(int cap = 5)
        : frontIdx(0), rearIdx(-1), count(0), capacity(cap) {
        if (cap <= 0) throw std::invalid_argument("CircularQueue capacity must be > 0");
        data = new T[capacity];
    }

    CircularQueue(const CircularQueue& other)
        : frontIdx(other.frontIdx), rearIdx(other.rearIdx),
          count(other.count), capacity(other.capacity) {
        data = new T[capacity];
        for (int i = 0; i < capacity; i++) data[i] = other.data[i];
    }

    CircularQueue& operator=(const CircularQueue& other) {
        if (this == &other) return *this;
        delete[] data;
        frontIdx = other.frontIdx; rearIdx = other.rearIdx;
        count = other.count; capacity = other.capacity;
        data = new T[capacity];
        for (int i = 0; i < capacity; i++) data[i] = other.data[i];
        return *this;
    }

    ~CircularQueue() { delete[] data; }

    void enqueue(const T& val) {
        if (isFull()) throw std::overflow_error("CircularQueue: queue is full");
        rearIdx = (rearIdx + 1) % capacity;
        data[rearIdx] = val;
        count++;
    }

    T dequeue() {
        if (isEmpty()) throw std::underflow_error("CircularQueue: dequeue on empty queue");
        T val = data[frontIdx];
        frontIdx = (frontIdx + 1) % capacity;
        count--;
        return val;
    }

    T& front() {
        if (isEmpty()) throw std::underflow_error("CircularQueue: front on empty queue");
        return data[frontIdx];
    }

    bool isFull()  const { return count == capacity; }
    bool isEmpty() const { return count == 0; }
    int  size()    const { return count; }
    int  getCapacity() const { return capacity; }

    // Non-destructive snapshot front→back
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count);
        for (int i = 0; i < count; i++)
            result.push_back(data[(frontIdx + i) % capacity]);
        return result;
    }

    std::string toJSON() const {
        std::ostringstream oss;
        oss << "{\"type\":\"CircularQueue\",\"capacity\":" << capacity
            << ",\"size\":" << count << ",\"items\":[";
        for (int i = 0; i < count; i++) {
            if (i > 0) oss << ",";
            oss << data[(frontIdx + i) % capacity];
        }
        oss << "]}";
        return oss.str();
    }
};

// ═══════════════════════════════════════════════════════════════
//  3. PriorityQueue<T>  –  min-heap (lower priority value = higher urgency)
// ═══════════════════════════════════════════════════════════════
template <typename T>
class PriorityQueue {
private:
    struct HeapNode {
        T    data;
        int  priority; // lower number = higher priority
    };

    std::vector<HeapNode> heap;

    void heapifyUp(int idx) {
        while (idx > 0) {
            int parent = (idx - 1) / 2;
            if (heap[parent].priority > heap[idx].priority) {
                std::swap(heap[parent], heap[idx]);
                idx = parent;
            } else break;
        }
    }

    void heapifyDown(int idx) {
        int n = (int)heap.size();
        while (true) {
            int smallest = idx;
            int left  = 2 * idx + 1;
            int right = 2 * idx + 2;
            if (left  < n && heap[left].priority  < heap[smallest].priority) smallest = left;
            if (right < n && heap[right].priority < heap[smallest].priority) smallest = right;
            if (smallest == idx) break;
            std::swap(heap[smallest], heap[idx]);
            idx = smallest;
        }
    }

public:
    PriorityQueue() = default;

    void insert(const T& val, int priority) {
        heap.push_back({val, priority});
        heapifyUp((int)heap.size() - 1);
    }

    T extractMin() {
        if (isEmpty()) throw std::underflow_error("PriorityQueue: extractMin on empty queue");
        T top = heap[0].data;
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty()) heapifyDown(0);
        return top;
    }

    const T& peekMin() const {
        if (isEmpty()) throw std::underflow_error("PriorityQueue: peek on empty queue");
        return heap[0].data;
    }

    int peekPriority() const {
        if (isEmpty()) throw std::underflow_error("PriorityQueue: peek on empty queue");
        return heap[0].priority;
    }

    bool isEmpty() const { return heap.empty(); }
    int  size()    const { return (int)heap.size(); }

    std::string toJSON() const {
        std::ostringstream oss;
        oss << "{\"type\":\"PriorityQueue\",\"size\":" << heap.size() << ",\"items\":[";
        for (size_t i = 0; i < heap.size(); i++) {
            if (i > 0) oss << ",";
            oss << "{\"priority\":" << heap[i].priority << ",\"data\":" << heap[i].data << "}";
        }
        oss << "]}";
        return oss.str();
    }
};
