-- Dialogue tree engine

function enterDialog(dialog)
	-- Make sure settings have been provided
	if dialog.settings == nil then
		kiavcErr('Invalid dialogue tree (missing settings)')
		return
	end
	if dialog.settings.id == nil then
		kiavcErr('Invalid dialogue tree (missing dialogue ID)')
		return
	end
	if dialog.settings.font == nil then
		kiavcErr('Invalid dialogue tree (missing dialogue font)')
		return
	end
	if dialog.settings.color == nil or dialog.settings.selected == nil then
		kiavcErr('Invalid dialogue tree (missing dialogue colors)')
		return
	end
	if dialog.settings.background == nil then
		kiavcErr('Invalid dialogue tree (missing dialogue background)')
		return
	end
	if dialog.settings.area == nil then
		kiavcErr('Invalid dialogue tree (missing dialogue area)')
		return
	end
	-- Make sure we have a main too
	if dialog.main == nil then
		kiavcErr('Invalid dialogue tree (missing main entrypoint)')
		return
	end
	-- FIXME Quickly validate the other blocks now
	for name, block in pairs(dialog) do
		if name ~= "settings" then
			if block.options == nil and block.steps == nil then
				kiavcErr('Invalid dialogue tree (no options or steps in block ' .. name .. ')')
				return
			end
			if block.options ~= nil and block.steps ~= nil then
				kiavcErr('Invalid dialogue tree (block ' .. name .. ' has both options and steps)')
				return
			end
			if block.options ~= nil and block.options[1] == nil then
				kiavcErr('Invalid dialogue tree (options is not an array in block ' .. name .. ')')
				return
			end
			if block.steps ~= nil and block.steps[1] == nil then
				kiavcErr('Invalid dialogue tree (steps is not an array in block ' .. name .. ')')
				return
			end
		end
	end
	-- Create an item in the game state
	local dialogId = dialog.settings.id
	if state.dialogues[dialogId] == nil then
		state.dialogues[dialogId] = { opened = 0 }
	end
	state.dialogues[dialogId].opened = state.dialogues[dialogId].opened + 1
	-- Run the dialogue now
	local running = true
	local current = "main"
	while(running) do
		-- Get the current block
		local block = dialog[current]
		if state.dialogues[dialogId][current] == nil then
			state.dialogues[dialogId][current] = {}
		end
		if block.options ~= nil then
			-- We have options to display, check which ones to pass to the engine
			local count = 0
			local lines = {}
			for _, option in ipairs(block.options) do
				if count == 4 then
					-- We only display maximum 4 options per dialogue
					break
				end
				if option.notif then
					-- This only has to be displayed if not matching a specific state
					for name, value in pairs(option.notif) do
						if state[name] == value then
							goto continue
						end
					end
				end
				if option.onlyif then
					-- This only has to be displayed if matching a specific state
					for name, value in pairs(option.onlyif) do
						if state[name] ~= value then
							goto continue
						end
					end
				end
				if option.once == true and state.dialogues[dialogId][current][option.name] and
						state.dialogues[dialogId][current][option.name].selected > 0 then
					-- This has to be displayed only if not previously selected, which happened
					goto continue
				end
				if state.dialogues[dialogId][current][option.name] == nil then
					state.dialogues[dialogId][current][option.name] = { displayed = 0, selected = 0 }
				end
				state.dialogues[dialogId][current][option.name].displayed = state.dialogues[dialogId][current][option.name].displayed + 1
				-- Add a new dialogue line that can be selected
				count = count + 1
				local dialogLine = { id = dialogId }
				for name, value in pairs(option) do
					if name == 'text' or name == 'say' then
						dialogLine[name] = text(value)
					else
						dialogLine[name] = value
					end
				end
				lines[#lines+1] = dialogLine
				::continue::
			end
			local option = nil
			if count > 1 then
				-- Prepare the dialogue in the engine
				startDialog(dialog.settings)
				-- Add the selected options as dialogue lines in the engine
				for _, line in ipairs(lines) do
					addDialogLine(line)
				end
				-- Wait for the dialogue selection
				kiavcLog("Waiting for dialogue '" .. dialogId .. "'")
				waitDialog(dialogId)
				kiavcLog("Dialog '" .. dialogId .. "' awaken")
				local s = selectedDialog[dialogId]
				kiavcLog('  -- Option ' .. s .. ' selected')
				for _, o in ipairs(lines) do
					if o.name == s then
						option = o
						break
					end
				end
			else
				-- There's only one option, go with that right away
				option = lines[1]
				kiavcLog('  -- Option ' .. option.name .. ' selected (only one available)')
			end
			if option then
				state.dialogues[dialogId][current][option.name].selected = state.dialogues[dialogId][current][option.name].selected + 1
				-- Say the selected line as the active actor: if "say" exists, we
				-- use that, otherwise we print the "text" used for the dialogue
				if option.look then
					-- Have the actor look in the specified direction
					activeActor:look(option.look)
				end
				if option.say then
					activeActor:say(option.say)
				else
					activeActor:say(option.text)
				end
				waitFor(activeActor.id)
				if option.state then
					-- FIXME Set a game variable
					for name, value in pairs(option.state) do
						state[name] = value
					end
				end
				-- Check if there's a "next": if so, change block
				if option.next then
					kiavcLog('Moving to block ' .. option.next)
					current = option.next
					goto changeblock
				end
			end
		else
			-- We have a series of steps to perform
			for _, step in ipairs(block.steps) do
				if step.notif then
					-- This only has to be performed if not matching a specific state
					for name, value in pairs(step.notif) do
						if state[name] == value then
							goto skip
						end
					end
				end
				if step.onlyif then
					-- This only has to be performed if matching a specific state
					for name, value in pairs(step.onlyif) do
						if state[name] ~= value then
							goto skip
						end
					end
				end
				-- Check which actor will perform this step
				local actor = activeActor
				if step.actor then
					actor = actors[step.actor]
				end
				if step.look then
					-- Have the actor look in the specified direction
					actor:look(step.look)
				end
				if step.say then
					-- Have the actor say something
					actor:say(text(step.say))
					waitFor(actor.id)
				end
				if step.sleep then
					-- Wait for the specified amount of time
					waitMs(step.sleep)
				end
				if step.state then
					-- FIXME Set a game variable
					for name, value in pairs(step.state) do
						state[name] = value
					end
				end
				if step.load then
					-- We have a Lua command to execute
					local f = load(step.load)
					f()
				end
				-- Check if there's a "next": if so, change block
				if step.next then
					kiavcLog('Moving to block ' .. step.next)
					current = step.next
					goto changeblock
				end
				-- If we're done, stop here
				if step.exitDialog == true then
					stopDialog(dialogId)
					return
				end
				::skip::
			end
		end
		::changeblock::
	end
end
