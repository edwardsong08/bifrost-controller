# Work log

## 2026-10-06 - Profile resource packaging correction

- Upgraded the owner's Program Files installation to 0.1.3 after normal app
  Quit, preserving a private user-data backup. Installer exited 0; read-only
  companion metadata reported 0.1.3 and startup saved community-catalog.json.
- A read-only library request returned an empty list. Both resource files used
  the basename resources.qrc; the generated resource identity collided.
- Version 0.1.4 renames the TROA resource file, explicitly initializes it and
  reads validated cached templates even if embedded defaults are unavailable.
  Windows build run 37420565778 at 0413dc22 succeeded with WITH_TESTS=OFF.
- Published v0.1.4-preview with verified installer/portable/source/checksums.
  Installer is 17,255,095 bytes, SHA256
  595bdb3c40c7e9ffc1709a79a9fc509454c19d00ca2cc25ee5d100944b2847dd.
  Marked the superseded 0.1.3 release's known issue.
- Stopped the temporary hidden app instance and stateless companions for file
  replacement, installed 0.1.4 (installer exit 0), and reopened the visible app.
  Read-only MCP metadata reports 0.1.4 and three community templates, including
  STO with three layouts. Existing data is preserved; no controller or rule is
  present, so no profile was activated. Codex-native tools still need restart.
- Website PR #102 merged at 781d2fb. Public page reports 0.1.4 and TROA download
  endpoint delivered the checksum-matched 17,255,095-byte installer at 06:07Z.
  User's app is open on the STO profile page. Reconnect DualSense before rule
  setup/activation; no game or Steam settings were changed.
  No gameplay, automated, visual or hardware tests ran.

## 2026-10-06 - STO template and independent community updates

- Owner requested default game profiles and startup/manual update delivery.
  They approved the closest PS5-style STO mapping after exact console parity
  was ruled out, and confirmed default PC keybindings.
- Added the DualSense Space/Ground/Menus community template, managed L2/R2
  trigger support and PS5 button labels. Create remains free for an application
  rule; no game files, Steam settings or active controller bindings were changed.
- Added Profiles > Update profiles and an optional startup download (enabled
  by default). Fixed HTTPS preview-branch catalog, bounded size/time, schema
  validation, atomic cache and offline fallback preserve personal profiles and
  active native mappings. New templates never activate themselves.
- Bumped preview package/companion version to 0.1.3. Windows compilation and
  installer/portable/source packaging passed in run 37418863441 at 5c9b689d
  with WITH_TESTS=OFF. Published v0.1.3-preview with all four assets; GitHub
  digests match the build manifest. Public installer readback also matches
  SHA256 0896eb8c8c92345af4f958f86f383710ff204d09c776bf5e21de5a23009a8fbc
  (16,883,255 bytes). Owner's installed 0.1.2 has not been upgraded locally.
- Read-only companion metadata reported 0.1.2, assistant access enabled, no
  connected controllers and no saved application rules. No mappings changed.
  This was a metadata read, not a Codex-native integration acceptance test.
- Formatting/diff review performed. No automated, visual or
  hardware tests ran. Game-control assumptions and setup differences are
  documented in docs/profiles/star-trek-online-dualsense.md.

## 2026-10-06 - Codex Desktop MCP registration

- Owner explicitly requested desktop inspection of the open app and connection
  of its MCP integration with Codex Desktop, followed by their own restart.
- Computer Use captured the running preview's MCP setup screen: local access
  was already enabled, one controller was connected, and no MCP request had
  been received in this app session. Current installation is under Program Files.
- Registered `bifrost-controller` globally via `codex mcp add`, pointing to
  `C:\Program Files\Bifrost Controller\bin\bifrost-controller-mcp.exe`.
  `codex mcp get bifrost-controller` confirms enabled stdio configuration.
  Preserved pre-registration config in the user's .codex folder, outside Git.
- No mappings changed. No automated or hardware tests ran. A Codex restart
  and an actual MCP request are pending; do not claim the app is connected yet.

## 2026-10-06 - Public Windows preview release

- Owner explicitly authorized installer delivery from the live TROA site.
  Published prerelease v0.1.2-preview at exact built commit
  c0a8d9837ebf743107ca52fa52cbbbe773855184. Included installer, portable ZIP,
  corresponding GPL source ZIP and SHA256SUMS.txt; all uploaded asset digests
  match local originals. Publication did not rebuild or test the application.
- Public release URL: https://github.com/edwardsong08/bifrost-controller/releases/tag/v0.1.2-preview.
  Installer is 16,876,924 bytes; SHA256 remains
  d38fd60c9e68fc820404b0e94cae4333e73b7027577d5b23ab810fce2b9d9b75.
  Authenticode status is NotSigned. Signing remains future release work.
- Confirmed the exact build workflow enables CHECK_FOR_UPDATES. The inherited
  startup checker displays a download button for a newer stable version and
  opens its GitHub release page; it does not install updates. Prereleases are
  excluded from that stable endpoint. Bundled templates are not remotely synced.
- Website publication follow-up is tracked separately; delivery is verified
  through the deployed page/download response, not inferred from this release.

## 2026-10-06 - Bifrost Controller preview installed and opened

