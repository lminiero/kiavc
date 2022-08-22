-- Global properties
costumes = {}

-- Costumes class
Costume = {
	animations = {},
	setAnimation =
		function(self, type, direction, animId)
			if animId == nil then
				kiavcError('Invalid animation ID')
				return
			end
			if animations[animId] == nil then
				kiavcError('Invalid animation ID ' .. animId)
				return
			end
			if self.animations[type] == nil then
				self.animations[type] = {}
			end
			self.animations[type][direction] = animId
			-- Tell the engine about the new directional anim
			setCostumeAnimation(self.id, type, direction, animId)
		end
}
function Costume:new(costume)
	if costume == nil then
		kiavcError('Invalid costume')
		return nil
	end
	if costume.id == nil then
		kiavcError('Missing costume ID')
		return nil
	end
	if costumes[costume.id] ~= nil then
		kiavcError('Costume ' .. costume.id .. ' already registered')
		return nil
	end
	setmetatable(costume, self)
	self.__index = self
	costumes[costume.id] = costume
	-- Register the costume at the engine
	registerCostume(costume)
	-- If still images were provided, set them now
	if costume.still ~= nil then
		local still = costume.still
		costume.still = nil
		for direction, anim in pairs(still) do
			costume:setAnimation('still', direction, anim)
		end
	end
	-- If walking animations were provided, set them now
	if costume.walking ~= nil then
		local walking = costume.walking
		costume.walking = nil
		for direction, anim in pairs(walking) do
			-- FIXME
			costume:setAnimation('walking', direction, anim)
		end
	end
	-- If talking animations were provided, set them now
	if costume.talking ~= nil then
		local talking = costume.talking
		costume.talking = nil
		for direction, anim in pairs(talking) do
			-- FIXME
			costume:setAnimation('talking', direction, anim)
		end
	end
	return costume
end
