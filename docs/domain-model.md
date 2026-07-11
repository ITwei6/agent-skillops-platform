# Domain Model Draft

## Core Entities

### User

Represents a developer, reviewer, or admin.

Important fields:

- `id`
- `name`
- `email`
- `role`
- `team_id`

### Team

Represents a group that owns skills and review policies.

### ExperienceRecord

Represents an individual developer's agent-assisted work experience.

Examples:

- A Codex troubleshooting session summary.
- A Cursor-generated refactor report.
- A LangGraph workflow output.
- A manually written engineering lesson.

Important fields:

- `id`
- `project_id`
- `submitter_id`
- `title`
- `summary`
- `source_type`
- `source_refs`
- `created_at`

### SkillDraft

Represents a proposed reusable skill before publication.

Important fields:

- `id`
- `experience_id`
- `name`
- `description`
- `skill_md`
- `status`
- `version`
- `risk_level`

### Review

Represents human review of a skill draft.

Important fields:

- `id`
- `skill_draft_id`
- `reviewer_id`
- `decision`
- `comments`
- `created_at`

### PublishedSkill

Represents a team-approved skill version.

Important fields:

- `id`
- `skill_draft_id`
- `name`
- `version`
- `package_artifact_id`
- `published_by`
- `published_at`
- `deprecated_at`

### Artifact

Represents an uploaded reference, generated package, report, or archive.

Important fields:

- `id`
- `owner_type`
- `owner_id`
- `file_name`
- `content_type`
- `storage_key`
- `sha256`
- `created_at`

## Skill Lifecycle

```text
draft
  -> submitted
  -> needs_changes
  -> submitted
  -> approved
  -> published
  -> deprecated
```

Alternative terminal path:

```text
submitted -> rejected
```
