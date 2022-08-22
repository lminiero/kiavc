-- Global properties
animations = {}

-- Animations class
Animation = {
	frames = 1,
	r = nil, g = nil, b = nil
}
function Animation:new(anim)
	if anim == nil then
		kiavcError('Invalid animation')
		return nil
	end
	if anim.id == nil then
		kiavcError('Missing animation ID')
		return nil
	end
	if anim.path == nil then
		kiavcError('Missing animation path')
		return nil
	end
	if anim.frames ~= nil and anim.frames < 1 then
		kiavcError('Invalid animations frames value')
		return nil
	end
	if animations[anim.id] ~= nil then
		kiavcError('Animation ' .. anim.id .. ' already registered')
		return nil
	end
	setmetatable(anim, self)
	self.__index = self
	animations[anim.id] = anim
	-- Register the image at the engine
	registerAnimation(anim)
	return anim
end

-- Create an Image alias
Image = Animation
