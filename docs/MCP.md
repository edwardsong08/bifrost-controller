# Local MCP access

The Windows package includes two executables:

- bifrost-controller.exe: the interface and existing native mapping engine.
- bifrost-controller-mcp.exe: a local MCP stdio companion; no Python or Node installation is needed.

Installed Windows packages from 0.1.7 also have two small, stable launchers at the
installation root: `bifrost-launcher.exe` and `bifrost-mcp-host.exe`. The setup page
copies the MCP host command. It selects the current versioned companion while
passing stdin/stdout directly to it. Portable packages keep their existing command.
These format-1 launchers have no Qt or MinGW DLL dependencies and are installed
once. Future changes must use a new launcher filename/protocol rather than
overwriting a connected host.

Upgrades install into `versions/<version>/bin` with a matching `share` directory,
then atomically select the completed release in `current.txt`. Codex and other MCP
clients can remain open: a connected companion uses its existing payload until
the client reconnects, and its requests still reach the running mapper. Existing
pre-0.1.7 MCP paths are kept working by leaving their old `bin` runtime untouched.
When convenient, copy the new host command from Assistant setup and reconnect the
MCP client once; this is not required to install an update.

The finish-page launch offers to restart an older mapper. Save/Discard/Cancel
remains in effect; no process is killed. 0.1.7 and later handle this request even
with Close to tray enabled. A legacy mapper set to Close to tray may need
**Bifrost > Quit** once, then the updated Start menu shortcut. Setup never launches
the mapper elevated just because setup required administrator permission. If
normal-user launch fails, open it manually from the shortcut. Personal data keeps
its existing location. Old payloads are retained; automatic disk cleanup is deferred.
Same-version reinstalls skip existing files; this is an upgrade mechanism, not an
in-place repair of a corrupt, running payload.

Keep the mapper open. Choose **Assistant · MCP** in the header/sidebar,
or **Assistant · MCP > Open MCP setup** in the menu (Ctrl+Shift+M). Enable **local MCP access** and copy
JSON or Codex TOML settings into a compatible MCP client's configuration. The command is the absolute path
shown in the setup page, with no arguments (`bifrost-mcp-host.exe` for installed
Windows releases from 0.1.7, `bifrost-controller-mcp.exe` for portable/Linux copies).
The copied JSON uses the
common `mcpServers` format; clients with another configuration format should use the
same command and empty argument list.

MCP speaks JSON-RPC over stdin/stdout and negotiates protocol version 2025-11-25.
The companion communicates with the running mapper over a Qt local socket/named pipe,
restricted to the same Windows or Linux user. It exposes no HTTP service. Disabling
local MCP access closes the listener and its existing connections. The setup page shows
whether the companion is installed, local access is enabled, and when the running app last
received an MCP request. Availability is not proof that a particular AI app is connected.

## Tools

| Tool | Purpose |
| --- | --- |
| mapper_status | Version, profile directory, supported inputs, and supported keyboard keys |
| list_controllers | Live controller ids, capabilities, active profile names, and unsaved changes |
| list_profiles | Community templates and personal profiles |
| read_profile | Structured definition and current SHA256 revision |
| validate_profile | Validate a definition without changing anything |
| save_profile | Create a draft or save changes with an expected revision |
| list_profile_revisions | Find preserved older personal revisions |
| restore_profile_revision | Restore an older definition without activating it |
| activate_profile | Apply a specific revision to one connected controller |
| unload_profile | Unload one controller's mapping |
| export_profile | Compile an exact managed revision into a native .amgp without activating it |
| application_context | Focused app, assigned/actual profile and named layout for each controller |
| list_application_rules | Per-controller application rules and their revision |
| save_application_rule | Save an executable/profile/layout/shortcut rule with a revision check |
| remove_application_rule | Remove a rule without deleting its profile |

## Profile workflow

1. List controllers and read the intended profile.
2. To customize a bundled template, change its id to a personal lowercase slug and give it a name.
3. Preserve fields from the read result and edit its bindings. Validate the draft.
4. Save it. For updates, supply `expected_revision` from the last read; omit it when creating a new id.
5. Read the saved definition, then activate it with its exact revision and the live `controller_id`.

Saving and restoring definitions do not alter active controller mappings. Activation
refuses to discard unsaved GUI edits. A reconnect changes the live controller id;
list controllers again instead of reusing stale ids.

Definitions use schema version 1, categories desktop/browser/game/custom, and
controller `sdl-gamecontroller`. `mapper_status` lists supported inputs and named keys.
Each binding has exactly one action: `keys` (a simultaneous chord of up to four keys),
`mouse_button` (1 left, 2 middle, 3 right, 4-7 wheel, 8-9 side buttons), or `mouse_move`
(up/right/down/left). Binding labels are optional. Dead zone is 1000-30000, default 8000.

```json
{
  "schema_version": 1,
  "id": "my-desktop",
  "name": "My Desktop",
  "description": "Personal desktop bindings",
  "category": "desktop",
  "controller": "sdl-gamecontroller",
  "dead_zone": 8000,
  "bindings": [
    {"input": "a", "label": "Confirm", "keys": ["Enter"]},
    {"input": "x", "label": "Address bar", "keys": ["Ctrl", "L"]},
    {"input": "right_stick_up", "mouse_move": "up"}
  ]
}
```

The profile store uses atomic writes, SHA256 conflict checks, and preserved older
revisions. Generated legacy .amgp mappings have content-addressed filenames so an
updated definition reloads through the existing mapping engine.

