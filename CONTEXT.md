# Bifrost Controller context

## 0.1.7 installation while Codex stays open - 2026-10-06

Owner reported needing to close Bifrost and Codex for every installer update.
Install Windows releases into immutable versions/<version>/bin and share payloads.
Stable format-1 Win32 launchers select an atomically published current.txt marker;
MCP host loads no Qt/MinGW DLLs and directly inherits only three duplicated stdio
handles into a versioned companion. Existing pre-0.1.7 bin/MCP paths are retained,
so migrating does not require closing Codex. Connected workers keep their existing
version until the client reconnects. Format-1 launchers must never change in place.
Finish-page launch offers an orderly mapper restart with Save/Discard/Cancel;
0.1.7 bypasses Close to tray for that request, never forces termination. A legacy
Close-to-tray mapper may require Bifrost > Quit once. Setup launches as the same
non-elevated desktop user or directs the user to the Start menu; it never falls
back to launching the mapper as the elevated installer. New startup links/profile
associations use the stable GUI command. Personal data locations stay unchanged.
Same-version reinstall skips existing payload files. Old payload cleanup deferred.
Compilation, packaging, release and live distribution are pending. No automated,
visual, hardware or gameplay tests. Do not restart/install the owner's app.

## Purpose

Build Bifrost Controller, TROA's modern desktop controller mapping app alongside Bifrost Server Manager in the TROA gaming software ecosystem, based on Edward's AntiMicroX fork. Implementation of the Windows preview is authorized.

## 0.1.6 template validation correction - 2026-10-06

Owner's 0.1.5 screenshot and actual read-only MCP metadata show only Desktop and
Browser templates. The validator checks controller_family values but omitted the
same field from its final allowed-field set. Every game/Steam template is therefore
rejected, and catalog download fails atomically. Actual MCP read of the STO template
reports Unknown profile field: controller_family. Fix the allowed-field set and
report the exact template/validation error for future rejected downloads. Preserve
strict unknown-field rejection, atomic catalog persistence and controller-model
compatibility. 0.1.6 compile/package succeeded from ec7955aa5d701172b01838c0f431fea0fcfc2e59
in Windows Actions 37432156127 (artifact 11398151009), tests disabled.
Published v0.1.6-preview includes installer, portable app, exact source and hashes.
Installer: 17,813,326 bytes, SHA256 6ce203eaa13dba26947e695df721874c496dc54782cd5007186a02c07623b1ba.
Source archive matches the build commit; static runtime/import/license inspection
passed. Website PR #104 merged at 05356458aeb57752cd3e7d299f8d252bb30d8220. At 2026-10-06T08:05:09.792565+00:00,
public page and TROA download readback confirmed 0.1.6 and the exact installer bytes.
This verifies distribution. Owner manually installs; the running 0.1.5 app has not
been replaced or restarted. No automated, visual, hardware or gameplay tests ran.

No automated, visual, hardware or gameplay tests are authorized. Do not install or
restart the owner's app; he upgrades manually. Native PR #1 remains draft.

## Current audit implementation — 2026-10-06

Version 0.1.5 was released on codex/troa-controller-mapper; 0.1.6 corrects its
template-validation regression. Edward requested
a fresh task-based UI, full source audit, Apply in Settings, Xbox plus both Valve
Steam Controller hardware generations, STO layouts, app-update popups and removal
of language mode. English-only startup retires the previous Language preference.
See docs/UI-AUDIT.md for inspected surfaces, fixes and explicit limitations.

The workspace now leads with actual state and separates Map controls, Profile
library, App rules and Assistant · MCP. Native saves are atomic, managed exports
immutable, profile activation honors the selected layout, and unavailable inputs
block incompatible templates. The reader filename-state fix addresses the user's
false “profile could not be loaded” report. Settings Apply keeps the dialog open,
reports persistence errors, and distinguishes immediate changes from those needing
Rescan/reopen. Inert repeat controls are hidden; legacy auto profiles are opt-in.

