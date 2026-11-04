#pragma once
#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <vector>

template <typename T>
class ArrayDS {
private:
    T* data;
    int capacity;
    int length;

    void resize() {
        capacity *= 2;
        T* newData = new T[capacity];
        for (int i = 0; i < length; i++) newData[i] = data[i];
        delete[] data;
        data = newData;
    }

    int partition(int low, int high) {
        // Median-of-three pivot selection
        int mid = low + (high - low) / 2;
        if (data[low] > data[mid]) std::swap(data[low], data[mid]);
        if (data[low] > data[high]) std::swap(data[low], data[high]);
        if (data[mid] > data[high]) std::swap(data[mid], data[high]);
        T pivot = data[mid];
        std::swap(data[mid], data[high - 1]);
        int i = low;
        int j = high - 1;
        while (true) {
            while (data[++i] < pivot) {}
            while (data[--j] > pivot) {}
            if (i >= j) break;
            std::swap(data[i], data[j]);
        }
        std::swap(data[i], data[high - 1]);
        return i;
    }

    void quickSortHelper(int low, int high) {
        if (high <= low + 1) {
            if (high == low + 1 && data[high] < data[low])
                std::swap(data[low], data[high]);
            return;
        }
        int p = partition(low, high);
        quickSortHelper(low, p - 1);
        quickSortHelper(p + 1, high);
    }

public:
    ArrayDS(int initialCapacity = 16)
        : capacity(initialCapacity), length(0) {
        data = new T[capacity];
    }

    ArrayDS(const ArrayDS& other) : capacity(other.capacity), length(other.length) {
        data = new T[capacity];
        for (int i = 0; i < length; i++) data[i] = other.data[i];
    }

    ArrayDS& operator=(const ArrayDS& other) {
        if (this == &other) return *this;
        delete[] data;
        capacity = other.capacity;
        length = other.length;
        data = new T[capacity];
        for (int i = 0; i < length; i++) data[i] = other.data[i];
        return *this;
    }

    ~ArrayDS() { delete[] data; }

    // Insert at specific index (0-based); use length to append
    void insert(const T& val, int index) {
        if (index < 0 || index > length)
            throw std::out_of_range("Insert index out of range");
        if (length == capacity) resize();
        for (int i = length; i > index; i--) data[i] = data[i - 1];
        data[index] = val;
        length++;
    }

    // Append convenience
    void append(const T& val) { insert(val, length); }

    void deleteAt(int index) {
        if (index < 0 || index >= length)
            throw std::out_of_range("Delete index out of range");
        for (int i = index; i < length - 1; i++) data[i] = data[i + 1];
        length--;
    }

    T& at(int index) {
        if (index < 0 || index >= length)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    const T& at(int index) const {
        if (index < 0 || index >= length)
            throw std::out_of_range("Index out of range");
        return data[index];
    }

    int size() const { return length; }
    bool empty() const { return length == 0; }

    void traverse() const {
        std::cout << "[ ";
        for (int i = 0; i < length; i++) {
            std::cout << data[i];
            if (i < length - 1) std::cout << ", ";
        }
        std::cout << " ]" << std::endl;
    }

    // Linear Search: returns index or -1
    int linearSearch(const T& val) const {
        for (int i = 0; i < length; i++)
            if (data[i] == val) return i;
        return -1;
    }

    // Binary Search: array must be sorted, returns index or -1
    int binarySearch(const T& val) const {
        int low = 0, high = length - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (data[mid] == val) return mid;
            else if (data[mid] < val) low = mid + 1;
            else high = mid - 1;
        }
        return -1;
    }

    // Bubble Sort O(n^2)
    void bubbleSort(bool ascending = true) {
        for (int i = 0; i < length - 1; i++) {
            bool swapped = false;
            for (int j = 0; j < length - i - 1; j++) {
                bool cond = ascending ? (data[j] > data[j + 1]) : (data[j] < data[j + 1]);
                if (cond) { std::swap(data[j], data[j + 1]); swapped = true; }
            }
            if (!swapped) break;
        }
    }

    // Quick Sort O(n log n) average
    void quickSort(bool ascending = true) {
        if (length < 2) return;
        // For descending, sort ascending then reverse
        if (length >= 3)
            quickSortHelper(0, length - 1);
        else if (length == 2 && data[0] > data[1])
            std::swap(data[0], data[1]);
        if (!ascending) std::reverse(data, data + length);
    }

    std::string getBubbleSortComplexity() const {
        return "Bubble Sort - Best: O(n), Average: O(n^2), Worst: O(n^2), Space: O(1), Stable: Yes";
    }

    std::string getQuickSortComplexity() const {
        return "Quick Sort - Best: O(n log n), Average: O(n log n), Worst: O(n^2), Space: O(log n), Stable: No";
    }

    // Returns a copy as std::vector
    std::vector<T> toVector() const {
        return std::vector<T>(data, data + length);
    }

    // JSON representation – uses T's toJSON() if available via overload;
    // for primitive types we rely on operator<<
    std::string toJSON() const {
        std::ostringstream oss;
        oss << "[";
        for (int i = 0; i < length; i++) {
            if (i > 0) oss << ",";
            oss << data[i];
        }
        oss << "]";
        return oss.str();
    }

    // Specialisation helper called by catalog for Product JSON
    // (defined after Product is known; catalog.h provides the specialisation)
};
