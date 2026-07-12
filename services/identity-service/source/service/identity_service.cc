#include "skillops/identity/identity_service.h"

#include "skillops/common/json.h"
#include "skillops/identity/user_record.hxx"
#include "tew_scaffold/odb.h"

#include "user_record-odb.hxx"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>

#include <odb/transaction.hxx>
#include <odb/result.hxx>

namespace skillops::identity {

namespace {

std::uint64_t ParseUserId(const std::string& value) {
    if (value.rfind("user_", 0) != 0) {
        return 0;
    }
    return static_cast<std::uint64_t>(std::stoull(value.substr(5)));
}

tewodb::mysql_settings ToMysqlSettings(const skillops::common::DatabaseConfig& config) {
    tewodb::mysql_settings settings;
    settings.host = config.host;
    settings.port = config.port;
    settings.user = config.user.empty() ? "root" : config.user;
    settings.passwd = config.password;
    settings.db = config.schema;
    return settings;
}

std::shared_ptr<odb::database> OpenDatabase(const skillops::common::DatabaseConfig& config) {
    return tewodb::DBFactory::mysql(ToMysqlSettings(config));
}

std::string BuildCreateTableSql() {
    return
        "CREATE TABLE IF NOT EXISTS users ("
        "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,"
        "team_id VARCHAR(64) NOT NULL,"
        "name VARCHAR(128) NOT NULL,"
        "email VARCHAR(256) NOT NULL,"
        "role VARCHAR(32) NOT NULL,"
        "status VARCHAR(32) NOT NULL,"
        "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP"
        ")";
}

User ToUser(const UserRecord& record) {
    User user;
    user.id = "user_" + std::to_string(record.id());
    user.team_id = record.team_id();
    user.name = record.name();
    user.email = record.email();
    user.role = record.role();
    user.status = record.status();
    return user;
}

}  // namespace

IdentityService::IdentityService(std::optional<skillops::common::DatabaseConfig> database)
    : database_(std::move(database)) {
    LoadFromDatabase();
}

void IdentityService::EnsureDatabaseSchema() {
    if (!database_.has_value() || !database_->enabled) {
        return;
    }

    const auto db = OpenDatabase(*database_);
    db->execute(BuildCreateTableSql());
}

void IdentityService::LoadFromDatabase() {
    if (!database_.has_value() || !database_->enabled) {
        return;
    }

    EnsureDatabaseSchema();
    const auto db = OpenDatabase(*database_);

    odb::transaction tx(db->begin());
    typedef odb::result<UserRecord> Result;
    Result rows(db->query<UserRecord>());
    for (auto it = rows.begin(); it != rows.end(); ++it) {
        const auto user = ToUser(*it);
        users_.push_back(user);
        next_user_id_ = std::max(next_user_id_, ParseUserId(user.id) + 1);
    }
    tx.commit();
}

void IdentityService::InsertUserToDatabase(const User& user) {
    if (!database_.has_value() || !database_->enabled) {
        return;
    }

    EnsureDatabaseSchema();
    const auto db = OpenDatabase(*database_);

    UserRecord record;
    record.team_id(user.team_id);
    record.name(user.name);
    record.email(user.email);
    record.role(user.role);
    record.status(user.status);

    odb::transaction tx(db->begin());
    db->persist(record);
    tx.commit();
}

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
    InsertUserToDatabase(user);
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
