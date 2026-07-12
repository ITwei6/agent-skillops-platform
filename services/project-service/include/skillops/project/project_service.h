#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace skillops::project {

struct Project {
    std::string id;
    std::string team_id;
    std::string name;
    std::string description;
    std::string repo_url;
    std::string status;
    std::string created_by;
};

struct CreateProjectRequest {
    std::string team_id;
    std::string name;
    std::string description;
    std::string repo_url;
    std::string created_by;
};

class ProjectService {
public:
    Project CreateProject(const CreateProjectRequest& request);
    std::vector<Project> ListProjects() const;
    std::optional<Project> GetProject(const std::string& id) const;

private:
    std::uint64_t next_project_id_{1};
    std::vector<Project> projects_;
};

std::string ProjectToJson(const Project& project);
std::string ProjectListToJson(const std::vector<Project>& projects);

}  // namespace skillops::project
