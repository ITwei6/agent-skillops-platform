#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace skillops::skill {

struct SkillDraft {
    std::string id;
    std::string project_id;
    std::string experience_id;
    std::string name;
    std::string description;
    std::string skill_md;
    std::string status;
};

struct PublishedSkill {
    std::string id;
    std::string draft_id;
    std::string project_id;
    std::string name;
    std::string description;
    std::string skill_md;
    std::string version;
    std::string changelog;
    std::string status;
};

struct CreateSkillDraftRequest {
    std::string project_id;
    std::string experience_id;
    std::string name;
    std::string description;
    std::string skill_md;
};

struct UpdateSkillDraftRequest {
    std::string name;
    std::string description;
    std::string skill_md;
};

class SkillService {
public:
    SkillDraft CreateDraft(const CreateSkillDraftRequest& request);
    std::optional<SkillDraft> GetDraft(const std::string& id) const;
    std::optional<SkillDraft> UpdateDraft(const std::string& id, const UpdateSkillDraftRequest& request);
    std::optional<SkillDraft> SubmitDraft(const std::string& id);
    std::optional<PublishedSkill> PublishDraft(const std::string& id, const std::string& version, const std::string& changelog);
    std::optional<PublishedSkill> GetPublishedSkillVersion(const std::string& skill_id, const std::string& version) const;
    std::vector<PublishedSkill> ListPublishedSkills(const std::string& project_id, const std::string& query) const;

private:
    std::uint64_t next_draft_id_{1};
    std::uint64_t next_skill_id_{1};
    std::vector<SkillDraft> drafts_;
    std::vector<PublishedSkill> published_skills_;
};

std::string SkillDraftToJson(const SkillDraft& draft);
std::string PublishedSkillToJson(const PublishedSkill& skill);
std::string PublishedSkillListToJson(const std::vector<PublishedSkill>& skills);
std::string SkillPackageToJson(const PublishedSkill& skill);

}  // namespace skillops::skill
