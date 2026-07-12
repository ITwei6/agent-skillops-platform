#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace skillops::artifact {

struct Artifact {
    std::string id;
    std::string project_id;
    std::string owner_type;
    std::string owner_id;
    std::string file_name;
    std::string content_type;
    std::string storage_key;
    std::string sha256;
    std::uint64_t size_bytes = 0;
    std::string status;
};

struct CreateArtifactRequest {
    std::string project_id;
    std::string owner_type;
    std::string owner_id;
    std::string file_name;
    std::string content_type;
    std::string sha256;
    std::uint64_t size_bytes = 0;
};

class ArtifactService {
public:
    Artifact CreateArtifact(const CreateArtifactRequest& request);
    std::optional<Artifact> GetArtifact(const std::string& id) const;
    std::vector<Artifact> ListArtifacts(
        const std::string& project_id,
        const std::string& owner_type,
        const std::string& owner_id) const;

private:
    std::uint64_t next_artifact_id_{1};
    std::vector<Artifact> artifacts_;
};

std::string ArtifactToJson(const Artifact& artifact);
std::string ArtifactListToJson(const std::vector<Artifact>& artifacts);
std::string ArtifactDownloadToJson(const Artifact& artifact);

}  // namespace skillops::artifact
