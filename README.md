# TROA PC Controller Mapper

A Windows-first controller mapping app for TROA's community, based on AntiMicroX.

This initial preview keeps the native C++/Qt/SDL input engine and introduces a refreshed
workspace, a managed profile library, and a local MCP companion for creating profiles
with a compatible assistant. Linux remains a future product target.

## In this preview

- Controller mappings, calibration, profiles, and advanced assignments inherited from AntiMicroX.
- A new workspace with clear navigation, light/dark appearance, and live controller status.
- Desktop and browser starter templates using SDL's standard controller layout.
- Personal profile copies with atomic saving, revision conflict checks, and preserved history.
- Local MCP tools to inspect controllers and create, read, validate, save, restore, activate, or unload profiles.
- A separate app identity, settings, installer, and update destination so AntiMicroX can remain installed.

## Downloads

Windows installer and portable ZIP are built by **TROA Windows preview** in GitHub Actions.
They are preview artifacts until a release is published. Corresponding source and SHA256
checksums are included. A TROA website product/download page is planned; no site deployment
is claimed by this repository.

Install the branded Windows package and open **TROA PC Controller Mapper**. Use the sidebar
for Controller mappings, Profile library, and Assistant access. The desktop/browser catalog
is a starting point to customize, rather than a claim of controller or game compatibility.

## Assistant access

The package includes `troa-controller-mcp.exe`; no Python or Node runtime is required.
Keep the app open, enable Assistant access, and copy its connection settings into a compatible
MCP client. See [MCP tools and profile workflow](docs/MCP.md).

MCP configuration remains separate from the input loop. Normal controller mapping does not
require an AI service or internet connection. Bundled templates update with app releases;
personal copies are stored separately. GUI edits to generated mappings remain legacy .amgp
files and are not automatically converted back to managed JSON definitions.

## Development

The application uses C++17, Qt Widgets, SDL2, and CMake. The refreshed interface deliberately
reuses the existing mapping widgets and dialogs while the profile/API layer is kept separate.
Qt6 is the first preview build target; inherited Qt5 and Linux build paths remain in source.

See [build instructions](BUILDING.md), [repository context](CONTEXT.md), and
[upstream contribution guidance](CONTRIBUTING.md). The CMake application target remains
`antimicrox` internally but produces `troa-pc-controller-mapper.exe`. The companion target
is `troa-controller-mcp`.

No automated, visual, or hardware tests are run for this preview, at the project owner's
request. Compilation and packaging results are recorded separately from runtime acceptance.
Triggers, gyro, timed macros, auto-profile configuration, and legacy import remain outside
this first MCP profile schema and can be configured through the inherited interface.

## Attribution and license

This is an independently maintained derivative of [AntiMicroX](https://github.com/AntiMicroX/antimicrox),
initially based on commit `dbb6349603e6ee2426d300eee9570e1ed44fb9f5`.
Original copyright notices and GPL licensing are retained. See [NOTICE](NOTICE.md),
[LICENSE](LICENSE), and the [original README](UPSTREAM_README.md).

The repository remains owned by Edward's personal GitHub account. TROA may distribute
public releases; collaborator permissions are managed separately from website distribution.
