#include "skillops/skill/skill_service.h"

#include "skillops/common/json.h"

#include <stdexcept>

namespace skillops::skill {

SkillDraft SkillService::CreateDraft(const CreateSkillDraftRequest& request) {
    if (request.project_id.empty() || request.experience_id.empty() || request.name.empty() ||
        request.description.empty() || request.skill_md.empty()) {
        throw std::invalid_argument("project_id, experience_id, name, description and skill_md are required");
    }

    SkillDraft draft;
    draft.id = "draft_" + std::to_string(next_draft_id_++);
    draft.project_id = request.project_id;
    draft.experience_id = request.experience_id;
    draft.name = request.name;
    draft.description = request.description;
    draft.skill_md = request.skill_md;
    draft.status = "draft";
    drafts_.push_back(draft);
    return draft;
}

std::optional<SkillDraft> SkillService::GetDraft(const std::string& id) const {
    for (const auto& draft : drafts_) {
        if (draft.id == id) {
            return draft;
        }
    }
    return std::nullopt;
}

std::optional<SkillDraft> SkillService::UpdateDraft(const std::string& id, const UpdateSkillDraftRequest& request) {
    for (auto& draft : drafts_) {
        if (draft.id != id) {
            continue;
        }
        if (!request.name.empty()) {
            draft.name = request.name;
        }
        if (!request.description.empty()) {
            draft.description = request.description;
        }
        if (!request.skill_md.empty()) {
            draft.skill_md = request.skill_md;
        }
        return draft;
    }
    return std::nullopt;
}

std::optional<SkillDraft> SkillService::SubmitDraft(const std::string& id) {
    for (auto& draft : drafts_) {
        if (draft.id == id) {
            draft.status = "submitted";
            return draft;
        }
    }
    return std::nullopt;
}

std::optional<PublishedSkill> SkillService::PublishDraft(
    const std::string& id,
    const std::string& version,
    const std::string& changelog) {
    if (version.empty()) {
        throw std::invalid_argument("version is required");
    }

    for (auto& draft : drafts_) {
        if (draft.id != id) {
            continue;
        }

        draft.status = "published";
        PublishedSkill skill;
        skill.id = "skill_" + std::to_string(next_skill_id_++);
        skill.draft_id = draft.id;
        skill.project_id = draft.project_id;
        skill.name = draft.name;
        skill.description = draft.description;
        skill.skill_md = draft.skill_md;
        skill.version = version;
        skill.changelog = changelog;
        skill.status = "active";
        published_skills_.push_back(skill);
        return skill;
    }
    return std::nullopt;
}

std::vector<PublishedSkill> SkillService::ListPublishedSkills(const std::string& project_id, const std::string& query) const {
    std::vector<PublishedSkill> filtered;
    for (const auto& skill : published_skills_) {
        if (!project_id.empty() && skill.project_id != project_id) {
            continue;
        }
        if (!query.empty() && skill.name.find(query) == std::string::npos && skill.description.find(query) == std::string::npos) {
            continue;
        }
        filtered.push_back(skill);
    }
    return filtered;
}

std::optional<PublishedSkill> SkillService::GetPublishedSkillVersion(const std::string& skill_id, const std::string& version) const {
    for (const auto& skill : published_skills_) {
        if (skill.id == skill_id && skill.version == version) {
            return skill;
        }
    }
    return std::nullopt;
}

std::string SkillDraftToJson(const SkillDraft& draft) {
    return "{\"draft_id\":" + skillops::common::JsonString(draft.id) +
           ",\"project_id\":" + skillops::common::JsonString(draft.project_id) +
           ",\"experience_id\":" + skillops::common::JsonString(draft.experience_id) +
           ",\"name\":" + skillops::common::JsonString(draft.name) +
           ",\"description\":" + skillops::common::JsonString(draft.description) +
           ",\"skill_md\":" + skillops::common::JsonString(draft.skill_md) +
           ",\"status\":" + skillops::common::JsonString(draft.status) + "}";
}

std::string PublishedSkillToJson(const PublishedSkill& skill) {
    return "{\"skill_id\":" + skillops::common::JsonString(skill.id) +
           ",\"draft_id\":" + skillops::common::JsonString(skill.draft_id) +
           ",\"project_id\":" + skillops::common::JsonString(skill.project_id) +
           ",\"name\":" + skillops::common::JsonString(skill.name) +
           ",\"description\":" + skillops::common::JsonString(skill.description) +
           ",\"skill_md\":" + skillops::common::JsonString(skill.skill_md) +
           ",\"version\":" + skillops::common::JsonString(skill.version) +
           ",\"changelog\":" + skillops::common::JsonString(skill.changelog) +
           ",\"status\":" + skillops::common::JsonString(skill.status) + "}";
}

std::string PublishedSkillListToJson(const std::vector<PublishedSkill>& skills) {
    std::string items = "[";
    for (std::size_t i = 0; i < skills.size(); ++i) {
        if (i > 0) {
            items += ",";
        }
        items += PublishedSkillToJson(skills[i]);
    }
    items += "]";
    return "{\"items\":" + items + ",\"page\":1,\"page_size\":20,\"total\":" + std::to_string(skills.size()) + "}";
}

std::string SkillPackageToJson(const PublishedSkill& skill) {
    const auto package_artifact_id = "pkg_" + skill.id + "_" + skill.version;
    return "{\"skill_id\":" + skillops::common::JsonString(skill.id) +
           ",\"version\":" + skillops::common::JsonString(skill.version) +
           ",\"package_artifact_id\":" + skillops::common::JsonString(package_artifact_id) +
           ",\"download_url\":" + skillops::common::JsonString("/api/v1/skills/" + skill.id + "/versions/" + skill.version + "/package") +
           ",\"status\":\"package_placeholder\"}";
}

}  // namespace skillops::skill
