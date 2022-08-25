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
	-- If images/animations were provided, set them now
	local sets = { 'still', 'talking', 'walking', 'usehigh', 'usemid', 'uselow' }
	for _, type in ipairs(sets) do
		local set = costume[type]
		if set then
			for direction, anim in pairs(set) do
				costume:setAnimation(type, direction, anim)
			end
		end
		costume[type] = nil
	end
	return costume
end
