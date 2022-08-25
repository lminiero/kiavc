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
	animation = 'skull',
	uiAnimation = 'skull-inventory',
	interaction = { direction = 'right', use = 'low', x = 150, y = 166 },
	plane = -1,
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
			self.onLeftClick = 'select'
			activeActor:setState('still')
		end,
		useWith = function(self, object)
			if self.owner == nil then
				activeActor:say(text('skullUse'))
				return
			end
			if object == "fire" then
				activeActor:say(text('skullUseFire'))
			elseif object == "skyline" then
				activeActor:say(text('skullUseSkyline'))
			elseif object == "restaurant" then
				activeActor:look("down")
				activeActor:say(text('skullUseRestaurant'))
			elseif object == "girls" then
				activeActor:look("down")
				activeActor:say(text('skullUseGirls'))
			elseif object == "boat" then
				activeActor:look("down")
				activeActor:say(text('skullUseBoat'))
			elseif object == "props" then
				activeActor:say(text('defaultUseWith'))
			end
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'pickUp'
})

-- Let's define the map of possible object interactions for this object
objectInteractions.skull =
	{ skyline = true, restaurant = true, girls = true, fire = true, boat = true, props = true }

-- By default, we put the skull in the Melee harbour room
skull:moveTo('outskirts', 170, 150)
skull:show()
