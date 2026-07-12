#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PIDS=()

cleanup() {
    for pid in "${PIDS[@]}"; do
        kill "${pid}" 2>/dev/null || true
    done
}

start_service() {
    local name="$1"
    local binary="${ROOT_DIR}/build/services/${name}/${name}"
    local config="${ROOT_DIR}/deploy/config/${name}.yaml"

    "${binary}" --config="${config}" >"/tmp/skillops-${name}.log" 2>&1 &
    PIDS+=("$!")
    echo "${name} pid=${PIDS[-1]}"
}

main() {
    trap cleanup EXIT
    cd "${ROOT_DIR}"

    start_service identity-service
    start_service project-service
    start_service experience-service
    start_service skill-service
    start_service review-service
    start_service artifact-service
    start_service gateway-service

    echo "gateway: http://127.0.0.1:8080"
    echo "press Ctrl+C to stop all services"
    wait
}

main "$@"
