# Agent Instructions

## 项目定位

本仓库用于设计和实现一个基于 C++ 微服务的团队 SkillOps 平台。

平台核心目标：

- 收集个人 Codex/Agent 工作经验
- 生成或规范化 Skill 草稿
- 自动检查敏感信息、重复内容、触发范围和格式问题
- 支持技术审核
- 发布团队级 Skill
- 管理版本、安装包、反馈和过期状态

## 当前协作模式

当前阶段以架构设计和项目管理为主。

本窗口承担：

- 产品与业务设计
- 项目总体架构
- 接口、数据库、MQ 设计
- 工程规范
- 子服务边界设计
- 实现顺序和验收标准

本窗口不负责直接编写业务代码。后续编码由另一个 Codex 实现窗口根据文档完成。

## 工程约束

- 必须先完成设计文档，再进入编码。
- 不为了微服务而微服务。
- C++ 服务维护平台核心数据。
- Python/LangGraph Worker 只负责 RAG 和 Agent 分析，不直接绕过审核发布 Skill。
- Skill 内容、审核意见、上传文件、生成包都属于可审计资产。
- 不提交密钥、账号、token、私有会话原文或未脱敏敏感信息。

## 预期技术栈

- C++17
- CMake
- C++ 微服务脚手架
- brpc / protobuf / etcd
- MySQL / Redis / RabbitMQ / Elasticsearch / FastDFS
- Python / LangGraph / RAG Worker

## 文档优先级

后续所有实现应优先阅读：

```text
docs/服务端文档/
docs/子服务文档/
docs/实施计划.md
```

如果实现过程中发现设计不合理，应先反馈设计窗口，不直接擅自改变核心业务方向。
