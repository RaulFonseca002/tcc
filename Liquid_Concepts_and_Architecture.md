# Liquid Concepts and Architecture v0.2

**Status:** Living baseline after M1  
**Scope:** Conceptual architecture, vocabulary, and accepted design direction  
**Project:** Liquid Layer  
**Engine / framework:** Liquid  
**Current development stage:** Solid, M2 intent lifetime and expiration  
**Date:** June 2026

---

## 1. Purpose

This document defines the conceptual baseline for the Liquid project as implementation progresses. It captures the current architectural direction, vocabulary, stage names, and design boundaries agreed so far.

This is not the coding guide. The coding guide will define C++ conventions, header structure, file responsibilities, call order, tests, and implementation-level contracts.

---

## 2. Naming and Stages

The project is divided into three conceptual layers.

| Name | Meaning | Current status |
|---|---|---|
| **Liquid Layer** | Final application/research project focused on adaptive smart environments for neurodivergent people. | Future project layer |
| **Liquid** | Standalone engine/framework/runtime used by Liquid Layer and by simulation/data-collection tools. | Engine repo |
| **Solid** | First implementation stage of Liquid: deterministic ECS-inspired runtime. | M1 complete; M2 current |
| **Liquid stage** | Later implementation stage: adaptive/LLM layer that creates, modifies, and explains Solid blocks. | Future focus |

The internal code should not overuse metaphorical names. Folder and file names should describe responsibility: `vocabulary`, `core`, `runtime`, `events`, `behaviors`, `intents`, `systems`, and `adapters`.

---

## 3. Architectural Thesis

Liquid separates adaptive reasoning from deterministic execution.

> Liquid uses probabilistic models to discover, negotiate, and synthesize behavior, but uses a deterministic ECS-inspired runtime to execute, constrain, audit, and reproduce behavior.

The key metaphor is:

- **Solid:** behavior blocks that are explicit, deterministic, inspectable, testable, and replayable.
- **Liquid:** adaptive reasoning that molds those blocks around the user's context, preferences, and routines.
- **Liquid Layer:** the user-facing system where the liquid layer shapes solid behavior to reduce cognitive friction.

The LLM should not be the authority that directly controls the environment every frame. It should help infer context, propose behavior, generate scripts, explain decisions, and adapt preferences. Once behavior becomes active, it should be represented as Solid runtime data and handled by deterministic systems.

---

## 4. Why This Architecture Exists

Liquid is motivated by two problems that pull in opposite directions.

First, smart environments need reliability. Physical actions close to the user must be predictable, auditable, and constrained. Background behaviors should not unexpectedly override explicit user intent, and actions should be traceable to their source.

Second, neurodivergent support often depends on ambiguous personal context. Difficulty starting tasks, hyperfocus, transition friction, sensory sensitivity, sleep irregularity, and routine instability are not solved well by fixed generic rules. The system needs adaptive interpretation and personalization.

The architecture therefore uses a rigid deterministic runtime underneath and a flexible adaptive layer above it.

---

## 5. Stage 1: Solid

Solid is the first implementation stage. Its goal is to build the standalone deterministic engine that later systems will use.

Solid includes:

- a minimal custom ECS-inspired core;
- behaviors as the main domain object instead of generic entities;
- components as plain data;
- systems as logic over component sets;
- intents as first-class runtime objects;
- intent lifetime modeled through intent data and factories;
- deterministic frame execution;
- event input and resolved effect output;
- logs and replay support from the beginning;
- simulation-friendly adapters.

Solid excludes for now:

- LLM behavior generation;
- voice input;
- custom wake words;
- neurodivergence-specific application routines;
- production smart-home hardware integration;
- fine-tuning.

Solid should be usable as a standalone framework for simulation and testing before Liquid Layer exists.

---

## 6. Stage 2: Liquid

Liquid is the adaptive layer added on top of Solid.

Liquid may include:

