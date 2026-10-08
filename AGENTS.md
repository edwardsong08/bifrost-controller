# Repository instructions

- Read CONTEXT.md and the latest LOGS.md entry before working here.
- This is Edward's fork of AntiMicroX. Keep upstream attribution and the existing GPL license intact.
- Working product name: Bifrost Controller, for TROA's community. Target Windows first and keep platform boundaries suitable for possible Linux support later.
- Current phase: implement the authorized Bifrost Controller Windows preview with modern UI, local MCP profile management, and branded packages. Preserve the existing C++/Qt input engine while separating new profile and assistant responsibilities.
- Edward explicitly requested no test runs. Do not add or run automated, visual, or hardware tests; compile/package the app and report behavior as untested.
- Keep personal GitHub ownership and collaborator access unchanged. Organization admins do not automatically receive permissions on a personal repository.
- Follow CONTRIBUTING.md and .clang-format when code changes are authorized.
- Match the code-formatting workflow's clang-format 14.0.0; do not use a newer major formatter for this repository. Source-formatting checks are separate from application tests.
- Preserve existing controller profiles and keyboard/mouse mapping behavior. Check compatibility when changing serialization or device handling.
- Use a codex/ branch for implementation work. origin is Edward's fork; upstream is the official AntiMicroX repository.
- Distinguish the installed official release from any future locally compiled fork. Record actual build and runtime verification, including gaps.
- Keep CONTEXT.md current and append dated work and validation notes to LOGS.md.
- Bifrost is English-only for now. Do not reintroduce language selection or load stored language preferences.
- Edward will download and install the next update manually from TROA. Do not replace or restart his installed app without a new request.

## Official template policy

- Edward's requested mapping revisions are changes to Bifrost's official defaults unless he explicitly asks for a separate named template. Do not create a personal/community variant as a substitute for updating the defaults.
- Maintain one official template per game and controller family. Keep its existing builtin ID stable; revise its bindings/description in place so library updates replace the existing entry.
- Space, Ground, Menus and similar modes are layouts inside that template, not extra library templates.
- Carry shared action corrections across the supported controller variants, adapting physical controls appropriately. Keep genuinely controller-specific bindings distinct.
- Create a separate published community variant only when Edward explicitly requests a new template and a requester name. Include that name in its display name and document its deliberate differences from the official default.
- Preserve users' saved profiles and application-rule mapping files. Updating official library defaults does not authorize overwriting their private saved mappings or deleting existing personal copies.
- These are official Bifrost templates, not publisher-endorsed game controls or native gamepad emulation.
