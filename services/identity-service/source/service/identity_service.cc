#include "skillops/identity/identity_service.h"

#include "skillops/common/json.h"

#include <stdexcept>

namespace skillops::identity {

User IdentityService::CreateUser(const CreateUserRequest& request) {
    if (request.team_id.empty() || request.name.empty() || request.email.empty() || request.role.empty()) {
        throw std::invalid_argument("team_id, name, email and role are required");
    }

    User user;
    user.id = "user_" + std::to_string(next_user_id_++);
    user.team_id = request.team_id;
    user.name = request.name;
    user.email = request.email;
    user.role = request.role;
    user.status = "active";
    users_.push_back(user);
    return user;
}

std::optional<User> IdentityService::CurrentUser() const {
    if (users_.empty()) {
        return std::nullopt;
    }
    return users_.front();
}

std::string UserToJson(const User& user) {
    return "{\"user_id\":" + skillops::common::JsonString(user.id) +
           ",\"team_id\":" + skillops::common::JsonString(user.team_id) +
           ",\"name\":" + skillops::common::JsonString(user.name) +
           ",\"email\":" + skillops::common::JsonString(user.email) +
           ",\"role\":" + skillops::common::JsonString(user.role) +
           ",\"status\":" + skillops::common::JsonString(user.status) + "}";
}

}  // namespace skillops::identity
