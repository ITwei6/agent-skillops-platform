# Roadmap

## Phase 0: Bootstrap

- Define project vision.
- Define service boundaries.
- Define skill lifecycle states.
- Prepare local repository.

## Phase 1: Minimal Platform

- Create C++ gateway skeleton.
- Create skill metadata model.
- Create experience submission API.
- Create skill draft API.
- Store metadata in MySQL.

## Phase 2: Review Workflow

- Add reviewer roles.
- Add review states: `draft`, `submitted`, `needs_changes`, `approved`,
  `rejected`, `published`, `deprecated`.
- Add review comments and audit history.
- Add basic search over skill titles and descriptions.

## Phase 3: Agent-assisted Skill Generation

- Add RabbitMQ job submission.
- Add Python LangGraph worker.
- Generate normalized `SKILL.md` from experience records.
- Add safety checks for secrets and overly broad triggers.
- Add duplicate-skill detection using RAG.

## Phase 4: Team Registry

- Publish approved skill versions.
- Generate downloadable skill archives.
- Add install metadata.
- Track usage feedback and ratings.

## Phase 5: Maintenance Intelligence

- Detect outdated skills when linked references change.
- Suggest merges between similar skills.
- Generate reviewer summaries.
- Add skill quality dashboards.
