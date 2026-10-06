# Game library setup

These are console-inspired **PC keyboard/mouse starting templates**. No hardware
or gameplay tests have been performed. They do not reproduce console radial menus,
context-sensitive button logic, controller prompts or analog movement. Keyboard
movement uses digital directions. The right stick provides mouse look in gameplay
and a pointer in menus; the original Steam Controller uses its right touchpad.

Each game has DualSense, Xbox and physical Steam Controller 2015/2026 variants.
Use **Profile library > Update profiles**, select the game and controller variant,
review the layout, and choose **Use this profile**. The library preview is separate
from the current mapping. Copy to My profiles for an independently maintained
definition, or export/save a native `.amgp` mapping for native editor customization.

## Minecraft Java

Java has Gameplay and Inventory / Menus layouts. Left stick sends WASD; right
stick sends mouse movement. Face-button positions: bottom jumps, right sneaks,
left opens inventory, top drops an item. Triggers attack/mine and use/place.
Shoulders scroll the hotbar; left-stick press sprints; right-stick press changes
perspective. Inventory remains Java's mouse interface. D-pad can swap offhand,
open chat and choose hotbar slots 1/9. Customized controls and modpacks may need
changes. Minecraft Java runs through a Java executable, so a rule for shared
`javaw.exe` also matches other applications using that exact Java runtime; prefer
a dedicated runtime and review this scope before adding the rule.

Minecraft Bedrock has a separate native setup guide in the library. Prefer its
built-in compatible controller support. Bifrost does not supply a Java controller
mod or a virtual Xbox controller.

## Palworld

Prefer native controller controls where your PC edition recognizes the device.
The keyboard/mouse alternative has Gameplay, Building and Menus layouts. Review
and configure the PC key assignments listed in the template: WASD movement,
Space jump, Shift sprint, Ctrl roll, C crouch, F interact/work, R reload, E summon,
Q sphere, 4 Pal commands, Tab inventory, B build, R building rotation, C dismantle
and M map. Keyboard/mouse actions can differ by version and context. If a required
action cannot be assigned to that key, customize the Bifrost mapping rather than
assuming console parity. Left/right triggers send aim/attack mouse buttons.
Shoulders scroll weapons or lists. Building and menu changes are manual.

## Space Engineers 1

For Space Engineers 1, not Space Engineers 2. Prefer native controller controls
where compatible. The keyboard/mouse alternative has Engineer, Flight, Building
and Menus layouts. Left stick sends WASD and right stick sends mouse look/pointer.
Flight includes ascend/descend, roll, dampeners, power, parking and cockpit exit.
Building exposes the six keyboard block-rotation directions. Review PC bindings
for Space/C ascend/descend, F use, I inventory, K control panel, G block selection,
X jetpack, Z dampeners, P parking, Y power, V camera, Q/E roll and
Insert/Delete/Home/End/PageUp/PageDown rotations. Context changes some key meanings.
Complex toolbar actions and customized/modded controls may need personal changes.

## Native controls and layout switching

For native support, add an App rule for the actual game process and choose
**Use the game's native controller controls**. Bifrost pauses keyboard/mouse
output only while that app has foreground focus; mappings and unsaved edits are
preserved. This prevents Bifrost-generated output from competing with native input.
It does not emulate an unsupported controller, disable Steam Input or suppress
another remapper's output. Configure those separately as appropriate for the game.

For a keyboard/mouse template, create an App rule pointing to the exported/saved
mapping. Select only the defined layouts and reserve Create / View / Back
(normalized SDL index 4) to cycle them on release. The templates leave that button
unassigned. Bifrost does not detect inventory, building, cockpit or character state
inside a game; changing layouts is manual. Notifications can appear on the focused
application's monitor; exclusive fullscreen can hide desktop notices.

Reference guidance: [Minecraft controls](https://www.minecraft.net/en-us/article/minecraft-controls),
[Java/Bedrock controller support](https://learn.microsoft.com/en-us/minecraft/creator/documents/differencesbetweenbedrockandjava),
[Palworld PC controller support](https://store.steampowered.com/app/1623730/Palworld/?l=english),
[Space Engineers game help](https://www.spaceengineersgame.com/how-to-play/).
