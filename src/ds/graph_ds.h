#pragma once
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <functional>
#include <limits>

class Graph {
private:
    int numVertices;
    std::vector<std::string> cityNames;
    // Adjacency matrix  (0 = no edge, positive = weight)
    std::vector<std::vector<int>> adjMatrix;
    // Adjacency list: vertex → list of (neighbor, weight)
    std::map<int, std::vector<std::pair<int,int>>> adjList;

    void validateVertex(int v) const {
        if (v < 0 || v >= numVertices)
            throw std::out_of_range("Vertex index out of range: " + std::to_string(v));
    }

public:
    Graph(int vertices = 0) : numVertices(vertices) {
        if (vertices > 0) {
            for (int i = 0; i < vertices; i++) cityNames.push_back("Node " + std::to_string(i));
            adjMatrix.assign(vertices, std::vector<int>(vertices, 0));
        }
    }

    Graph(int vertices, const std::vector<std::string>& names)
        : numVertices(vertices), cityNames(names) {
        if ((int)names.size() != vertices)
            throw std::invalid_argument("City names count must match vertex count");
        adjMatrix.assign(vertices, std::vector<int>(vertices, 0));
    }

    void setVertices(int vertices) {
        numVertices = vertices;
        cityNames.clear();
        for (int i = 0; i < vertices; i++) cityNames.push_back("Node " + std::to_string(i));
        adjMatrix.assign(vertices, std::vector<int>(vertices, 0));
        adjList.clear();
    }

    std::vector<int> dfs(int start) const {
        return const_cast<Graph*>(this)->DFS(start);
    }

    std::vector<int> bfs(int start) const {
        return const_cast<Graph*>(this)->BFS(start);
    }

    std::vector<std::vector<int>> warshall() const {
        auto tc = const_cast<Graph*>(this)->warshallTransitiveClosure();
        std::vector<std::vector<int>> res(numVertices, std::vector<int>(numVertices, 0));
        for (int i = 0; i < numVertices; i++)
            for (int j = 0; j < numVertices; j++)
                res[i][j] = tc[i][j] ? 1 : 0;
        return res;
    }

    // ── Edge operations ───────────────────────────────────────
    void addEdge(int u, int v, int weight = 1) {
        validateVertex(u); validateVertex(v);
        if (weight <= 0) throw std::invalid_argument("Edge weight must be positive");
        adjMatrix[u][v] = weight;
        adjMatrix[v][u] = weight; // undirected
        // Adjacency list (avoid duplicates)
        auto& lu = adjList[u];
        auto& lv = adjList[v];
        auto it = std::find_if(lu.begin(), lu.end(),
                               [v](const std::pair<int,int>& p){ return p.first == v; });
        if (it == lu.end()) lu.push_back({v, weight});
        else                it->second = weight;
        auto it2 = std::find_if(lv.begin(), lv.end(),
                                [u](const std::pair<int,int>& p){ return p.first == u; });
        if (it2 == lv.end()) lv.push_back({u, weight});
        else                 it2->second = weight;
    }

    void removeEdge(int u, int v) {
        validateVertex(u); validateVertex(v);
        adjMatrix[u][v] = 0;
        adjMatrix[v][u] = 0;
        auto& lu = adjList[u];
        lu.erase(std::remove_if(lu.begin(), lu.end(),
                 [v](const std::pair<int,int>& p){ return p.first == v; }), lu.end());
        auto& lv = adjList[v];
        lv.erase(std::remove_if(lv.begin(), lv.end(),
                 [u](const std::pair<int,int>& p){ return p.first == u; }), lv.end());
    }

    // ── DFS ───────────────────────────────────────────────────
    std::vector<int> DFS(int start) const {
        validateVertex(start);
        std::vector<bool> visited(numVertices, false);
        std::vector<int>  order;
        std::function<void(int)> dfs = [&](int v) {
            visited[v] = true;
            order.push_back(v);
            for (const auto& nb : getNeighbors(v))
                if (!visited[nb.first]) dfs(nb.first);
        };
        dfs(start);
        return order;
    }

    // ── BFS ───────────────────────────────────────────────────
    std::vector<int> BFS(int start) const {
        validateVertex(start);
        std::vector<bool> visited(numVertices, false);
        std::vector<int>  order;
        std::queue<int>   q;
        visited[start] = true;
        q.push(start);
        while (!q.empty()) {
            int v = q.front(); q.pop();
            order.push_back(v);
            for (const auto& nb : getNeighbors(v)) {
                if (!visited[nb.first]) {
                    visited[nb.first] = true;
                    q.push(nb.first);
                }
            }
        }
        return order;
    }

