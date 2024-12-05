-- The envelope is an object we add to the inventory right away, just
-- to show something there and allow the user to interact with it.

-- First of all, let's register the images/animations we'll use
Image:new({ id = 'envelope', path = './assets/images/envelope.png' });
Image:new({ id = 'envelope-inventory', path = './assets/images/envelope-large.png' });

-- Then we define the object, its properties, and how to interact with it
local envelope = Object:new({
	id = 'envelope',
	name = 'envelopeName',
	animations = { default = 'envelope' },
	uiAnimation = 'envelope-inventory',
	plane = 0,
	verbs = {
		lookAt = function(self)
			-- We use looking at the envelope as a crude and silly way
			-- to explain the player what they can do in the demo
			activeActor:look('down')
			-- The tutorial is made of 16 sentences: we read the first one
			-- here, then show the letter (moving to a different "room")
			-- and read the others only the first time (otherwise we
			-- just print the content of the letters and do nothing else)
			if not state.readLetter then
				activeActor:say(text('envelopeTutorial1'))
				waitFor(activeActor.id)
				activeActor:say(text('envelopeTutorial2'))
				waitFor(activeActor.id)
			end
			rooms['letter']:enter()
		end
	},
	objectInteractionNotOwned = function()
		activeActor:say(text('envelopeUse'))
	end,
	objectInteractions = {
		fire = function()
			activeActor:say(text('envelopeUseFire'))
		end,
		skyline = function()
			activeActor:say(text('envelopeUseSkyline'))
		end,
		restaurant = function()
			activeActor:look("down")
			activeActor:say(text('envelopeUseRestaurant'))
		end,
		girls = function()
			activeActor:look("down")
			activeActor:say(text('envelopeUseGirls'))
		end,
		boat = function()
			activeActor:look("down")
			activeActor:say(text('envelopeUseBoat'))
		end,
		skull = function()
			activeActor:look("down")
			activeActor:say(text('envelopeUseSkull'))
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'select'
})

-- By default, we put the envelope in the inventory, which we'll do later
