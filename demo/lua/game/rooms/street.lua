-- We create a room for the street, which has a few props (objects
-- you can't pick up and/or interact with), an NPC (which we define
-- elsewhere) and a few scripts that do things automatically.

-- First we register the background image, with no z-plane layer. We're
-- using a nice street background by https://ansimuz.itch.io/ for this test
Image:new({ id = 'street-bg', path = './assets/images/street.png' })
Image:new({ id = 'street-bg-back', path = './assets/images/street-back.png' })
Image:new({ id = 'street-bg-far', path = './assets/images/street-far.png' })
-- Then we register the background music track we'll use
local luke9 = Music:new({ id = 'luke9', path = './assets/music/luke9.ogg' })
-- Now we define the room itself
Room:new({
	id = 'street',
	background = 'street-bg',
	layers = {
		{ id = 'street-back', image = 'street-bg-back', plane = -100 },
		{ id = 'street-far', image = 'street-bg-far', plane = -200 }
	},
	walkboxes = {
		{ x1 = 127, y1 = 148, x2 = 172, y2 = 150, scale = 0.38, speed = 0.5 },
		{ x1 = 118, y1 = 150, x2 = 180, y2 = 154, scale = 0.45, speed = 0.6 },
		{ x1 = 106, y1 = 154, x2 = 190, y2 = 158, scale = 0.5, speed = 0.7 },
		{ x1 = 80, y1 = 158, x2 = 200, y2 = 162, scale = 0.6, speed = 0.8 },
		{ x1 = 60, y1 = 162, x2 = 216, y2 = 168, scale = 0.7, speed = 0.9 },
		{ x1 = 0, y1 = 166, x2 = 14, y2 = 180, scale = 0.76, name = 'barrier1' },
		{ x1 = 14, y1 = 166, x2 = 580, y2 = 180, scale = 0.76 },
		{ x1 = 580, y1 = 166, x2 = 608, y2 = 180, scale = 0.76 },
	},
	triggers = {
		barrier1 = function(actor)
			-- When we trigger the walkbox called 'barrier', we start
			-- a script that prevents us from going further: triggers
			-- on walkboxes can be helpful to do different things, and
			-- since we know which actor caused the trigger, we can
			-- choose to react differently depending on the scenario
			startCutscene()
			activeActor:look('down')
			activeActor:say(text('streetBarrier1'))
			waitFor(activeActor.id)
			waitMs(500)
			activeActor:say(text('streetBarrier2'))
			waitFor(activeActor.id)
			activeActor:walkTo(58, 170)
			waitFor(activeActor.id)
			stopCutscene()
		end,
		barrier2 = function()
			rooms['outskirts']:enter()
		end
	},
	onenter = function(self)
		-- When we enter the room, we fade in the local music track
		luke9:play(1000)
		if previousRoom ~= nil and previousRoom.id == 'outskirts' then
			-- If we're coming from the 'outskirts' room, we place the main
			-- actor on the right side, not on the left as in the intro
			activeActor:moveTo('street', 570, 174)
			activeActor:look('left')
		end
		-- We automatically start a script to show noises out of the girls bar
		self:startScript('noises', noisesScript)
		-- We also start a script to make the NPC do something
		self:startScript('searching', npcSearchingScript)
	end,
	onleave = function(self)
		-- When we leave the room, we fade out the local music track
		-- and then fade in/out for a little while
		luke9:stop(1000)
		fadeOut(250)
		waitFor('fade')
		fadeIn(250)
	end
})


-- This room has a few props, mostly "decorative" objects that you can
-- interact with. Since they never leave the room, it's easier to define
-- them here, rather than creating a separate object file which makes
-- more sense for object that you can carry around.