Windows uses pinned SDL3 3.4.18 through sdl2-compat 2.32.74 and retains native SDL2
profile indices. The v2 community catalog adds separate 2015/2026 Steam hardware
templates and sensor/extra-input support; the old catalog remains compatible with
0.1.4. Ten v2 templates include STO DualSense, Xbox and both Steam models.
Touchpads use directional zones. No hardware/gameplay acceptance is claimed.

App-update checks at startup and every six hours include public preview releases,
show a popup once per new version and retain a manual menu action. The popup opens
TROA's download page. Updates are user initiated, not silent installation. Existing
users need one manual upgrade before receiving these prompts.

Read-only calls through the actual Codex Bifrost MCP connector succeeded against
the running 0.1.4 app this session. It reports enabled access and a standard-layout
DualSense with the STO PS5-style profile, set 1, without unsaved changes. This
supersedes older empty-controller/pending-client notes below. No mapping was
changed by these diagnostic calls. The running app is under Program Files.

Windows compile/package succeeded at 29cddd14cea52fbafeecd6f5f516111a240d01ae
in Actions run 37428909955 (artifact 11396865666), with tests disabled.
Published v0.1.5-preview contains the installer, portable app, exact corresponding
source and checksums. Installer: 17,813,983 bytes, SHA256 4ea9164fc47ed9e14575be12700db7d2859af9f5adb00a72832e7f0235f1603b.
Static package inspection confirmed SDL2/SDL3, Qt TLS, dependency imports and
licenses; source archive contents match the build commit. Inherited AntiMicroX
Release workflow was disabled on this fork to avoid unwanted legacy distribution.
Website PR #103 merged at 79fcea2faf10e2c93cbd150b3bd60165f189b947. At 2026-10-06T07:36:43.501269+00:00,
the live page reported 0.1.5 and TROA's public download endpoint delivered HTTP 200
with the exact installer bytes/hash. This verifies distribution, not new native
UI/controller/gameplay acceptance. Edward's installed 0.1.4 app was not replaced.

Edward will upgrade manually; do not install/restart his app on his behalf.
No automated, visual or hardware tests are authorized. Native PR #1 stays draft;
public release distribution is authorized separately from merging that PR.

## Community profile updates - 2026-10-06

Owner requested game-specific defaults delivered at startup or through an update
button. After discussing console-only UI/automation, they approved the closest
PS5-style STO profile and confirmed default PC keyboard bindings.
Version 0.1.3 adds left/right triggers, a Space/Ground/Menus STO template, and
independent community catalog downloads at startup or Profiles > Update profiles.
Personal copies, active mappings and application-rule files are not replaced.
Validated catalogs are atomically cached; offline/error paths keep the last
download and compiled defaults. The fixed HTTPS catalog is profiles/catalog.json
on codex/troa-controller-mapper. The startup option can be disabled.
0.1.2 needs one installer upgrade to obtain this feature. Windows compilation
and packaging passed with WITH_TESTS=OFF in run 37418863441 from
5c9b689dc9669b29dd9b88b05dbcb3806700a726. Public release v0.1.3-preview contains
the checksum-verified installer, portable package and matching source. Installer
SHA256 is 0896eb8c8c92345af4f958f86f383710ff204d09c776bf5e21de5a23009a8fbc
(16,883,255 bytes). The owner was upgraded to 0.1.3 under Program Files;
startup downloaded community-catalog.json and existing settings were preserved.
A subsequent read-only library request returned no profiles: the two Qt resource
files had the same basename. Version 0.1.4 fixes this packaging collision with
a unique troa_resources.qrc and explicit initialization, and lets cache reads
continue when embedded defaults are unavailable. Run 37420565778 succeeded at
0413dc22e833a76c60982a98a0e998487df0a669 with WITH_TESTS=OFF. Published
v0.1.4-preview contains the installer, portable app, matching source and checksums.
Installer: 17,255,095 bytes, SHA256
595bdb3c40c7e9ffc1709a79a9fc509454c19d00ca2cc25ee5d100944b2847dd.
Owner's upgraded and reopened app reports 0.1.4; its read-only library lists
Desktop, Browser and the STO template with three layouts. Existing data stayed
in place. The temporary hidden 0.1.3 instance and stateless MCP companions were
stopped solely for file replacement. No controller or application rule is present;
no mapping was activated. Website PR #102 merged at 781d2fb; the public page
reports 0.1.4 and its TROA endpoint delivered the checksum-matched 17,255,095-byte
installer at 2026-10-06T06:07:04Z. The app is open on the STO profile page.
Do not describe 0.1.3 profile delivery as working.
Gameplay is untested. See docs/profiles/star-trek-online-dualsense.md
for keyboard assumptions, camera/ability differences and the Create-button rule.
Read-only metadata through the installed local companion confirmed assistant
access and an empty application-rule list; no controller was detected during
that read. Reconnect the DualSense before profile activation and Applications
rule setup. This is not a Codex-native tool connection or gameplay test.

