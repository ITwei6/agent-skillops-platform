# Architecture Draft

## High-level Flow

```text
Developer / Reviewer / Admin
        |
        v
gateway-service
        |
        +--> identity-service
        +--> experience-service
        +--> skill-service
        +--> review-service
        +--> artifact-service
        |
        v
     RabbitMQ
        |
        v
analysis-worker / package-worker
```

## Service Boundaries

### gateway-service

External HTTP entry point. Handles request routing, authentication checks, and
response shaping.

### identity-service

Owns users, teams, roles, reviewer permissions, and API keys.

### experience-service

Owns submitted agent work records, task summaries, source links, and raw draft
metadata. It should not be the final skill registry.

### skill-service

Owns skill drafts, normalized skill structure, versions, publication status,
registry metadata, and install package references.

### review-service

Owns review state transitions, comments, approval records, rejection reasons,
and audit history.

### artifact-service

Owns uploaded documents, generated `SKILL.md` packages, archives, reports, and
checksums. Stores binary data in FastDFS and metadata in MySQL.

### analysis-worker

Python worker using RAG and LangGraph. It performs asynchronous tasks:

- Generate a skill draft from experience.
- Check for sensitive data.
- Compare with similar existing skills.
- Evaluate trigger descriptions and scope.
- Suggest review comments.
- Mark skills as possibly outdated when references change.

## Data Stores

- MySQL: system of record for users, skills, reviews, versions, and metadata.
- Redis: cache, rate limit state, short-lived workflow status.
- RabbitMQ: asynchronous generation, analysis, packaging, and notification jobs.
- Elasticsearch: search over skills, experience records, review notes, and
  generated references.
- FastDFS: uploaded files and generated skill archives.
- Etcd: service discovery for C++ microservices.

## Microservice Rationale

The first implementation should avoid excessive splitting. The service split is
justified where lifecycle, scaling, or failure behavior differs:

- Gateway is the external boundary.
- Skill and review state must be auditable and stable.
- Artifact storage has different data volume and access patterns.
- Analysis workers can be slow, model-dependent, and failure-prone, so they are
  asynchronous.

If a boundary does not create operational or domain clarity, keep it in the same
service until the need is real.
