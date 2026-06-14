# DEVELOPMENT_TRACKING.md — Liquid Project Tracking

This file tracks the practical development path for the `liquid` engine.

The goal is to keep the current work small enough to implement and test without being overwhelmed.

---

## Project Layers

### Liquid Layer

The final application/research project.

Purpose:

- help neurodivergent people reduce cognitive friction;
- prepare environments for tasks;
- adapt routines to individual preferences;
- collect data through real and simulated interactions.

Liquid Layer is not the current coding target.

---

### Liquid

The standalone engine/framework/runtime.

Purpose:

- provide a deterministic behavior runtime;
- later provide an adaptive layer that can generate or modify behavior blocks;
- be reusable by both the final Liquid Layer application and future simulation/data-collection tools.

The repository currently being planned is `liquid`.

---

### Stage 1 — Solid

Current stage.

Purpose:

- build the deterministic ECS-inspired foundation;
- model behaviors, components, intents, queries, and registries;
- avoid LLM, Lua, hardware, and application-specific logic at first.

Solid should be small, deterministic, testable, and independent.

---

### Stage 2 — Liquid

Future stage.

Purpose:

- add the adaptive/LLM layer;
- generate or modify solid behavior blocks;
- help users express routines in natural language;
- keep the deterministic Solid runtime as the execution authority.

Not active yet.

---

### Stage 3 — Liquid Layer

Future application stage.

Purpose:

- build the neurodivergent-support smart environment project;
- connect real/simulated environments;
- collect user interaction data;
- explore gamified or public testing flows.

Not active yet.

---

## Current Development Policy

Start flat and small.

Do not create the full future architecture upfront. Add folders only when the current milestone needs them.

Current minimal structure:

```text
liquid/
  CMakeLists.txt
  AGENTS.md
  DEVELOPMENT_TRACKING.md
  Liquid_Concepts_and_Architecture.md
  ARTICLE_NOTES.md

  include/
    liquid/
      Ids.hpp
      ComponentStorage.hpp
      ComponentRegistry.hpp
      Coordinator.hpp
      BehaviorRegistry.hpp
      IntentRegistry.hpp
      SystemRegistry.hpp

  src/
    ComponentRegistry.cpp
    Coordinator.cpp
    BehaviorRegistry.cpp
    IntentRegistry.cpp
    SystemRegistry.cpp

  tests/
    test_ids.cpp
    test_component_storage.cpp
    test_component_registry.cpp
    test_coordinator.cpp
    test_behavior_registry.cpp
    test_intent_registry.cpp
    test_system_registry.cpp
    test_stress.cpp
```

---

## Stage 1 — Solid Milestones

### M1 — Modified ECS Core

**Status:** Done

Completion note:

M1 is complete. The ECS core now has behavior identity, typed component registration/storage, coordinator-managed behavior signatures, behavior-owned intent pools, template-addressed systems with membership callbacks, expanded assert tests, deterministic stress coverage, and an opt-in sanitizer build mode.

Goal:

Build the smallest core that can create behaviors, create immutable intents owned by behaviors, create shared named component instances, query components by registered type handle and name, track behavior access, and clean up correctly.

Components are shared plain data instances queried by `ComponentType<T>` handles and component names. `ComponentStorage<T>` is the typed slot pool for one component type and owns access records for slots in that storage. `ComponentRegistry` owns component type IDs, the type-erased storage map, and globally unique names per component type.
Behaviors do not own components exclusively; they receive read/write access to named components. Access is stored in the typed storage as `BehaviorId -> vector<ComponentSlotAccess>`, where each access records a slot and read/write mode. Systems see this through `Coordinator`/`ComponentRegistry` as `name -> ComponentSlotId` for a behavior and component type.
Component types are registered with explicit stable names in M1 and return typed runtime handles. `Signature` is the shared component-type bitset used by both behavior composition and system requirements. `Coordinator` owns behavior signatures, behavior existence checks, cross-manager access checks, and cleanup sequencing.
Packed composite keys are used for owner/local intent IDs; component access uses behavior/type/slot indexing instead of packed access IDs.
System coordination is part of the M1 ECS core. `SystemRegistry` registers `System` instances by concrete system type, stores each system's `Signature` and matching behavior membership, and emits membership callbacks when behaviors enter or leave a system. `Coordinator` compares behavior signatures to system signatures and updates membership when access or lifetimes change.

