# TROA PC Controller Mapper context

## Purpose

Build toward TROA PC Controller Mapper, a modern controller mapping app for TROA's community based on Edward's AntiMicroX fork. Implementation of the first Windows preview is authorized.

## Product direction - 2026-10-05

- Working product name: TROA PC Controller Mapper.
- Audience: TROA's community.
- Initial platform: Windows. Preserve architectural room for Linux support later; Linux is not part of initial release acceptance.
- Requested features: a modern interface, MCP access for collaboratively creating and editing profiles, and curated desktop/browser/game/controller profiles delivered through updates.
- Planned distribution: branded installer/portable packages and corresponding source in GitHub Releases, consumed by a future TROA website product/download page. Website work and publishing have not begun.
- Ownership: Edward keeps the personal GitHub repository. Collaborator access stays unchanged at his request; TROA organization admins do not automatically have personal-repository access. TROA can distribute public release artifacts.
- Initial implementation retains C++17/Qt Widgets/SDL2 and modernizes the workspace around the existing mapping controls. Profile management and local MCP are separate layers; a QML migration or Rust remake is deferred.
- Preserve AntiMicroX attribution, existing copyright notices, and applicable GPL obligations in derived releases.

## Repository

- Fork: https://github.com/edwardsong08/troa-pc-controller-mapper
- Upstream: https://github.com/AntiMicroX/antimicrox
- Default branch: master
- Initial checkout: dbb6349603e6ee2426d300eee9570e1ed44fb9f5
- Local folder: C:\Users\edwar\Desktop\ACTIVE PROJECTS\TOOLS AND FUN\antimicrox
- Stack: C++17, Qt 6 with Qt 5 fallback, SDL2, CMake.
- License: existing GPL license; preserve upstream notices.

## App installed for inspection

- Official upstream Windows release 3.6.1, downloaded from GitHub Releases.
- Executable: C:\Users\edwar\AppData\Local\Programs\AntiMicroX\bin\antimicrox.exe
- Installer and provenance record: sibling antimicrox-downloads folder.
- Installer SHA256 matched the official GitHub asset digest.
- This is the upstream binary, not a build of the fork.

## Source map

- src/gui/: Qt windows, mapping dialogs, calibration, settings.
- src/inputdaemon.cpp and src/inputdevice.cpp: input processing and device management.
- src/eventhandlers/winsendinputeventhandler.cpp: Windows keyboard/mouse output.
- src/joybuttontypes/ and src/gamecontroller/: mapping behavior and controllers.
- share/gamecontrollerdb_windows.txt: Windows controller database.
- CMakeLists.txt and .github/workflows/: build, packaging, and CI.
- tests/: Qt GUI test sources; CMake currently requests Qt5Test.

## Candidate action list for discussion

These are inspection targets, not approved changes or confirmed runtime bugs.

1. Establish a baseline with Edward's controller: detection, mapping, mouse movement, dead zones, calibration, profile save/load, reconnect, tray behavior, and automatic profiles.
2. Choose a repeatable local Windows build setup aligned with existing CI; add clear developer instructions once the setup is verified.
3. Review controller mapping usability: first-run guidance, controller layout, terminology, accessibility, and high-DPI behavior after inspecting the running app.
4. Assess test viability: Qt5Test is requested despite Qt6 being preferred, and GuiTests linkage is commented out. Existing CI builds without enabling WITH_TESTS. Reproduce before fixing.
5. Review Windows input diagnostics: SendInput return values are ignored at several call sites. Consider reporting failures once user impact is established.
6. Decide fork identity, release packaging, and update behavior before shipping: the current update check points to upstream AntiMicroX releases.

## Verification limits

No local source build or hardware controller acceptance test has been completed. Desktop screenshot automation timed out during setup. Edward requested the actual app rather than opening the source in VS Code.

## Authorized implementation - 2026-10-05

- Edward approved proceeding and explicitly requested no test runs. Compilation/packaging are permitted; runtime behavior must be described as untested.
- Work branch: codex/troa-controller-mapper.
- New code: src/troa/identity.h, profilestore.*, localapi.*, modernshell.*, and mcp_main.cpp.
- Bundled profile catalog: profiles/catalog.json. Personal definitions and history live under the user data directory.
- MCP operations use a same-user Qt local socket and stdio companion; assistant access has a visible off switch.
- Windows preview workflow packages an installer, portable ZIP, corresponding source ZIP, and SHA256 checksums. No ctest or other test commands run.
- Inherited advanced dialogs remain available. MCP schema initially covers standard SDL buttons, D-pad, and sticks, with key chords and mouse actions.