- Renamed the personal fork to edwardsong08/bifrost-controller and updated origin, product URLs, and draft PR #1; permissions remain unchanged.
- Final runtime source c0a8d9837ebf743107ca52fa52cbbbe773855184 adds scrollable application-rule editing on smaller screens. Windows compilation/installer/portable/source packaging succeeded in Actions run 37411297952 with WITH_TESTS=OFF, artifact 11390075068.
- Downloaded to ignored distribution/bifrost-preview-0.1.2. Installer SHA256 d38fd60c9e68fc820404b0e94cae4333e73b7027577d5b23ab810fce2b9d9b75; portable faa6edce0b2c2c6607f18e9eecf323317ced79283b84725b16e8f29219f46919; source 875aa034612015942eb0758cdfbaa2981442a6458a042049ab31a6f3fd4a5edb. All matched SHA256SUMS.txt.
- Read-only old-preview metadata confirmed no unsaved controller edits; closed it gracefully and backed up its user data. Installed Bifrost Controller (exit 0), uninstalled obsolete TROA preview (exit 0), and confirmed shared settings preserved exactly. Official AntiMicroX remains separately installed.
- Windows registry lists Bifrost Controller 0.1.2 and the MCP companion is present. Used the supported --show command; process 48124 reports the Bifrost Controller main window and Responding=true. This confirms launch only.
- No automated, visual, or hardware tests ran. Game-specific mappings, switching behavior, overlay behavior, and real AI-client connection remain untested/unconfigured. Exclusive fullscreen can obscure desktop notices.
- No public GitHub Release or website deployment. Edward subsequently authorized main-site pages/menu and a local installer download for review before any website push; that work is tracked in the main-site repository.

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
- Source review found that native combo selection queues profile loading. Added an explicit synchronous GUI-thread load for application/API requests, preserving the inherited interactive flow and reporting actual reader success before selecting a layout. Rule display now uses native profile names rather than content hashes, with readable normalized DualSense switch-button names.
- Application detection uses bounded Win32 executable queries with limited process-query permissions, and new persisted settings use the existing settings mutex. No privilege elevation or application-input hook is added.

## 2026-10-05 - Bifrost Controller product identity

- Edward selected Bifrost Controller as the final working name, alongside Bifrost Server Manager in TROA's gaming software ecosystem.
- Updated visible app/installer identity, executable/MCP names, repository/update URLs, README, and current context. Historical records retain the earlier temporary name.
- Kept preview settings/data locations and TROA setting keys compatible so existing personal profiles and revisions remain intact. TROA artwork and the site/Bifrost palette remain.
- Renamed personal fork / new Windows package compilation and installation are pending. Repository ownership/access and upstream attribution remain unchanged; no Server Manager integration or site release is claimed.

## 2026-10-06 - Source audit, task-based workspace and 0.1.5 preparation

- Owner requested a fresh UI assessment, removal of unwanted/broken controls,
  Settings Apply, both Steam Controller hardware models, Xbox/STO profiles, and
  app-update popups. Latest steering makes the product English-only.
- Implemented the workspace/state/menu/settings/save/activation corrections
  recorded in docs/UI-AUDIT.md, including atomic native saves and false native
  profile-load confirmation. Kept useful advanced mapping features accessible.
- Added capability-aware inputs, raw extras, Steam touchpad directional controls,
  managed sensor bindings and ten compatible v2 templates. Pinned official SDL3
  and SDL2 compatibility sources in the Windows workflow; no dependency tests run.
- Startup/six-hour release checks include previews, deduplicate automatic popups,
  and retain manual update checking. Installer execution remains manual.
- Actual read-only Codex MCP tools reached installed 0.1.4 and returned the
  DualSense/STO set-1 state. No diagnostic activation or user mapping edits occurred.
- Source formatting and Windows compile/package are next. No automated, visual,
  hardware or game tests were added or run. App remains untouched; owner installs
  the forthcoming public installer manually from TROA.

- Follow-up source review covered device information, calibration and controller
  layout normalization. Filter unavailable logical slots, avoid calibrating virtual
  touchpads as sticks, and preserve extra bindings when editing standard layouts.
  Fixed missing normalization-table indices that otherwise fell back to row zero.
- Controller information's reject override previously deleted the dialog without
  emitting finished, leaving mapping output ignored. Restore normal dialog reject
  completion, handle both normal result codes, avoid destroyed-device access, and
  explain the temporary pause in the dialog. Its legacy blue-only style is removed.

## 2026-10-06 - 0.1.5 public Windows release and delivery

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

- Native PR #1 remains a draft. No automated, visual, hardware or gameplay tests
  were run. Owner manually upgrades from TROA, normally without uninstalling.

## 2026-10-06 - Diagnose missing 0.1.5 templates

- Screenshot plus actual read-only MCP status/list show 0.1.5 and only two generic
  templates. STO read rejects controller_family as an unknown field. This is an
  app validator regression, independent of whether a controller is connected.
- Accept the already validated controller_family field in the final allowlist.
  Improve catalog feedback with exact ID and validation error; retain all-or-
  nothing persistence and hardware-model compatibility checks.
- Bump to 0.1.6 for a corrected installer. Build/release/site delivery are pending.
  No tests or user mapping changes; current installation remains untouched.
- Keep the older catalog.json endpoint compatible with 0.1.4 by omitting the
  new optional family metadata there. The v2 catalog retains exact model checks.

## 2026-10-06 - Corrected 0.1.6 installer published and live

0.1.6 compile/package succeeded from ec7955aa5d701172b01838c0f431fea0fcfc2e59
in Windows Actions 37432156127 (artifact 11398151009), tests disabled.
Published v0.1.6-preview includes installer, portable app, exact source and hashes.
Installer: 17,813,326 bytes, SHA256 6ce203eaa13dba26947e695df721874c496dc54782cd5007186a02c07623b1ba.
Source archive matches the build commit; static runtime/import/license inspection
passed. Website PR #104 merged at 05356458aeb57752cd3e7d299f8d252bb30d8220. At 2026-10-06T08:05:09.792565+00:00,
public page and TROA download readback confirmed 0.1.6 and the exact installer bytes.
This verifies distribution. Owner manually installs; the running 0.1.5 app has not
been replaced or restarted. No automated, visual, hardware or gameplay tests ran.

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
