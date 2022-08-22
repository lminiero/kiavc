-- Global properties
local defaultLang = 'en'
lang = defaultLang

-- Helper function to parse a translations table and check if there
-- are missing translations for some of the available text lines
function checkTranslations()
	if translations == nil then
		kiavcError('No translations file')
		return
	end
	-- Look for the available languages first
	local langs = {}
	for _, tr in pairs(translations) do
		for l, _ in pairs(tr) do
			if langs[l] == nil then
				langs[l] = true
			end
		end
	end
	kiavcLog('Available translations: ' .. dumpTable(langs))
	-- Check if all sentences are translated
	local missing = 0
	for id, tr in pairs(translations) do
		for l, _ in pairs(langs) do
			if tr[l] == nil then
				missing = missing + 1
				kiavcWarn("Missing '" .. l .."' translation for '" .. id .. "'")
			end
		end
	end
end

-- Helper function to get the text in the right language: if the sentence
-- is not available in the current language, it falls back to the default
-- language instead; if that's missing too, a custom string is returned
function text(id)
	if lang == nil then
		lang = defaultLang
	end
	if translations == nil then
		-- If there's no translation file, we return the id itself
		return id
	end
	local sentence = translations[id]
	if sentence == nil then
		kiavcWarn("Couldn't find sentence in translation with id '" .. id .. "'")
		return '*Missing sentence*'
	end
	if sentence[lang] then
		-- Found
		return sentence[lang]
	end
	-- Not found, try to fall back to the default language
	if lang ~= defaultLang and sentence[defaultLang] then
		-- Found
		kiavcWarn("Sentence '" .. id .. "' not available in '" .. lang .. "', falling back to '" .. defaultLang .. "'")
		return sentence[defaultLang]
	end
	-- If we got here, there's no fallback either
	kiavcWarn("Sentence '" .. id .. "' not available in '" .. lang .. "' or fallback language")
	return '*Missing translation*'
end
