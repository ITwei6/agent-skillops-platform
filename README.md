# Agent SkillOps Platform

Agent SkillOps Platform is a team-oriented platform for turning individual
Codex/Agent work experience into reviewed, versioned, reusable team skills.

The goal is not to build a stronger coding agent. The goal is to manage the
assets produced around agents: experience summaries, skill drafts, review
records, reusable workflows, project references, and installation packages.

## Problem

Individual developers can ask Codex or another agent to summarize a task and
generate a skill. That works for personal reuse, but it does not solve team
problems:

- Is the skill correct and safe?
- Does it leak credentials, internal URLs, or private context?
- Does it duplicate an existing team skill?
- Which project, code version, or document version does it apply to?
- Who reviewed and approved it?
- How does the team install, update, or roll back it?
- Is the skill outdated after the project changes?

This platform treats agent-produced experience as a governed engineering asset.

## Initial Scope

The first version focuses on the skill lifecycle:

1. Submit experience or a skill draft.
2. Normalize it into a structured skill proposal.
3. Run automated checks for quality, safety, duplication, and scope.
4. Let technical reviewers approve, reject, or request changes.
5. Publish approved skills into a team skill registry.
6. Search, install, version, and evaluate skills.

## Planned Tech Direction

- C++ microservices built with the existing C++ scaffold.
- MySQL for core metadata.
- Redis for cache, review state, and rate limiting.
- RabbitMQ for asynchronous skill analysis and packaging jobs.
- Elasticsearch for searching experience records, skills, and review history.
- FastDFS for uploaded artifacts and generated skill packages.
- Etcd and brpc for internal service discovery and RPC.
- Python/LangGraph workers for RAG, skill generation, and automated review.

## Service Draft

- `gateway-service`: external HTTP API.
- `identity-service`: users, teams, roles, and reviewer permissions.
- `experience-service`: submitted task summaries and agent outputs.
- `skill-service`: skill drafts, versions, publication state, and registry.
- `review-service`: human review workflow and audit history.
- `artifact-service`: uploaded references and generated skill packages.
- `analysis-worker`: RAG and Agent-based checks.

## Current Status

This repository is in planning/bootstrap stage.
