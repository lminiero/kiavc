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
				startScript(startNpcDialog)
				self:look('left')
			end
		end,
		giveTo = function(self, object)
			disableInput()
			rooms['street']:stopScript('searching')
			self:look('left')
			activeActor:say(text("npcGiveSkull1"))
			waitFor(activeActor.id)
			activeActor:setState('usemid')
			if object == 'skull' then
				self:say(text("npcGiveSkull2"))
				waitFor(self.id)
			end
			self:say(text("npcGiveSkull3"))
			waitFor(self.id)
			rooms['street']:startScript('searching', npcSearchingScript)
			activeActor:setState('still')
			enableInput()
		end
	},
	onRightClick = 'lookAt',
	onLeftClick = 'talkTo'
})
-- By default, we put the actor in the streets room
npc:moveTo('street', 460, 180)
npc:show()

-- Basic test of interaction with the NPC: first of all we create the
-- dialog tree, starting with the settings and the point of entry (main),
-- and then we define all the blocks the dialogue may go through. At the
-- end, we pass this object to the enterDialog() function to run it.
local npcDialog = {
	-- All dialogue objects need settings: this is the object that is
	-- automatically passed to the startDialog() core engine function
	settings = { id = 'test', font = 'dialogues', color = blue, selected = cyan,
		background = { r = 0, g = 0, b = 0, a = 128 },
		area = { x1 = 0, y1 = 144, x2 = 320, y2 = 180 } },
	-- All dialogue objects also need a "main" block, which acts as an
	-- entry point for the dialogue: it could present some options that
	-- the user can pick (as the example below) or just some steps that
	-- introduce the conversation before actually making it interactive
	main = {
		options = {
			{ name = "1", text = "npcTalk1", once = true, next = "greeting" },
			{ name = "2", text = "npcTalk2", notif = { npcAskedWhat = true }, next = "looking" },
			{ name = "3", text = "npcTalk3", once = true, next = "clone" },
			{ name = "4", text = "npcTalk4", next = "done" }
		}
	},
	-- All the blocks below are possible iterations in the dialog: any
	-- of them contain either a series of steps (actors doing or saying
	-- something), or options that can be presented to the user. Adding
	-- a "next" property to one of the items moves to the selected block
	greeting = {
		steps = {
			{ actor = "npc", say = "npcResp1_1" },
			{ actor = "npc", say = "npcResp1_2" },
			{ actor = "npc", say = "npcResp1_3", state = { npcSaidHi = true }, next = "main" }
		}
	},
	looking = {
		steps = {
			{ actor = "npc", say = "npcResp2_1" },
			{ actor = "npc", say = "npcResp2_2" },
			{ say = "npcTalk2_mh", next = "list" }
		}
	},
	list = {
		options = {
			{ name = "1", text = "npcTalk2_list1", once = true, next = "no" },
			{ name = "2", text = "npcTalk2_list2", once = true, next = "no" },
			{ name = "3", text = "npcTalk2_list3", once = true, next = "no" },
			{ name = "4", text = "npcTalk2_list4", once = true, next = "no" },
			{ name = "5", text = "npcTalk2_list5", once = true, next = "nohat" },
			{ name = "6", text = "npcTalk2_list6", once = true, next = "noglasses" },
			{ name = "7", text = "npcTalk2_list7", once = true, next = "no" },
			{ name = "8", text = "npcTalk2_list8", once = true, next = "no" },
			{ name = "9", text = "npcTalk2_list9", once = true, next = "no" },
			{ name = "10", text = "npcTalk2_list10", once = true, next = "no" },
			{ name = "11", text = "npcTalk2_list11", once = true, next = "no" },
			{ name = "12", text = "npcTalk2_list12", once = true, next = "no" },
			{ name = "13", text = "npcTalk2_list13", once = true, next = "no" },
			{ name = "14", text = "npcTalk2_list14", once = true, next = "no" },
			{ name = "15", text = "npcTalk2_list15", state = { npcAskedWhat = true }, next = "main" },
		}
	},
	no = {
		steps = {
			{ actor = "npc", say = "npcResp2_no", next = "list" },
		}
	},
	nohat = {
		steps = {
			{ actor = "npc", say = "npcResp2_nohat1" },
			{ actor = "npc", look = "down", sleep = 500 },
			{ actor = "npc", look = "left", say = "npcResp2_nohat2" },
			{ actor = "npc", sleep = 1000 },
			{ actor = "npc", say = "npcResp2_nohat3", next = "list" },
		}
	},
	noglasses = {
		steps = {
			{ actor = "npc", say = "npcResp2_noglasses", next = "list" },
		}
	},
	clone = {
		steps = {
			{ actor = "npc", say = "npcResp3_1", state = { npcAskedWhere = true }, next = "main" }
		}
	},
	-- This is our way out, since there's an "exitDialog = true" item
	done = {
		steps = {
			{ actor = "npc", say = "npcResp4_1" },
			{ load = "rooms['street']:startScript('searching', npcSearchingScript)", exitDialog = true }
		}
	}
}

-- Actually run the dialogue defined in the npcDialog object
function startNpcDialog()
	enterDialog(npcDialog)
end
