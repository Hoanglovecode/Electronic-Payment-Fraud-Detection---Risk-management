#ifndef EPFD_MODELS_LOCATION_HPP
#define EPFD_MODELS_LOCATION_HPP

#include <string>
#include <cmath>

namespace epfd {

class Location {
public:
    Location() = default;
    Location(double latitude, double longitude, std::string city, std::string country, std::string postal_code = "");

    // Getters
    double getLatitude() const noexcept { return latitude_; }
    double getLongitude() const noexcept { return longitude_; }
    const std::string& getCity() const noexcept { return city_; }
    const std::string& getCountry() const noexcept { return country_; }
    const std::string& getPostalCode() const noexcept { return postal_code_; }

    // Setters
    void setCoordinates(double latitude, double longitude);
    void setCity(std::string city) { city_ = std::move(city); }
    void setCountry(std::string country) { country_ = std::move(country); }
    void setPostalCode(std::string postal_code) { postal_code_ = std::move(postal_code); }

    // Domain methods
    bool isValid() const noexcept;
    double distanceKmTo(const Location& other) const noexcept; // Haversine formula
    std::string toString() const;

private:
    double latitude_{0.0};
    double longitude_{0.0};
    std::string city_;
    std::string country_;
    std::string postal_code_;
};

} // namespace epfd

#endif // EPFD_MODELS_LOCATION_HPP
