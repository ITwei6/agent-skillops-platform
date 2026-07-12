#pragma once

#include <cstdint>
#include <string>

#include <odb/core.hxx>

namespace skillops::project {

#pragma db object table("projects")
class ProjectRecord {
public:
    ProjectRecord() = default;

    std::uint64_t id() const { return id_; }
    void id(std::uint64_t value) { id_ = value; }

    const std::string& team_id() const { return team_id_; }
    void team_id(const std::string& value) { team_id_ = value; }

    const std::string& name() const { return name_; }
    void name(const std::string& value) { name_ = value; }

    const std::string& description() const { return description_; }
    void description(const std::string& value) { description_ = value; }

    const std::string& repo_url() const { return repo_url_; }
    void repo_url(const std::string& value) { repo_url_ = value; }

    const std::string& status() const { return status_; }
    void status(const std::string& value) { status_ = value; }

    const std::string& created_by() const { return created_by_; }
    void created_by(const std::string& value) { created_by_ = value; }

private:
    friend class odb::access;

    #pragma db id auto
    std::uint64_t id_{0};

    std::string team_id_;
    std::string name_;
    std::string description_;
    std::string repo_url_;
    std::string status_;
    std::string created_by_;
};

}  // namespace skillops::project