## Product direction - 2026-10-05

- Current product name: Bifrost Controller (renamed from the temporary TROA PC Controller Mapper name).
- Audience: TROA's community.
- Initial platform: Windows. Preserve architectural room for Linux support later; Linux is not part of initial release acceptance.
- Requested features: a modern interface, MCP access for collaboratively creating and editing profiles, and curated desktop/browser/game/controller profiles delivered through updates.
- Distribution: branded Windows preview installer/portable packages and corresponding source in GitHub Releases, consumed by the published TROA product/download page. Current corrected package is 0.1.4.
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

- Current Bifrost Controller preview 0.1.4 from successful run 37420565778, built from 0413dc22e833a76c60982a98a0e998487df0a669. Read-only metadata lists the three community templates. No controller is connected and no rule/profile activation was performed.
- Executable: C:\Users\edwar\AppData\Local\Programs\Bifrost Controller\bin\bifrost-controller.exe
- Current running copy, inspected at the owner's request on 2026-10-06: C:\Program Files\Bifrost Controller\bin\bifrost-controller.exe. Its MCP companion is C:\Program Files\Bifrost Controller\bin\bifrost-controller-mcp.exe. The user-level installation path above is historical.
- Codex Desktop global stdio MCP server `bifrost-controller` is configured and enabled using the running copy's companion. The app's MCP setup screen shows local access enabled and one connected controller. Codex restart and a real MCP tool call remain pending; configuration is not evidence of a completed client connection.
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

- Owner explicitly authorized making the Windows installer available on the live
  TROA site. Published GitHub prerelease `v0.1.2-preview` at the exact built
  commit `c0a8d9837ebf743107ca52fa52cbbbe773855184`, containing installer,
  portable ZIP, corresponding source ZIP and SHA256SUMS.txt. All GitHub asset
  digests match local originals. No rebuild or additional tests were performed.
- The website follow-up pins its download to that release; live delivery must
  be verified independently. Earlier notes withholding publication describe
  their historical authorization state.
- The compiled preview enables CHECK_FOR_UPDATES. At startup it checks the
  latest stable GitHub release and exposes a download-page button for a newer
  version; it does not install updates. Prereleases are excluded from this
  stable check. Profile templates are bundled with the application, not synced
  remotely. Preview-channel updating and curated catalog delivery are future work.
- Personal repository is now edwardsong08/bifrost-controller; draft PR #1 remains open and attached. Ownership/collaborator access is unchanged.
- Bifrost Controller 0.1.2 compiled, packaged, installed, and opened as detailed above. Earlier pending-build notes describe their historical state.
- Edward authorized two new main-site pages and a final Gaming > Bifrost Downloads submenu for local review before pushing. Server Manager is a coming-soon overview; Controller is Windows-only with installer setup and GitHub access. This website work is separate and does not authorize a public release or site deployment.

