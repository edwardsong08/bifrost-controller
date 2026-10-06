# Local MCP access

The Windows package includes two executables:

- troa-pc-controller-mapper.exe: the interface and existing native mapping engine.
- troa-controller-mcp.exe: a local MCP stdio companion; no Python or Node installation is needed.

Keep the mapper open and enable **Assistant access** in its sidebar. Copy the connection
settings into a compatible MCP client's configuration. The command is the absolute path
to the installed troa-controller-mcp.exe, with no arguments. The copied JSON uses the
common `mcpServers` format; clients with another configuration format should use the
same command and empty argument list.

MCP speaks JSON-RPC over stdin/stdout and negotiates protocol version 2025-11-25.
The companion communicates with the running mapper over a Qt local socket/named pipe,
restricted to the same Windows or Linux user. It exposes no HTTP service. Disabling
Assistant access closes the listener and its existing connections.

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

Community templates are embedded in the application and updated with app releases.
Personal profiles are stored separately and are not overwritten by template updates.
The first catalog contains desktop and browser starters. Game profiles will be authored
with Edward; no game-specific bindings or hardware acceptance are claimed yet.

## Current boundaries

MCP profile definitions cover standard buttons, D-pad, and both sticks. Triggers,
gyro, complex timed macros, auto-profile rules, and legacy profile import are still
managed through the inherited interface. MCP does not expose script execution,
arbitrary file access, or direct keyboard/mouse injection. GUI edits to a generated
mapping should be saved as a legacy .amgp file; they do not automatically rewrite its
managed JSON definition. Website-specific browser switching is not implemented.

No automated or hardware tests were run for this preview, per Edward's request.
