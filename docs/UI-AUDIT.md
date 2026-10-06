# Bifrost Controller interface audit — 0.1.5

Edward authorized an end-to-end source audit, a fresh task-based interface design,
English-only operation, and Windows packaging. Automated, visual, and hardware
tests remain explicitly excluded. This document records inspected source and
implemented fixes; it does not certify controller/gameplay acceptance.

## Product decisions

- Open on an Overview showing actual connected devices, profile/layout state,
  unsaved work, and the last/focused application. Setup guidance is available on
  demand instead of occupying the opening screen.
- Use five destinations: Overview, Map controls, Profile library, App rules,
  Assistant · MCP. Settings and light/dark appearance remain discoverable.
- Preserve the TROA logo, controller symbolism, ivory/charcoal/gold palette and
  Bifrost surfaces. Use one palette across the workspace, menus and dialogs.
- Ship English only. Remove the language settings page and retire stored language
  choices on startup; do not package unused translation catalogs.
- Retain working advanced native assignment tools. Hide inert key-repeat controls
  and keep old automatic-profile settings behind a compatibility option.

## Reviewed surfaces and resulting changes

| Surface | Source review / resulting behavior |
| --- | --- |
| Bifrost menu | Settings, app updates, Hide and Quit; no conflicting text-entry shortcuts. Hide depends on tray availability. |
| Controllers menu | Rescan, device information, calibration and key checker. Device-specific tools disable without a controller. |
| Assistant menu | MCP setup and copyable configuration; local availability is distinguished from received client requests. |
| Help menu | Bifrost quick start, fork source/issues, attributed upstream mapping reference and license/framework information. |
| Overview | Actual profile/layout state, focused-app context and direct links to the three everyday tasks. |
| Map controls | Prominent Open, Save and Save a copy; recent-list removal in More; layout naming/copying retained. Less frequent mapping options move to Mapping tools. |
| Input editors | Existing keyboard/mouse, chords, advanced timing/macro, stick, trigger, D-pad, sensor and calibration editors retained. Unsupported SDL controls are filtered from the main mapping surface. These dialogs are not a complete new editor implementation. |
| Device dialogs | Information counts/display exclude unavailable logical slots; normal Close/Escape/title-bar paths restore paused mapping output. Calibration excludes virtual touchpads and absent sticks. Controller-layout normalization ignores unknown button rows and preserves extra standardized bindings instead of overwriting the first row or discarding extras. |
| Profile library | Wrapping descriptions, responsive stacked/detail layout, visible controller selection, explicit selected layout application and preflight capability feedback. Copy to my library is distinguished from native Save a copy. |
| Apply profile | Fix the native reader's missing filename state, which could produce a false load failure. Request and report the selected mapping set. Preserve current mappings when required inputs are unavailable. |
| Native profile saving | Atomic QSaveFile commit and error reporting; cancellation/failure cannot count as a successful Save. Managed exports are read-only cache files; save a personal copy before editing. |
| Profile switching/removal | Preserve unsaved-change prompts, including failed Save and Save-As list reordering. Removing a recent entry keeps its file. |
| General settings | Apply saves without closing; OK closes only after success. Invalid folders and OS/settings write errors receive inline feedback. Reset cancellation no longer continues into another reset prompt. |
| Device layouts | Persisted changes explicitly require closing Settings and Rescan; do not imply existing SDL handles changed immediately. |
| Windows integration | Own startup shortcut and file-association registration. Add Open with support without taking another app's default association. |
| Pointer settings | Existing smoothing/polling/spring settings remain. The Windows pointer-precision checkbox affects the Windows setting. |
| Diagnostics | Log level applies immediately; log destination changes require reopening. |
| App rules | Show assigned versus active mapping, blocked switches and shortcut conflicts; disable notice-screen selection when notices are off. Report settings write failures instead of claiming saved rules. |
| Layout notices | Focused screen by default, primary/all options. Non-activating overlays; exclusive fullscreen visibility is not guaranteed. |
| App updates | Startup and six-hour checks include published preview releases. Popup once per new version, manual check always available, download-page action retained. Installation remains user initiated. |
| Community updates | Independent compatible catalog, validated atomic cache, offline fallback. Never replace personal definitions or activate a download automatically. |
| Local MCP | Same-user pipe/stdio boundary; expanded capability metadata, touchpad/raw-extra/sensor inputs and selected-layout activation. No arbitrary scripts or direct injection tool. |

## Controller and game boundaries

