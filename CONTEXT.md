# Bifrost Controller context

## Purpose

Build Bifrost Controller, TROA's modern desktop controller mapping app alongside Bifrost Server Manager in the TROA gaming software ecosystem, based on Edward's AntiMicroX fork. Implementation of the Windows preview is authorized.

## Product direction - 2026-10-05

- Current product name: Bifrost Controller (renamed from the temporary TROA PC Controller Mapper name).
- Audience: TROA's community.
- Initial platform: Windows. Preserve architectural room for Linux support later; Linux is not part of initial release acceptance.
- Requested features: a modern interface, MCP access for collaboratively creating and editing profiles, and curated desktop/browser/game/controller profiles delivered through updates.
- Planned distribution: branded installer/portable packages and corresponding source in GitHub Releases, consumed by a future TROA website product/download page. Website work and publishing have not begun.
- Ownership: Edward keeps the personal GitHub repository. Collaborator access stays unchanged at his request; TROA organization admins do not automatically have personal-repository access. TROA can distribute public release artifacts.
- Initial implementation retains C++17/Qt Widgets/SDL2 and modernizes the workspace around the existing mapping controls. Profile management and local MCP are separate layers; a QML migration or Rust remake is deferred.
- Preserve AntiMicroX attribution, existing copyright notices, and applicable GPL obligations in derived releases.

## Repository

- Fork: https://github.com/edwardsong08/bifrost-controller
- Upstream: https://github.com/AntiMicroX/antimicrox
- Default branch: master
- Initial checkout: dbb6349603e6ee2426d300eee9570e1ed44fb9f5
- Local folder: C:\Users\edwar\Desktop\ACTIVE PROJECTS\TOOLS AND FUN\antimicrox
- Stack: C++17, Qt 6 with Qt 5 fallback, SDL2, CMake.
- License: existing GPL license; preserve upstream notices.

## Apps installed for inspection

- Bifrost Controller preview 0.1.2 from successful GitHub Actions run 37411297952, built from c0a8d9837ebf743107ca52fa52cbbbe773855184.
- Executable: C:\Users\edwar\AppData\Local\Programs\Bifrost Controller\bin\bifrost-controller.exe
- MCP companion: same bin directory, bifrost-controller-mcp.exe. A client connection has not been configured or exercised.
- Installer, portable ZIP, source ZIP, and checksums: distribution/bifrost-preview-0.1.2 (ignored local artifacts). All three hashes matched the manifest; installer SHA256 d38fd60c9e68fc820404b0e94cae4333e73b7027577d5b23ab810fce2b9d9b75.
- Installer exit code 0; Windows uninstall registry lists version 0.1.2. The built-in --show command exposed the branded main window; process 48124 reports title Bifrost Controller and is responding.
- Replaced the obsolete TROA 0.1.0 preview (uninstaller exit 0). Backed up shared user data under distribution/bifrost-preview-0.1.2/previous-user-data; existing settings matched exactly after install/uninstall. The compatibility data folder and settings filename remain unchanged. Historical 0.1.0 packages remain under distribution/preview.
- No automated, visual, or hardware tests were run. An open process is not controller/profile/MCP acceptance.

- Official upstream Windows release 3.6.1, downloaded from GitHub Releases.
- Executable: C:\Users\edwar\AppData\Local\Programs\AntiMicroX\bin\antimicrox.exe
- Installer and provenance record: sibling antimicrox-downloads folder.
- Installer SHA256 matched the official GitHub asset digest.
- AntiMicroX remains installed separately from the new fork preview.

## Source map

- src/gui/: Qt windows, mapping dialogs, calibration, settings.
- src/inputdaemon.cpp and src/inputdevice.cpp: input processing and device management.
- src/eventhandlers/winsendinputeventhandler.cpp: Windows keyboard/mouse output.
- src/joybuttontypes/ and src/gamecontroller/: mapping behavior and controllers.
- share/gamecontrollerdb_windows.txt: Windows controller database.
- CMakeLists.txt and .github/workflows/: build, packaging, and CI.
- tests/: Qt GUI test sources; CMake currently requests Qt5Test.

## Next action list for discussion

These remain future work unless Edward authorizes the next implementation phase.

1. Collect Edward's feedback on the installed preview and choose which inherited mapping dialogs to modernize next.
2. Connect a compatible MCP client and collaboratively author game-specific and controller-specific profiles.
3. Extend the managed profile schema to triggers and advanced macros as needed; application switching and named layouts are now implemented in the preview.
4. Review the separately authorized local TROA website pages/menu, then obtain release/push approval before publishing versioned installer, portable, and corresponding source assets together.
5. Establish the release/update process, including curated catalog changes and Windows signing when available.
6. Finish Linux branding and packaging before treating it as a supported product platform.

## Verification limits

