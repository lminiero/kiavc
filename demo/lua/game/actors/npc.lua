-- Here we register a different actor instance, and since I'm cheap and
-- I can't draw, we'll use the same costume as the detective (main actor),
-- so we'll call him his twin. Since this is an NPC, we configure some
-- additional properties for this actor, like where to place them, how
-- to interact with them, and stuff like that.

-- Now let's register the actor and their settings
local npc = Actor:new({
	id = 'npc',
	name = 'npcName',
	description = 'npcDesc',
	costume = 'detective-costume',
	font = 'dialogues',
	textColor = cyan,
	interaction = { direction = 'right', x = 420, y = 180 },
	plane = 5,
	scaleFactor = 0.76,
	verbs = {
		talkTo = function(self)
			if state.npcSaidHi and state.npcAskedWhat and state.npcAskedWhere then
				activeActor:look('down')
				activeActor:say(text('npcTalk'))
			else
				rooms['street']:stopScript('searching')
				startScript(npcDialog)
				self:look('left')
			end
		end,
		giveTo = function(self, object)
			disableInput()
			rooms['street']:stopScript('searching')
			self:look('left')
			activeActor:say(text("npcGiveSkull1"))
			waitFor(activeActor.id)
			if object == 'skull' then
				self:say(text("npcGiveSkull2"))
				waitFor(self.id)
			end
			self:say(text("npcGiveSkull3"))
			waitFor(self.id)
			rooms['street']:startScript('searching', npcSearchingScript)
			enableInput()
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'talkTo'
})
-- By default, we put the actor in the streets room
npc:moveTo('street', 460, 180)
npc:show()

-- Basic test of interaction with the npc
function npcDialog()
	while(true) do
		-- Let's show the dialog on top, with a semitransparent background
		startDialog({ id = 'test', font = 'dialogues', color = blue, selected = cyan,
			background = { r = 0, g = 0, b = 0, a = 128 },
			area = { x1 = 0, y1 = 144, x2 = 320, y2 = 180 } })
		local lines = {
			{ name = "1", text = "npcTalk1", say = "npcTalk1" },
			{ name = "2", text = "npcTalk2", say = "npcTalk2" },
			{ name = "3", text = "npcTalk3", say = "npcTalk3" },
			{ name = "4", text = "npcTalk4", say = "npcTalk4" },
		}
		for _, line in ipairs(lines) do
			if line.name == "1" and state.npcSaidHi then goto continue end
			if line.name == "2" and state.npcAskedWhat then goto continue end
			if line.name == "3" and state.npcAskedWhere then goto continue end
			addDialogLine({ id = 'test', name = line.name, text = text(line.text) })
			::continue::
		end
		kiavcLog('Waiting for dialog')
		waitDialog('test')
		kiavcLog('Dialog awaken')
		local s = selectedDialog['test']
		kiavcLog('  -- Line ' .. s)
		local line = nil
		for _, l in ipairs(lines) do
			if l.name == s then
				line = l
				break
			end
		end
		if line then
			activeActor:say(text(line.say))
			waitFor(activeActor.id)
			if s == '1' then
				state.npcSaidHi = true
				npc:say(text("npcResp11"))
				waitFor(npc.id)
				waitMs(500)
				npc:say(text("npcResp12"))
				waitFor(npc.id)
				npc:say(text("npcResp13"))
				waitFor(npc.id)
			elseif s == '2' then
				state.npcAskedWhat = true
				npc:say(text("npcResp21"))
				waitFor(npc.id)
				npc:say(text("npcResp22"))
				waitFor(npc.id)
			elseif s == '3' then
				state.npcAskedWhere = true
				npc:say(text("npcResp31"))
				waitFor(npc.id)
			elseif s == '4' then
				stopDialog('test')
				npc:say(text("npcResp41"))
				waitFor(npc.id)
				rooms['street']:startScript('searching', npcSearchingScript)
				break
			end
		end
	end
end
