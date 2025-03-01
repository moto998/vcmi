< [Documentation](../Readme.md) / Game Mechanics

# List of features added in VCMI

## High resolutions

VCMI supports resolutions higher than original game, which ran only in 800 x 600. It also allows a number of additional features:

- High-resolution screens of any ascpect ratio are supported.
- In-game GUI can be freely scaled
- Adventure map can be freely zoomed

Assets from Heroes of Might & Magic III HD - Remake released by Ubisoft in 2015 - are **not** supported.

## Extended engine limits

Some of game features have already been extended in comparison to Shadow of Death:

- Support for 32-bit graphics with alpha channel. Supported formats are .def, .bmp, .png and .tga
- Support for maps of any size, including rectangular shapes
- No limit of number of map objects, such as dwellings and stat boosters
- Hero experience capacity currently at 2^64, which equals 199 levels with typical progression
- Heroes can have primary stats up to 2^16.
- Unlimited backpack (by default). This can be toggled off to restore original 64-slot backpack limit.

The list of implemented cheat codes and console commands is [here](Cheat_codes.md).

# New mechanics (Optional)

## Stack Experience module

VCMI natively suppoorts stack experience feature known from WoG. Any creature - old or modded - can get stack experience bonuses. However, this feature needs to be enabled as a part of WoG VCMI submod.

Stack experience interface has been merged with regular creature window. Among old functionalities, it includes new useful info:

- Click experience icon to see detailed info about creature rank and experience needed for next level. This window works only if stack experience module is enabled (true by default).
- Abilities description contain information about actual values and types of bonuses received by creature - be it default ability, stack experience, artifact or other effect. These descriptions use custom text files which have not been translated.
- [Stack Artifact](#stack-artifact-module). You can choose enabled artifact with arrow buttons. There is also additional button below to pass currently selected artifact back to hero.

## Commanders module

VCMI offers native support for Commanders. Commanders are part of WoG mod for VCMI and require it to be enabled. However, once this is done, any new faction can use its own Commander, too.

## Mithril module

VCMI natively supports Mithril resource known from WoG. However, it is not currently used by any mod.

## Stack Artifact module

In original WoG, there is one available Stack Artifact - Warlord's Banner, which is related directly to stack experience. VCMI natively supports any number of Stack Artifacts regardless if of Stack Experience module is enabled or not. However, currently no mods make use of this feature and it hasn't been tested for many years.

# List of bugs fixed in VCMI

These bugs were present in original Shadow of Death game, however the team decided to fix them to bring back desired behaviour:

## List of game mechanics changes

Some of H3 mechanics can't be straight considered as bug, but default VCMI behaviour is different:

- Pathfinding. Hero can't grab artifact while flying when all tiles around it are guarded without triggering attack from guard.
- Battles. Hero that won battle, but only have temporary summoned creatures alive going to appear in tavern like if he retreated. 
- Battles. Spells from artifacts like AOTD are autocasted on beginning of the battle, not beginning of turn.
- Spells. Dimension Door spell doesn't allow to teleport to unexplored tiles. 

# List of extended GUI features

## Adventure map

### New Shortcuts

- [LCtrl] + [R] - Quick restart of current scenario.
- [LCtrl] + Arrows - scrolls Adventure Map behind an open window.
- [LCtrl] pressed blocks Adventure Map scrolling (it allows us to leave the application window without losing current focus).
- NumPad 5 - centers view on selected hero.
- NumPad Enter functions same as normal Enter in the game (it didn't in H3).
- [LCtrl] + LClick – perform a normal click (same as no hero is selected). This make it possible to select other hero instead of changing path of current hero.

## Pathfinder

VCMI introduces improved pathfinder, which may find the way on adventure map using ships,Subterranean Gates and Monoliths. Simply click your destination anywhere on adventure map and it will find shortest path, if if target position is reachable.

### Quest log

VCMI itroduces custom Quest Log window. It can display info about Seer Hut or Quest Guard mission, but also handle Borderguard and Border Gate missions. When you choose a quest from the list on the left, it's description is shown. Additionally, on inner minimap you can see small icons indicating locations of quest object. Clicking these objects immediately centers adventure map on desired location.

### Power rating

When hovering cursor over neutral stack on adventure map, you may notice additional info about relative threat this stack poses to curently selected hero. This feature has been originally introduced in Heroes of Might and Magic V.

### Minor GUI features

Some windows and dialogs now display extra info and images to make game more accessible for new players. This can be turned off, if desired.

## Battles

### Stack Queue

Stack queue is a feature coming straight from HoMM5, which allows you to see order of stacks on the battlefield, sorted from left to right. To toggle in on/off, press [Q] during the battle. There is smaller and bigger version of it, the second one is available only in higher resolutions.

### Attack range

In combat, some creatures, such as Dragon or Cerberi, may attack enemies on multiple hexes. All such attacked stacks will be highlighted if the attack cursor is hovered over correct destination tile. Whenever battle stack is hovered, its movement range is highlighted in darker shade. This can help when you try to avoid attacks of melee units.

## Town Screen

### Quick Army Management

- [LShift] + LClick – splits a half units from the selected stack into an empty slot.
- [LCtrl] + LClick – splits a single unit from the selected stack into an empty slot.
- [LCtrl] + [LShift] + LClick – split single units from the selected stack into all empty hero/garrison slots
- [Alt] + LClick – merge all splitted single units into one stack
- [Alt] + [LCtrl] + LClick - move all units of selected stack to the city's garrison or to the met hero 
- [Alt] + [LShift] + LClick - dismiss selected stack`
- Directly type numbers in the Split Stack window to split them in any way you wish

### Interface Shortcuts

It's now possible to open Tavern (click on town icon), Townhall, Quick Recruitment and Marketplace (click on gold) from various places:

- Town screen (left bottom)
- Kingdom overview for each town
- Infobox (only if info box army management is enabled)

### Quick Recruitment

Mouse click on castle icon in the town screen open quick recruitment window, where we can purhase in fast way units.

## Pregame - Scenario / Saved Game list

- Mouse wheel - scroll through the Scenario list.
- [Home] - move to the top of the list.
- [End] - move to the bottom of the list.
- NumPad keys can be used in the Save Game screen (they didn't work in H3).

## Fullscreen

- [F4] - Toggle fullscreen mode on/off.

## FPS counter

It's the new feature meant for testing game performance on various platforms.

## Color support in game text

Additional color are supported for text fields (e.g. map description). Uses HTML color syntax (e.g. #abcdef) / HTML predefined colors (e.g. green).

##### Original Heroes III Support

`This is white`

<span style="color:white;background-color:black;">This is white</span>

`{This is yellow}`

<span style="color:yellow;background-color:black;">This is yellow</span>

##### New

`{#ff0000|This is red}`

<span style="color:red">This is red</span>

`{green|This is green}`

<span style="color:green">This is green</span>


# Manuals and guides

- https://heroes.thelazy.net/index.php/Main_Page Wiki that aims to be a complete reference to Heroes of Might and Magic III. 
