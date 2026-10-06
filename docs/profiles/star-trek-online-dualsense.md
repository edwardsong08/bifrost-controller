# Star Trek Online - PS5-style controls on PC

This community starting template uses a PS5 DualSense through SDL's standard
game-controller mapping, with default PC keyboard binds. It is an independently
maintained, console-inspired layout, not an official console controller port.
The owner approved the closest practical layout after discussing these limits.
No game or hardware-input tests have been performed.

## Install and select

1. Install Bifrost Controller 0.1.5 or newer. This fixes false profile-load failure
   feedback and applies the selected Space/Ground/Menus layout. Version 0.1.2 cannot download
   catalogs or use triggers in managed profiles; 0.1.3 has a bundled-resource
   packaging issue and must also be upgraded.
2. In **Profiles**, press **Update profiles** or leave startup downloads enabled.
3. Choose **Star Trek Online - PS5-style (PC)** and your DualSense. Review the
   Space, Ground and Menus assignments, then **Make a personal copy** if you
   want a version that community updates cannot replace.
4. Save any existing edited mapping before **Use this profile**. In Controllers,
   save the mapping as a native `.amgp` file.
5. In **Applications**, add a rule for STO's actual `GameClient.exe` (not the
   launcher), the saved native profile and your DualSense. Use layouts 1 Space,
   2 Ground and 3 Menus; choose Create / Share (normalized SDL button 4) as the
   layout-switch button and optionally Ctrl+Alt+F8 as the keyboard shortcut.
   Create is deliberately unassigned in every template layout.
6. While STO is focused, release Create to cycle layouts. Switching is manual:
   Bifrost cannot detect whether your character has entered space or ground.
   The existing switch notice appears on the game window's monitor by default.
   Borderless/windowed mode is needed if exclusive fullscreen hides overlays.

Avoid simultaneous keyboard/mouse emulation from Steam Input or another mapper;
otherwise one button can cause duplicate actions. Bifrost does not change Steam
settings, game preferences or account bindings for you.

## Main controls

| Control | Space | Ground | Menus |
| --- | --- | --- | --- |
| Left stick | Ship pitch/turn (S/W/A/D) | Movement/strafe (W/S/Q/E) | Unassigned |
| Right stick | Pointer; hold R3 for camera drag | Pointer; hold L2 for camera drag | Pointer |
| R2 | Energy weapons (Space) | Primary attack / slot 1 | Left click |
| R1 | Torpedoes (Ctrl+Space) | Secondary attack / slot 2 | Scroll down |
| L1 | Ship/item shortcut / slot 5 | Captain shortcut / slot 5 | Scroll up |
| L2 | Captain shortcut / slot 6 | Camera drag (right mouse) | Right click |
| Cross | Interact (F) | Jump (Space) | Left click |
| Circle | Tactical shortcut / slot 3 | Melee / slot 3 | Cancel |
| Square | Science shortcut / slot 1 | Kit/item shortcut / slot 4 | Interact |
| Triangle | Engineering shortcut / slot 2 | Swap weapon (Z) | Map (M) |
| L3 | Distribute shields (Delete) | Hold to sprint (Shift) | Enter |
| R3 | Hold for camera drag | Crouch (C) | Tab |
| D-pad Up / Down | Throttle + / - (E / Q) | Interact / holster (F / H) | Arrow navigation |
| D-pad Left / Right | Scan / next target (V / Tab) | Scan / next target (V / Tab) | Arrow navigation |
| Options | Escape | Escape | Escape |
| Create | Reserved for the Applications rule | Reserved for the Applications rule | Reserved for the Applications rule |

Numbered keys address PC tray slots, not power categories. Arrange powers in the
tray to suit the labels; Science, Engineering, Tactical and Captain buttons do
not automatically discover powers. Ground RPG/Shooter settings and ship pitch
inversion can change the expected actions. Check the game's Controls/Key Binds
screen before relying on these assumptions; adjust a personal copy as needed.

## Differences from PlayStation STO

- Console radial menus and its redesigned HUD belong to the game. This template
  cannot add them to the PC version or automate powers based on health/cooldowns.
- Cross cannot conditionally choose jump versus interact; use Ground D-pad Up
  for interact and Cross for jump. Ground R3 crouches; it does not infer a roll.
- Camera dragging is a PC mouse approximation. Native analog steering, console
  target-camera behavior, contextual input and touchpad gestures are not promised.
- Abilities use explicit tray slots; hold-to-open wheels and contextual captain
  menus are not available in this managed schema. The PS button is unassigned.
- Community downloads change library templates only. Active native mappings,
  saved application-rule files and personal copies remain unchanged until the
  user deliberately applies or exports a newer revision.

## References and maintenance

The STO executive producer explains the console-specific radial menus and UI
rewrite in [PlayStation's announcement](https://blog.playstation.com/2016/05/12/star-trek-online-explores-strange-new-worlds-on-ps4/).
The developer's [Console UI article](https://www.playstartrekonline.com/en/news/article/9981193)
is an additional reference. Bindings above are a proposed PC starting layout;
they have not been verified against the current game client. Do not describe
this catalog entry as exact console parity or a tested gameplay preset.