Current implementation notes:

- `ComponentType<T>` is the public typed handle returned from `register_component<T>(...)`.
- `TypeName -> ComponentTypeId` is resolved at registration; `ComponentTypeId` is then the internal key for storages, name maps, and behavior signatures.
- `Coordinator` is the outside-facing interface for creating, managing, reading/writing, and deleting behaviors, intents, and components.
- Component removal clears access to the removed slot before recycling it, erases name/slot indexes, and lets `Coordinator` reset affected behavior signatures.
- Behavior destruction clears component access maps, erases behavior signatures, destroys owned intents, and recycles the behavior ID.
- System execution is not part of M1. Systems are template-addressed by concrete type and currently have `Signature` requirements, matching behavior membership, and membership-event callbacks.
- `LIQUID_ENABLE_SANITIZERS` is available as an opt-in CMake option for AddressSanitizer and UBSan on Clang/GCC.
- Latest verification: full CMake build and all eight M1 tests pass in `/tmp/liquid-m1-tests`; sanitizer build and all eight tests pass in `/tmp/liquid-m1-sanitizers`.

Future tracking:

- M2 intent lifetime and expiration should keep valid behavior ownership enforced through `Coordinator`.
- Future resolution systems should consume `name -> ComponentSlotId` maps and resolve component data through coordinator/registry APIs.
- M1 system coordination now covers inherited system objects, `Signature` requirements, behavior-to-system matching, and system membership updates.
- When systems exist, `Coordinator` should remain the cross-manager boundary that updates system membership after behavior signatures change through access grants, access revokes, component removal, or behavior destruction.
- Deterministic system execution order and frame-loop scheduling are future runtime responsibilities after the M1 system registry/interface exists.
- Future runtime loops, systems, and adapters should not keep long-lived raw component pointers; prefer behavior IDs, typed component handles, names, and slots that can be revalidated.

Scope:

- IDs;
- behavior registry;
- intent registry;
- component storage;
- component registry;
- coordinator;
- system registry/interface;
- basic behavior/component query mechanism;
- tests;
- CMake boilerplate.

Files expected:

```text
include/liquid/Ids.hpp
include/liquid/ComponentStorage.hpp
include/liquid/ComponentRegistry.hpp
include/liquid/Coordinator.hpp
include/liquid/BehaviorRegistry.hpp
include/liquid/IntentRegistry.hpp
include/liquid/SystemRegistry.hpp

src/ComponentRegistry.cpp
src/Coordinator.cpp
src/BehaviorRegistry.cpp
src/IntentRegistry.cpp
src/SystemRegistry.cpp

tests/test_component_storage.cpp
tests/test_component_registry.cpp
tests/test_coordinator.cpp
tests/test_behavior_registry.cpp
tests/test_intent_registry.cpp
tests/test_system_registry.cpp
tests/test_ids.cpp
tests/test_stress.cpp
```

Success criteria:

- [x] Create a `BehaviorId`.
- [x] Create an `IntentId` owned by a behavior.
- [x] Create named component instances.
- [x] Retrieve components by type/name.
- [x] Remove components safely.
- [x] Track behavior access as a `name -> ComponentSlotId` map per behavior and component type.
- [x] Enforce read/write access at the `Coordinator` boundary before behavior code receives access.
- [x] Destroy a behavior and clean its owned intents and component access maps.
- [x] Recycle behavior IDs through registry APIs.
- [x] Recycle intent IDs through registry APIs.
- [x] Register systems with signatures.
- [x] Update system membership when behavior signatures change.
- [x] Remove destroyed behaviors from system membership.
- [x] Compile and run all M1 tests.
- [x] Compile and run expanded stress/property-style tests.
- [x] Compile and run all tests with opt-in AddressSanitizer and UBSan.

Out of scope:

- runtime frame loop;
- event queue;
- intent resolution;
- intent expiration systems;
- Lua;
- LLM;
- adapters;
- simulation CLI.

---

### M2 — Intent Lifetime and Expiration

**Status:** Current

Goal:

Add the first lifetime model for immutable intent records.

Expected concepts:

- intent lifetime metadata;
- until-time expiration;
- until-cancelled expiration;
- expiration status/reason;
- cleanup of expired intents.

