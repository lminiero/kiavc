-- Global properties
cursors = {}

-- Cursors class
Cursor = {
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
			-- Tell the engine about the new animation
			setCursorAnimation(self.id, animId)
		end,
	show =
		function(self)
			-- Tell the engine to show the cursor
			showCursor(self.id)
		end,
	hide =
		function(self)
			-- Tell the engine to hide the cursor
			hideCursor(self.id)
		end
}
function Cursor:new(cursor)
	if cursor == nil then
		kiavcError('Invalid cursor')
		return nil
	end
	if cursor.id == nil then
		kiavcError('Missing cursor ID')
		return nil
	end
	if cursors[cursor.id] ~= nil then
		kiavcError('Cursor ' .. cursor.id .. ' already registered')
		return nil
	end
	setmetatable(cursor, self)
	self.__index = self
	cursors[cursor.id] = cursor
	-- Register the cursor at the engine
	registerCursor(cursor)
	-- If an animation was provided, set it now
	if cursor.animation ~= nil then
		cursor:setAnimation(cursor.animation)
	end
	return cursor
end
