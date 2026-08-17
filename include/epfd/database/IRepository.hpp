#ifndef EPFD_DATABASE_I_REPOSITORY_HPP
#define EPFD_DATABASE_I_REPOSITORY_HPP

#include <vector>
#include <optional>
#include <cstddef>

namespace epfd {

/**
 * @brief Generic Repository Pattern Interface (DIP & ISP).
 * Isolates business logic from data storage mechanism (In-Memory, SQLite, PostgreSQL).
 */
template <typename Entity, typename ID = std::string>
class IRepository {
public:
    virtual ~IRepository() = default;

    virtual bool save(const Entity& entity) = 0;
    virtual std::optional<Entity> findById(const ID& id) const = 0;
    virtual std::vector<Entity> findAll() const = 0;
    virtual bool remove(const ID& id) = 0;
    virtual size_t count() const = 0;
    virtual void clear() = 0;
};

} // namespace epfd

#endif // EPFD_DATABASE_I_REPOSITORY_HPP
