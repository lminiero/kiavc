-- Global properties
actors = {}
activeActor = nil

-- Actors class
Actor = {
	textColor = white,
	outlineColor = black,
	speed = 6,
	objects = {},
	onAction = 'giveTo',
	defaultVerbs = {
		lookAt = function(self)
			kiavcLog('Looking at ' .. self.id)
			if translations == nil then
				if self.description == nil then
					self.description = "Nothing to see here."
				end
				activeActor:say(self.description)
			else
				if self.description == nil then
					self.description = "defaultLook"
				end
				activeActor:say(text(self.description))
			end
		end,
		pickUp = function(self)
			kiavcLog('Trying to pick up ' .. self.id)
			if translations == nil then
				activeActor:say("I can't pick that up.")
			else
				activeActor:say(text('defaultTake'))
			end
		end,
		open = function(self)
			kiavcLog('Trying to open ' .. self.id)
			if translations == nil then
				activeActor:say("I can't open that.")
			else
				activeActor:say(text('defaultOpen'))
			end
		end,
		close = function(self)
			kiavcLog('Trying to close ' .. self.id)
			if translations == nil then
				activeActor:say("I can't close that.")
			else
				activeActor:say(text('defaultClose'))
			end
		end,
		use = function(self)
			kiavcLog('Trying to use ' .. self.id)
			if translations == nil then
				activeActor:say("I can't use that.")
			else
				activeActor:say(text('defaultUse'))
			end
		end,
		useWith = function(self, object)
			if object == nil or objects[object] == nil then
				return
			end
			kiavcLog('Trying to use ' .. self.id .. ' with ' .. object)
			if self.id == object then
				return
			end
			if translations == nil then
				activeActor:say("I can't use them together.")
			else
				activeActor:say(text('defaultUseWith'))
			end
		end,
		giveTo = function(self, object)
			if object == nil or objects[object] == nil then
				return
			end
			kiavcLog('Trying to give ' .. object .. ' to ' .. self.id)
			if translations == nil then
				activeActor:say("I don't think they want it.")
			else
				activeActor:say(text('defaultGiveActor'))
			end
		end,
		talkTo = function(self)
			kiavcLog('Trying to talk to ' .. self.id)
			if translations == nil then
				activeActor:say("I can't talk to them.")
			else
				activeActor:say(text('defaultTalkActor'))
			end
		end
	},
	setCostume =
		function(self, costumeId)
			if costumeId == nil then
				kiavcError('Invalid costume ID')
				return
			end
			if costumes[costumeId] == nil then
				kiavcError('Invalid costume ID ' .. costumeId)
				return
			end
			self.costume = costumeId
			-- Tell the engine about the new costume
			setActorCostume(self.id, costumeId)
		end,
	setFont =
		function(self, font)
			if font == nil then
				kiavcError('Invalid font')
				return
			end
			if fonts[font] == nil then
				kiavcError('Invalid font ' .. font)
				return
			end
			self.font = font
		end,
	setTextColor =
		function(self, r, g, b)
			if r == nil  or g == nil  or b == nil then
				kiavcError('Invalid color')
				return
			end
			self.textColor = { r = r, g = g, b = b }
		end,
	setOutlinetColor =
		function(self, r, g, b)
			if r == nil  or g == nil  or b == nil then
				self.outlineColor = nil
				return
			end
			self.outlineColor = { r = r, g = g, b = b }
		end,
	setSpeed =
		function(self, speed)
			if speed == nil or speed < 1 then
				kiavcError("Invalid actor speed")
				return
			end
			self.speed = speed
			setActorSpeed(self.id, self.speed)
		end,
	moveTo =
		function(self, roomId, x, y)
			if roomId == nil then
				kiavcError('Invalid room ID')
				return
			end
			local room = rooms[roomId]
			if room == nil then
				kiavcError('Invalid room ID ' .. roomId)
				return
			end
			-- FIXME Track that the actor is now in this room
			if self.room then
				local prevRoom = rooms[roomId]
				if prevRoom then
					prevRoom.actors[self.id] = nil
				end
			end
			room.actors[self.id] = true
			self.room = roomId
			-- Tell the engine about where to put the actor
			moveActorTo(self.id, roomId, x, y)
		end,
	scale =
		function(self, scale)
			self.scaleFactor = scale
			-- Tell the engine to use the provided z-plane for the actor
			scaleActor(self.id, scale)
		end,
	setAlpha =
		function(self, alpha)
			self.alpha = alpha
			-- Tell the engine to use the provided alpha for the actor
			setActorAlpha(self.id, alpha)
		end,
	setPlane =
		function(self, zplane)
			self.plane = zplane
			-- Tell the engine to use the provided z-plane for the actor
			setActorPlane(self.id, zplane)
		end,
	setInteraction =
		function(self, interaction)
			if interaction == nil or interaction.direction == nil or
					interaction.x == nil or interaction.y == nil then
				kiavcError('Invalid coordinates')
				self.interaction = nil
				return
			end
			-- Take note of how to interact with the object
			self.interaction = {
				direction = interaction.direction,
				x = interaction.x,
				y = interaction.y
			}
		end,
	setState =
		function(self, state)
			-- Tell the engine to switch the actor to a different animation
			setActorState(self.id, state)
		end,
	show =
		function(self)
			-- Tell the engine to show the actor in the room they're in
			showActor(self.id)
		end,
	follow =
		function(self)
			-- Tell the engine to have the camera follow this actor
			followActor(self.id)
		end,
	hide =
		function(self)
			-- Tell the engine the actor will be invisible
			hideActor(self.id)
		end,
	fadeIn =
		function(self, ms)
			-- Tell the engine to fade the actor in
			fadeActorIn(self.id, ms)
		end,
	fadeOut =
		function(self, ms)
			-- Tell the engine to fade the actor out
			fadeActorOut(self.id, ms)
		end,
	walkTo =
		function(self, x, y)
			-- Tell the engine the actor to make the actor walk
			walkActorTo(self.id, x, y)
		end,
	look =
		function(self, direction)
			-- Tell the engine to change the current direction for this actor
			setActorDirection(self.id, direction)
		end,
	say =
		function(self, text)
			-- Tell the engine to show some text from this actor
			sayActor({ id = self.id, font = self.font, text = text,
				color = self.textColor, outline = self.outlineColor })
		end,
	leftClick =
		function(self)
			-- If there's a left click handler, invoke that
			if activeActor == nil then
				return
			end
			if self.interaction ~= nil then
				activeActor:walkTo(self.interaction.x, self.interaction.y)
				waitFor(activeActor.id)
				activeActor:look(self.interaction.direction)
				waitMs(200)
			end
			if selectedObject ~= nil and self.onAction ~= nil and self.verbs[self.onAction] ~= nil then
				self.verbs[self.onAction](self, selectedObject)
			elseif self.onLeftClick ~= nil and self.verbs[self.onLeftClick] ~= nil then
				self.verbs[self.onLeftClick](self, selectedObject)
			end
		end,
	rightClick =
		function(self)
			-- If there's a right click handler, invoke that
			if activeActor == nil then
				return
			end
			if self.interaction ~= nil then
				activeActor:walkTo(self.interaction.x, self.interaction.y)
				waitFor(activeActor.id)
				activeActor:look(self.interaction.direction)
				waitMs(200)
			end
			if self.onRightClick ~= nil and self.verbs[self.onRightClick] ~= nil then
				self.verbs[self.onRightClick](self, selectedObject)
			end
		end,
	startScript =
		function(self, name, func)
			-- If a script with that name exists already, stop it
			self:stopScript(name)
			-- Start the script as a coroutine
			kiavcLog("Starting actor script '" .. name .. "'")
			self.scripts[name] = coroutine.create(func)
			return coroutine.resume(self.scripts[name])
		end,
	stopScript =
		function(self, name)
			if self.scripts[name] ~= nil then
				kiavcLog("Stopping actor script '" .. name .. "'")
				debug.sethook(self.scripts[name], function()error("interrupted")end, "l")
				self.scripts[name] = nil
			end
		end,
	stopAllScripts =
		function(self)
			for name, co in pairs(self.scripts) do
				self:stopScript(name)
			end
		end
}
function Actor:new(actor)
	if actor == nil then
		kiavcError('Invalid actor')
		return nil
	end
	if actor.id == nil then
		kiavcError('Missing actor ID')
		return nil
	end
	if actors[actor.id] ~= nil then
		kiavcError('Actor ' .. actor.id .. ' already registered')
		return nil
	end
	setmetatable(actor, self)
	self.__index = self
	actors[actor.id] = actor
	-- Register the actor at the engine
	registerActor(actor)
	-- If a costume was provided, set it now
	if actor.costume ~= nil then
		actor:setCostume(actor.costume)
	end
	-- If a scale was provided, set it now
	if actor.scaleFactor ~= nil then
		actor:scale(actor.scaleFactor)
	end
	-- If an alpha was provided, set it now
	if actor.alpha ~= nil then
		actor:setAlpha(actor.alpha)
	end
	-- If a z-plane was provided, set it now
	if actor.plane ~= nil then
		actor:setPlane(actor.plane)
	end
	-- If a movement speed, set it now
	if actor.speed ~= nil then
		actor:setSpeed(actor.speed)
	end
	-- If verbs were provided, set them now
	if actor.verbs == nil then
		actor.verbs = {}
	end
	for verb, action in pairs(actor.defaultVerbs) do
		if actor.verbs[verb] == nil then
			actor.verbs[verb] = action
		end
	end
	actor.scripts = {}
	return actor
end

-- Helper function to set the actor the user is controlling
function setActiveActor(id)
	if id == nil then
		kiavcError('Missing actor ID')
		return nil
	end
	local actor = actors[id]
	if actor == nil then
		kiavcError('No such actor ' .. id)
		return nil
	end
	activeActor = actor
	-- Notify the engine about the change
	controlledActor(id)
end
