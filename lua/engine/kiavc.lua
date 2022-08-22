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
kiavcRequire('engine/translation')

-- Table where we keep the state of the dynamic variables
state = {}

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
	else
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
		coroutine.resume(co)
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
		coroutine.resume(co)
	end
end

-- Helper function to react to a walkbox trigger
function triggerWalkbox(id, name)
	kiavcLog("Walkbox '" .. name .. "' triggered in room '" .. id .. "'")
	local room = rooms[id]
	if room ~= nil then
		room:triggerWalkbox(name)
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
		debug.sethook(action, function()error("interrupted")end, "l")
		action = nil
	end
	action = coroutine.create(func)
	return coroutine.resume(action)
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
		coroutine.resume(co)
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