-- The file is is animated, so let's register the animation file first
Animation:new({ id = 'fire-loop', path = './assets/images/fire.png', frames = 8 });
-- Now let's create the object for the fire
local fire = Object:new({
	id = 'fire',
	name = 'fireName',
	animation = 'fire-loop',
	plane = 0,
	description = "fireDesc",
	interaction = { direction = 'up', use = 'mid', x = 498, y = 166 },
	verbs = {
		use = function(self)
			self:use()
			waitMs(1000)
			activeActor:say(text('fireUse'))
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'use'
})
fire:moveTo('street', 506, 162)
fire:show()

-- The skyline is part of the background, so we don't register any image
local skyline = Object:new({
	id = 'skyline',
	name = 'skylineName',
	description = "skylineDesc",
	hover = { x1 = 74, y1 = 0, x2 = 215, y2 = 75 },
	interaction = { direction = 'up', x = 148, y = 168 },
	onRightClick = 'lookAt',
	onLeftClick = 'lookAt'
})
skyline:moveTo('street', 74, 0)
skyline:show()

-- The restaurant is part of the background, so we don't register any image
local restaurant = Object:new({
	id = 'restaurant',
	name = 'restaurantName',
	description = "restaurantDesc",
	hover = { x1 = 232, y1 = 112, x2 = 300, y2 = 154 },
	interaction = { direction = 'up', x = 266, y = 166 },
	verbs = {
		use = function(self)
			activeActor:look('down')
			activeActor:say(text('restaurantUse'))
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'use'
})
restaurant:moveTo('street', 232, 112)
restaurant:show()

-- The girls bar is part of the background, so we don't register any image
local girls = Object:new({
	id = 'girls',
	name = 'girlsName',
	description = "girlsDesc",
	hover = { x1 = 315, y1 = 125, x2 = 338, y2 = 150 },
	interaction = { direction = 'up', x = 326, y = 166 },
	verbs = {
		use = function(self)
			disableInput()
			self:use()
			soundfxs['locked-fx']:play()
			waitMs(1000)
			activeActor:look('down')
			activeActor:say(text('girlsUse'))
			enableInput()
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'use'
})
girls:moveTo('street', 315, 125)
girls:show()

-- This is a fake object, with no interaction except using it to change room
local gateway = Object:new({
	id = 'gateway',
	name = 'gatewayName',
	hover = { x1 = 580, y1 = 166, x2 = 608, y2 = 180 },
	interaction = { direction = 'right', x = 590, y = 174 },
	verbs = {
		use = function(self)
			rooms['outskirts']:enter()
		end
	},
	onRightClick = 'use',
	onLeftClick = 'use'
})
gateway:moveTo('street', 580, 166)
gateway:show()


-- Finally, we define the functions we'll use for scripts the room may
-- want to launch when something happens. In this case, we have a couple
-- of scripts that start when the room is entered, and that are closed
-- automatically when leaving the room.

-- This is the script we use for showing noises on the SCUMM bar
function noisesScript()
	local color = green
	local outline = black
	while(true) do
		showText({ text = '*Crash*', font = 'dialogues', color = color, outline = outline, x = 320, y = 127, duration = 500 })
		waitMs(600)
		showText({ text = '*Bang*', font = 'dialogues', color = color, outline = outline, x = 340, y = 120, duration = 500 })
		waitMs(600)
		showText({ text = '*Pow*', font = 'dialogues', color = color, outline = outline, x = 320, y = 124, duration = 500 })
		waitMs(600)
		showText({ text = '*Boom*', font = 'dialogues', color = color, outline = outline, x = 340, y = 130, duration = 500 })
		waitMs(600)
	end
end

-- This is the script we use for the automated npc actions
function npcSearchingScript()
	local npc = actors['npc']
	if npc == nil then return end
	local current = 1
	local messages = { "npcSearch1", "npcSearch2", "npcSearch3", "npcSearch4" }
	while(true) do
		npc:look('up')
		waitMs(1000)
		npc:look('left')
		waitMs(1000)
		npc:look('right')
		waitMs(1000)
		if current == 4 then
			npc:look('down')
		else
			npc:look('up')
		end
		npc:say(text(messages[current]))
		waitFor('npc')
		current = current+1
		if current > #messages then
			current = 1
		end
	end
end
