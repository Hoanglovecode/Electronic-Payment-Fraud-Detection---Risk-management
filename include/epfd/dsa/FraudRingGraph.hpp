#ifndef EPFD_DSA_FRAUD_RING_GRAPH_HPP
#define EPFD_DSA_FRAUD_RING_GRAPH_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

namespace epfd {

enum class NodeType {
    CUSTOMER,
    DEVICE,
    IP,
    CARD,
    MERCHANT
};

struct GraphNode {
    std::string id;
    NodeType type{NodeType::CUSTOMER};
};

/**
 * @brief Undirected graph for fraud ring and syndicate detection using BFS.
 */
class FraudRingGraph {
public:
    FraudRingGraph() = default;

    void addEdge(const std::string& node_a, NodeType type_a,
                 const std::string& node_b, NodeType type_b);

    std::unordered_set<std::string> getNeighbors(const std::string& node_id) const;
    size_t getDegree(const std::string& node_id) const;

    /**
     * @brief Finds all connected entities up to max_depth hops using BFS.
     */
    std::unordered_set<std::string> findConnectedComponent(const std::string& root_id, size_t max_depth = 3) const;

    /**
     * @brief Detects if a device or IP is shared across more than `threshold` distinct customers.
     */
    bool isSharedSuspiciously(const std::string& entity_id, size_t threshold = 3) const;

    size_t nodeCount() const;
    size_t edgeCount() const;
    void clear();

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, NodeType> node_types_;
    std::unordered_map<std::string, std::unordered_set<std::string>> adj_list_;
};

} // namespace epfd

#endif // EPFD_DSA_FRAUD_RING_GRAPH_HPP
