#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace skillops::review {

struct ReviewRecord {
    std::string id;
    std::string draft_id;
    std::string project_id;
    std::string reviewer_id;
    std::string decision;
    std::string comments;
    std::string status;
};

struct CreateReviewRequest {
    std::string draft_id;
    std::string project_id;
    std::string reviewer_id;
    std::string decision;
    std::string comments;
};

class ReviewService {
public:
    ReviewRecord CreateReview(const CreateReviewRequest& request);
    std::vector<ReviewRecord> ListQueue(const std::string& project_id, const std::string& status) const;

private:
    std::uint64_t next_review_id_{1};
    std::vector<ReviewRecord> reviews_;
};

std::string ReviewRecordToJson(const ReviewRecord& review);
std::string ReviewQueueToJson(const std::vector<ReviewRecord>& reviews);

}  // namespace skillops::review