- agents that interpret user requests;
- generation of behavior proposals;
- translation of natural language into Solid runtime objects;
- adaptation of preferences and thresholds;
- explanation of why behaviors were created or resolved;
- optional async script/LLM checks;
- future data-driven personalization.

Liquid must respect Solid's constraints:

- agents do not mutate world state directly;
- agents propose intents or behavior proposals;
- generated behavior must be validated before activation;
- long-running model calls are asynchronous;
- deterministic runtime remains the final authority for execution and conflict resolution.

---

## 7. Stage 3: Liquid Layer

Liquid Layer is the final application/research system focused on neurodivergent support.

Its goal is to reduce cognitive friction by preparing environments, lowering initiation cost, supporting transitions, and adapting to individual routines. Examples include:

- preparing a study environment;
- soft interruption during hyperfocus;
- sensory-aware lighting and sound changes;
- sleep and routine support;
- task-start support;
- gentle reminders that account for context.

Liquid Layer should be built on top of Liquid, not mixed into the engine. The engine should remain generic enough to run simulations, games, data-collection tools, and real smart-environment adapters.

---

## 8. Core Concepts

### Behavior

A behavior is a runtime object that represents something the system may do over time. Behaviors replace the generic ECS word "entity" in the project vocabulary.

A behavior may be persistent, temporary, scheduled, manually triggered, or generated later by an agent. Behaviors do not directly mutate external devices. They hold component state and are the owner/source for intents.

### Agent

An agent is a behavior-like actor driven by adaptive reasoning. Agents live conceptually beside behaviors, but they follow stricter boundaries. They may propose intents, propose new behaviors, or ask for confirmation. They do not directly mutate components or bypass the resolver.

### Component

A component is plain engine-side data. Components may contain physical adapter references such as a device string, but they should not contain behavior logic, virtual methods, or hidden ownership rules. Logic belongs in systems.

The component type is the signature that systems query for. A user/device-facing component name identifies a concrete shared component instance inside that component type.

Behaviors do not own components exclusively. Multiple behaviors may have read/write access to the same named component through access records.

### System

A system is logic that processes matching component sets during a frame. Systems are responsible for updating lifetimes, resolving intents, advancing behavior runs, applying events, cleanup, and producing effects.

For example, a future script system would process behaviors with script-related components and create intents from those behavior owners.

### Intent

An intent is a proposed component-state change or external effect. It is not immediately applied. Intents are produced by behaviors, agents, systems, events, or schedules; the resolver decides which intents become component changes or effects.

Intents are not components. They are immutable records with owner/source, target, value or operation, priority, lifetime metadata, and merge/conflict policy data.

Examples:

- set a light brightness;
- send a notification;
- reserve an audio output;
- request a simulated user prompt;
- create a future behavior proposal.

### Lifetime

Intent lifetime is modeled as intent data, not as component storage and not as a single enum with all logic inside the resolver.

A lifetime-managed intent can carry common lifetime status plus a specific lifetime policy, such as until-time, until-frame, until-cancelled, or future script-based lifetime data.

Normal code should use factories/bundles to create valid intent records and avoid missing required fields.

### Resolved Effect

A resolved effect is the result of intent resolution. It is the core's accepted output for a target after conflicts have been resolved.

### Command / Adapter Action

A command is the adapter-facing operation produced from a resolved effect. The adapter translates this command into an external action such as MQTT publish, CLI output, simulation state update, or future UI/voice behavior.

### Frame

A frame is one deterministic execution step of the runtime. The frame collects inputs, applies events, runs systems, resolves intents, emits effects, logs what happened, and advances time/frame number.

---

## 9. Intent Resolution Model

The current conceptual flow is:

```text
Behavior / Agent components
    -> processed by systems, events, or schedules
    -> creates an Intent owned by a behavior or agent
    -> Intent exists as immutable runtime data
    -> Expiration logic marks expired intents
    -> IntentResolver resolves active intents by target
    -> Accepted intents change component state or produce effects
    -> ResolvedEffect is produced
    -> Adapter turns effect into external command/action
```

