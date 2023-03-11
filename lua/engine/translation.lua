-- Global properties
local defaultLang = 'en'
lang = defaultLang

-- Helper function to get the text in the right language: if the sentence
-- is not available in the current language, it falls back to the default
-- language instead; if that's missing too, a custom string is returned
function text(id)
	if lang == nil then
		lang = defaultLang
	end
	if loadedLang == nil or loadedLang ~= lang then
		-- Load the translation file for this language
		kiavcRequire('game/localization/text-' .. lang)
		if loadedLang == nil or loadedLang ~= lang then
			-- Translation file doesn't exist?
			kiavcWarn("No such translation file for '" .. lang .. "'")
			if loadedLang == nil then
				loadedLang = lang
			else
				lang = loadedLang
			end
		end
	end
	if translations == nil then
		-- If there's no translation file, we return the id itself
		return id
	end
	local sentence = translations[id]
	if sentence == nil then
		kiavcWarn("Couldn't find '" .. lang .. "' sentence with label '" .. id .. "'")
		return '*Missing sentence*'
	end
	-- Done
	return sentence
end
