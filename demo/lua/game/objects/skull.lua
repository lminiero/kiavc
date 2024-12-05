-- The skull is one of the objects we can actually pick up and use
-- around, so just for the sake of simplicity and convenience we define
-- it in a separate file, whereas props or objects that stay confined
-- in a room can be defined in the room file itself.

-- First of all, let's register the images/animations we'll use
Image:new({ id = 'skull', path = './assets/images/skull.png' });
Image:new({ id = 'skull-inventory', path = './assets/images/skull-large.png' });

-- Then we define the object, its properties, and how to interact with it
local skull = Object:new({
	id = 'skull',
	name = 'skullName',
	animations = { default = 'skull' },
	uiAnimation = 'skull-inventory',
	interaction = { direction = 'right', use = 'low', x = 150, y = 166 },
	plane = 0,
	verbs = {
		lookAt = function(self)
			if state.lookedAtSkull == true then
				activeActor:say(text('skullDesc'))
				return
			end
			state.lookedAtSkull = true
			disableInput()
			activeActor:say(text('skullTalk1'))
			waitFor(activeActor.id)
			waitMs(2000)
			activeActor:look('down')
			activeActor:say(text('skullTalk2'))
			waitFor(activeActor.id)
			enableInput()
		end,
		pickUp = function(self)
			if self.owner then
				-- It's in the inventory already
				return
			end
			if state.lookedAtSkull ~= true then
				self.verbs.lookAt(self)
			end
			-- If we got here, we haven't picked it up yet
			activeActor:look('down')
			activeActor:say(text('skullTake'))
			waitFor(activeActor.id)
			activeActor:look(self.interaction.direction)
			self:use()
			waitMs(500)
			self:addToInventory(activeActor.id)
			activeActor:setState('still')
		end
	},
	objectInteractionNotOwned = function()
		activeActor:say(text('skullUse'))
	end,
	objectInteractions = {
		fire = function()
			activeActor:say(text('skullUseFire'))
		end,
		skyline = function()
			activeActor:say(text('skullUseSkyline'))
		end,
		restaurant = function()
			activeActor:look("down")
			activeActor:say(text('skullUseRestaurant'))
		end,
		girls = function()
			activeActor:look("down")
			activeActor:say(text('skullUseGirls'))
		end,
		boat = function()
			activeActor:look("down")
			activeActor:say(text('skullUseBoat'))
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'pickUp'
})

-- By default, we put the skull in the Melee harbour room
skull:moveTo('outskirts', 178, 166)
skull:show()
