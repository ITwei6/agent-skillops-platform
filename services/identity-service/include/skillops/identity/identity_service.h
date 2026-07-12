#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "skillops/common/config.h"

namespace skillops::identity {

struct User {
    std::string id;
    std::string team_id;
    std::string name;
    std::string email;
    std::string role;
    std::string status;
};

struct CreateUserRequest {
    std::string team_id;
    std::string name;
    std::string email;
    std::string role;
};

class IdentityService {
public:
    explicit IdentityService(std::optional<skillops::common::DatabaseConfig> database = std::nullopt);

    User CreateUser(const CreateUserRequest& request);
    std::optional<User> CurrentUser() const;

private:
    void LoadFromDatabase();
    void EnsureDatabaseSchema();
    void InsertUserToDatabase(const User& user);

    std::uint64_t next_user_id_{1};
    std::vector<User> users_;
    std::optional<skillops::common::DatabaseConfig> database_;
};

std::string UserToJson(const User& user);

}  // namespace skillops::identity
