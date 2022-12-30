-- Global properties
rooms = {}

-- Rooms class
Room = {
	layers = {},
	actors = {},
	objects = {},
	walkboxes = {},
	triggers = {},
	enter =
		function(self)
			-- If there was a previous room, tell it to leave
			nextRoom = self
			if activeRoom ~= nil then
				activeRoom:leave()
			end
			nextRoom = nil
			-- Show the room and mark it as the active one
			activeRoom = self
			showRoom(self.id)
			-- Finally, invoke the onenter callback, if configured
			if self.onenter ~= nil then
				self:onenter()
			end
		end,
	leave =
		function(self)
			-- Stop all the room scripts, if any
			self:stopAllScripts()
			-- Invoke the onleave callback, if configured
			previousRoom = self
			if self.onleave ~= nil then
				self:onleave()
			end
		end,
	setBackground =
		function(self, imageId)
			if imageId == nil then
				kiavcError('Invalid image ID')
				return
			end
			if animations[imageId] == nil then
				kiavcError('Invalid image ID ' .. imageId)
				return
			end
			self.background = imageId
			-- Tell the engine about the new background
			setRoomBackground(self.id, imageId)
		end,
	addLayer =
		function(self, layer)
			if layer == nil then
				kiavcError('Invalid layer')
				return
			end
			if layer.id == nil then
				kiavcError('Invalid layer ID')
				return
			end
			if layer.image == nil then
				kiavcError('Invalid image ID')
				return
			end
			if animations[layer.image] == nil then
				kiavcError('Invalid image ID ' .. layer.image)
				return
			end
			table.insert(self.layers, layer)
			-- Tell the engine about the new layer
			addRoomLayer(self.id, layer.id, layer.image, layer.plane)
		end,
	removeLayer =
		function(self, layerId)
			if layerId == nil then
				kiavcError('Invalid layer ID')
				return
			end
			-- Tell the engine to remove the layer
			removeRoomLayer(self.id, layerId)
		end,
	addWalkbox =
		function(self, walkbox)
			if walkbox == nil then
				kiavcError('Invalid walkbox')
				return
			end
			if walkbox.x1 == nil or walkbox.y1 == nil or walkbox.x2 == nil or walkbox.y2 == nil then
				kiavcError('Invalid walkbox coordinates')
				return
			end
			table.insert(self.walkboxes, walkbox)
			-- Tell the engine about the new walkbox
			addRoomWalkbox(self.id, walkbox)
		end,
	enableWalkbox =
		function(self, walkboxId)
			if walkboxId == nil then
				kiavcError('Invalid walkbox')
				return
			end
			for _, walkbox in ipairs(self.walkboxes) do
				if walkbox.id == walkboxId then
					walkbox.disabled = nil
					break
				end
			end
			-- Tell the engine this walkbox is now enabled
			enableRoomWalkbox(self.id, walkboxId)
		end,
	disableWalkbox =
		function(self, walkboxId)
			if walkboxId == nil then
				kiavcError('Invalid walkbox')
				return
			end
			for _, walkbox in ipairs(self.walkboxes) do
				if walkbox.id == walkboxId then
					walkbox.disabled = true
					break
				end
			end
			-- Tell the engine this walkbox is now disabled
			disableRoomWalkbox(self.id, walkboxId)
		end,
	triggerWalkbox =
		function(self, walkboxId, actorId)
			if walkboxId == nil then
				kiavcError('Invalid walkbox')
				return
			end
			if self.triggers[walkboxId] ~= nil then
				self:startScript(walkboxId, self.triggers[walkboxId], actorId)
			end
		end,
	show =
		function(self)
			-- Tell the engine this is the current room
			showRoom(self.id)
		end,
	startScript =
		function(self, name, func, arg)
			-- If a script with that name exists already, stop it
			self:stopScript(name)
			-- Start the script as a coroutine
			kiavcLog("Starting room script '" .. name .. "'")
			self.scripts[name] = coroutine.create(func)
			return coroutine.resume(self.scripts[name], arg)
		end,
	stopScript =
		function(self, name)
			if self.scripts[name] ~= nil then
				kiavcLog("Stopping room script '" .. name .. "'")
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
function Room:new(room)
	if room == nil then
		kiavcError('Invalid room')
		return nil
	end
	if room.id == nil then
		kiavcError('Missing room ID')
		return nil
	end
	if rooms[room.id] ~= nil then
		kiavcError('Room ' .. room.id .. ' already registered')
		return nil
	end
	setmetatable(room, self)
	self.__index = self
	rooms[room.id] = room
	-- Register the room at the engine
	registerRoom(room)
	-- If a background was provided, set it now
	if room.background ~= nil then
		room:setBackground(room.background)
	end
	-- If layers were provided, set them now
	if room.layers ~= nil then
		local layers = room.layers
		room.layers = {}
		for _, layer in ipairs(layers) do
			room:addLayer(layer)
		end
	end
	-- If walkboxes were provided, set them now
	if room.walkboxes ~= nil then
		local walkboxes = room.walkboxes
		room.walkboxes = {}
		for _, walkbox in ipairs(walkboxes) do
			room:addWalkbox(walkbox)
		end
	end
	recalculateRoomWalkboxes(room.id)
	room.scripts = {}
	return room
end