Windows 0.1.2 compilation and packaging succeeded in GitHub Actions run 37411297952 with WITH_TESTS=OFF. Downloaded installer, portable, and source archive hashes matched the included manifest. Installation, registry metadata, settings preservation, and a responding branded window were confirmed. No automated, visual, or hardware tests are authorized for this preview; game bindings, switching behavior, and an actual AI-client connection remain untested. Desktop screenshot automation timed out during upstream installation. Edward requested the actual app rather than opening the source in VS Code.

## Authorized implementation - 2026-10-05

- Edward approved proceeding and explicitly requested no test runs. Compilation/packaging are permitted; runtime behavior must be described as untested.
- Work branch: codex/troa-controller-mapper.
- New code: src/troa/identity.h, profilestore.*, localapi.*, modernshell.*, and mcp_main.cpp.
- Bundled profile catalog: profiles/catalog.json. Personal definitions and history live under the user data directory.
- MCP operations use a same-user Qt local socket and stdio companion; assistant access has a visible off switch.
- Windows preview workflow packages an installer, portable ZIP, corresponding source ZIP, and SHA256 checksums. No ctest or other test commands run.
- Inherited advanced dialogs remain available. MCP schema initially covers standard SDL buttons, D-pad, and sticks, with key chords and mouse actions.

## UI refinement - 2026-10-05

- Edward requested clearer MCP discovery, an Apple-like level of simplicity/polish, and the real TROA logo used by deployed sites. Preserve the full mapping features.
- Version 0.1.1 adds Get started guidance, calmer light/dark styling, readable profile assignment tables, controller-aware PlayStation button names, and explicit MCP setup entry points in header/sidebar/menu.
- MCP setup includes JSON and Codex TOML, copyable executable path and starter request, companion availability, and last-request status. Enabling access is not presented as a connected AI client.
- Branding uses the exact PNG bytes served at the deployed /favicon.ico; the original controller symbol remains in controller guidance/navigation.
- The title-bar close action now uses the existing save/discard/cancel flow for edited mappings.
- A read-only diagnostic of the installed 0.1.0 app returned MCP status and a DualSense controller with no unsaved edits. No mappings were changed. Desktop inspection still timed out; no test suites or hardware-input tests are run.
- Updated preview compilation/installation is pending.

## Application context and TROA styling - 2026-10-05

- Edward requested clear per-controller app/profile state, Space/Ground-style layout switching by keyboard/controller, and a visible monitor-aware notice. He also requested color/style inspiration from the deployed TROA main site and Bifrost.
- Version 0.1.2 adds Applications rules with exact executable matching and stable controller identifiers, native profile selection, named mapping sets, focus-scoped keyboard shortcuts, and unused normalized SDL controller buttons to cycle layouts on release.
- Controller and Applications views distinguish assigned versus active profile/layout and show blocked switches. Existing native mapping sets/input release handling are retained. Modern rules take priority over legacy auto profiles only for their matching app/controller.
- Switch notices are non-activating, click-through, and topmost. Default display is the focused app's monitor with primary fallback; primary/all display options exist. Exclusive fullscreen desktop-overlay visibility is not guaranteed.
- Managed JSON schema 1 remains compatible and now accepts optional named layouts. MCP exposes context/rule management and exact-revision native export for assigning managed multi-layout profiles.
- Source palette comes from current deployed main-site CSS (ivory #f4f0e8, charcoal #0d0e0f, gold #d4a84f), with Bifrost navy surfaces and compact navigation. No other TROA repository is modified.
- UI/branding-only 0.1.1 compiled and packaged successfully in run 37408876833 on 8285def9c4e551ee6682c7b1469f286b7bc3bc16. It has not been installed; the expanded 0.1.2 build is pending. No test runs are performed.

## Bifrost ecosystem identity - 2026-10-05

- Edward renamed the app Bifrost Controller so it belongs to the Bifrost/TROA gaming software ecosystem.
- App title, desktop/installer identity, binaries, update URLs, MCP server/config, README, and repository name use Bifrost Controller / bifrost-controller.
- Preview settings filename, data folder, and internal TROA settings keys remain compatible with 0.1.0, preserving existing profiles/history/settings. The code folder and codex/troa-controller-mapper branch remain in place for continuity.
- Personal ownership and collaborator permissions stay unchanged. TROA logo and main-site/Bifrost styling stay in use. This branding does not claim a new Server Manager API integration.
- The renamed 0.1.2 package is pending compilation and local installation. No tests, public release, or website deployment are performed.

## Current delivery - 2026-10-06

- Personal repository is now edwardsong08/bifrost-controller; draft PR #1 remains open and attached. Ownership/collaborator access is unchanged.
- Bifrost Controller 0.1.2 compiled, packaged, installed, and opened as detailed above. Earlier pending-build notes describe their historical state.
- Edward authorized two new main-site pages and a final Gaming > Bifrost Downloads submenu for local review before pushing. Server Manager is a coming-soon overview; Controller is Windows-only with installer setup and GitHub access. This website work is separate and does not authorize a public release or site deployment.