    // ── Warshall Transitive Closure ───────────────────────────
    std::vector<std::vector<bool>> warshallTransitiveClosure() const {
        std::vector<std::vector<bool>> reach(numVertices,
                                             std::vector<bool>(numVertices, false));
        // Initialise from adjacency matrix
        for (int i = 0; i < numVertices; i++)
            for (int j = 0; j < numVertices; j++)
                reach[i][j] = (i == j) || (adjMatrix[i][j] != 0);
        // Warshall's algorithm
        for (int k = 0; k < numVertices; k++)
            for (int i = 0; i < numVertices; i++)
                for (int j = 0; j < numVertices; j++)
                    if (reach[i][k] && reach[k][j]) reach[i][j] = true;
        return reach;
    }

    // ── Accessors ─────────────────────────────────────────────
    const std::vector<std::vector<int>>& getAdjMatrix() const { return adjMatrix; }

    std::map<int, std::vector<std::pair<int,int>>> getAdjList() const {
        // Fill in any vertices that have no edges
        std::map<int, std::vector<std::pair<int,int>>> full = adjList;
        for (int i = 0; i < numVertices; i++)
            if (full.find(i) == full.end()) full[i] = {};
        return full;
    }

    int  getNumVertices() const { return numVertices; }
    const std::string& getCityName(int v) const { validateVertex(v); return cityNames[v]; }
    const std::vector<std::string>& getCityNames() const { return cityNames; }

    // ── JSON output ───────────────────────────────────────────
    std::string toJSON() const {
        std::ostringstream oss;

        // Cities
        oss << "{\"cities\":[";
        for (int i = 0; i < numVertices; i++) {
            if (i > 0) oss << ",";
            oss << "{\"id\":" << i << ",\"name\":\"" << cityNames[i] << "\"}";
        }
        oss << "]";

        // Edges (from upper triangle of adj matrix to avoid duplication)
        oss << ",\"edges\":[";
        bool firstEdge = true;
        for (int i = 0; i < numVertices; i++) {
            for (int j = i + 1; j < numVertices; j++) {
                if (adjMatrix[i][j] != 0) {
                    if (!firstEdge) oss << ",";
                    oss << "{\"from\":" << i
                        << ",\"to\":" << j
                        << ",\"fromCity\":\"" << cityNames[i] << "\""
                        << ",\"toCity\":\"" << cityNames[j] << "\""
                        << ",\"weight\":" << adjMatrix[i][j] << "}";
                    firstEdge = false;
                }
            }
        }
        oss << "]";

        // DFS from vertex 0
        oss << ",\"dfs\":[";
        auto dfsOrder = DFS(0);
        for (size_t i = 0; i < dfsOrder.size(); i++) {
            if (i > 0) oss << ",";
            oss << "\"" << cityNames[dfsOrder[i]] << "\"";
        }
        oss << "]";

        // BFS from vertex 0
        oss << ",\"bfs\":[";
        auto bfsOrder = BFS(0);
        for (size_t i = 0; i < bfsOrder.size(); i++) {
            if (i > 0) oss << ",";
            oss << "\"" << cityNames[bfsOrder[i]] << "\"";
        }
        oss << "]";

        // Transitive closure
        oss << ",\"transitiveClosure\":[";
        auto tc = warshallTransitiveClosure();
        for (int i = 0; i < numVertices; i++) {
            if (i > 0) oss << ",";
            oss << "[";
            for (int j = 0; j < numVertices; j++) {
                if (j > 0) oss << ",";
                oss << (tc[i][j] ? "true" : "false");
            }
            oss << "]";
        }
        oss << "]";

        oss << "}";
        return oss.str();
    }

private:
    // Helper: get neighbors of v (from adjList or from matrix if not in list)
    std::vector<std::pair<int,int>> getNeighbors(int v) const {
        auto it = adjList.find(v);
        if (it != adjList.end()) return it->second;
        // fallback: build from matrix
        std::vector<std::pair<int,int>> nb;
        for (int j = 0; j < numVertices; j++)
            if (adjMatrix[v][j] != 0) nb.push_back({j, adjMatrix[v][j]});
        return nb;
    }
};
