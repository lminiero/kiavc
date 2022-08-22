-- We create a room for the outskirts, which has a few props (objects
-- you can't pick up and/or interact with) and an object we can pick up.

-- First we register the background image, again with no z-plane layer
-- since I'm too lazy to prepare one. We're using the static version of
-- https://dusan-pavkovic-warlord.itch.io/skadovsk for this test
Image:new({ id = 'outskirts-bg', path = './assets/images/outskirts.png' })
-- Then we register the background music track we'll use
local luke1 = Music:new({ id = 'luke1', path = './assets/music/luke1.ogg' })
-- Now we define the room itself
Room:new({
	id = 'outskirts',
	background = 'outskirts-bg',
	walkboxes = {
		{ x1 = 28, y1 = 122, x2 = 66, y2 = 134 },
		{ x1 = 40, y1 = 134, x2 = 194, y2 = 176 },
		{ x1 = 140, y1 = 124, x2 = 186, y2 = 134 },
		{ x1 = 194, y1 = 155, x2 = 226, y2 = 180 },
	},
	onenter = function(self)
		-- When we enter the room, we fade in the local music track
		luke1:play(1000)
		if previousRoom ~= nil and previousRoom.id == 'street' then
			-- If we're coming from the 'street' room, we place the main
			-- actor next on the top left
			activeActor:moveTo('outskirts', 30, 126)
			activeActor:look('down')
		end
	end,
	onleave = function(self)
		-- When we leave the room, we fade out the local music track
		-- and then fade in/out for a little while
		luke1:stop(1000)
		fadeOut(250)
		waitFor('fade')
		fadeIn(250)
	end
})


-- This room has a few props and an object we can pick up: that object,
-- though, is defined in a separate file, since there we also define
-- the possible interactions it can have.

-- The boat is part of the background, so we don't register any image
local boat = Object:new({
	id = 'boat',
	name = 'boatName',
	description = "boatDesc",
	hover = { x1 = 162, y1 = 80, x2 = 216, y2 = 120 },
	interaction = { direction = 'up', x = 182, y = 132 },
	verbs = {
		use = function(self)
			activeActor:say(text('boatUse'))
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'use'
})
boat:moveTo('outskirts', 162, 80)
boat:show()

-- The props are part of the background, so we don't register any image
local props = Object:new({
	id = 'props',
	name = 'propsName',
	description = "propsDesc",
	hover = { x1 = 228, y1 = 142, x2 = 312, y2 = 188 },
	interaction = { direction = 'right', x = 204, y = 180 },
	verbs = {
		lookAt = function(self)
			activeActor:look('down')
			activeActor:say(text('propsDesc'))
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'pickUp'
})
props:moveTo('outskirts', 228, 142)
props:show()

-- This is a fake object, with no interaction except using it to change room
local city = Object:new({
	id = 'city',
	name = 'cityName',
	hover = { x1 = 0, y1 = 60, x2 = 66, y2 = 134 },
	interaction = { direction = 'up', x = 40, y = 128 },
	verbs = {
		use = function(self)
			rooms['street']:enter()
		end
	},
	onRightClick = 'use',
	onLeftClick = 'use'
})
city:moveTo('outskirts', 0, 60)
city:show()
