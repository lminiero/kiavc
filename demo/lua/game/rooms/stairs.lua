-- We create a room for the temple stairs, which we mainly use to test
-- vertical scrolling, since it extends in height rather than width.

-- First we register the background image, with no z-plane layer either.
-- We're using a gorgoues picture by PixelArtJourney for this test
-- https://twitter.com/PixelArtJourney/status/1560261114516029446
Image:new({ id = 'stairs-bg', path = './assets/images/stairs.png' })
-- Then we register the background music track we'll use
local luke13 = Music:new({ id = 'luke13', path = './assets/music/luke13.ogg' })
-- Now we define the room itself
Room:new({
	id = 'stairs',
	background = 'stairs-bg',
	walkboxes = {
		{ x1 = 148, y1 = 164, x2 = 212, y2 = 172, scale=0.5, speed = 0.6, name = 'closed' },
		{ x1 = 138, y1 = 172, x2 = 220, y2 = 183, scale=0.55, speed = 0.7 },
		{ x1 = 138, y1 = 183, x2 = 222, y2 = 195, scale=0.62, speed = 0.8 },
		{ x1 = 140, y1 = 195, x2 = 222, y2 = 210, scale=0.7, speed = 0.9 },
		{ x1 = 120, y1 = 210, x2 = 236, y2 = 236, scale=0.76 },
		{ x1 = 96, y1 = 236, x2 = 238, y2 = 248, scale=0.82, speed = 1.2 },
		{ x1 = 80, y1 = 248, x2 = 242, y2 = 276, scale=0.88, speed = 1.3 },
		{ x1 = 68, y1 = 276, x2 = 244, y2 = 292, scale=0.94, speed = 1.4 },
		{ x1 = 36, y1 = 292, x2 = 266, y2 = 390, speed = 1.5 },
	},
	triggers = {
		closed = function(actor)
			-- Another example of a walkbox trigger: in this case, we use
			-- it to just have the actor say something when getting here
			activeActor:look('up')
			activeActor:say(text('stairsClosed1'))
			waitFor(activeActor.id)
			activeActor:look('down')
			activeActor:say(text('stairsClosed2'))
			waitFor(activeActor.id)
		end
	},
	onenter = function(self)
		if previousRoom ~= nil and previousRoom.id == 'letter' then
			-- If we're coming from the letter closeup, check the state
			if state.readLetter == 2 then
				-- Have the actor say one final line
				state.readLetter = 3
				activeActor:say(text('envelopeTutorial16'))
			end
		else
			-- When we enter the room, we fade in the local music track
			luke13:play(1000)
		end
		if previousRoom ~= nil and previousRoom.id == 'street' then
			-- If we're coming from the 'street' room, we place the main
			-- actor on the bottom and have them walk up a bit
			activeActor:moveTo('stairs', 160, 390)
			activeActor:look('up')
			disableInput()
			activeActor:walkTo(158, 280)
			waitFor(activeActor.id)
			if(state.visitedStairs == nil) then
				state.visitedStairs = true
				activeActor:look('down')
				activeActor:say(text('stairsContinuity1'))
				waitFor(activeActor.id)
				activeActor:say(text('stairsContinuity2'))
				waitFor(activeActor.id)
			end
			enableInput()
		end
	end,
	onleave = function(self)
		-- When we leave the room, we fade out the local music track
		-- and then fade in/out for a little while
		if nextRoom.id ~= 'letter' then
			-- We only stop the track if we're not going to show the letter, though
			luke13:stop(1000)
		end
		fadeOut(250)
		waitFor('fade')
		fadeIn(250)
	end
})


-- This is a fake object, with no interaction except using it to change room
local city = Object:new({
	id = 'cityFromStairs',
	name = 'cityName',
	hover = { x1 = 36, y1 = 292, x2 = 266, y2 = 320 },
	interaction = { direction = 'down', x = 160, y = 390 },
	verbs = {
		use = function(self)
			rooms['street']:enter()
		end
	},
	onRightClick = 'use',
	onLeftClick = 'use'
})
city:moveTo('stairs', 36, 292)
city:show()
