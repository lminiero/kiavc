-- This is the file where we keep the localized text strings. The format
-- is quite dumb, as it"s basically a big table where the key is the
-- string we want to load, and the content are different strings indexed
-- by the language each string is in.

translations = {
	defaultLook = {
		en = "Nothing to see here.",
		it = "Non c'è niente da vedere.",
	},
	defaultOwned = {
		en = "I picked that up already.",
		it = "L'ho già raccolto.",
	},
	defaultTake = {
		en = "I can't pick that up.",
		it = "Non si può raccogliere.",
	},
	defaultOpen = {
		en = "I can't open that.",
		it = "Non si può aprire.",
	},
	defaultClose = {
		en = "I can't close that.",
		it = "Non si può chiudere.",
	},
	defaultUse = {
		en = "I can't use that.",
		it = "Non si può usare.",
	},
	defaultUseWith = {
		en = "I can't use those together.",
		it = "Non si possono usare assieme.",
	},
	defaultGiveObject = {
		en = "I can't give that anything.",
		it = "Non gli si può dare nulla.",
	},
	defaultGiveActor = {
		en = "I don't think they want it.",
		it = "Non penso che lo voglia.",
	},
	defaultTalkObject = {
		en = "I can't talk to that.",
		it = "Non ci si può parlare.",
	},
	defaultTalkActor = {
		en = "I can't talk to them.",
		it = "Non ci si può parlare.",
	},
	intro = {
		en = "This place looks dangerous and scary...",
		it = "Questo posto sembra pericoloso ed inquietante...",
	},
	fireName = {
		en = "fire",
		it = "fuoco",
	},
	fireDesc = {
		en = "What a cool animation!",
		it = "Che animazione gagliarda!",
	},
	fireUse = {
		en = "Aaah, nothing better than warming up at a fire in a cold night like this.",
		it = "Aaah, non c'è niente di meglio di riscaldarsi al fuoco in una notte fredda come questa.",
	},
	skylineName = {
		en = "skyline",
		it = "skyline",
	},
	skylineDesc = {
		en = "What a cool cyberpunk background!",
		it = "Che gagliardo sfondo cyberpunk!",
	},
	restaurantName = {
		en = "restaurant",
		it = "ristorante",
	},
	restaurantDesc = {
		en = "Looks like a cozy place.",
		it = "Sembra un posto accogliente.",
	},
	restaurantUse = {
		en = "I'm not hungry.",
		it = "Non ho fame.",
	},
	girlsName = {
		en = "mysterious place",
		it = "posto misterioso",
	},
	girlsDesc = {
		en = "Is it always that noisy?",
		it = "Chissà se è sempre così rumoroso?",
	},
	girlsUse = {
		en = "I'm afraid it's locked.",
		it = "Temo che sia chiuso a chiave.",
	},
	gatewayName = {
		en = "outskirts",
		it = "periferia",
	},
	streetBarrier1 = {
		en = "No need to go there, for now.",
		it = "Non c'è ragione di andare da quella parte, per ora.",
	},
	streetBarrier2 = {
		en = "I'll just stay around here.",
		it = "Meglio restare qui nei paraggi.",
	},
	npcSearch1 = {
		en = "Where is it?!",
		it = "Dov'è?!",
	},
	npcSearch2 = {
		en = "I'm sure I left it here!",
		it = "Sono sicuro di averlo lasciato qui!",
	},
	npcSearch3 = {
		en = "Aaaarghhhhh!!",
	},
	npcSearch4 = {
		en = "If only I had my glasses...",
		it = "Se solo avessi i miei occhiali...",
	},
	hoteldoorDesc = {
		en = "I'm pretty sure this is not where I came from.",
		it = "Sono abbastanza sicuro che non sia da dove sono entrato.",
	},
	boatName = {
		en = "boat",
		it = "barca",
	},
	boatDesc = {
		en = "Nice boat.",
		it = "Bella nave.",
	},
	boatUse = {
		en = "It's no time for traveling.",
		it = "Non è momento di viaggiare.",
	},
	propsName = {
		en = "props",
		it = "oggetti di scena"
	},
	propsDesc = {
		en = "Shouldn't these be animated?",
		it = "Non dovrebbero essere animati?"
	},
	cityName = {
		en = "city",
		it = "città"
	},
	skullName = {
		en = "skull",
		it = "teschio",
	},
	skullTalk1 = {
		en = "Murray, is it you?",
		it = "Murray, sei tu?",
	},
	skullTalk2 = {
		en = "I think I'm talking to someone else's skull.",
		it = "Credo che sto parlando al teschio di qualcun altro.",
	},
	skullDesc = {
		en = "Just a boring old skull.",
		it = "Solo un vecchio e banale teschio.",
	},
	skullTake = {
		en = "I'll bring it with me.",
		it = "Lo porterò con me.",
	},
	skullUse = {
		en = "Why would you do that to a poor skull?",
		it = "Perché fare qualcosa del genere ad un povero teschio?",
	},
	skullUseFire = {
		en = "I think he has gone through hell already.",
		it = "Penso che abbia già passato le pene dell'inferno.",
	},
	skullUseSkyline = {
		en = "I can't throw it that far.",
		it = "Non riesco a lanciarlo così lontano.",
	},
	skullUseGirls = {
		en = "I think he's past having fun.",
		it = "I suoi giorni di gozzoviglie sono finiti.",
	},
	skullUseRestaurant = {
		en = "Crashing a window with a skull may be considered vandalism.",
		it = "Fracassare una vetrina con un teschio potrebbe essere considerato vandalismo.",
	},
	skullUseBoat = {
		en = "It's afraid of water.",
		it = "Ha paura dell'acqua.",
	},
	npcName = {
		en = "shady figure",
		it = "losco figuro",
	},
	npcDesc = {
		en = "That character looks really suspicious.",
		it = "È un individuo alquanto sospetto.",
	},
	npcTalk = {
		en = "I have nothing else to say to him.",
		it = "Non ho nient'altro da dirgli.",
	},
	npcTalk1 = {
		en = "Hi, nice to meet you!",
		it = "Ciao, piacere di conoscerti!",
	},
	npcTalk2 = {
		en = "What are you looking for?",
		it = "Cosa stai cercando?",
	},
	npcTalk3 = {
		en = "Hey, don't I know you from somewhere?",
		it = "Hey, ma non ci siamo conosciuti da qualche parte?",
	},
	npcTalk4 = {
		en = "I should go.",
		it = "Meglio che vada.",
	},
	npcResp1_1 = {
		en = "I wish I could say the same...",
		it = "Vorrei poter dire lo stesso...",
	},
	npcResp1_2 = {
		en = "Can't you see I'm busy?",
		it = "Non vedi che sono impegnato?",
	},
	npcResp1_3 = {
		en = "I can't find it!",
		it = "Non lo trovo!",
	},
	npcResp2_1 = {
		en = "I can't remember!",
		it = "Non me lo ricordo!",
	},
	npcResp2_2 = {
		en = "Not that it matters, since I can't find it anyway...",
		it = "Non che conti qualcosa, dato che comunque non lo trovo...",
	},
	npcTalk2_mh = {
		en = "Mh...",
	},
	npcTalk2_list1 = {
		en = "A restaurant?",
		it = "Un ristorante?",
	},
	npcTalk2_list2 = {
		en = "A bar?",
		it = "Un bar?",
	},
	npcTalk2_list3 = {
		en = "A street?",
		it = "Una strada?",
	},
	npcTalk2_list4 = {
		en = "A parking spot?",
		it = "Un parcheggio?",
	},
	npcTalk2_list5 = {
		en = "A hat?",
		it = "Un cappello?",
	},
	npcTalk2_list6 = {
		en = "Your glasses?",
		it = "I tuoi occhiali?",
	},
	npcTalk2_list7 = {
		en = "A pet?",
		it = "Un animale domestico?",
	},
	npcTalk2_list8 = {
		en = "A friend?",
		it = "Un amico?",
	},
	npcTalk2_list9 = {
		en = "A girlfriend?",
		it = "Una fidanzata?",
	},
	npcTalk2_list10 = {
		en = "A boyfriend?",
		it = "Un fidanzato?",
	},
	npcTalk2_list11 = {
		en = "Some company?",
		it = "Compagnia?",
	},
	npcTalk2_list12 = {
		en = "A fine leather jacket?",
		it = "Una bella giacca di pelle?",
	},
	npcTalk2_list13 = {
		en = "The Secret of Monkey Island?",
		it = "Il Segreto di Monkey Island?",
	},
	npcTalk2_list14 = {
		en = "An open source point and click adventure engine?",
		it = "Un motore open source per avventure punta e clicca?",
	},
	npcTalk2_list15 = {
		en = "I'm out of ideas.",
		it = "Non mi viene in mente altro.",
	},
	npcResp2_no = {
		en = "No.",
	},
	npcResp2_nohat1 = {
		en = "What..?",
		it = "Cosa..?",
	},
	npcResp2_nohat2 = {
		en = "No!",
	},
	npcResp2_nohat3 = {
		en = "I'm *wearing* a hat!",
		it = "Ho un cappello *in testa*!",
	},
	npcResp2_noglasses = {
		en = "No, but they'd definitely help...",
		it = "No, ma sicuramente aiuterebbero...",
	},
	npcResp3_1 = {
		en = "I don't think so, I've never seen you in my life.",
		it = "Non penso proprio, non ti ho mai visto in vita mia",
	},
	npcResp4_1 = {
		en = "Yeah, whatever.",
		it = "Sì, vabbeh.",
	},
	npcGiveSkull1 = {
		en = "Is this what you're looking for?",
		it = "È questo che stai cercando?",
	},
	npcGiveSkull2 = {
		en = "Nice skull, but no...",
		it = "Bel teschio, ma no...",
	},
	npcGiveSkull3 = {
		en = "It's definitely not what I lost.",
		it = "Sicuramente non è quello che ho perso.",
	},
}

-- Let's check if the translations file is correct
checkTranslations()
