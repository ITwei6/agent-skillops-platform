#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

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
    User CreateUser(const CreateUserRequest& request);
    std::optional<User> CurrentUser() const;

private:
    std::uint64_t next_user_id_{1};
    std::vector<User> users_;
};

std::string UserToJson(const User& user);

}  // namespace skillops::identity
