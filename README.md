# Bifrost Controller

Bifrost Controller is TROA's Windows-first desktop controller mapping app, alongside
Bifrost Server Manager in the TROA gaming software ecosystem. It is based on AntiMicroX.

This initial preview keeps the native C++/Qt/SDL input engine and introduces a refreshed
workspace, a managed profile library, and a local MCP companion for creating profiles
with a compatible assistant. Linux remains a future product target.

## In this preview

- Controller mappings, calibration, profiles, and advanced assignments inherited from AntiMicroX.
- A new workspace with clear navigation, light/dark appearance, and live controller status.
- Desktop/browser starter templates and a console-inspired Star Trek Online DualSense template with Space, Ground and Menus layouts.
- Community templates download at startup or through **Profiles > Update profiles**, with a local cache for offline use.
- Personal profile copies with atomic saving, revision conflict checks, and preserved history.
- Local MCP tools to inspect controllers and create, read, validate, save, restore, activate, or unload profiles.
- Per-application profile rules, named layouts (such as Space/Ground), keyboard/controller switching, and monitor-aware switch notices.
- The deployed TROA logo, TROA ivory/charcoal/gold colors, and Bifrost-inspired navy panels.
- A separate app identity, settings, installer, and update destination so AntiMicroX can remain installed.

## Downloads

Windows installer and portable ZIP are built by **Bifrost Controller Windows preview** in GitHub Actions.
They are preview artifacts until a release is published. Corresponding source and SHA256
checksums are included. Published previews are linked from
[TROA's Controller page](https://therealmsofasgard.com/gaming-hub/bifrost-controller).

Install the branded Windows package and open **Bifrost Controller**. Use the sidebar
for Get started, Controllers, Profiles, Applications, and MCP & AI setup. The desktop/browser catalog
is a starting point to customize, rather than a claim of controller or game compatibility.

## MCP & AI setup

The package includes `bifrost-controller-mcp.exe`; no Python or Node runtime is required.
Choose **MCP setup** in the header, **MCP & AI setup** in the sidebar, or **MCP & AI > Open MCP setup**
in the menu (Ctrl+Shift+M). Enable local access and copy JSON or Codex TOML connection settings
into a compatible MCP client. The page distinguishes an available server from requests actually
received; enabling access does not automatically connect an AI app. See [MCP tools and profile workflow](docs/MCP.md).

MCP configuration remains separate from the input loop. Normal controller mapping does not
require an AI service or internet connection. Version 0.1.4 includes independent community
template downloads and fixes a bundled-resource issue in 0.1.3. Older apps need
upgrading once to gain working template delivery. Disable
startup downloads in Profiles if desired, or use **Update profiles** manually. Downloads
are validated before atomic caching, never activate mappings or replace personal copies,
and retain existing templates on failure. The preview catalog is maintained at
`profiles/catalog.json` on `codex/troa-controller-mapper`; future catalog revisions must
remain compatible or require an app upgrade. Personal copies are stored separately.
GUI edits to generated mappings remain legacy .amgp
files and are not automatically converted back to managed JSON definitions.

The initial TROA preview's settings and user-data folder names are intentionally retained
for upgrade compatibility. Rebranding preserves personal profiles, revisions, and settings.

## Development

The application uses C++17, Qt Widgets, SDL2, and CMake. The refreshed interface deliberately
reuses the existing mapping widgets and dialogs while the profile/API layer is kept separate.
Qt6 is the first preview build target; inherited Qt5 and Linux build paths remain in source.

See [build instructions](BUILDING.md), [repository context](CONTEXT.md), and
[upstream contribution guidance](CONTRIBUTING.md). The CMake application target remains
`antimicrox` internally but produces `bifrost-controller.exe`. The companion target
is `bifrost-controller-mcp`.

No automated, visual, or hardware tests are run for this preview, at the project owner's
request. Compilation and packaging results are recorded separately from runtime acceptance.
Left/right triggers are supported in managed profiles from 0.1.3. Gyro, timed macros,
and legacy import remain outside this MCP profile schema.

See [Star Trek Online setup and control differences](docs/profiles/star-trek-online-dualsense.md)
before using its starting template. This does not reproduce the console UI or ability
automation, and gameplay has not been tested.

## Attribution and license

This is an independently maintained derivative of [AntiMicroX](https://github.com/AntiMicroX/antimicrox),
initially based on commit `dbb6349603e6ee2426d300eee9570e1ed44fb9f5`.
Original copyright notices and GPL licensing are retained. See [NOTICE](NOTICE.md),
[LICENSE](LICENSE), and the [original README](UPSTREAM_README.md).

The repository remains owned by Edward's personal GitHub account. TROA may distribute
public releases; collaborator permissions are managed separately from website distribution.
