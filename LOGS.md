# Work log

## 2026-10-05 - Fork and Windows installation

- Created edwardsong08/antimicrox as a fork of AntiMicroX/antimicrox through GitHub's API using the existing authenticated account.
- Cloned master at dbb6349603e6ee2426d300eee9570e1ed44fb9f5 and configured origin and upstream remotes.
- Inspected README.md, BUILDING.md, CONTRIBUTING.md, CMakeLists.txt, Windows CI/release configuration, GUI test setup, and Windows input output code.
- Added AGENTS.md, CONTEXT.md, and LOGS.md as local repository context. No application source changes, commits, or pushes were made.
- Edward clarified that he wanted the installed app opened, not the source in VS Code.
- Installed official AntiMicroX 3.6.1 using its NSIS Windows installer into the user Programs folder. Installer exit code: 0; uninstall registry lists version 3.6.1.
- Verified installer SHA256: 467648540ab4bcf83641cb22b3359669bf1b9130b87d4be0aa438221ef3eb06b, matching the published GitHub release digest.
- Launched the installed executable. A second invocation reported that AntiMicroX was already running. Desktop automation timed out, so visual and controller behavior remain unverified.
- Next: inspect the actual interface with Edward and prioritize the candidate action list in CONTEXT.md.

## 2026-10-05 - Product identity and distribution direction

- Edward selected the working name TROA PC Controller Mapper and TROA's community as the audience.
- Initial target is Windows, with Linux possible later. A TROA site page and installer download are planned.
- Discussed modern UI, MCP profile management, and curated profiles delivered through updates. No framework or rewrite decision is final.
- Reviewed GitHub personal-repository access and release links while discussing personal ownership with TROA collaboration/distribution.
- Updated local repository context only. Repository name, application branding, GitHub permissions, website, and installed app were not changed.

## 2026-10-05 - Authorized TROA Windows implementation

- Edward approved implementation and expressly declined testing. No automated, visual, or controller tests are being run; only compile/package work is planned.
- Renamed the personal GitHub fork to edwardsong08/troa-pc-controller-mapper and updated origin. Existing collaborator access remains unchanged at Edward's request.
- Retained the C++17/Qt/SDL engine and added a modern workspace, light/dark appearance, branded identity/icon/settings, profile library, and assistant access page.
- Added a same-user local API and a native MCP stdio companion with controller/profile tools. No network listener or separate script runtime is needed.
- Added structured desktop/browser templates, personal copies, atomic saves, SHA256 revision conflict checks, preserved history, and compiled legacy mapping exports.
- Added Windows installer/portable preview packaging with corresponding source and checksums. Compilation is pending.
- Website product/download page and curated game-specific profiles remain future work. No public product release or site deployment occurred.

## 2026-10-05 - Windows preview compilation and packaging

- Pushed implementation commit d0e19159114dbcc4c6a257a27d1082da5b8b116d on codex/troa-controller-mapper and created draft PR #1.
- GitHub Actions run 37405802429 compiled the installer and portable application successfully with WITH_TESTS=OFF.
- That run failed in the final source/checksum step because Git was unavailable in the MSYS shell. No download artifact was uploaded from that failed run.
- Commit 49d81549239fcde1aa09e55c7a9fa305208983a3 moves source archiving/checksums to native Windows PowerShell and updates the next-action list. Rebuild run 37406545582 succeeded.
- Collaborator access remains unchanged. The personal fork does not grant TROA organization admins automatic access.
- Downloaded artifact 11387454096 to ignored distribution/preview. Installer, portable ZIP, and source ZIP hashes all matched SHA256SUMS.txt.
- Installer SHA256: 8570e4c1158dd9bbfd5a405fdddbc5337a110c97deebe02b357dfe42118dbcd4.
- Installed the fork preview into C:\Users\edwar\AppData\Local\Programs\TROA PC Controller Mapper; installer exit code 0 and uninstall registry version 0.1.0.
- Opened troa-pc-controller-mapper.exe; Windows reports its branded main window is responding. This confirms launch only, not functional acceptance.
- No automated, visual, or hardware tests ran. MCP client configuration and operations, profile activation, and controller behavior remain untested. The upstream application remains installed separately.
- No public GitHub Release, website downloader, or TROA site deployment was published. Preview packages are GitHub Actions artifacts and local files; the implementation remains in draft PR #1.

## 2026-10-05 - Clearer workspace, MCP setup, and deployed TROA branding

- Edward reported difficulty finding MCP and authorized further UI improvements for a clear, polished experience. He requested the deployed TROA logo and retained controller symbolism.
- Confirmed the running app is the branded 0.1.0 preview. A read-only MCP diagnostic returned enabled access and a standard-layout DualSense Wireless Controller with no unsaved changes; no mappings were activated or changed.
- Native desktop inspection timed out again. Changes use source inspection and compilation; no automated, visual, or hardware-input test runs are being performed.
- Reworked Get started, controller navigation, profile tables/selection feedback, light/dark appearance, and scrollable setup pages. Clearer existing mapping control names retain their original actions.
- Added header/sidebar/menu MCP setup entries, Ctrl+Shift+M, JSON/TOML configuration, path/settings/request copy actions, and honest availability/last-request indicators.
- Used the exact deployed TROA favicon artwork for app/tray/installer branding. The site favicon is PNG content; preserve its bytes in troa-logo.png and package a genuine Windows ICO. Controller art remains on controller navigation/guidance.
- Fixed title-bar close to use the existing save/discard/cancel flow instead of quitting directly.
- Preview version is 0.1.1. Build and installation are pending; source remains in draft PR #1 and repository access is unchanged.

## 2026-10-05 - Focused application profiles and named layouts

- Edward added per-application profile clarity, keyboard/controller switching between modes within a game, and visible notices on the focused game's monitor (with display options). He requested TROA main-site and Bifrost visual inspiration.
- Added a Windows application-context manager and modern rule editor, focused/open/closed rule state, actual profile/layout status, exact executable/controller matching, and native profile loading without discarding edits.
- Reused the native eight-set mapping engine for named modes. Keyboard shortcuts are focus-scoped; unused normalized controller buttons cycle on release. Existing assignments are not cleared or overwritten. Switching uses the device thread's existing set transition handling.
- Added topmost, non-activating, click-through switch notices, focused/primary/all monitor selection, and explicit exclusive-fullscreen limitations.
- Extended managed schema 1 compatibly with optional named layouts and added MCP context/rule/export tools. No game-specific bindings are fabricated.
- Read the actual deployed TROA CSS and Bifrost source styles: ivory/charcoal/gold, quiet navy surfaces, compact navigation. Other TROA repos remain untouched.
- 0.1.1 compilation/packaging succeeded in run 37408876833. Expanded preview 0.1.2 compilation/installation is pending; no automated, visual, or controller tests are run.
