# Autonomous Agents

A Raylib project with two unique autonomous agents moving in a 2D world and collecting food (“targets”).

[Click to view a demo!](https://github.com/akulladdha/AutonomousAgents/blob/main/assets/gif.gif)

### Agent One — “Vacuum Cleaner”
- Seeks the **densest cluster of food** (not just the nearest item).
- Uses a score = distance ÷ (1 + nearby neighbors) to prefer clusters.
- Every few seconds, vacuums all food within its pickup radius.
- Think of it like a Roomba: efficient at clearing the biggest messes first.

### Agent Two — “Follower”
- **Follows Agent One** most of the time (like a sidekick).
- If food is very close, it detours to grab it instead.
- Periodically collects nearby food, then **drops some behind it** (“consume half, drop half”).
- Acts like a younger sibling—sticks with the leader but can’t resist nearby snacks.

---

## Quick Start

### Dependencies
- **Raylib** (4.x) and `raymath` (bundled with Raylib)
- **C++17** compiler
- **CMake** ≥ 3.16

#### macOS (Homebrew)
```bash
brew install raylib cmake