Important rules:

- The resolver does not ask each behavior what it wants every frame.
- Systems, events, schedules, or agents add intents to the world, usually on behalf of a behavior or agent owner.
- Active intents remain until lifetime metadata marks them expired or cleanup removes them.
- Intent queues may be organized by target key, with a priority heap per target so conflict and merge checks stay local to the affected component or effect.
- Explicit user intent should normally outrank background behaviors.
- LLM-based arbitration is not a default conflict policy.
- If an LLM or script check is slow, it should run asynchronously and report back through events.

---

## 10. Component and Intent Data Design

Accepted baseline:

- no C++ inheritance between components;
- components remain plain data;
- component storage is only a typed slot pool for one component type;
- `ComponentRegistry` maps template component types plus global component names to storage slots;
- behavior queries start from a component type, so storage itself does not need to know its component type;
- behavior state, capabilities, access, and system eligibility are represented through ordinary shared named component instances plus access records;
- access relationships connect a behavior to a named component with read/write permissions;
- behavior-facing queries expose `name -> ComponentAccess`, where the name is the user/device-facing key and `ComponentAccess` stores an index into an access-record vector;
- `Coordinator` owns cross-manager behavior permission checks and behavior signatures;
- trusted component resolution looks up the access record by ID after coordinator validation;
- intents are immutable proposal records, not component rows;
- intent target keys should identify the component instance or external effect being changed;
- intent queues can use per-target priority heaps so the resolver can inspect the strongest candidate and merge compatible proposals;
- use factories or bundles so required component sets and intent fields are readable and hard to misuse;
- use validation systems to detect invalid behavior component combinations and invalid intent records.

Example conceptual composition:

```text
Shared component instances:
  LightState "officeLight"
    physicalDevice: "zigbee://office-light"
    brightness: 40

Behavior access:
  lightTracking -> "officeLight", LightState, ReadWrite
  focusActive -> "officeLight", LightState, Read

Intent record:
  owner BehaviorId
  target key
  operation/value
  priority
  lifetime metadata
  merge/conflict policy
```

The scheduler/runtime should not need to know every intent policy detail. Dedicated resolver/expiration logic processes intent records and applies accepted changes deterministically.

---

## 11. Repository and Folder Direction

Current intended engine repo:

```text
liquid/
  CMakeLists.txt
  README.md

  include/
    liquid/
      vocabulary/
      core/
      runtime/
      events/
      behaviors/
      intents/
      systems/
      adapters/

  src/
    core/
    runtime/
    events/
    behaviors/
    intents/
    systems/
    adapters/

  apps/
    liquid_sim_cli/

  tests/
    vocabulary/
    core/
    runtime/
    intents/
    systems/

  docs/
    design/
    adr/
```

The `include/liquid/` prefix is used because Liquid is intended to be a reusable engine/framework, not only a private app. Public include paths should avoid generic names such as `<core/World.hpp>` or `<events/Event.hpp>`.

Folder responsibility:

| Folder | Responsibility |
|---|---|
| `vocabulary/` | Shared project language: IDs, time, priority, result types, names. |
| `core/` | Minimal ECS-inspired world, component storage, behavior identity, queries. |
| `runtime/` | Frame context, frame loop, runtime orchestration. |
| `events/` | Events entering the runtime and event queue/log definitions. |
| `behaviors/` | Behavior metadata, behavior components, behavior factories. |
| `intents/` | Intent records, queues, factories/bundles, resolver, resolved effects. |
| `systems/` | Systems that run over world data during frames. |
| `adapters/` | External boundary: CLI, simulation, MQTT, future web/voice. |

---

## 12. Accepted Decisions So Far

