-- Here we register an actor instance for the detective, who'll be the
-- main actor in our tiny demo game. We first register a costume for
-- different states (staying still, talking, walking) and then the actor
-- itself. Notice that for NPCs you don't really need all states: if an
-- NPC is staying behind a counter or just sitting, for instance, you
-- can skip the walking animation.

-- Let's register all the images/animations for the costume: we use some
-- cool animations made available by https://brokencellgames.itch.io
Image:new({ id = 'detective-still-left', path = './assets/images/dvstopl.png' })
Image:new({ id = 'detective-still-right', path = './assets/images/dvstopr.png' })
Image:new({ id = 'detective-still-up', path = './assets/images/dvstopu.png' })
Image:new({ id = 'detective-still-down', path = './assets/images/dvstopd.png' })
Animation:new({ id = 'detective-walking-left', path = './assets/images/dvwalkl.png', frames = 8 })
Animation:new({ id = 'detective-walking-right', path = './assets/images/dvwalkr.png', frames = 8 })
Animation:new({ id = 'detective-walking-up', path = './assets/images/dvwalku.png', frames = 8 })
Animation:new({ id = 'detective-walking-down', path = './assets/images/dvwalkd.png', frames = 8 })
Animation:new({ id = 'detective-talking-left', path = './assets/images/dvtalkl.png', frames = 3 })
Animation:new({ id = 'detective-talking-right', path = './assets/images/dvtalkr.png', frames = 3 })
Animation:new({ id = 'detective-talking-up', path = './assets/images/dvtalku.png', frames = 3 })
Animation:new({ id = 'detective-talking-down', path = './assets/images/dvtalkd.png', frames = 3 })
-- Initialize the costume. Remember that actors and costumes are not
-- "married", and that different actors can actually use the same costume
Costume:new({
	id = 'detective-costume',
	still = {
		left = 'detective-still-left',
		right = 'detective-still-right',
		up = 'detective-still-up',
		down = 'detective-still-down'
	},
	walking = {
		left = 'detective-walking-left',
		right = 'detective-walking-right',
		up = 'detective-walking-up',
		down = 'detective-walking-down'
	},
	talking = {
		left = 'detective-talking-left',
		right = 'detective-talking-right',
		up = 'detective-talking-up',
		down = 'detective-talking-down'
	}
})

-- Now let's register the actor and their settings
local detective = Actor:new({
	id = 'detective',
	costume = 'detective-costume',
	font = 'dialogues',
	textColor = white,
	scaleFactor = 0.76,
	speed = 4
})
