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
