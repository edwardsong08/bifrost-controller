# Bifrost Controller

Bifrost Controller is an independently maintained derivative of AntiMicroX,
initially based on commit dbb6349603e6ee2426d300eee9570e1ed44fb9f5.

Upstream: https://github.com/AntiMicroX/antimicrox

Original copyright notices and the GNU General Public License are preserved.
Original contributors retain their copyrights. See individual source headers and LICENSE.

Changes beginning October 5, 2026 include TROA product identity, a refreshed Qt interface,
a structured profile library, local MCP access, and Windows preview packaging.
These changes are distributed under GPL version 3 or, where permitted by the source
notices, any later version. No endorsement by the AntiMicroX project is implied.

The Windows SDL controller database has its own license in share/LICENSE_SDL_GameControllerDB.
Qt, SDL, MinGW runtime libraries, and other bundled dependencies retain their respective licenses.

The TROA helm artwork is the deployed website favicon from
https://therealmsofasgard.com/favicon.ico, retrieved October 5, 2026.
The website serves PNG bytes; src/images/troa-logo.png preserves those bytes unchanged.
The native Windows ICO is packaged from this artwork by other/package-troa-logo.py.
The controller symbol remains a separate workspace illustration.

Distributions of modified binaries must include access to the corresponding source,
including the build and packaging configuration for that version.
