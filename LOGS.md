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
