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

struct AnalysisJob {
    std::string id;
    std::string job_type;
    std::string target_type;
    std::string target_id;
    std::string project_id;
    std::string status;
    std::string error_message;
    std::uint64_t retry_count = 0;
};

class ExperienceService {
public:
    Experience CreateExperience(const CreateExperienceRequest& request);
    std::vector<Experience> ListExperiences(const std::string& project_id) const;
    std::optional<Experience> GetExperience(const std::string& id) const;
    std::optional<AnalysisJob> GetAnalysisJob(const std::string& id) const;
    std::optional<AnalysisJob> RetryAnalysisJob(const std::string& id);

private:
    std::uint64_t next_experience_id_{1};
    std::uint64_t next_analysis_job_id_{1};
    std::vector<Experience> experiences_;
    std::vector<AnalysisJob> analysis_jobs_;
};

std::string ExperienceToJson(const Experience& experience);
std::string ExperienceListToJson(const std::vector<Experience>& experiences);
std::string AnalysisJobToJson(const AnalysisJob& job);

}  // namespace skillops::experience
