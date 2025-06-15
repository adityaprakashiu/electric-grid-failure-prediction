#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <limits>

using namespace std;

// Structure to represent an edge (transmission line)
struct Edge {
    int to; // Destination node
    double capacity; // Max capacity in MW
    double currentLoad; // Current load in MW
};

// Structure to represent a node (substation)
struct Node {
    string name; // Node identifier
    double load; // Current power demand in MW
    double maxCapacity; // Max capacity in MW
};

// Graph class to represent the electric grid
class Graph {
private:
    vector<Node> nodes; // List of nodes
    vector<vector<Edge>> adj; // Adjacency list for edges
    int numNodes;

    // DFS to check if grid is connected
    void DFS(int v, vector<bool>& visited) {
        visited[v] = true;
        for (const Edge& e : adj[v]) {
            if (!visited[e.to]) {
                DFS(e.to, visited);
            }
        }
    }

public:
    Graph(int n) : numNodes(n) {
        nodes.resize(n);
        adj.resize(n);
    }

    // Add a node (substation)
    void addNode(int idx, string name, double load, double maxCapacity) {
        if (load < 0 || maxCapacity <= 0 || load > maxCapacity) {
            cout << "Invalid load or max capacity for node " << name << ". Load must be >= 0, max capacity > 0, and load <= max capacity.\n";
            exit(1);
        }
        nodes[idx] = {name, load, maxCapacity};
    }

    // Add an edge (transmission line)
    void addEdge(int from, int to, double capacity, double currentLoad) {
        if (from < 0 || from >= numNodes || to < 0 || to >= numNodes) {
            cout << "Invalid node index: " << from << " or " << to << ". Must be between 0 and " << (numNodes - 1) << ".\n";
            exit(1);
        }
        if (capacity <= 0 || currentLoad < 0) {
            cout << "Invalid load or capacity for edge " << from << "-" << to 
                 << ". Capacity must be > 0, load >= 0.\n";
            exit(1);
        }
        // Removed: currentLoad <= capacity check to allow initial overloads
        adj[from].push_back({to, capacity, currentLoad});
        adj[to].push_back({from, capacity, currentLoad}); // Undirected
    }

    // Check if the grid is connected
    bool isConnected() {
        vector<bool> visited(numNodes, false);
        DFS(0, visited);
        for (bool v : visited) {
            if (!v) return false;
        }
        return true;
    }

    // Check for overloaded nodes or edges
    void checkOverloads(vector<string>& overloadedNodes, vector<pair<int, int>>& overloadedEdges) {
        // Check nodes
        for (int i = 0; i < numNodes; i++) {
            if (nodes[i].load > nodes[i].maxCapacity) {
                overloadedNodes.push_back(nodes[i].name);
            }
        }

        // Check edges
        for (int u = 0; u < numNodes; u++) {
            for (const Edge& e : adj[u]) {
                if (e.currentLoad > e.capacity && u < e.to) { // Strict inequality for overload
                    overloadedEdges.push_back({u, e.to});
                }
            }
        }
    }

    // Simulate load increase and predict failures
    void predictFailures(double loadIncreasePercent) {
        if (loadIncreasePercent < 0) {
            cout << "Load increase percentage must be >= 0.\n";
            exit(1);
        }
        cout << "\nSimulating load increase by " << loadIncreasePercent << "%\n";
        vector<string> overloadedNodes;
        vector<pair<int, int>> overloadedEdges;

        // Increase loads
        for (int i = 0; i < numNodes; i++) {
            nodes[i].load *= (1 + loadIncreasePercent / 100.0);
        }
        for (int u = 0; u < numNodes; u++) {
            for (Edge& e : adj[u]) {
                e.currentLoad *= (1 + loadIncreasePercent / 100.0);
            }
        }

        // Check overloads
        checkOverloads(overloadedNodes, overloadedEdges);

        // Output results
        if (overloadedNodes.empty() && overloadedEdges.empty()) {
            cout << "No overloads detected after load increase.\n";
        } else {
            if (!overloadedNodes.empty()) {
                cout << "Overloaded Nodes:\n";
                for (const string& name : overloadedNodes) {
                    cout << "- " << name << "\n";
                }
            }
            if (!overloadedEdges.empty()) {
                cout << "Overloaded Transmission Lines:\n";
                for (const auto& e : overloadedEdges) {
                    cout << "- Between " << nodes[e.first].name << " and " << nodes[e.second].name << "\n";
                }
            }
        }

        // Check connectivity
        if (!isConnected()) {
            cout << "Warning: Grid is disconnected after load increase!\n";
        } else {
            cout << "Grid remains connected.\n";
        }

        // Reset loads for future simulations
        for (int i = 0; i < numNodes; i++) {
            nodes[i].load /= (1 + loadIncreasePercent / 100.0);
        }
        for (int u = 0; u < numNodes; u++) {
            for (Edge& e : adj[u]) {
                e.currentLoad /= (1 + loadIncreasePercent / 100.0);
            }
        }
    }

