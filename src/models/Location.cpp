#include "epfd/models/Location.hpp"
#include <sstream>
#include <stdexcept>

namespace epfd {

namespace {
constexpr double EARTH_RADIUS_KM = 6371.0;
constexpr double PI = 3.14159265358979323846;

inline double toRadians(double degrees) {
    return degrees * (PI / 180.0);
}
} // namespace

Location::Location(double latitude, double longitude, std::string city, std::string country, std::string postal_code)
    : city_(std::move(city)), country_(std::move(country)), postal_code_(std::move(postal_code)) {
    setCoordinates(latitude, longitude);
}

void Location::setCoordinates(double latitude, double longitude) {
    if (latitude < -90.0 || latitude > 90.0) {
        throw std::invalid_argument("Latitude must be between -90 and 90 degrees");
    }
    if (longitude < -180.0 || longitude > 180.0) {
        throw std::invalid_argument("Longitude must be between -180 and 180 degrees");
    }
    latitude_ = latitude;
    longitude_ = longitude;
}

bool Location::isValid() const noexcept {
    return (latitude_ >= -90.0 && latitude_ <= 90.0) &&
           (longitude_ >= -180.0 && longitude_ <= 180.0) &&
           !country_.empty();
}

double Location::distanceKmTo(const Location& other) const noexcept {
    if (!isValid() || !other.isValid()) {
        return 0.0;
    }

    double lat1 = toRadians(latitude_);
    double lon1 = toRadians(longitude_);
    double lat2 = toRadians(other.latitude_);
    double lon2 = toRadians(other.longitude_);

    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    double a = std::sin(dlat / 2.0) * std::sin(dlat / 2.0) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dlon / 2.0) * std::sin(dlon / 2.0);

    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return EARTH_RADIUS_KM * c;
}

std::string Location::toString() const {
    std::ostringstream oss;
    oss << city_;
    if (!city_.empty() && !country_.empty()) {
        oss << ", ";
    }
    oss << country_ << " (" << latitude_ << ", " << longitude_ << ")";
    return oss.str();
}

} // namespace epfd
