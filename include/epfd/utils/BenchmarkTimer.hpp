#ifndef EPFD_UTILS_BENCHMARK_TIMER_HPP
#define EPFD_UTILS_BENCHMARK_TIMER_HPP

#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <sstream>
#include <iomanip>

namespace epfd {

struct LatencyStats {
    size_t sample_count{0};
    double total_time_ms{0.0};
    double tps{0.0};
    double min_us{0.0};
    double max_us{0.0};
    double mean_us{0.0};
    double p50_us{0.0};
    double p90_us{0.0};
    double p95_us{0.0};
    double p99_us{0.0};

    std::string toString() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "Samples: " << sample_count
            << " | Throughput: " << tps << " TPS"
            << " | Mean: " << mean_us << " us"
            << " | P50: " << p50_us << " us"
            << " | P95: " << p95_us << " us"
            << " | P99: " << p99_us << " us"
            << " | Min: " << min_us << " us"
            << " | Max: " << max_us << " us";
        return oss.str();
    }
};

class BenchmarkTimer {
public:
    BenchmarkTimer() = default;

    void recordMicroseconds(double us) {
        samples_us_.push_back(us);
    }

    void clear() {
        samples_us_.clear();
    }

    LatencyStats computeStats() {
        LatencyStats stats;
        stats.sample_count = samples_us_.size();
        if (samples_us_.empty()) return stats;

        std::vector<double> sorted = samples_us_;
        std::sort(sorted.begin(), sorted.end());

        double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
        stats.min_us = sorted.front();
        stats.max_us = sorted.back();
        stats.mean_us = sum / sorted.size();
        stats.total_time_ms = sum / 1000.0;
        stats.tps = stats.total_time_ms > 0 ? (sorted.size() * 1000.0 / stats.total_time_ms) : 0.0;

        stats.p50_us = getPercentile(sorted, 50.0);
        stats.p90_us = getPercentile(sorted, 90.0);
        stats.p95_us = getPercentile(sorted, 95.0);
        stats.p99_us = getPercentile(sorted, 99.0);

        return stats;
    }

private:
    static double getPercentile(const std::vector<double>& sorted, double percentile) {
        if (sorted.empty()) return 0.0;
        size_t idx = static_cast<size_t>((percentile / 100.0) * (sorted.size() - 1));
        return sorted[idx];
    }

    std::vector<double> samples_us_;
};

} // namespace epfd

#endif // EPFD_UTILS_BENCHMARK_TIMER_HPP
