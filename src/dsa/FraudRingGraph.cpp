#include "epfd/dsa/FraudRingGraph.hpp"

namespace epfd {

void FraudRingGraph::addEdge(const std::string& node_a, NodeType type_a,
                             const std::string& node_b, NodeType type_b) {
    std::lock_guard<std::mutex> lock(mutex_);
    node_types_[node_a] = type_a;
    node_types_[node_b] = type_b;

    adj_list_[node_a].insert(node_b);
    adj_list_[node_b].insert(node_a);
}

dsa::HashSet<std::string> FraudRingGraph::getNeighbors(const std::string& node_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = adj_list_.find(node_id);
    if (it != adj_list_.end()) {
        return it->second;
    }
    return {};
}

size_t FraudRingGraph::getDegree(const std::string& node_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = adj_list_.find(node_id);
    if (it != adj_list_.end()) {
        return it->second.size();
    }
    return 0;
}

dsa::HashSet<std::string> FraudRingGraph::findConnectedComponent(const std::string& root_id, size_t max_depth) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!adj_list_.contains(root_id)) {
        return {};
    }

    dsa::HashSet<std::string> visited;
    dsa::Queue<std::pair<std::string, size_t>> q;

    visited.insert(root_id);
    q.push({root_id, 0});

    while (!q.empty()) {
        auto curr_pair = q.front();
        std::string curr = curr_pair.first;
        size_t depth = curr_pair.second;
        q.pop();

        if (depth >= max_depth) {
            continue;
        }

        auto it = adj_list_.find(curr);
        if (it != adj_list_.end()) {
            for (const auto& neighbor : it->second) {
                if (!visited.contains(neighbor)) {
                    visited.insert(neighbor);
                    q.push({neighbor, depth + 1});
                }
            }
        }
    }

    return visited;
}

bool FraudRingGraph::isSharedSuspiciously(const std::string& entity_id, size_t threshold) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = adj_list_.find(entity_id);
    if (it == adj_list_.end()) {
        return false;
    }

    size_t customer_count = 0;
    for (const auto& neighbor : it->second) {
        auto type_it = node_types_.find(neighbor);
        if (type_it != node_types_.end() && type_it->second == NodeType::CUSTOMER) {
            customer_count++;
        }
    }

    return customer_count >= threshold;
}

size_t FraudRingGraph::nodeCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return node_types_.size();
}

size_t FraudRingGraph::edgeCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    size_t total = 0;
    for (const auto& entry : adj_list_) {
        total += entry.second.size();
    }
    return total / 2; // undirected
}

void FraudRingGraph::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    node_types_.clear();
    adj_list_.clear();
}

} // namespace epfd
