#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GATEWAY_PORT="${GATEWAY_PORT:-19080}"
IDENTITY_PORT="${IDENTITY_PORT:-19081}"
PROJECT_PORT="${PROJECT_PORT:-19082}"
EXPERIENCE_PORT="${EXPERIENCE_PORT:-19083}"
SKILL_PORT="${SKILL_PORT:-19084}"
REVIEW_PORT="${REVIEW_PORT:-19085}"
ARTIFACT_PORT="${ARTIFACT_PORT:-19086}"

PIDS=()

cleanup() {
    for pid in "${PIDS[@]}"; do
        kill "${pid}" 2>/dev/null || true
    done
}

start_service() {
    local name="$1"
    local port="$2"
    local extra_args="${3:-}"
    local binary="${ROOT_DIR}/build/services/${name}/${name}"
    local config="${ROOT_DIR}/deploy/config/${name}.yaml"

    "${binary}" --config="${config}" --host=127.0.0.1 --port="${port}" ${extra_args} >"/tmp/skillops-${name}.log" 2>&1 &
    PIDS+=("$!")
}

request() {
    local method="$1"
    local url="$2"
    local body="${3:-}"

    if [[ -n "${body}" ]]; then
        curl -fsS -X "${method}" "${url}" -H "Content-Type: application/json" -d "${body}"
    else
        curl -fsS -X "${method}" "${url}"
    fi
}

assert_contains() {
    local value="$1"
    local expected="$2"
    if [[ "${value}" != *"${expected}"* ]]; then
        echo "Expected response to contain: ${expected}" >&2
        echo "Actual response: ${value}" >&2
        exit 1
    fi
}

wait_for_gateway() {
    local url="http://127.0.0.1:${GATEWAY_PORT}/healthz"
    for _ in $(seq 1 30); do
        if curl -fsS "${url}" >/tmp/skillops-gateway-health.json 2>/tmp/skillops-gateway-curl.err; then
            return 0
        fi
        sleep 1
    done
    echo "gateway-service did not become ready" >&2
    cat /tmp/skillops-gateway.log >&2 || true
    exit 1
}

main() {
    trap cleanup EXIT
    cd "${ROOT_DIR}"

    start_service identity-service "${IDENTITY_PORT}"
    start_service project-service "${PROJECT_PORT}"
    start_service experience-service "${EXPERIENCE_PORT}"
    start_service skill-service "${SKILL_PORT}"
    start_service review-service "${REVIEW_PORT}"
    start_service artifact-service "${ARTIFACT_PORT}"
    start_service gateway-service "${GATEWAY_PORT}" \
        "--identity-host=127.0.0.1 --identity-port=${IDENTITY_PORT} --project-host=127.0.0.1 --project-port=${PROJECT_PORT} --experience-host=127.0.0.1 --experience-port=${EXPERIENCE_PORT} --skill-host=127.0.0.1 --skill-port=${SKILL_PORT} --review-host=127.0.0.1 --review-port=${REVIEW_PORT} --artifact-host=127.0.0.1 --artifact-port=${ARTIFACT_PORT}"

    wait_for_gateway

    local base="http://127.0.0.1:${GATEWAY_PORT}/api/v1"
    local response

    response="$(request POST "${base}/users" '{"team_id":"team_1","name":"Admin","email":"admin@example.com","role":"owner"}')"
    assert_contains "${response}" '"user_id":"user_1"'
    assert_contains "${response}" '"request_id":"req_'

    response="$(request POST "${base}/projects" '{"team_id":"team_1","name":"SkillOps","description":"Team skill governance","repo_url":"https://example.com/repo.git","created_by":"user_1"}')"
    assert_contains "${response}" '"project_id":"proj_1"'

    response="$(request POST "${base}/artifacts" '{"project_id":"proj_1","owner_type":"experience","owner_id":"exp_1","file_name":"session-summary.md","content_type":"text/markdown","sha256":"abc123","size_bytes":128}')"
    assert_contains "${response}" '"artifact_id":"art_1"'

    response="$(request POST "${base}/experiences" '{"project_id":"proj_1","title":"Docker SSH troubleshooting","source_type":"codex_session","summary":"Troubleshoot Docker development container access.","artifact_ids":["art_1"]}')"
    assert_contains "${response}" '"experience_id":"exp_1"'
    assert_contains "${response}" '"analysis_job_id":"job_1"'

    response="$(request GET "${base}/analysis-jobs/job_1")"
    assert_contains "${response}" '"status":"queued"'

    response="$(request POST "${base}/skill-drafts" '{"project_id":"proj_1","experience_id":"exp_1","name":"docker-ssh-troubleshooting","description":"Troubleshoot Docker SSH access.","skill_md":"name: docker-ssh-troubleshooting\nworkflow: check ssh and docker ports"}')"
    assert_contains "${response}" '"draft_id":"draft_1"'

    response="$(request POST "${base}/skill-drafts/draft_1/submit")"
    assert_contains "${response}" '"status":"submitted"'

    response="$(request POST "${base}/skill-drafts/draft_1/reviews" '{"project_id":"proj_1","reviewer_id":"user_1","decision":"approve","comments":"Looks good."}')"
    assert_contains "${response}" '"status":"approved"'

    response="$(request POST "${base}/skill-drafts/draft_1/publish" '{"version":"1.0.0","changelog":"Initial version"}')"
    assert_contains "${response}" '"skill_id":"skill_1"'

    response="$(request GET "${base}/skills/skill_1/versions/1.0.0")"
    assert_contains "${response}" '"version":"1.0.0"'

    response="$(request GET "${base}/skills/skill_1/versions/1.0.0/package")"
    assert_contains "${response}" '"package_artifact_id":"pkg_skill_1_1.0.0"'

    response="$(request GET "${base}/skills?query=docker&project_id=proj_1")"
    assert_contains "${response}" '"total":1'

    echo "skillops e2e smoke passed"
}

main "$@"
