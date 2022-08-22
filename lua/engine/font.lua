-- Global properties
fonts = {}

-- Fonts class
Font = {
	size = 1
}
function Font:new(font)
	if font == nil then
		kiavcError('Invalid font')
		return nil
	end
	if font.id == nil then
		kiavcError('Missing font ID')
		return nil
	end
	if font.path == nil then
		kiavcError('Missing font path')
		return nil
	end
	if font.size ~= nil and font.size < 1 then
		kiavcError('Invalid font size')
		return nil
	end
	if fonts[font.id] ~= nil then
		kiavcError('Font ' .. font.id .. ' already registered')
		return nil
	end
	setmetatable(font, self)
	self.__index = self
	fonts[font.id] = font
	-- Register the font at the engine
	registerFont(font)
	return font
end
