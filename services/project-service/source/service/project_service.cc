#include "skillops/project/project_service.h"

#include "skillops/common/json.h"
#include "skillops/project/project_record.hxx"
#include "tew_scaffold/odb.h"

#include "project_record-odb.hxx"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <utility>

#include <odb/transaction.hxx>
#include <odb/result.hxx>

namespace skillops::project {

namespace {

std::uint64_t ParseProjectId(const std::string& value) {
    if (value.rfind("proj_", 0) != 0) {
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
        "CREATE TABLE IF NOT EXISTS projects ("
        "id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,"
        "team_id VARCHAR(64) NOT NULL,"
        "name VARCHAR(128) NOT NULL,"
        "description VARCHAR(512) NOT NULL,"
        "repo_url VARCHAR(512) NOT NULL,"
        "status VARCHAR(32) NOT NULL,"
        "created_by VARCHAR(64) NOT NULL,"
        "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP"
        ")";
}

Project ToProject(const ProjectRecord& record) {
    Project project;
    project.id = "proj_" + std::to_string(record.id());
    project.team_id = record.team_id();
    project.name = record.name();
    project.description = record.description();
    project.repo_url = record.repo_url();
    project.status = record.status();
    project.created_by = record.created_by();
    return project;
}

}  // namespace

ProjectService::ProjectService(std::optional<skillops::common::DatabaseConfig> database)
    : database_(std::move(database)) {
    LoadFromDatabase();
}

void ProjectService::EnsureDatabaseSchema() {
    if (!database_.has_value() || !database_->enabled) {
        return;
    }

    const auto db = OpenDatabase(*database_);
    db->execute(BuildCreateTableSql());
}

void ProjectService::LoadFromDatabase() {
    if (!database_.has_value() || !database_->enabled) {
        return;
    }

    EnsureDatabaseSchema();
    const auto db = OpenDatabase(*database_);

    odb::transaction tx(db->begin());
    typedef odb::result<ProjectRecord> Result;
    Result rows(db->query<ProjectRecord>());
    for (auto it = rows.begin(); it != rows.end(); ++it) {
        const auto project = ToProject(*it);
        projects_.push_back(project);
        next_project_id_ = std::max(next_project_id_, ParseProjectId(project.id) + 1);
    }
    tx.commit();
}

void ProjectService::InsertProjectToDatabase(const Project& project) {
    if (!database_.has_value() || !database_->enabled) {
        return;
    }

    EnsureDatabaseSchema();
    const auto db = OpenDatabase(*database_);

    ProjectRecord record;
    record.team_id(project.team_id);
    record.name(project.name);
    record.description(project.description);
    record.repo_url(project.repo_url);
    record.status(project.status);
    record.created_by(project.created_by);

    odb::transaction tx(db->begin());
    db->persist(record);
    tx.commit();
}

Project ProjectService::CreateProject(const CreateProjectRequest& request) {
    if (request.team_id.empty() || request.name.empty()) {
        throw std::invalid_argument("team_id and name are required");
    }

    Project project;
    project.id = "proj_" + std::to_string(next_project_id_++);
    project.team_id = request.team_id;
    project.name = request.name;
    project.description = request.description;
    project.repo_url = request.repo_url;
    project.status = "active";
    project.created_by = request.created_by.empty() ? "system" : request.created_by;
    projects_.push_back(project);
    InsertProjectToDatabase(project);
    return project;
}

std::vector<Project> ProjectService::ListProjects() const {
    return projects_;
}

std::optional<Project> ProjectService::GetProject(const std::string& id) const {
    for (const auto& project : projects_) {
        if (project.id == id) {
            return project;
        }
    }
    return std::nullopt;
}

std::string ProjectToJson(const Project& project) {
    return "{\"project_id\":" + skillops::common::JsonString(project.id) +
           ",\"team_id\":" + skillops::common::JsonString(project.team_id) +
           ",\"name\":" + skillops::common::JsonString(project.name) +
           ",\"description\":" + skillops::common::JsonString(project.description) +
           ",\"repo_url\":" + skillops::common::JsonString(project.repo_url) +
           ",\"status\":" + skillops::common::JsonString(project.status) +
           ",\"created_by\":" + skillops::common::JsonString(project.created_by) + "}";
}

std::string ProjectListToJson(const std::vector<Project>& projects) {
    std::string items = "[";
    for (std::size_t i = 0; i < projects.size(); ++i) {
        if (i > 0) {
            items += ",";
        }
        items += ProjectToJson(projects[i]);
    }
    items += "]";
    return "{\"items\":" + items + ",\"page\":1,\"page_size\":20,\"total\":" + std::to_string(projects.size()) + "}";
}

}  // namespace skillops::project
