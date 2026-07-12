#include "skillops/common/http_server.h"
#include "skillops/common/json.h"
#include "skillops/common/logging.h"
#include "skillops/skill/skill_service.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::string ReadArgValue(const std::string& arg, const std::string& prefix) {
    return arg.rfind(prefix, 0) == 0 ? arg.substr(prefix.size()) : "";
}

skillops::common::HttpResponse Envelope(
    int status,
    const std::string& reason,
    const std::string& code,
    const std::string& message,
    const std::string& data) {
    return skillops::common::HttpResponse::Json(status, reason, skillops::common::JsonEnvelope(code, message, "req_1", data));
}

std::string QueryValue(const std::string& query, const std::string& key) {
    const auto prefix = key + "=";
    std::size_t start = 0;
    while (start <= query.size()) {
        const auto end = query.find('&', start);
        const auto item = query.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (item.rfind(prefix, 0) == 0) {
            return item.substr(prefix.size());
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return "";
}

std::string PathSuffix(const std::string& path, const std::string& prefix) {
    if (path.rfind(prefix, 0) != 0) {
        return "";
    }
    return path.substr(prefix.size());
}

struct SkillVersionPath {
    std::string skill_id;
    std::string version;
    bool package = false;
};

SkillVersionPath ParseSkillVersionPath(const std::string& path) {
    constexpr const char* prefix = "/internal/v1/skills/";
    constexpr const char* versions_segment = "/versions/";
    constexpr const char* package_suffix = "/package";

    SkillVersionPath result;
    if (path.rfind(prefix, 0) != 0) {
        return result;
    }

    auto value = path.substr(std::string(prefix).size());
    const auto versions_pos = value.find(versions_segment);
    if (versions_pos == std::string::npos) {
        return result;
    }

    result.skill_id = value.substr(0, versions_pos);
    result.version = value.substr(versions_pos + std::string(versions_segment).size());
    if (result.version.size() > std::string(package_suffix).size() &&
        result.version.substr(result.version.size() - std::string(package_suffix).size()) == package_suffix) {
        result.version = result.version.substr(0, result.version.size() - std::string(package_suffix).size());
        result.package = true;
    }
    return result;
}

}  // namespace

int main(int argc, char** argv) {
    std::string host = "0.0.0.0";
    std::uint16_t port = 18084;

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        const auto host_value = ReadArgValue(arg, "--host=");
        const auto port_value = ReadArgValue(arg, "--port=");
        if (!host_value.empty()) {
            host = host_value;
        } else if (!port_value.empty()) {
            port = static_cast<std::uint16_t>(std::stoi(port_value));
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 2;
        }
    }

    try {
        skillops::common::InitLogging();
        skillops::skill::SkillService service;
        skillops::common::HttpServer server(
            host,
            port,
            [&service](const skillops::common::HttpRequest& request) {
                if (request.method == "GET" && request.path == "/healthz") {
                    return Envelope(200, "OK", "OK", "success", "{\"service\":\"skill-service\"}");
                }

                if (request.method == "POST" && request.path == "/internal/v1/skill-drafts") {
                    try {
                        const auto draft = service.CreateDraft({
                            skillops::common::ExtractJsonString(request.body, "project_id"),
                            skillops::common::ExtractJsonString(request.body, "experience_id"),
                            skillops::common::ExtractJsonString(request.body, "name"),
                            skillops::common::ExtractJsonString(request.body, "description"),
                            skillops::common::ExtractJsonString(request.body, "skill_md"),
                        });
                        return Envelope(200, "OK", "OK", "success", skillops::skill::SkillDraftToJson(draft));
                    } catch (const std::invalid_argument&) {
                        return Envelope(400, "Bad Request", "VALIDATION_INVALID_ARGUMENT", "invalid skill draft payload", "{}");
                    }
                }

                const auto draft_suffix = PathSuffix(request.path, "/internal/v1/skill-drafts/");
                if (!draft_suffix.empty()) {
                    if (request.method == "POST" && draft_suffix.size() > 7 && draft_suffix.substr(draft_suffix.size() - 7) == "/submit") {
                        const auto draft_id = draft_suffix.substr(0, draft_suffix.size() - 7);
                        const auto draft = service.SubmitDraft(draft_id);
                        if (!draft.has_value()) {
                            return Envelope(404, "Not Found", "SKILL_DRAFT_NOT_FOUND", "skill draft not found", "{}");
                        }
                        return Envelope(200, "OK", "OK", "success", skillops::skill::SkillDraftToJson(*draft));
                    }

                    if (request.method == "POST" && draft_suffix.size() > 8 && draft_suffix.substr(draft_suffix.size() - 8) == "/publish") {
                        const auto draft_id = draft_suffix.substr(0, draft_suffix.size() - 8);
                        try {
                            const auto skill = service.PublishDraft(
                                draft_id,
                                skillops::common::ExtractJsonString(request.body, "version"),
                                skillops::common::ExtractJsonString(request.body, "changelog"));
                            if (!skill.has_value()) {
                                return Envelope(404, "Not Found", "SKILL_DRAFT_NOT_FOUND", "skill draft not found", "{}");
                            }
                            return Envelope(200, "OK", "OK", "success", skillops::skill::PublishedSkillToJson(*skill));
                        } catch (const std::invalid_argument&) {
                            return Envelope(400, "Bad Request", "VALIDATION_INVALID_ARGUMENT", "invalid publish payload", "{}");
                        }
                    }

                    if (request.method == "GET") {
                        const auto draft = service.GetDraft(draft_suffix);
                        if (!draft.has_value()) {
                            return Envelope(404, "Not Found", "SKILL_DRAFT_NOT_FOUND", "skill draft not found", "{}");
                        }
                        return Envelope(200, "OK", "OK", "success", skillops::skill::SkillDraftToJson(*draft));
                    }

                    if (request.method == "PATCH") {
                        const auto draft = service.UpdateDraft(draft_suffix, {
                            skillops::common::ExtractJsonString(request.body, "name"),
                            skillops::common::ExtractJsonString(request.body, "description"),
                            skillops::common::ExtractJsonString(request.body, "skill_md"),
                        });
                        if (!draft.has_value()) {
                            return Envelope(404, "Not Found", "SKILL_DRAFT_NOT_FOUND", "skill draft not found", "{}");
                        }
                        return Envelope(200, "OK", "OK", "success", skillops::skill::SkillDraftToJson(*draft));
                    }
                }

                if (request.method == "GET" && request.path == "/internal/v1/skills") {
                    return Envelope(
                        200,
                        "OK",
                        "OK",
                        "success",
                        skillops::skill::PublishedSkillListToJson(service.ListPublishedSkills(
                            QueryValue(request.query, "project_id"),
                            QueryValue(request.query, "query"))));
                }

                if (request.method == "GET") {
                    const auto version_path = ParseSkillVersionPath(request.path);
                    if (!version_path.skill_id.empty() && !version_path.version.empty()) {
                        const auto skill = service.GetPublishedSkillVersion(version_path.skill_id, version_path.version);
                        if (!skill.has_value()) {
                            return Envelope(404, "Not Found", "SKILL_VERSION_NOT_FOUND", "skill version not found", "{}");
                        }
                        if (version_path.package) {
                            return Envelope(200, "OK", "OK", "success", skillops::skill::SkillPackageToJson(*skill));
                        }
                        return Envelope(200, "OK", "OK", "success", skillops::skill::PublishedSkillToJson(*skill));
                    }
                }

                return Envelope(404, "Not Found", "SYSTEM_NOT_FOUND", "not found", "{}");
            });

        skillops::common::LogInfo("skill-service listening on " + host + ":" + std::to_string(port));
        return server.Run();
    } catch (const std::exception& ex) {
        std::cerr << "skill-service failed: " << ex.what() << std::endl;
        return 1;
    }
}
