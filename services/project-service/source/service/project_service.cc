#include "skillops/project/project_service.h"

#include "skillops/common/json.h"

#include <stdexcept>

namespace skillops::project {

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
