#include "skillops/artifact/artifact_service.h"

#include "skillops/common/json.h"

#include <stdexcept>

namespace skillops::artifact {

Artifact ArtifactService::CreateArtifact(const CreateArtifactRequest& request) {
    if (request.project_id.empty() || request.owner_type.empty() || request.owner_id.empty() || request.file_name.empty()) {
        throw std::invalid_argument("project_id, owner_type, owner_id and file_name are required");
    }

    Artifact artifact;
    artifact.id = "art_" + std::to_string(next_artifact_id_++);
    artifact.project_id = request.project_id;
    artifact.owner_type = request.owner_type;
    artifact.owner_id = request.owner_id;
    artifact.file_name = request.file_name;
    artifact.content_type = request.content_type.empty() ? "application/octet-stream" : request.content_type;
    artifact.storage_key = "memory://" + artifact.id + "/" + artifact.file_name;
    artifact.sha256 = request.sha256.empty() ? "pending" : request.sha256;
    artifact.size_bytes = request.size_bytes;
    artifact.status = "available";
    artifacts_.push_back(artifact);
    return artifact;
}

std::optional<Artifact> ArtifactService::GetArtifact(const std::string& id) const {
    for (const auto& artifact : artifacts_) {
        if (artifact.id == id) {
            return artifact;
        }
    }
    return std::nullopt;
}

std::vector<Artifact> ArtifactService::ListArtifacts(
    const std::string& project_id,
    const std::string& owner_type,
    const std::string& owner_id) const {
    std::vector<Artifact> filtered;
    for (const auto& artifact : artifacts_) {
        if (!project_id.empty() && artifact.project_id != project_id) {
            continue;
        }
        if (!owner_type.empty() && artifact.owner_type != owner_type) {
            continue;
        }
        if (!owner_id.empty() && artifact.owner_id != owner_id) {
            continue;
        }
        filtered.push_back(artifact);
    }
    return filtered;
}

std::string ArtifactToJson(const Artifact& artifact) {
    return "{\"artifact_id\":" + skillops::common::JsonString(artifact.id) +
           ",\"project_id\":" + skillops::common::JsonString(artifact.project_id) +
           ",\"owner_type\":" + skillops::common::JsonString(artifact.owner_type) +
           ",\"owner_id\":" + skillops::common::JsonString(artifact.owner_id) +
           ",\"file_name\":" + skillops::common::JsonString(artifact.file_name) +
           ",\"content_type\":" + skillops::common::JsonString(artifact.content_type) +
           ",\"storage_key\":" + skillops::common::JsonString(artifact.storage_key) +
           ",\"sha256\":" + skillops::common::JsonString(artifact.sha256) +
           ",\"size_bytes\":" + std::to_string(artifact.size_bytes) +
           ",\"status\":" + skillops::common::JsonString(artifact.status) + "}";
}

std::string ArtifactListToJson(const std::vector<Artifact>& artifacts) {
    std::string items = "[";
    for (std::size_t i = 0; i < artifacts.size(); ++i) {
        if (i > 0) {
            items += ",";
        }
        items += ArtifactToJson(artifacts[i]);
    }
    items += "]";
    return "{\"items\":" + items + ",\"page\":1,\"page_size\":20,\"total\":" + std::to_string(artifacts.size()) + "}";
}

std::string ArtifactDownloadToJson(const Artifact& artifact) {
    return "{\"artifact\":" + ArtifactToJson(artifact) +
           ",\"download_url\":" + skillops::common::JsonString("/api/v1/artifacts/" + artifact.id + "/download") +
           ",\"storage_key\":" + skillops::common::JsonString(artifact.storage_key) + "}";
}

}  // namespace skillops::artifact