The Windows package uses pinned official SDL3 3.4.18 through SDL2 compatibility
2.32.74, preserving the existing C++/Qt engine and standardized native indices.
Xbox and PlayStation names follow exposed capabilities. Extra grips, touch inputs,
touchpad directions and sensors are exposed only when the driver provides them.
Both Steam Controller hardware generations have distinct templates and compatibility
checks. Touchpads currently use directional mapping zones, not a new relative
trackpad/gesture engine. Steam Input can also emit keyboard/mouse actions; avoid
having it duplicate the same actions while Bifrost maps the physical controller.

Ten bundled templates include desktop/browser starting points and STO
Space/Ground/Menus profiles for DualSense, Xbox and both Steam Controller models.
These are default-PC-keyboard, console-inspired starting points. Console radial
menus and conditional ability automation are not implemented. Hardware/gameplay
behavior, ergonomics and the new UI composition remain untested at the owner's
request. No controller model should be advertised as certified.

Managed JSON definitions and native mapping files have distinct responsibilities:
the library/assistant edits definitions; Map controls edits native assignments.
Native edits do not round-trip into library JSON. Timed macros and legacy imports
remain native-editor features. Real installer/update behavior must be recorded
separately from successful compilation and source publication.

## Release evidence

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

## 0.1.6 template regression correction

The owner reported only Desktop/Browser in 0.1.5. Actual MCP read returned
`Space: Unknown profile field: controller_family`. The final allowlist omitted
a field checked earlier in validation. Accept that field, retain value/model
checks, and show the exact ID/validation reason when a catalog is rejected.
The legacy v1 catalog omits newer metadata for older client compatibility.

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

## 2026-10-06 - 0.1.7 compiled, published and delivered

Windows Actions 37437099049 compiled/packaged 95f19be285a141d749d6cff14272938ee804b620
successfully with tests disabled. Run duration: 11m53s (dependencies about 3m,
installer 4m26s, portable rebuild/package 3m44s). Earlier header-order and CPack
serialization failures required rebuilds; those attempts were not published.
Preview artifact 11400066962; installer layout artifact 11400296218.
Published v0.1.7-preview contains installer, portable app, exact source and hashes.
Installer: 18,386,680 bytes; SHA256 d4b5275d9d97d37601a725495585707d93440cbfc8bed7a0f353332049bc3ed5.
Exact source bytes and portable runtime imports/licenses inspected; no unresolved
dependencies. Generated NSIS layout confirms immutable versioned payloads,
atomic marker, stable launchers and no pre-upgrade uninstall. Both launchers import
only Windows system DLLs. These static checks do not prove installer execution.
Website PR #105 merged at bb84b98aa195d3e90e20a3acbbe749a651243298. At 2026-10-06T08:53:14.065508+00:00,
live product page and TROA download endpoint returned the exact 0.1.7 installer.
Owner requested simple public setup copy without one-user migration details.
Owner's installed app, personal profiles and Codex configuration remain untouched;
no automated, visual, installer execution, hardware or gameplay tests ran.
Native PR #1 remains a draft and unmerged. Manual upgrade normally requires no
uninstall; Codex may stay open. Legacy Close-to-tray may need Bifrost > Quit once.
Old payloads remain retained; stable launchers must never be overwritten in place.

## 2026-10-06 - 0.1.8 template and native-map consistency

Owner requested DualSense STO Ground Circle -> F/Interact and no crouch binding.
Update both downloadable catalogs and the bundled v2 template; Ground R3 becomes
unassigned. Keep Space and Menus controls unchanged. Actual read-only MCP showed
the owner's 0.1.7 DualSense using STO Space with no unsaved changes or App rules.
Source/native XML inspection found a compiler bug: stickbutton indices used D-pad
bit masks 1/2/4/8, but native sticks require compass ordinals 1/3/5/7. This affected
all managed stick/touchpad templates and explains diagonal/wrong/absent assignments.
Use engine enum constants for stick cardinal directions; leave D-pad/sensor masks
unchanged. Compiler-output hashes create new immutable exports on reapplication.
Map controls gains an active profile/layout banner and actual cardinal stick slot
summaries, refreshed after loads, layout changes and edits. Explicit activation
selects the editor's corresponding page immediately. Library feedback names Map
controls and distinguishes template preview from applied layout. Existing personal
copies/saved/App rule mapping files are not silently rewritten. Owner must reapply
the updated template and save/repoint rules as needed. Compile/release pending;
no automated, visual, installer execution or hardware/gameplay tests authorized.
