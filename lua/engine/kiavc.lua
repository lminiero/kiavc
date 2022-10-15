-- Include the definitions of all resources
kiavcRequire('engine/colors')
kiavcRequire('engine/animation')
kiavcRequire('engine/font')
kiavcRequire('engine/cursor')
kiavcRequire('engine/music')
kiavcRequire('engine/soundfx')
kiavcRequire('engine/room')
kiavcRequire('engine/actor')
kiavcRequire('engine/costume')
kiavcRequire('engine/object')
kiavcRequire('engine/dialog')
kiavcRequire('engine/translation')

-- Table where we keep the state of the dynamic variables
state = { dialogues = {} }

-- Functions to handle user input from the keyboard
inputTrigger = {}
function onUserInput(key, func)
	if key ~= nil and func ~= nil then
		inputTrigger[key] = func
	end
end
function userInput(key)
	if key ~= nil and inputTrigger[key] ~= nil then
		inputTrigger[key]()
	end
end

-- Function to handle hovering over things or people
local hoveringOn = nil
function hovering(id, on)
	-- Try objects first
	local target = objects[id]
	if target == nil then
		-- Try actors
		target = actors[id]
		if target == nil then
			-- Give up
			hoveringOn = nil
			if onHovering then onHovering() end
			return
		end
	end
	if on == true then
		-- FIXME
		hoveringOn = id
		if onHovering then onHovering(target) end
	elseif hoveringOn == id then
		hoveringOn = nil
		if onHovering then onHovering() end
	end
end

-- Function to handle left mouse clicks
function leftClick(x, y)
	kiavcLog('Left mouse button clicked: ' .. x .. 'x' .. y)
	-- Check if we were hovering on something
	startAction(function()
		if hoveringOn ~= nil then
			-- Try objects first
			local target = objects[hoveringOn]
			if target == nil then
				-- Try actors
				target = actors[hoveringOn]
				if target == nil then
					-- Give up
					return
				end
			end
			target:leftClick()
		else
			activeActor:walkTo(x, y)
		end
	end)
end

-- Function to handle right mouse clicks
function rightClick(x, y)
	kiavcLog('Right mouse button clicked: ' .. x .. 'x' .. y)
	-- Check if we were hovering on something
	startAction(function()
		if hoveringOn ~= nil then
			-- Try objects first
			local target = objects[hoveringOn]
			if target == nil then
				-- Try actors
				target = actors[hoveringOn]
				if target == nil then
					-- Give up
					setSelectedObject(nil)
					return
				end
			end
			target:rightClick()
		elseif selectedObject == nil then
			activeActor:walkTo(x, y)
		end
		setSelectedObject(nil)
	end)
end

-- Function to update the "world" and the current state, in order
-- to trigger callbacks to the engine for rendering things accordingly
currentTicks = 0
function updateWorld(ticks)
	-- TODO Update the world, state, etc.
	currentTicks = ticks
	-- Check if there's coroutines waiting on a timer
	checkScheduled()
	if action ~= nil and coroutine.status(action) == 'dead' then
		action = nil
	end
	if cutscene ~= nil and coroutine.status(cutscene) == 'dead' then
		cutscene = nil
		cutsceneEscape = nil
	end
end

-- Helper function to run a command from the core as a coroutine
function runCommand(cmd)
	if cmd == nil then return end
	local f = load(cmd)
	local co = coroutine.create(function()
		f()
	end)
	local res = { coroutine.resume(co) }
	while coroutine.status(co) ~= "dead" do
		res = { coroutine.resume(co, coroutine.yield()) }
	end
	if res[1] ~= true then
		kiavcError(res[2])
	end
end

-- The following is code to run scripts as coroutines, including helpers
-- to wait some time or wait for specific actions to occur in the engine
local scheduled = {}
local waiting = {}

-- Helper function to wait a specified amount of milliseconds
function waitMs(ms)
	local co = coroutine.running()
	if co == nil then return end
	local wakeUp = currentTicks + ms
	scheduled[co] = wakeUp
	return coroutine.yield(co)
end

