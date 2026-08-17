#ifndef EPFD_FRAUD_I_FRAUD_DETECTOR_HPP
#define EPFD_FRAUD_I_FRAUD_DETECTOR_HPP

#include <string>
#include <vector>
#include "epfd/models/Transaction.hpp"
#include "epfd/models/FraudAlert.hpp"

namespace epfd {

/**
 * @brief High-level interface for fraud detectors (LSP & ISP).
 * Allows interchangeable usage of Rule-based, ML-based, or Hybrid detectors.
 */
class IFraudDetector {
public:
    virtual ~IFraudDetector() = default;

    virtual const std::string& getName() const noexcept = 0;
    virtual std::vector<FraudAlert> detect(const Transaction& tx) = 0;
    virtual double computeFraudScore(const Transaction& tx) = 0;
};

} // namespace epfd

#endif // EPFD_FRAUD_I_FRAUD_DETECTOR_HPP