Community templates are embedded for offline use. Starting with 0.1.3, compatible
catalog updates also download at startup or through **Profile library > Update profiles**.
Desktop and browser templates use the left stick for pointer movement and the right
stick for vertical/horizontal scrolling. The original Steam Controller uses its
single left stick for the pointer and right touchpad for scrolling. After a catalog
refresh, choose **Use this profile** to apply the revised template; existing saved
and active mappings are preserved until you deliberately apply it.
Downloads never activate profiles, replace personal copies or rewrite existing
application-rule mapping files. The v2 catalog includes twenty-two templates with separate STO Space/Ground/Menus
starting layouts for DualSense, Xbox, and Steam Controller 2015/2026 hardware. See
[its setup notes](profiles/star-trek-online-dualsense.md); gameplay is untested.
Minecraft Java, Palworld and Space Engineers 1 also have controller variants;
see [game template setup](profiles/game-library.md). The library groups variants
under their game/application. Controller compatibility filtering checks the device's
available inputs, not just its label. Favorites and personal collections are local
preferences; profile files and revision hashes stay unchanged by organization.
Files & backup exports definitions, personal-library/collection backups or native
`.amgp` mappings. Import validates the entire document before saving, skips existing
IDs and never activates mappings. Backups cover definitions and collections, not
native editor files, application rules or hardware calibration.

For an application with native controller support, `save_application_rule` accepts
`native_controls: true`. Supply the usual id/name/executable/persistent controller
id and current rules revision. Profile path, mapping modes and switching shortcuts
are cleared in this mode. Foreground activation pauses all Bifrost mapped output
and releases held actions; leaving the app resumes the preserved mapping. This
does not hide the physical controller or emulate an XInput/virtual controller.
The GUI exposes the same option in App rules. `application_context` and
`list_controllers` report `mapping_suspended`.

## Current boundaries

From 0.1.5, MCP definitions cover standard buttons, D-pad, sticks, triggers,
`left_touchpad_*` / `right_touchpad_*` directions and clicks, exposed
`raw_button_0` through `raw_button_63`, and gyro/accelerometer directions.
`list_controllers.available_inputs` is authoritative for the connected driver;
activation refuses required inputs that are unavailable. `activate_profile.set`
selects a named layout; it defaults to 1. Timed macros and legacy profile import
remain native-editor features. Touchpads use directional mapping zones. MCP does not expose script execution,
arbitrary file access, or direct keyboard/mouse injection. GUI edits to a generated
mapping should be saved as a legacy .amgp file; they do not automatically rewrite its
managed JSON definition. Website-specific browser switching is not implemented.

No automated or hardware tests were run for this preview, per Edward's request.

## Applications and named layouts

Use **App rules** in the mapper to assign a saved native profile to an application executable
and controller. The rule shows Open/Focused/Closed, and the live controller table distinguishes
the assigned profile from the mapping and layout actually active. Applications without a rule
keep the current mapping; the status says so. Modern rules take precedence over inherited automatic
rules for their matching controller/application. Unsaved controller changes pause automatic loading.

Layouts correspond to the existing eight mapping sets. For Star Trek Online, configure the actual
game .exe (not just its launcher), choose its saved profile, and name sets 1 and 2 **Space** and
**Ground**. Assign actions under Map controls and save. Choose a keyboard shortcut, a controller
button, or both to cycle the selected layouts. These labels do not invent game bindings. The
controller button must be unused in every selected layout; existing actions are never erased.
The notice appears for the actual profile/layout change, without taking focus or intercepting clicks.
Choose the focused app's display (default), primary display, or all displays. Exclusive fullscreen
games may hide desktop overlays; borderless/windowed mode is required in that case.

MCP can also author named layouts. Schema 1 now accepts optional `layouts`, with empty top-level
`bindings`, containing 1–8 objects `{ "set": 1, "name": "Space", "bindings": [...] }`.
Use unique set numbers 1–8 and include set 1. Each layout's bindings use the same supported input
and action schema as single-layout profiles. Existing schema 1 profiles continue unchanged.

After saving a managed profile, call `export_profile` with its id and exact revision. Use the
returned `profile_path` in `save_application_rule`. This does not activate the profile by itself.
`list_application_rules` supplies the expected revision for rule changes; `application_context`
supplies persistent controller identifiers and normalized SDL button indices (unlike the live
instance ids used by `activate_profile`). A rule has:

```json
{
  "id": "sto-dualsense",
  "name": "Star Trek Online",
  "executable": "C:/path/to/the/actual/GameClient.exe",
  "controller_id": "persistent-id-from-application_context",
  "profile_path": "native-path-from-export_profile",
  "modes": [{"set": 1, "name": "Space"}, {"set": 2, "name": "Ground"}],
  "keyboard_shortcut": "Ctrl+Alt+G",
  "controller_button": -1
}
```

Paths above are examples; use actual existing local files. Saving a rule enables it when its app
is focused, including immediately if that app is already focused. Keyboard chords use A–Z, 0–9
or F1–F11, optionally with Ctrl/Alt/Shift; conflicting registrations show a warning. Shortcuts
are registered only for the focused application and are removed on focus changes. Application
focus is polled every 500 ms. Layout selection is remembered during the running session.
Application rules and global keyboard shortcuts currently target Windows; native mapping remains
cross-platform. Website/tab-specific browser matching is still future work.
