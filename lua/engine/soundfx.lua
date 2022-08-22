-- Global properties
soundfxs = {}

-- SoundFX class
SoundFX = {
	play =
		function(self, fadeInMs)
			if fadeInMs == nil or fadeInMs < 0 then
				fadeInMs = 0
			end
			-- Tell the engine to start playing the sound effect
			playAudio(self.id, fadeInMs, false)
		end,
	pause =
		function(self)
			-- Tell the engine to pause the sound effect
			pauseAudio(self.id)
		end,
	resume =
		function(self)
			-- Tell the engine to resume the sound effect
			resumeAudio(self.id)
		end,
	stop =
		function(self, fadeOutMs)
			if fadeOutMs == nil or fadeOutMs < 0 then
				fadeOutMs = 0
			end
			-- Tell the engine to stop playing the sound effect
			stopAudio(self.id, fadeOutMs)
		end
}
function SoundFX:new(sfx)
	if sfx == nil then
		kiavcError('Invalid sound effect')
		return nil
	end
	if sfx.id == nil then
		kiavcError('Missing sound effect ID')
		return nil
	end
	if sfx.path == nil then
		kiavcError('Missing sound effect path')
		return nil
	end
	if soundfxs[sfx.id] ~= nil then
		kiavcError('SoundFX ' .. sfx.id .. ' already registered')
		return nil
	end
	setmetatable(sfx, self)
	self.__index = self
	soundfxs[sfx.id] = sfx
	-- Register the sfx track at the engine
	registerAudio(sfx)
	return sfx
end
