-- This is a fake room, which is actually a closeup of the letter we
-- have in the inventory when we read it. We have a paper-y canvas as
-- background, and then render text on top of it as if it were written.
-- A UI button in the top right corner allows us to go back.

-- This is our letter background
Image:new({ id = 'letter-bg', path = './assets/images/canvas.png' })
-- We use a different font for the letter
Font:new({ id = 'letter-font', path = './assets/fonts/notepen.ttf', size = 12 })

-- Fake object that we just click on to go back to the previous room
local back = Object:new({
	id = 'back',
	name = 'backName',
	hover = { x1 = 0, y1 = 0, x2 = 320, y2 = 180 },
	ui = true,
	verbs = {
		use = function(self)
			previousRoom:enter()
		end
	},
	onRightClick = 'use',
	onLeftClick = 'use'
})

-- Now we define the room itself: no walkbox or layer since this isn't
-- an actual room we walk in, but just a closeup on some detail.
local comingFrom = nil
Room:new({
	id = 'letter',
	background = 'letter-bg',
	onenter = function(self)
		-- When we enter the room, we print multiple text lines to
		-- show the contents of the letter as if it were written
		hideInventory()
		printLetter()
	end,
	onleave = function(self)
		fadeOut(250)
		waitFor('fade')
		-- Destroy the text we rendered
		for i = 1,3,1 do removeText('letter' .. i) end
		back:hide()
		-- Done
		showInventory()
		fadeIn(250)
	end
})

-- Helper function to print the content of the letter on the screen
function printLetter()
	showText({ id = 'letter1', font = 'letter-font', text = text('envelopeTutorial3'),
		color = black, x = 160, y = 10, absolute = true, plane = 40, duration = 0 })
	local content = nil
	for i = 5,14,1
	do
		if content == nil then
			content = text('envelopeTutorial' .. i)
		else
			content = content .. ' ' .. text('envelopeTutorial' .. i)
		end
	end
	showText({ id = 'letter2', font = 'letter-font', text = content,
		color = black, x = 160, y = 90, absolute = true, plane = 40, duration = 0 })
	showText({ id = 'letter3', font = 'letter-font', text = text('envelopeTutorial15'),
		color = black, x = 160, y = 170, absolute = true, plane = 40, duration = 0 })
	-- If this is the first time we read this letter, we also have the
	-- actor read it out loud, otherwise we just show the content
	if not state.readLetter then
		state.readLetter = 1
		for i = 3,15,1 do
			if i == 4 then
				activeActor:say(text('envelopeTutorial' .. i))
			else
				activeActor:say('"' .. text('envelopeTutorial' .. i) .. '"')
			end
			waitFor(activeActor.id)
		end
		state.readLetter = 2
		-- The first time we also go back to the previous room
		startScript(function()
			previousRoom:enter()
		end)
	else
		-- Finally, we show an object/icon to go back to the previous room
		back:show()
	end
end
