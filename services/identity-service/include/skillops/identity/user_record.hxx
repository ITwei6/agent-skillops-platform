#pragma once

#include <cstdint>
#include <string>

#include <odb/core.hxx>

namespace skillops::identity {

#pragma db object table("users")
class UserRecord {
public:
    UserRecord() = default;

    std::uint64_t id() const { return id_; }
    void id(std::uint64_t value) { id_ = value; }

    const std::string& team_id() const { return team_id_; }
    void team_id(const std::string& value) { team_id_ = value; }

    const std::string& name() const { return name_; }
    void name(const std::string& value) { name_ = value; }

    const std::string& email() const { return email_; }
    void email(const std::string& value) { email_ = value; }

    const std::string& role() const { return role_; }
    void role(const std::string& value) { role_ = value; }

    const std::string& status() const { return status_; }
    void status(const std::string& value) { status_ = value; }

private:
    friend class odb::access;

    #pragma db id auto
    std::uint64_t id_{0};

    std::string team_id_;
    std::string name_;
    std::string email_;
    std::string role_;
    std::string status_;
};

}  // namespace skillops::identity
