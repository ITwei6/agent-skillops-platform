#include "skillops/review/review_service.h"

#include "skillops/common/json.h"

#include <stdexcept>

namespace skillops::review {

namespace {

bool IsSupportedDecision(const std::string& decision) {
    return decision == "approve" || decision == "reject" || decision == "needs_changes" || decision == "comment";
}

std::string StatusFromDecision(const std::string& decision) {
    if (decision == "approve") {
        return "approved";
    }
    if (decision == "reject") {
        return "rejected";
    }
    if (decision == "needs_changes") {
        return "needs_changes";
    }
    return "commented";
}

}  // namespace

ReviewRecord ReviewService::CreateReview(const CreateReviewRequest& request) {
    if (request.draft_id.empty() || request.project_id.empty() || request.decision.empty()) {
        throw std::invalid_argument("draft_id, project_id and decision are required");
    }
    if (!IsSupportedDecision(request.decision)) {
        throw std::invalid_argument("unsupported review decision");
    }

    ReviewRecord review;
    review.id = "review_" + std::to_string(next_review_id_++);
    review.draft_id = request.draft_id;
    review.project_id = request.project_id;
    review.reviewer_id = request.reviewer_id.empty() ? "reviewer_system" : request.reviewer_id;
    review.decision = request.decision;
    review.comments = request.comments;
    review.status = StatusFromDecision(request.decision);
    reviews_.push_back(review);
    return review;
}

std::vector<ReviewRecord> ReviewService::ListQueue(const std::string& project_id, const std::string& status) const {
    std::vector<ReviewRecord> filtered;
    for (const auto& review : reviews_) {
        if (!project_id.empty() && review.project_id != project_id) {
            continue;
        }
        if (!status.empty() && review.status != status) {
            continue;
        }
        filtered.push_back(review);
    }
    return filtered;
}

std::string ReviewRecordToJson(const ReviewRecord& review) {
    return "{\"review_id\":" + skillops::common::JsonString(review.id) +
           ",\"draft_id\":" + skillops::common::JsonString(review.draft_id) +
           ",\"project_id\":" + skillops::common::JsonString(review.project_id) +
           ",\"reviewer_id\":" + skillops::common::JsonString(review.reviewer_id) +
           ",\"decision\":" + skillops::common::JsonString(review.decision) +
           ",\"comments\":" + skillops::common::JsonString(review.comments) +
           ",\"status\":" + skillops::common::JsonString(review.status) + "}";
}

std::string ReviewQueueToJson(const std::vector<ReviewRecord>& reviews) {
    std::string items = "[";
    for (std::size_t i = 0; i < reviews.size(); ++i) {
        if (i > 0) {
            items += ",";
        }
        items += ReviewRecordToJson(reviews[i]);
    }
    items += "]";
    return "{\"items\":" + items + ",\"page\":1,\"page_size\":20,\"total\":" + std::to_string(reviews.size()) + "}";
}

}  // namespace skillops::review