-- Helper function to check when timed coroutines must be awakened
function checkScheduled()
	local toWake = {}
	for co, wakeUp in pairs(scheduled) do
		if wakeUp < currentTicks then
			table.insert(toWake, co)
		end
	end
	-- Awaken the coroutines that have waited long enough
	for _, co in ipairs(toWake) do
		scheduled[co] = nil
		if coroutine.status(co) == "dead" then return end
		local res = { coroutine.resume(co) }
		if coroutine.status(co) == "dead" and res[1] ~= true then
			kiavcError(res[2])
		end
	end
end

-- Helper function to wait for a specific event
function waitFor(event)
	local co = coroutine.running()
	if co == nil then return end
	if waiting[event] == nil then
		waiting[event] = { co }
	else
		table.insert(waiting[event], co)
	end
	return coroutine.yield(co)
end

-- Helper function to signal a specific event
function signal(event)
	kiavcLog("Got '" .. event .. "' event")
	local toWake = waiting[event]
	if toWake == nil then
		return
	end
	waiting[event] = nil
	for _, co in ipairs(toWake) do
		if coroutine.status(co) == "dead" then return end
		local res = { coroutine.resume(co) }
		if coroutine.status(co) == "dead" and res[1] ~= true then
			kiavcError(res[2])
		end
	end
end

-- Helper function to react to a walkbox trigger
function triggerWalkbox(id, name, actor)
	kiavcLog("Walkbox '" .. name .. "' triggered in room '" .. id .. "' by actor '" .. actor .. "'")
	local room = rooms[id]
	if room ~= nil then
		room:triggerWalkbox(name, actor)
	end
end

-- Helper function to run functions as coroutines
function startScript(func)
	local co = coroutine.create(func)
	return coroutine.resume(co)
end

-- As above, but meant for actor actions, and so interruptable
action = nil
function startAction(func)
	if action ~= nil then
		debug.sethook(action, function()error("Action interrupted")end, "l")
		if coroutine.status(action) == 'suspended' then
			coroutine.resume(action)
		end
		action = nil
	end
	action = coroutine.create(func)
	return coroutine.resume(action)
end

-- As above, but meant for cutscenes, and so interruptable
cutscene = nil
cutsceneEscape = nil
function playCutscene(func, funcEscape)
	if cutscene ~= nil then
		endCutscene()
	end
	cutscene = coroutine.create(func)
	cutsceneEscape = funcEscape
	return coroutine.resume(cutscene)
end
function endCutscene()
	if cutscene ~= nil then
		debug.sethook(cutscene, function()error("Cutscene interrupted")end, "l")
		if coroutine.status(cutscene) == 'suspended' then
			coroutine.resume(cutscene)
		end
		if cutsceneEscape ~= nil then cutsceneEscape() end
	end
	cutscene = nil
	cutsceneEscape = nil
end

-- Helper function to wait for a dialog choice
waitingDialog = {}
selectedDialog = {}
function waitDialog(id)
	local co = coroutine.running()
	if co == nil then return end
	if waitingDialog[id] == nil then
		waitingDialog[id] = { co }
	else
		table.insert(waitingDialog[id], co)
	end
	selectedDialog[id] = nil
	return coroutine.yield(co)
end

-- Helper function to signal a dialog line has been selected
function dialogSelected(id, name)
	kiavcLog("Line '" .. name .. "' has been selected in dialog '" .. id .. "'")
	local toWake = waitingDialog[id]
	if toWake == nil then
		return
	end
	selectedDialog[id] = name
	waitingDialog[id] = nil
	for _, co in ipairs(toWake) do
		if coroutine.status(co) == "dead" then return end
		local res = { coroutine.resume(co) }
		if coroutine.status(co) == "dead" and res[1] ~= true then
			kiavcError(res[2])
		end
	end
end

-- Helper for logging tables
-- https://stackoverflow.com/a/27028488
function dumpTable(o)
	if type(o) == 'table' then
		local s = '{ '
		for k,v in pairs(o) do
			if type(k) ~= 'number' then k = '"'..k..'"' end
			s = s .. '['..k..'] = ' .. dumpTable(v) .. ','
		end
		return s .. '} '
	else
		return tostring(o)
	end
end
