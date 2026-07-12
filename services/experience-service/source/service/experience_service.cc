#include "skillops/experience/experience_service.h"

#include "skillops/common/json.h"

#include <stdexcept>

namespace skillops::experience {

Experience ExperienceService::CreateExperience(const CreateExperienceRequest& request) {
    if (request.project_id.empty() || request.title.empty() || request.source_type.empty() || request.summary.empty()) {
        throw std::invalid_argument("project_id, title, source_type and summary are required");
    }

    Experience experience;
    experience.id = "exp_" + std::to_string(next_experience_id_++);
    experience.project_id = request.project_id;
    experience.title = request.title;
    experience.source_type = request.source_type;
    experience.summary = request.summary;
    experience.artifact_ids = request.artifact_ids;
    experience.status = "analysis_queued";
    experience.analysis_job_id = "job_" + std::to_string(next_analysis_job_id_++);
    experiences_.push_back(experience);

    AnalysisJob job;
    job.id = experience.analysis_job_id;
    job.job_type = "generate_skill";
    job.target_type = "experience";
    job.target_id = experience.id;
    job.project_id = experience.project_id;
    job.status = "queued";
    analysis_jobs_.push_back(job);

    return experience;
}

std::vector<Experience> ExperienceService::ListExperiences(const std::string& project_id) const {
    if (project_id.empty()) {
        return experiences_;
    }

    std::vector<Experience> filtered;
    for (const auto& experience : experiences_) {
        if (experience.project_id == project_id) {
            filtered.push_back(experience);
        }
    }
    return filtered;
}

std::optional<Experience> ExperienceService::GetExperience(const std::string& id) const {
    for (const auto& experience : experiences_) {
        if (experience.id == id) {
            return experience;
        }
    }
    return std::nullopt;
}

std::optional<AnalysisJob> ExperienceService::GetAnalysisJob(const std::string& id) const {
    for (const auto& job : analysis_jobs_) {
        if (job.id == id) {
            return job;
        }
    }
    return std::nullopt;
}

std::optional<AnalysisJob> ExperienceService::RetryAnalysisJob(const std::string& id) {
    for (auto& job : analysis_jobs_) {
        if (job.id == id) {
            job.status = "queued";
            job.error_message = "";
            ++job.retry_count;
            return job;
        }
    }
    return std::nullopt;
}

std::string ExperienceToJson(const Experience& experience) {
    return "{\"experience_id\":" + skillops::common::JsonString(experience.id) +
           ",\"project_id\":" + skillops::common::JsonString(experience.project_id) +
           ",\"title\":" + skillops::common::JsonString(experience.title) +
           ",\"source_type\":" + skillops::common::JsonString(experience.source_type) +
           ",\"summary\":" + skillops::common::JsonString(experience.summary) +
           ",\"artifact_ids\":" + skillops::common::JsonStringArray(experience.artifact_ids) +
           ",\"status\":" + skillops::common::JsonString(experience.status) +
           ",\"analysis_job_id\":" + skillops::common::JsonString(experience.analysis_job_id) + "}";
}

std::string AnalysisJobToJson(const AnalysisJob& job) {
    return "{\"job_id\":" + skillops::common::JsonString(job.id) +
           ",\"job_type\":" + skillops::common::JsonString(job.job_type) +
           ",\"target_type\":" + skillops::common::JsonString(job.target_type) +
           ",\"target_id\":" + skillops::common::JsonString(job.target_id) +
           ",\"project_id\":" + skillops::common::JsonString(job.project_id) +
           ",\"status\":" + skillops::common::JsonString(job.status) +
           ",\"error_message\":" + skillops::common::JsonString(job.error_message) +
           ",\"retry_count\":" + std::to_string(job.retry_count) + "}";
}

std::string ExperienceListToJson(const std::vector<Experience>& experiences) {
    std::string items = "[";
    for (std::size_t i = 0; i < experiences.size(); ++i) {
        if (i > 0) {
            items += ",";
        }
        items += ExperienceToJson(experiences[i]);
    }
    items += "]";
    return "{\"items\":" + items + ",\"page\":1,\"page_size\":20,\"total\":" + std::to_string(experiences.size()) + "}";
}

}  // namespace skillops::experience
