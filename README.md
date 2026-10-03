# SMAGF — Smoke, Mirrors & Game Feel Showcase

> **A prototype proves that an idea can work. A showcase makes people believe that it will.**

SMAGF is a small **Unreal Engine 5.8 C++ showcase project** built as a companion to the conference talk:

**“Smoke, Mirrors, and Game Feel: Creating a Showcase That Sells (Horror Edition)”**

The project starts from a deliberately plain gameplay prototype and progressively adds presentation layers that change how the exact same core interaction **feels, reads, and sells itself**.

It is not intended to be a production-ready gameplay framework. It is a compact, inspectable example of how to turn a working prototype into a convincing showcase.

---

## Contents

- [What this project demonstrates](#what-this-project-demonstrates)
- [Showcase progression](#showcase-progression)
- [Core design principles](#core-design-principles)
- [Requirements](#requirements)
- [Getting started](#getting-started)
- [Building from source](#building-from-source)
- [Running the showcase](#running-the-showcase)
- [Showcase controls](#showcase-controls)
- [Enemy debug modes](#enemy-debug-modes)
- [Architecture overview](#architecture-overview)
- [Testing](#testing)
- [Repository structure](#repository-structure)
- [Using the project for a presentation](#using-the-project-for-a-presentation)
- [Troubleshooting](#troubleshooting)
- [Third-party content](#third-party-content)
- [Conference resources](#conference-resources)
- [License](#license)

---

## What this project demonstrates

SMAGF explores a simple question:

> **How far can presentation, feedback, focus, and staging push a basic prototype without replacing its core mechanic?**

The project demonstrates:

- a minimal native C++ gameplay baseline;
- cumulative Game Feel layers;
- deterministic showcase presets;
- procedural hit and death reactions;
- impact feedback and hit stop;
- projectile squash/stretch and readability;
- VFX, audio, shells, decals, camera response, and permanence;
- simple enemy pursuit and attack behaviour without a production AI stack;
- mouse-driven soft target focus;
- presentation-pressure logic that prevents repeated hit stop from becoming exhausting;
- debug controls designed for recording, iteration, and conference demonstrations;
- native Unreal automation tests plus broader runtime regression validation.

The emphasis is deliberately on **communication and perceived quality**, not on shipping a generalized framework.

---

## Showcase progression

The showcase is cumulative.

```text
Baseline
   ↓
Feeling
   ↓
Animation
   ↓
Audiovisual
   ↓
Smoke & Mirrors
   ↓
Focus
   ↓
Juice Responsibly
   ↓
Full Showcase
```

### Baseline

The smallest useful gameplay loop:

- player movement;
- mouse-driven aiming;
- weapon fire;
- projectile travel;
- damage;
- health;
- enemy death.

The Baseline is intentionally plain.

Its job is to prove:

> **The mechanic works.**

---

### Feeling

Adds immediate response to player actions and impacts:

- anticipation;
- recoil;
- impact response;
- enemy hit reaction;
- knockback;
- hit stop.

The mechanic is still the same, but the interaction begins to feel intentional.

---

### Animation

Adds procedural motion and readable deformation:

- fire anticipation animation;
- weapon recoil animation;
- character fire reaction;
- enemy hit reaction;
- directional death response;
- squash and stretch;
- overshoot recovery;
- procedural wobble;
- projectile stretch in flight;
- projectile squash on impact.

The goal is not realism. The goal is **clarity, weight, and motion readability**.

---

### Audiovisual

Adds the main audiovisual presentation layer:

- muzzle flash;
- shot light flash;
- projectile visual scaling;
- projectile trail;
- shell ejection;
- fire audio;
- layered fire audio;
- impact flash;
- impact effect;
- impact audio;
- enemy flash;
- death effects;
- camera feedback;
- decals / impact evidence;
- corpse presentation and bounded permanence.

This is where the prototype starts to read like a polished gameplay slice rather than a mechanics test.

---

### Smoke & Mirrors

Adds enough enemy behaviour to sell intent:

- NavMesh-based pursuit;
- acceleration and braking;
- grounded movement over uneven terrain;
- engagement-distance hysteresis;
- facing;
- Telegraph;
- slow readable hostile projectile;
- cooldown;
- pursuit reacquisition.

This is intentionally **not** a production AI architecture.

There is no requirement for:

- Behavior Trees;
- State Trees;
- Blackboards;
- EQS;
- AI Perception;
- crowd simulation.

The point is:

> **Use the cheapest behaviour that creates the perception you need.**

---

### Focus

Focus is a **mouse-driven soft target focus / camera aim-assist presentation layer**.

When the cursor approaches a living enemy in screen space:

- the enemy can become the focused target;
- camera framing subtly attracts toward that target;
- FOV can tighten slightly;
- target emphasis/light follows the selected enemy;
- optional Focus post-process contributions can apply.

Focus does **not**:

- snap the mouse cursor;
- bend projectiles;
- change logical aim;
- select targets because they Telegraph;
- select targets because they are damaged;
- redirect to the player when the player is hit;
- transfer to corpses on death.

The design principle is:

> **Focus amplifies player intent instead of redirecting player attention.**

---

### Juice Responsibly

More feedback is not always better.

SMAGF includes a lightweight **Presentation Pressure** system that reduces repeated interruption while preserving important moments.

Runtime importance levels:

- **Minor** — environment / non-damaging contacts;
- **Standard** — surviving enemy damage;
- **Major** — killing hits and hostile player hits.

Presentation Pressure accumulates from repeated events and decays over game time.

The current implementation primarily scales existing hit-stop duration:

```text
Low pressure:
Standard hit → full authored hit stop

High pressure:
Standard hit → shorter hit stop

Major event:
always full response
```

A representative measured example from development:

```text
Standard hit:
40 ms authored → ~26 ms under high pressure

Major kill:
80 ms authored → 80 ms resolved
```

Focus and Telegraph remain independent from Presentation Pressure.

The goal is:

> **Dense combat should feel controlled, not muted.**

---

### Full Showcase

Enables the complete cumulative presentation stack.

Use it to compare the finished showcase against the original Baseline.

---

## Core design principles

### 1. Prototype Architecture vs Presentation Architecture

The gameplay prototype answers:

> Does the mechanic work?

The presentation architecture answers:

> Can the audience immediately understand and believe in it?

These are related, but they are not the same problem.

---

### 2. One clear owner per responsibility

The project deliberately avoids a single presentation “God Object”.

Examples:

- local transform composition stays local;
- final camera composition has one owner;
- Focus selection and Focus presentation coordination are separate from camera transform ownership;
- hit-stop / global feedback policy is separate from damage;
- enemy behaviour decisions remain separate from navigation plumbing.

---

### 3. Player intent beats automatic attention theft

The first Focus experiment was event-driven.

Technically it worked.

In practice it produced unpleasant camera movement because Telegraphs, hits, deaths, and player reactions competed for attention.

The final system uses the cursor as the intent signal.

---

### 4. Semantic information should survive presentation pressure

Important gameplay information is never budgeted away.

Examples:

- Telegraph remains readable;
- damage still happens;
- death still happens;
- Focus selection stays stable;
- projectiles still collide;
- Major events retain full response.

---

### 5. Presentation ROI matters

A showcase does not need every possible production system.

Prefer changes that deliver a large improvement in:

- readability;
- feel;
- perceived quality;
- communication;

for a reasonable implementation cost.

---

## Requirements

Developed and validated with:

- **Unreal Engine 5.8.x**
- **Windows 11**
- **Visual Studio 2022 toolchain / MSVC**
- C++ project build support

The project was developed around Unreal Engine **5.8.3**.

For source navigation and development, JetBrains Rider can also be used, but it is not required to run the project.

---

## Getting started

1. Clone the repository.

2. Make sure Unreal Engine 5.8 is installed.

3. Right-click `SMAGF.uproject` and generate project files if required.

4. Build the `SMAGFEditor` target.

5. Open:

```text
SMAGF.uproject
```

6. Load the default showcase map:

```text
/Game/TopDown/Lvl_TopDown
```

Source asset:

```text
Content/TopDown/Lvl_TopDown.umap
```

7. Run PIE.

---

## Building from source

With Unreal Editor closed, the project can be built from PowerShell with:

```powershell
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat' `
SMAGFEditor Win64 Development `
'-Project=D:\Games\UEProjects\_VibeCoding\SMAGF\SMAGF.uproject' `
-WaitMutex `
-NoHotReloadFromIDE
```

If your repository is stored elsewhere, replace the project path accordingly.

The important target is:

```text
SMAGFEditor Win64 Development
```

---

## Running the showcase

The Game Feel Lab exposes the cumulative presentation stack and individual settings.

A useful comparison sequence is:

```text
Baseline
→ Feeling
→ Animation
→ Audiovisual
→ Smoke & Mirrors
→ Focus
→ Juice Responsibly
→ Full Showcase
```

For presentation recording, use the same camera position and interaction whenever possible so that each layer can be compared directly.

---

## Showcase controls

### Debug / presentation shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl + F` | Open / close **Game Feel Lab** |
| `Ctrl + R` | Restart the current map / return to a fresh showcase state |
| `Ctrl + G` | Cycle global enemy debug behaviour mode |

Gameplay uses the project’s existing Top Down movement, mouse aiming, and fire input configuration.

---

## Enemy debug modes

`Ctrl + G` cycles every living showcase enemy through:

```text
Aggressive
→ Calm
→ Passive
→ Aggressive
```

### Aggressive

Default behaviour.

Enemies:

- pursue the player;
- use normal engagement-distance logic;
- Telegraph;
- fire hostile projectiles;
- enter cooldown;
- reacquire pursuit.

---

### Calm

Movement enabled, offense disabled.

Enemies:

- still pursue the player;
- retain normal navigation and movement behaviour;
- hold around the normal engagement range;
- do not Telegraph;
- do not Fire;
- do not spawn new hostile projectiles.

Useful for:

- demonstrating hit reactions;
- demonstrating Focus;
- demonstrating Juice Responsibly;
- recording moving targets without hostile-fire interruption.

---

### Passive

Movement disabled, offense disabled.

Enemies remain at their current world positions but are still valid gameplay targets.

They continue to support:

- damage;
- hit reactions;
- Focus;
- Juice Responsibly;
- death;
- corpse presentation;
- impact evidence;
- VFX and audio.

This is effectively the training-dummy mode for showcase recording.

Already-fired hostile projectiles are not destroyed when changing mode.

A fresh world / restart returns enemies to **Aggressive**.

---

## Architecture overview

The project intentionally uses a small number of narrow owners rather than a generalized framework.

### `USMAGFShowcaseSubsystem`

World-level showcase state and runtime settings.

Responsibilities include:

- active showcase settings;
- presets;
- typed feature/group gating;
- Custom snapshot behaviour;
- authored-default reset.

---

### `USMAGFFeedbackSubsystem`

Global feedback policy.

Responsibilities include:

- hit-stop ownership;
- time-dilation safety;
- presentation pressure;
- importance-aware hit-stop scaling.

It does **not** own damage, AI, navigation, Focus, or camera transforms.

---

### `USMAGFReactionComponent`

Local presentation response.

It is the authoritative local transform composer for procedural visual reactions such as:

- recoil;
- hit response;
- death response;
- squash/stretch;
- local procedural motion.

---

### `ASMAGFPlayerCameraManager`

The final camera composition authority.

Camera contributions are layered in a controlled order rather than accumulated frame-to-frame.

It owns the final camera result, including the presentation contributions that belong to the camera.

---

### `ASMAGFShowcaseDirector`

Focus presentation coordination.

Responsibilities include:

- living-target registration;
- mouse-driven screen-space Focus acquisition;
- Focus target lifetime;
- Focus strength;
- target-emphasis light;
- Focus-related presentation state.

It does **not** own final camera transforms.

---

### `USMAGFEnemyShowcaseComponent`

Simple enemy behaviour state and decision logic.

Responsibilities include:

- pursuit;
- engagement-distance logic;
- Telegraph;
- Fire;
- Cooldown;
- debug behaviour mode interpretation.

Navigation plumbing remains intentionally lightweight.

---

## Testing

SMAGF uses two complementary validation layers.

### Native Unreal tests — C++

Native automation tests live with the project source:

```text
Source/SMAGF/Tests/
```

These are used for cases such as:

- math / easing;
- serialization;
- reflected schema;
- enum migration;
- asset-oriented checks;
- low-level presentation invariants.

They are compiled into the editor build and run through Unreal’s automation framework.

---

### Runtime regression — Python-driven PIE

During development, broader gameplay validation is also performed using Python-driven Unreal Editor / PIE harnesses.

These tests can:

- launch isolated PIE sessions;
- find runtime actors;
- change presets/settings;
- generate contacts and damage;
- sample transforms and state;
- verify navigation and Focus;
- compare presentation-pressure behaviour;
- write structured JSON / CSV / log evidence.

Generated validation evidence should be treated as build output rather than production game content.

If runtime regression scripts are published with the repository, keep the scripts under a dedicated test/tooling folder and keep generated logs/results ignored.

---

## Repository structure

Typical high-level layout:

```text
SMAGF/
├─ Config/
├─ Content/
│  └─ TopDown/
│     └─ Lvl_TopDown.umap
├─ Source/
│  └─ SMAGF/
│     ├─ Camera/
│     ├─ Combat/
│     ├─ Debug/
│     ├─ Feedback/
│     ├─ Presentation/
│     ├─ Showcase/
│     ├─ Targets/
│     └─ Tests/
├─ SMAGF.uproject
└─ README.md
```

Generated directories such as these should not be treated as source:

```text
Binaries/
DerivedDataCache/
Intermediate/
Saved/
.vs/
.idea/
```

Use the repository’s `.gitignore` to keep build products and local IDE state out of source control.

---

## Using the project for a presentation

A useful recording structure is:

### Baseline

Record the plain interaction first.

### Feeling

Repeat approximately the same interaction and framing.

### Animation

Show procedural reactions and projectile motion.

### Audiovisual

Show the same mechanic with VFX, sound, shells, decals, camera, and permanence.

### Smoke & Mirrors

Show the enemy pursuing, stopping, Telegraphing, firing, and resuming behaviour.

### Focus

Record:

```text
cursor away
→ no Focus

cursor approaches enemy
→ Focus acquired
→ subtle framing / emphasis

cursor leaves
→ smooth recovery
```

For two enemies, demonstrate that another enemy Telegraphing does not steal Focus.

### Juice Responsibly

An A/B comparison is more useful than a beauty shot.

Demonstrate:

```text
isolated Standard hit
→ full authored hit stop

repeated events
→ Presentation Pressure rises

later Standard hit
→ shorter hit stop

Major Kill
→ still full-strength

quiet recovery
→ Standard response returns
```

### Full Showcase

Finish with one clean 15–20 second sequence that combines the complete stack.

---

## Troubleshooting

### The project does not compile after pulling C++ changes

Close Unreal Editor and rebuild `SMAGFEditor Win64 Development` using the command in [Building from source](#building-from-source).

Avoid relying on Hot Reload for structural C++ changes.

---

### The wrong map or gameplay class is active

Open:

```text
/Game/TopDown/Lvl_TopDown
```

and verify the project is using the SMAGF native gameplay classes rather than an unrelated template override.

---

### `Ctrl + F`, `Ctrl + G`, or `Ctrl + R` does not respond

Make sure the PIE viewport has keyboard focus.

These are in-game showcase/debug shortcuts, not global Windows shortcuts.

---

### Focus does not select an enemy

Focus is screen-space and mouse-driven.

Check that:

- the relevant Focus layer is enabled;
- the target is alive;
- the target is inside the viewport;
- the mouse is sufficiently close to the projected target position.

Focus intentionally does not acquire corpses.

---

### A Calm enemy is not moving

Calm enemies still use the normal engagement-distance behaviour.

An enemy already inside its accepted engagement range may hold position rather than move continuously into the player.

Move farther away to test pursuit reacquisition.

---

### A Passive enemy still reacts when shot

That is expected.

Passive disables:

- navigation;
- pursuit;
- offense.

It does **not** disable:

- damage;
- hit reactions;
- Focus;
- Juice Responsibly;
- death;
- presentation.

---

## Third-party content

This project uses several third-party assets obtained through **Epic Games Store / FAB**.

At the time they were added to this example project, those assets were available free of charge and were acquired through the author's personal Epic/FAB account for use in this project.

**The inclusion or use of third-party assets in this repository does not transfer ownership, entitlement, or a separate license to those assets to anyone downloading this project.**

If you want to use any of these assets independently in your own personal or commercial project, please obtain them through their official **Epic Games Store / FAB** listing and add them to your own account first.

If an asset is no longer free at the time you want to use it, please purchase it or otherwise obtain the appropriate official license/entitlement for your own account before using it separately from this example project.

In other words:

> **Treat every third-party asset as something you must legitimately acquire for your own account before reusing it outside this repository.**

Before publishing, redistributing, or forking this repository, also verify the current redistribution terms for **every third-party asset** included under `Content/`.

If an asset license does not allow redistribution of its raw files in a public source repository:

- remove the asset from the public repository;
- replace it with a redistributable equivalent; or
- document where users can obtain it separately.

Do not assume that an asset being free or downloadable means its raw files can be redistributed publicly.

This is especially important for:

- Niagara / VFX packs;
- decal packs;
- audio assets;
- marketplace sample content.

---

## Conference resources

Conference talk:

**Smoke, Mirrors, and Game Feel: Creating a Showcase That Sells (Horror Edition)**

Links can be added here when public:

- **Slides:** _TBD_
- **Talk recording:** _TBD_
- **Public repository:** _TBD_
- **Release build:** _TBD_

Optional conference QR image:

```markdown
![SMAGF repository QR code](Docs/Images/repository-qr.png)
```

---

## Why this project exists

SMAGF is intentionally small.

It is not trying to answer:

> “How should a production game implement every one of these systems?”

It is trying to answer:

> **“What is the minimum presentation architecture needed to make an idea communicate clearly and convincingly?”**

The project’s main thesis is:

> **A prototype proves that an idea can work. A showcase makes people believe that it will.**

And the final lesson is just as important:

> **Do not improve everything. Control attention.**

---

## License

No repository license is assumed by this README.

Before publishing the project, add an explicit `LICENSE` file describing how the source code may be used.

Also verify that any third-party content included in the repository is compatible with that public distribution model.

Unreal Engine and related trademarks are property of Epic Games, Inc.