    // Display grid status
    void displayGrid() {
        cout << "\nGrid Status:\n";
        cout << "Nodes (Substations):\n";
        for (int i = 0; i < numNodes; i++) {
            cout << "Node " << nodes[i].name << ": Load = " << nodes[i].load 
                 << " MW, Max Capacity = " << nodes[i].maxCapacity << " MW\n";
        }
        cout << "Edges (Transmission Lines):\n";
        for (int u = 0; u < numNodes; u++) {
            for (const Edge& e : adj[u]) {
                if (u < e.to) { // Avoid duplicate edges
                    cout << "Between " << nodes[u].name << " and " << nodes[e.to].name 
                         << ": Load = " << e.currentLoad << " MW, Capacity = " << e.capacity << " MW\n";
                }
            }
        }
    }

    // Getter for node name
    string getNodeName(int idx) const {
        if (idx >= 0 && idx < numNodes) {
            return nodes[idx].name;
        }
        return "Unknown";
    }
};

// Main function to handle input and run the simulation
int main() {
    int numNodes, numEdges;
    cout << "Enter number of nodes (substations): ";
    cin >> numNodes;
    if (numNodes <= 0) {
        cout << "Number of nodes must be > 0.\n";
        return 1;
    }
    Graph grid(numNodes);

    // Input nodes
    for (int i = 0; i < numNodes; i++) {
        string name;
        double load, maxCapacity;
        cout << "Enter name, load (MW), and max capacity (MW) for node " << i << ": ";
        cin >> name >> load >> maxCapacity;
        grid.addNode(i, name, load, maxCapacity);
    }

    // Input edges
    cout << "Enter number of edges (transmission lines): ";
    cin >> numEdges;
    if (numEdges < 0) {
        cout << "Number of edges must be >= 0.\n";
        return 1;
    }
    for (int i = 0; i < numEdges; i++) {
        int u, v;
        double capacity, currentLoad;
        cout << "Enter edge " << i << ": from node index, to node index, load (MW), capacity (MW): ";
        cin >> u >> v >> currentLoad >> capacity;
        grid.addEdge(u, v, capacity, currentLoad);
    }

    // Display initial grid status
    grid.displayGrid();

    // Check initial overloads
    vector<string> overloadedNodes;
    vector<pair<int, int>> overloadedEdges;
    grid.checkOverloads(overloadedNodes, overloadedEdges);
    if (!overloadedNodes.empty() || !overloadedEdges.empty()) {
        cout << "\nInitial Overloads Detected:\n";
        if (!overloadedNodes.empty()) {
            cout << "Overloaded Nodes:\n";
            for (const string& name : overloadedNodes) {
                cout << "- " << name << "\n";
            }
        }
        if (!overloadedEdges.empty()) {
            cout << "Overloaded Transmission Lines:\n";
            for (const auto& e : overloadedEdges) {
                cout << "- Between " << grid.getNodeName(e.first) << " and " << grid.getNodeName(e.second) << "\n";
            }
        }
    } else {
        cout << "\nNo initial overloads detected.\n";
    }

    // Check initial connectivity
    if (grid.isConnected()) {
        cout << "Grid is connected.\n";
    } else {
        cout << "Grid is disconnected!\n";
    }

    // Simulate load increase
    double loadIncrease;
    cout << "Enter load increase percentage to simulate (e.g., 10 for 10%): ";
    cin >> loadIncrease;
    grid.predictFailures(loadIncrease);

    return 0;
}