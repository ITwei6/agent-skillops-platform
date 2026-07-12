#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace skillops::experience {

struct Experience {
    std::string id;
    std::string project_id;
    std::string title;
    std::string source_type;
    std::string summary;
    std::vector<std::string> artifact_ids;
    std::string status;
    std::string analysis_job_id;
};

struct CreateExperienceRequest {
    std::string project_id;
    std::string title;
    std::string source_type;
    std::string summary;
    std::vector<std::string> artifact_ids;
};

class ExperienceService {
public:
    Experience CreateExperience(const CreateExperienceRequest& request);
    std::vector<Experience> ListExperiences(const std::string& project_id) const;
    std::optional<Experience> GetExperience(const std::string& id) const;

private:
    std::uint64_t next_experience_id_{1};
    std::uint64_t next_analysis_job_id_{1};
    std::vector<Experience> experiences_;
};

std::string ExperienceToJson(const Experience& experience);
std::string ExperienceListToJson(const std::vector<Experience>& experiences);

}  // namespace skillops::experience
