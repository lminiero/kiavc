-- Global properties
musics = {}

-- Music class
Music = {
	play =
		function(self, fadeInMs)
			if fadeInMs == nil or fadeInMs < 0 then
				fadeInMs = 0
			end
			-- Tell the engine to start playing the track
			playAudio(self.id, fadeInMs, true)
		end,
	pause =
		function(self)
			-- Tell the engine to pause the track
			pauseAudio(self.id)
		end,
	resume =
		function(self)
			-- Tell the engine to resume the track
			resumeAudio(self.id)
		end,
	stop =
		function(self, fadeOutMs)
			if fadeOutMs == nil or fadeOutMs < 0 then
				fadeOutMs = 0
			end
			-- Tell the engine to stop playing the track
			stopAudio(self.id, fadeOutMs)
		end
}
function Music:new(music)
	if music == nil then
		kiavcError('Invalid music track')
		return nil
	end
	if music.id == nil then
		kiavcError('Missing music ID')
		return nil
	end
	if music.path == nil then
		kiavcError('Missing music path')
		return nil
	end
	if musics[music.id] ~= nil then
		kiavcError('Music ' .. music.id .. ' already registered')
		return nil
	end
	setmetatable(music, self)
	self.__index = self
	musics[music.id] = music
	-- Register the music track at the engine
	registerAudio(music)
	return music
end
