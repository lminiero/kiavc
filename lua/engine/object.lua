-- Global properties
objects = {}
objectInteractions = {}
selectedObject = nil

-- Objects class
Object = {
	inventory = false,
	ui = false,
	interactable = true,
	onAction = 'useWith',
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
				if self.owner == activeActor.id then
					activeActor:say("I picked that up already.")
				else
					activeActor:say("I can't pick that up.")
				end
			else
				if self.owner == activeActor.id then
					activeActor:say(text('defaultOwned'))
				else
					activeActor:say(text('defaultTake'))
				end
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
			if objectInteractions[object] and objectInteractions[object][self.id] == true then
				local obj = objects[object]
				obj.verbs.useWith(obj, self.id)
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
			if translations == nil then
				activeActor:say("I can't give that anything.")
			else
				activeActor:say(text('defaultGiveObject'))
			end
		end,
		talkTo = function(self)
			if translations == nil then
				activeActor:say("I can't talk to that.")
			else
				activeActor:say(text('defaultTalkObject'))
			end
		end,
		select = function(self)
			if self.owner == nil then
				return
			end
			if selectedObject == nil then
				setSelectedObject(self.id)
			else
				if selectedObject ~= self.id and self.verbs.useWith then
					self.verbs.useWith(self, selectedObject)
				end
			end
		end
	},
	setAnimation =
		function(self, animId)
			if animId == nil then
				kiavcError('Invalid animation ID')
				return
			end
			if animations[animId] == nil then
				kiavcError('Invalid animation ID ' .. animId)
				return
			end
			self.animation = animId
			-- Tell the engine about the new directional anim
			setObjectAnimation(self.id, animId)
		end,
	setUiAnimation =
		function(self, animId)
			if animId == nil then
				kiavcError('Invalid animation ID')
				return
			end
			if animations[animId] == nil then
				kiavcError('Invalid animation ID ' .. animId)
				return
			end
			self.uiAnimation = animId
			-- Tell the engine about the new directional anim
			setObjectUiAnimation(self.id, animId)
		end,
	setParent =
		function(self, parentId)
			local parent = nil
			if parentId ~= nil then
				parent = objects[parentId]
				if parent == nil then
					kiavcError('Invalid parent object ID ' .. parentId)
					return
				end
			end
			self.parent = parentId
			-- Tell the engine about where to put the object
			if parent == nil then
				removeObjectParent(self.id)
			else
				setObjectParent(self.id, parentId)
			end
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
			-- FIXME Track that the object is now in this room
			if self.room then
				local prevRoom = rooms[roomId]
				if prevRoom then
					prevRoom.objects[self.id] = nil
				end
			end
			if self.owner then
				local actor = actors[self.owner]
				if actor then
					actor.objects[self.id] = nil
				end
			end
			room.objects[self.id] = true
			self.room = roomId
			-- Tell the engine about where to put the object
			moveObjectTo(self.id, roomId, x, y)
		end,
	addToInventory =
		function(self, owner)
			if owner == nil then
				return
			end
			local actor = actors[owner]
			if actor == nil then
				return
			end
			-- Mark the object as owned, and remove it from the room
			if self.room then
				local room = rooms[self.room]
				if room then
					room.objects[self.id] = nil
				end
			end
			self.room = nil
			if self.owner then
				local prevActor = actors[self.owner]
				if prevActor then
					prevActor.objects[self.id] = nil
				end
			end
			actor.objects[self.id] = true
			self.owner = actor.id
			-- Tell the engine the object is now in the inventory
			addObjectToInventory(self.id, actor.id)
			self:setUi(true)
			-- Invoke the callback that manages the inventory
			if onInventoryUpdated then
				onInventoryUpdated(actor.id, self.id, true)
			end
		end,
	removeFromInventory =
		function(self, owner)
			if owner == nil or self.owner ~= owner then
				return
			end
			local actor = actors[owner]
			if actor == nil then
				return
			end
			actor[self.id] = nil
			self.owner = nil
			self:setUi(false)
			-- Tell the engine the object is now in the inventory
			removeObjectFromInventory(self.id, actor.id)
			-- Invoke the callback that manages the inventory
			if onInventoryUpdated then
				onInventoryUpdated(actor.id, self.id, true)
			end
		end,
	setHover =
		function(self, coords)
			if coords == nil or coords.x1 == nil or coords.y1 == nil or
					coords.x2 == nil or coords.y2 == nil then
				kiavcError('Invalid coordinates')
				self.hover = nil
				return
			end
			self.hover = {
				x1 = coords.x1,
				y1 = coords.y1,
				x2 = coords.x2,
				y2 = coords.y2
			}
			-- Tell the engine about how to detect hovering on the object
			setObjectHover(self.id, self.hover)
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
				use = interaction.use,
				x = interaction.x,
				y = interaction.y
			}
		end,
	setInteractable =
		function(self, interactable)
			if interactable == nil or (interactable ~= true and interactable ~= false) then
				kiavcError('Invalid interactable state')
				return
			end
			self.interactable = interactable
			-- Tell the engine whether this object is interactable
			setObjectInteractable(self.id, interactable)
		end,
	setUi =
		function(self, ui)
			if ui == nil or (ui ~= true and ui ~= false) then
				kiavcError('Invalid UI value')
				return
			end
			self.ui = ui
			-- Tell the engine whether this object is part of the UI
			setObjectUi(self.id, ui)
		end,
	setUiPosition =
		function(self, x, y)
			if self.ui ~= true then
				kiavcError('Not an UI object')
				return
			end
			-- Tell the engine where to position this object if part of the UI
			setObjectUiPosition(self.id, x, y)
		end,
	scale =
		function(self, scale)
			self.scaleFactor = scale
			-- Tell the engine to use the provided z-plane for the object
			scaleObject(self.id, scale)
		end,
	setPlane =
		function(self, zplane)
			self.plane = zplane
			-- Tell the engine to use the provided z-plane for the object
			setObjectPlane(self.id, zplane)
		end,
	show =
		function(self)
			-- Tell the engine to show the object in the room they're in
			showObject(self.id)
		end,
	hide =
		function(self)
			-- Tell the engine the object will be invisible
			hideObject(self.id)
		end,
	fadeIn =
		function(self, ms)
			-- Tell the engine to fade the object in
			fadeObjectIn(self.id, ms)
		end,
	fadeOut =
		function(self, ms)
			-- Tell the engine to fade the object out
			fadeObjectOut(self.id, ms)
		end,
	use =
		function(self)
			-- Tell the engine to switch the actor to a "using" animation
			local where = 'mid'
			if self.interaction and self.interaction.use then
				where = self.interaction.use
			end
			if activeActor then
				setActorState(activeActor.id, 'use' .. where)
			end
		end,
	leftClick =
		function(self)
			-- If there's a left click handler, invoke that
			if activeActor == nil then
				return
			end
			if self.owner == nil and self.interaction ~= nil then
				activeActor:walkTo(self.interaction.x, self.interaction.y)
				waitFor(activeActor.id)
				activeActor:look(self.interaction.direction)
				waitMs(200)
			end
			if selectedObject ~= nil and self.onAction ~= nil and self.verbs[self.onAction] ~= nil then
				if self.onAction == 'useWith' and objectInteractions[selectedObject] and objectInteractions[selectedObject][self.id] == true then
					local obj = objects[selectedObject]
					obj.verbs.useWith(obj, self.id)
				else
					self.verbs[self.onAction](self, selectedObject)
				end
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
			if self.owner == nil and self.interaction ~= nil then
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
			kiavcLog("Starting object script '" .. name .. "'")
			self.scripts[name] = coroutine.create(func)
			return coroutine.resume(self.scripts[name])
		end,
	stopScript =
		function(self, name)
			if self.scripts[name] ~= nil then
				kiavcLog("Stopping object script '" .. name .. "'")
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
function Object:new(object)
	if object == nil then
		kiavcError('Invalid object')
		return nil
	end
	if object.id == nil then
		kiavcError('Missing object ID')
		return nil
	end
	if objects[object.id] ~= nil then
		kiavcError('Object ' .. object.id .. ' already registered')
		return nil
	end
	setmetatable(object, self)
	self.__index = self
	objects[object.id] = object
	-- Register the object at the engine
	registerObject(object)
	-- If an animation was provided, set it now
	if object.animation ~= nil then
		object:setAnimation(object.animation)
	end
	-- If an UI animation was provided, set it now
	if object.uiAnimation ~= nil then
		object:setUiAnimation(object.uiAnimation)
	end
	-- If an UI property was provided, set it now
	if object.ui ~= nil then
		object:setUi(object.ui)
	end
	-- If an interactable property was provided, set it now
	if object.interactable ~= nil then
		object:setInteractable(object.interactable)
	end
	-- If a scale was provided, set it now
	if object.scaleFactor ~= nil then
		object:scale(object.scaleFactor)
	end
	-- If a z-plane was provided, set it now
	if object.plane ~= nil then
		object:setPlane(object.plane)
	end
	-- If hover coordinates were provided, set them now
	if object.hover ~= nil then
		object:setHover(object.hover)
	end
	-- If interaction details were provided, set them now
	if object.interaction ~= nil then
		object:setInteraction(object.interaction)
	end
	-- If verbs were provided, set them now
	if object.verbs == nil then
		object.verbs = {}
	end
	for verb, action in pairs(object.defaultVerbs) do
		if object.verbs[verb] == nil then
			object.verbs[verb] = action
		end
	end
	object.scripts = {}
	return object
end

-- Helper function to set currently selected object
function setSelectedObject(id)
	kiavcLog(dumpTable(id))
	if id == nil then
		kiavcLog("Selected no object")
		selectedObject = nil
		if onSelectedObject then
			onSelectedObject()
		end
		return
	end
	local object = objects[id]
	if object == nil then
		kiavcError('No such object ' .. id)
		selectedObject = nil
		if onSelectedObject then
			onSelectedObject()
		end
		return
	end
	kiavcLog("Selected object '" .. id .. "'")
	selectedObject = id
	if onSelectedObject then
		onSelectedObject(id)
	end
end
