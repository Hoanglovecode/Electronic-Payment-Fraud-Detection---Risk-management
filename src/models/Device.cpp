#include "epfd/models/Device.hpp"
#include <sstream>

namespace epfd {

Device::Device(std::string device_id, 
               std::string device_fingerprint, 
               std::string ip_address, 
               std::string user_agent,
               bool is_emulator,
               bool is_rooted_or_jailbroken,
               Location location)
    : device_id_(std::move(device_id)),
      device_fingerprint_(std::move(device_fingerprint)),
      ip_address_(std::move(ip_address)),
      user_agent_(std::move(user_agent)),
      is_emulator_(is_emulator),
      is_rooted_or_jailbroken_(is_rooted_or_jailbroken),
      location_(std::move(location)) {}

bool Device::isHighRiskEnvironment() const noexcept {
    return is_emulator_ || is_rooted_or_jailbroken_;
}

std::string Device::toString() const {
    std::ostringstream oss;
    oss << "Device[id=" << device_id_ 
        << ", ip=" << ip_address_ 
        << ", emulator=" << (is_emulator_ ? "true" : "false")
        << ", rooted=" << (is_rooted_or_jailbroken_ ? "true" : "false")
        << "]";
    return oss.str();
}

} // namespace epfd
