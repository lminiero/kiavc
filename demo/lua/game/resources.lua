-- We use this file to register all resources we'll need in the game,
-- like rooms, objects we can interact with, actors, etc. Notice that
-- we're using different folders just as a way to simplify managing
-- resources, but you can load everything in a single file as well if
-- you prefer: for instance, it makes sense to register actors or
-- objects/props that will only be in a single room, or originate there,
-- in the resource file you use for the room itself.

-- Let's register the cursors we'll use to interact
Font:new({ id = 'cursor-font', path = './assets/fonts/m3x6.ttf', size = 16 })
-- This cursor has a yellow background, that we need to tell the engine to treat as transparency
Animation:new({ id = 'cursor-anim', path = './assets/images/mouse.png', frames = 6, transparency = { r = 192, g = 192, b = 0 }})
Cursor:new({ id = 'cursor-main', animation = 'cursor-anim' })
-- This cursor is transparent already instead
Animation:new({ id = 'cursor-hotspot-anim', path = './assets/images/mouse-hotspot.png', frames = 6 })
Cursor:new({ id = 'cursor-hotspot', animation = 'cursor-hotspot-anim' })

-- Let's register the font we'll use when actors are talking
Font:new({ id = 'dialogues', path = './assets/fonts/m5x7.ttf', size = 16 })

-- Let's register a "locked door" sound effect, that we may need in multiple rooms
SoundFX:new({ id = 'locked-fx', path = './assets/soundfx/locked.ogg' })

-- Load rooms
kiavcRequire('game/rooms/street')
kiavcRequire('game/rooms/outskirts')
kiavcRequire('game/rooms/letter')
kiavcRequire('game/rooms/stairs')

-- Load objects
kiavcRequire('game/objects/envelope')
kiavcRequire('game/objects/skull')

-- Load actors
kiavcRequire('game/actors/detective')
kiavcRequire('game/actors/npc')

-- Set the language to use for the localization strings
lang = 'en'
