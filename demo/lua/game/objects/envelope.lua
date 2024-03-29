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
		end,
		useWith = function(self, object)
			if self.owner == nil then
				activeActor:say(text('envelopeUse'))
				return
			end
			if object == "fire" then
				activeActor:say(text('envelopeUseFire'))
			elseif object == "skyline" then
				activeActor:say(text('envelopeUseSkyline'))
			elseif object == "restaurant" then
				activeActor:look("down")
				activeActor:say(text('envelopeUseRestaurant'))
			elseif object == "girls" then
				activeActor:look("down")
				activeActor:say(text('envelopeUseGirls'))
			elseif object == "boat" then
				activeActor:look("down")
				activeActor:say(text('envelopeUseBoat'))
			elseif object == "skull" then
				activeActor:look("down")
				activeActor:say(text('envelopeUseSkull'))
			elseif object == "props" then
				activeActor:say(text('defaultUseWith'))
			end
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'select'
})

-- Let's define the map of possible object interactions for this object
objectInteractions.envelope =
	{ skyline = true, restaurant = true, girls = true, fire = true, boat = true, skull = true, props = true }

-- By default, we put the envelope in the inventory, which we'll do later
