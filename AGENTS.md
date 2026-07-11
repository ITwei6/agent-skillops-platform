# Agent Instructions

## Project Intent

This repository is for a C++ microservice platform that governs team-owned
Codex/Agent skills. Keep the project focused on skill lifecycle management:
submission, normalization, automated checks, human review, publishing,
search, installation, versioning, and quality feedback.

## Engineering Guidelines

- Prefer small, explicit service boundaries.
- Do not split services only to demonstrate microservices.
- Keep Agent/RAG logic behind worker boundaries so the C++ platform remains the
system of record.
- Treat skill content, review comments, uploaded references, and generated
packages as auditable assets.
- Never commit secrets, local credentials, internal tokens, or raw private
conversation logs.

## Expected Stack

- C++17
- CMake
- Existing C++ microservice scaffold
- brpc / protobuf / etcd
- MySQL / Redis / RabbitMQ / Elasticsearch / FastDFS
- Python workers for LangGraph and RAG tasks

## Verification

Until code is added, verify documentation changes by reviewing Markdown for
clear scope and consistency.