Current implementation direction:

- Keep intent lifetime as intent metadata, not component storage.
- Keep `Coordinator` as the public boundary for behavior-owned intent creation and cleanup.
- Add deterministic expiration logic that receives explicit time/frame input rather than depending on wall-clock globals.
- Mark or classify expired intents without mutating their semantic request contents.
- Cleanup may remove expired intents through the existing registry/coordinator flow.

Likely new files:

```text
include/liquid/IntentLifetime.hpp
include/liquid/IntentExpiration.hpp
src/IntentExpiration.cpp
tests/test_intent_expiration.cpp
```

Keep structure flat unless there are enough files to justify splitting folders.

Success criteria:

- [ ] Represent persistent, until-time, and until-cancelled intent lifetime metadata.
- [ ] Query whether an intent is alive, expired, or cancelled.
- [ ] Expire until-time intents deterministically from explicit input.
- [ ] Leave until-cancelled intents alive until cancellation or cleanup.
- [ ] Clean expired intents without breaking owner-pool recycling.
- [ ] Preserve all completed M1 behavior/component/system tests.

---

### M3 — Intent Resolution

**Status:** Planned

Goal:

Resolve alive intents into selected effects.

Expected concepts:

- intent target;
- intent value;
- intent priority;
- target-key queues or heaps for competing intents;
- merge policy for compatible intents;
- intent source metadata;
- resolver policies;
- selected effect output.

Likely new files:

```text
include/liquid/Intent.hpp
include/liquid/IntentResolver.hpp
src/IntentResolver.cpp
tests/test_intent_resolution.cpp
```

---

### M4 — Minimal Frame Loop

**Status:** Planned

Goal:

Introduce a minimal deterministic loop that calls the existing managers/systems in order.

Expected concepts:

- frame number;
- frame context;
- begin frame;
- run intent expiration;
- resolve intents;
- cleanup;
- frame log stub.

This is the first milestone where a `runtime/` folder may become justified.

---

### M5 — Lua Behavior Scripting

**Status:** Planned

Goal:

Allow Lua behaviors to create intents through controlled APIs.

Important rule:

Lua creates new intents. It does not mutate existing intents.

---

### M6 — Simulation CLI

**Status:** Planned

Goal:

Add a small executable that runs Solid without hardware.

This is the first milestone where `apps/` becomes useful.

---

## Stage 2 — Liquid Milestones

Not detailed yet.

Expected future areas:

- LLM-assisted behavior generation;
- behavior proposal flow;
- user confirmation;
- adaptation based on logs/preferences;
- no LLM authority inside deterministic conflict resolution by default.

---

## Stage 3 — Liquid Layer Milestones

Not detailed yet.

Expected future areas:

- neurodivergent-support scenarios;
- environment preparation routines;
- gamified/simulated data collection;
- consent/privacy/data export design;
- real smart-home adapters.

---

## Milestone Advancement Rule

When a milestone is completed:

1. Mark its status as `Done`.
2. Add a short completion note.
3. Promote the next milestone to `Current`.
4. Expand only the new current milestone.
5. Update `AGENTS.md` so Codex knows the new allowed scope.
6. Do not expand future stages unless the user explicitly asks.

---

## Current Notes

- The current coding focus is M2 intent lifetime and expiration.
- M1 modified ECS core is complete and should be treated as foundation, not active scope.
- The project owner will implement core logic manually.
- Codex should generate headers, tests, CMake, and boilerplate unless explicitly asked to implement logic.
- The current structure is intentionally small to keep the project controllable.
- Superposition is cloned at `/home/raul/Desktop/superposition` and should be used as the persistent ECS reference.
- For component-manager work, keep the distinction clear: component type is the system query signature; component names are the user/device-facing way to find shared component instances.
- Prefer registered component IDs for type lookup, while using string names for user-created component instances.
- Access relationships carry read/write permissions and are exposed as `name -> ComponentSlotId` maps for each behavior and component type.
- Use named constants for packed key shifts/masks instead of repeating raw numeric literals when a packed key is still useful.
- Intents are not component rows. They are immutable proposals to change component state or emit effects.
- Intent lifetime is metadata on immutable intent records, not component storage.
- Script components and systems that execute them are future work; the completed M1 storage/query model only makes that path possible later.
