#ifndef EPFD_MODELS_DEVICE_HPP
#define EPFD_MODELS_DEVICE_HPP

#include <string>
#include "epfd/models/Location.hpp"

namespace epfd {

class Device {
public:
    Device() = default;
    Device(std::string device_id, 
           std::string device_fingerprint, 
           std::string ip_address, 
           std::string user_agent = "",
           bool is_emulator = false,
           bool is_rooted_or_jailbroken = false,
           Location location = Location{});

    // Getters
    const std::string& getDeviceId() const noexcept { return device_id_; }
    const std::string& getDeviceFingerprint() const noexcept { return device_fingerprint_; }
    const std::string& getIpAddress() const noexcept { return ip_address_; }
    const std::string& getUserAgent() const noexcept { return user_agent_; }
    bool isEmulator() const noexcept { return is_emulator_; }
    bool isRootedOrJailbroken() const noexcept { return is_rooted_or_jailbroken_; }
    const Location& getLocation() const noexcept { return location_; }

    // Setters
    void setDeviceId(std::string device_id) { device_id_ = std::move(device_id); }
    void setDeviceFingerprint(std::string fingerprint) { device_fingerprint_ = std::move(fingerprint); }
    void setIpAddress(std::string ip) { ip_address_ = std::move(ip); }
    void setUserAgent(std::string ua) { user_agent_ = std::move(ua); }
    void setIsEmulator(bool val) noexcept { is_emulator_ = val; }
    void setIsRootedOrJailbroken(bool val) noexcept { is_rooted_or_jailbroken_ = val; }
    void setLocation(Location loc) { location_ = std::move(loc); }

    // Domain methods
    bool isHighRiskEnvironment() const noexcept;
    std::string toString() const;

private:
    std::string device_id_;
    std::string device_fingerprint_;
    std::string ip_address_;
    std::string user_agent_;
    bool is_emulator_{false};
    bool is_rooted_or_jailbroken_{false};
    Location location_;
};

} // namespace epfd

#endif // EPFD_MODELS_DEVICE_HPP
