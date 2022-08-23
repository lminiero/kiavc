-- We may want to make sure we're using a specific version of the engine
major, minor, patch = getVersion()
if major ~= 0 or minor ~= 1 or patch ~= 0 then
	kiavcLog('Unsupported KIAVC engine version')
	return
end

-- Let's load all the resources first
kiavcRequire('game/resources')

-- After that, we should set the resolution, fps and scaling. The
-- resolution is what we work on internally (for backgrounds, sprites,
-- animations, etc.), while scaling how much this should be scaled
-- when presented as an actual window: for instance, we can choose to
-- work on a 320x200 canvas, but then scale that three times larger.
-- The fps determines how often we draw, so the frames per second: note
-- this only impacts the rendering frequency, NOT the engine speed
setResolution({ width = 320, height = 180, fps = 60, scale = 4 })

-- We can also specify the title of the window
setTitle('KIAVC engine demo')

-- We can also specify the window icon
--~ setIcon('./assets/images/icon.png')

-- We enable the scripting console, and specify which font to use
Font:new({ id = 'console', path = './assets/fonts/orangekid.ttf', size = 24 })
enableConsole('console')

-- Let's intercept some keys to trigger some actions
function scaleScreen(size)
	setResolution({ width = 320, height = 180, fps = 60, scale = size })
end
onUserInput('1', function()
	scaleScreen(1)
end)
onUserInput('2', function()
	scaleScreen(2)
end)
onUserInput('3', function()
	scaleScreen(3)
end)
onUserInput('4', function()
	scaleScreen(4)
end)
onUserInput('5', function()
	scaleScreen(5)
end)
onUserInput('6', function()
	scaleScreen(6)
end)
onUserInput('F', function()
	if fullscreen == nil or fullscreen == false then
		fullscreen = true
	else
		fullscreen = false
	end
	setFullscreen(fullscreen)
end)
onUserInput('F8', showConsole)
onUserInput('F10', function()
	if scanlines == nil or scanlines == false then
		scanlines = true
	else
		scanlines = false
	end
	setScanlines(scanlines)
end)
onUserInput('F11', function()
	if walkboxes == nil or walkboxes == false then
		walkboxes = true
	else
		walkboxes = false
	end
	debugWalkboxes(walkboxes)
end)
onUserInput('F12', function()
	saveScreenshot('./screenshot-' .. currentTicks .. '.png')
end)
onUserInput('Escape', quit)
onUserInput('Q', quit)
onUserInput('.', skipActorsText)

-- Let's configure what to do when we're hovering on something
onHovering = function(target)
	if target == nil then
		-- Hide the cursor text
		hideCursorText()
	else
		-- Show the actor/object name on the cursor
		local name = text(target.name)
		showCursorText({
			font = 'cursor-font',
			text = name,
			color = white,
			outline = black
		})
	end
end

-- Let's configure what to do when an object is selected or deselected
onSelectedObject = function()
	if selectedObject == nil then
		-- Let's reset the cursors
		setMainCursor('cursor-main')
		setHotspotCursor('cursor-hotspot')
	else
		local object = objects[selectedObject]
		if object then
			local objCursor = object.id .. '-cursor'
			if cursors[objCursor] == nil then
				Cursor:new({ id = objCursor, animation = object.animation })
			end
			setMainCursor(objCursor)
			setHotspotCursor(objCursor)
		end
	end
end

-- Let's start now: we start from the Melee harbour, and show an intro
setMainCursor('cursor-main')
setHotspotCursor('cursor-hotspot')
showCursor()
setActiveActor('detective')
activeActor:follow()
activeActor:moveTo('street', 148, 148)
activeActor:show()
rooms['street']:enter()

-- This is the script we'll use for the intro cutscene: we have the
-- main actor come in from the right, walk to a specific coordinate,
-- look around and then say something. We use the startCutscene() and
-- stopCutscene() commands to hide the cursor and prevent interaction.
function intro()
	startCutscene()
	fadeIn(1000)
	activeActor:walkTo(148, 170)
	waitFor(activeActor.id)
	waitMs(800)
	activeActor:look('right')
	waitMs(800)
	activeActor:look('left')
	waitMs(800)
	activeActor:look('right')
	waitMs(800)
	activeActor:look('down')
	activeActor:say(text('intro'))
	waitFor(activeActor.id)
	stopCutscene()
end
-- Start the cutscene as soon as the game starts
startScript(intro)