- Liquid Layer is the final application/research project.
- Liquid is the standalone engine/framework/runtime.
- Solid is the first stage: deterministic ECS-inspired runtime.
- The engine should be its own repo, reusable by both the final application and future simulation/data-collection environments.
- The internal folders should use technical names, not the metaphors `solid` and `liquid`.
- We will build a minimal custom ECS instead of starting with an ECS library.
- The domain term is behavior, not entity, where possible.
- M1 uses simple world-local `BehaviorId` and `IntentId` handles.
- ID recycling is allowed through registry APIs in M1.
- Strong or generational IDs are deferred unless needed later.
- Composite lookup keys may pack two 16-bit handles into one 32-bit value when that makes ownership or target lookup simpler and deterministic.
- `IntentId` uses the high 16 bits for owner behavior and low 16 bits for the local intent index.
- Components should be plain data.
- Component type is the system query signature; component names are the stable user/device-facing keys for shared component instances.
- Access relationships hold permissions between behaviors and named component instances.
- `ComponentType<T>` is the typed runtime handle returned by explicit component registration.
- Component type lookup is template-driven in M1, following the Superposition-style manager flow.
- `Coordinator` owns behavior signatures, behavior existence checks, cross-manager permission checks, and cleanup sequencing.
- Systems are registered by concrete type in M1 and store `Signature` requirements, behavior membership, and membership callbacks.
- Systems should own behavior logic.
- Intents are immutable proposed component-state changes or effects, not component rows and not immediate actions.
- Intent owner pools are aligned with behavior creation/destruction through `Coordinator`.
- The completed M1 test suite uses assert binaries, deterministic stress coverage, and an opt-in AddressSanitizer/UBSan CMake mode.
- Intent resolution should be target-oriented; per-target priority queues/heaps are the preferred starting point unless implementation evidence suggests otherwise.
- A decided intent becomes a resolved effect, then an adapter command/action.
- Intent lifetime is modeled through intent data plus factories/bundles.
- No C++ inheritance between components in v1.
- LLM conflict resolution is not a default mechanism.
- LLM/script work should be async when it may be slow.
- Replay/logging must influence the design from the start.

---

## 13. Open Decisions

These are intentionally not finalized in this document:

- future C++ APIs after the completed M1 core;
- higher-level runtime query APIs beyond M1 `name -> ComponentSlotId` access maps;
- exact intent queue and target-key representation;
- serialization format;
- event log format;
- replay format;
- Go integration approach;
- LLM integration approach;
- behavior trigger representation;
- adapter API;
- frame scheduling details beyond the high-level order;
- whether script customization uses Go directly, WASM, Lua, or another boundary.

These belong in the implementation guide and ADRs.

---

## 14. Source Notes

This document builds on the earlier NeurOS design baseline and the implementation discussion that followed it.

External references supporting the direction:

1. Flecs documentation describes ECS entities as identifiers and components as data attached to entities. This supports separating identity, data, and systems.
   - https://www.flecs.dev/flecs/md_docs_2EntitiesComponents.html
2. Bevy describes entities as unique things assigned groups of components, then processed by systems over component types. This supports component-type query signatures.
   - https://bevy.org/learn/quick-start/getting-started/ecs/
3. EnTT's registry/storage/query model supports component-type storage and queries without making commands or intents into components.
   - https://github.com/skypjack/entt/wiki/Entity-Component-System
4. Unity ECS documentation emphasizes data-oriented ECS for control and determinism, and its component model separates component data from systems.
   - https://unity.com/ecs
   - https://docs.unity.cn/Packages/com.unity.entities%401.0/manual/concepts-components.html
5. Home Assistant's AI agents article explicitly warns that AI models hallucinate and should not be completely trusted, while also showing that AI can be useful through constrained APIs such as intents.
   - https://www.home-assistant.io/blog/2024/06/07/ai-agents-for-the-smart-home/
6. Modern CMake project structure commonly separates public headers under `include/<project>/`, implementation under `src/`, apps, tests, docs, and external dependencies.
   - https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html
7. Architecture Decision Records capture significant decisions, context, and consequences, and will be used later to avoid losing reasoning.
   - https://adr.github.io/
   - https://docs.arc42.org/tips/9-5/
