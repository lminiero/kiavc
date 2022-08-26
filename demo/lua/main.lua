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

-- A few other things we may want to do by default
--~ grabMouse(true)
--~ fullscreen = true
--~ setFullscreen(fullscreen)
--~ scanlines = true
--~ setScanlines(scanlines)

-- We enable the scripting console, and specify which font to use
Font:new({ id = 'console', path = './assets/fonts/orangekid.ttf', size = 24 })
enableConsole('console')

-- Let's intercept some keys to trigger some actions
function scaleScreen(size)
	setResolution({ width = 320, height = 180, fps = 60, scale = size })
end
-- When we press any number from 1 to 6, we scale the screen accordingly
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
-- When we press F, we get in or out of fullscreen
onUserInput('F', function()
	if fullscreen == nil or fullscreen == false then
		fullscreen = true
	else
		fullscreen = false
	end
	setFullscreen(fullscreen)
end)
-- Pressing F8 enables the interactive console
onUserInput('F8', showConsole)
-- Pressing F10 enables or disables scanlines
onUserInput('F10', function()
	if scanlines == nil or scanlines == false then
		scanlines = true
	else
		scanlines = false
	end
	setScanlines(scanlines)
end)
-- Pressing F11 enables or disables the debugging of walkboxes
onUserInput('F11', function()
	if walkboxes == nil or walkboxes == false then
		walkboxes = true
	else
		walkboxes = false
	end
	debugWalkboxes(walkboxes)
end)
-- Pressing F12 saves a screenshot
onUserInput('F12', function()
	saveScreenshot('./screenshot-' .. currentTicks .. '.png')
end)
-- Pressing Esc exits a cutscene (if one is playing) or leaves the game
onUserInput('Escape', function ()
	if cutscene ~= nil then
		endCutscene()
	else
		quit()
	end
end)
-- Pressing the dot skips the text of the actor currently speaking
onUserInput('.', skipActorsText)

-- We create our own object for the inventory, e.g., to keep track of
-- positions in the list and decide how to render it: in fact, the
-- engine leaves UI related aspects to the script, and otherwise only
-- keeps track of who owns what or what's where, but not if they should
-- displayed or how. Notice that for the sake of simplicity we're keeping
-- a single object as we only have a single main actor, but you may want
-- to keep different objects for different actors.
inventory = {}
local inventoryVisible = false
function showInventory()
	inventoryVisible = true
	showObject('inventory')
	for _, id in ipairs(inventory) do
		showObject(id)
	end
end
function hideInventory()
	if activeActor == nil then return end
	inventoryVisible = false
	hideObject('inventory')
	for index, id in ipairs(inventory) do
		hideObject(id)
	end
end
Image:new({ id = 'inventory-bg', path = './assets/images/inventory.png' })
Object:new({ id = 'inventory', uiAnimation = 'inventory-bg', ui = true, interactable = false, plane = 10 })
setObjectUiPosition('inventory', 0, 0);

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

-- Let's configure what to do when the inventory of an actor is updated
onInventoryUpdated = function(actorId, objectId, owned)
	local actor = actors[actorId]
	local object = objects[objectId]
	if actor == nil or object == nil then
		return
	end
	if owned then
		-- Add to our local inventory
		inventory[#inventory+1] = objectId
		showObject(objectId)
		setObjectPlane(objectId, 11)
	else
		-- Remove from our local inventory
		for index, id in ipairs(inventory) do
			if id == objectId then
				table.remove(inventory, index)
				hideObject(objectId)
				break
			end
		end
	end
	-- FIXME Update the position of all objects
	for index, id in ipairs(inventory) do
		setObjectUiPosition(objectId, index-1, 0);
	end
end

-- Let's configure what to do when an owned object is selected or deselected
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
showInventory()
rooms['street']:enter()

-- This is the script we'll use for the intro cutscene: we have the
-- main actor come in from the top, walk to a specific coordinate,
-- look around and then say something. We use the startCutscene()
-- command to hide the cursor and prevent interaction, and
-- stopCutscene() to enable it again and allow the user to play.
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
-- In case the cutscene is interrupted (Esc pressed), we have a different
-- function to put the actor in what was supposed to be the final state
function introEscape()
	kiavcLog('Escaped intro cutscene')
	activeActor:moveTo('street', 148, 170)
	activeActor:look('down')
	stopCutscene()
end
-- Start the cutscene as soon as the game starts
playCutscene(intro, introEscape)
