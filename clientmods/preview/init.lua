local modname = assert(core.get_current_modname())
local modstorage = core.get_mod_storage()
local mod_channel

dofile(core.get_modpath(modname) .. "example.lua")

core.register_on_shutdown(function()
	print("[PREVIEW] shutdown client")
end)
local id = nil

do
	local server_info = core.get_server_info()
	print("Server version: " .. server_info.protocol_version)
	print("Server ip: " .. server_info.ip)
	print("Server address: " .. server_info.address)
	print("Server port: " .. server_info.port)

	print("CSM restrictions: " .. dump(core.get_csm_restrictions()))
end

local show_stuff = false

--[[mod_channel = core.mod_channel_join("experimental_preview")

core.after(4, function()
	if mod_channel:is_writeable() then
		mod_channel:send_all("preview talk to experimental")
	end
end)--]]

local function create_text()
	--print("armor: " .. dump(core.localplayer:get_armor_groups()))
	local player = core.localplayer
	if not player then
		player = core.localplayer
		minetest.after(1,create_text)
		return
	end
	id = core.localplayer:hud_add({
			hud_elem_type = "text",
			name = "example",
			number = 0xff0000,
			position = {x=0, y=1},
			offset = {x=8, y=-8},
			text = "You are using the preview mod",
			scale = {x=200, y=60},
			alignment = {x=1, y=-1}})
	id2 = core.localplayer:hud_add({
			hud_elem_type = "box",
			name = "boxexample",
			number = 0xff00f0,
			--position = {x=0, y=1},
			--offset = {x=8, y=-8},
			--text = "You are using the preview mod",
			--scale = {x=200, y=60},
			world_pos = {x=5, y=5, z=5}})
end
minetest.after(5,create_text)

--[[core.register_on_modchannel_message(function(channel, sender, message)
	print("[PREVIEW][modchannels] Received message `" .. message .. "` on channel `"
			.. channel .. "` from sender `" .. sender .. "`")
	core.after(1, function()
		mod_channel:send_all("CSM preview received " .. message)
	end)
end)

core.register_on_modchannel_signal(function(channel, signal)
	print("[PREVIEW][modchannels] Received signal id `" .. signal .. "` on channel `"
			.. channel)
end)

core.register_on_inventory_open(function(inventory)
	print("INVENTORY OPEN")
	print(dump(inventory))
	return false
end)--]]

--[[core.register_on_placenode(function(pointed_thing, node)
	print("The local player place a node!")
	print("pointed_thing :" .. dump(pointed_thing))
	print("node placed :" .. dump(node))
	return false
end)--]]

core.register_on_item_use(function(itemstack, pointed_thing)
	if not show_stuff then return false end
	print("The local player used an item!")
	print("pointed_thing :" .. dump(pointed_thing))
	print("item = " .. itemstack:get_name())

	if not itemstack:is_empty() then
		return false
	end

	--[[local pos = core.camera:get_pos()
	local pos2 = vector.add(pos, vector.multiply(core.camera:get_look_dir(), 100))

	local rc = core.raycast(pos, pos2)
	local i = rc:next()
	print("[PREVIEW] raycast next: " .. dump(i))
	if i then
		print("[PREVIEW] line of sight: " .. (core.line_of_sight(pos, i.above) and "yes" or "no"))

		local n1 = core.find_nodes_in_area(pos, i.under, {"default:stone"})
		local n2 = core.find_nodes_in_area_under_air(pos, i.under, {"default:stone"})
		print(("[PREVIEW] found %s nodes, %s nodes under air"):format(
				n1 and #n1 or "?", n2 and #n2 or "?"))
	end--]]

	return false
end)

-- This is an example function to ensure it's working properly, should be removed before merge
--[[core.register_on_receiving_chat_message(function(message)
	print("[PREVIEW] Received message " .. message)
	return false
end)

-- This is an example function to ensure it's working properly, should be removed before merge
core.register_on_sending_chat_message(function(message)
	print("[PREVIEW] Sending message " .. message)
	return false
end)--]]

--[[core.register_on_chatcommand(function(command, params)
	print("[PREVIEW] caught command '"..command.."'. Parameters: '"..params.."'")
end)

-- This is an example function to ensure it's working properly, should be removed before merge
core.register_on_hp_modification(function(hp)
	print("[PREVIEW] HP modified " .. hp)
end)--]]

-- This is an example function to ensure it's working properly, should be removed before merge
--[[core.register_on_damage_taken(function(hp)
	print("[PREVIEW] Damage taken " .. hp)
end)--]]

-- This is an example function to ensure it's working properly, should be removed before merge
core.register_chatcommand("dump", {
	func = function(param)
		return true, dump(_G)
	end,
})

local function preview_minimap()
	local minimap = core.ui.minimap
	if not minimap then
		print("[PREVIEW] Minimap is disabled. Skipping.")
		return
	end
	minimap:set_mode(4)
	minimap:show()
	minimap:set_pos({x=5, y=50, z=5})
	minimap:set_shape(math.random(0, 1))

	print("[PREVIEW] Minimap: mode => " .. dump(minimap:get_mode()) ..
			" position => " .. dump(minimap:get_pos()) ..
			" angle => " .. dump(minimap:get_angle()))
end

core.after(2, function()
	print("[PREVIEW] loaded " .. modname .. " mod")
	modstorage:set_string("current_mod", modname)
	assert(modstorage:get_string("current_mod") == modname)
	--preview_minimap()
end)

--[[core.after(5, function()
	if core.ui.minimap then
		core.ui.minimap:show()
	end

	print("[PREVIEW] Time of day " .. core.get_timeofday())

	print("[PREVIEW] Node level: " .. core.get_node_level({x=0, y=20, z=0}) ..
		" max level " .. core.get_node_max_level({x=0, y=20, z=0}))

	print("[PREVIEW] Find node near: " .. dump(core.find_node_near({x=0, y=20, z=0}, 10,
		{"group:tree", "default:dirt", "default:stone"})))
end)--]]

--[[core.register_on_dignode(function(pos, node)
	print("The local player dug a node!")
	print("pos:" .. dump(pos))
	print("node:" .. dump(node))
	return false
end)--]]

local indent_string = "     "

local function indent(level, text, emphasize)
	local result = text
	for i = 1, level do
		if emphasize then
			result = "->  "        .. string.gsub(result, "\n", "\n" .. indent_string)
		else
			result = indent_string .. string.gsub(result, "\n", "\n" .. indent_string)
		end
	end
	return result
end

local function adjusted_dump(o)
	local result = dump(o, indent_string)
	if result == "{\n" .. indent_string .. "\n}" then result = "{}" end
	return result
end

local lpp = 14 --Lines per page

local reading_page = 1

local function show_book_formspec(itemstack)
	local player_name = "apprentice"--user:get_player_name()
	local meta = itemstack:get_meta()
	local title, text, owner = "", "", player_name
	local page, page_max, lines, string = 1, 1, {}, ""

	-- Backwards compatibility
	local old_data = minetest.deserialize(itemstack:get_metadata())
	if old_data then
		meta:from_table({ fields = old_data })
	end

	local data = meta:to_table().fields

	if data.owner then
		title = data.title
		text = data.text
		owner = data.owner

		for str in (text .. "\n"):gmatch("([^\n]*)[\n]") do
			lines[#lines+1] = str
		end

		if data.page then
			page = data.page
			page_max = data.page_max
			
			if reading_page<1 then reading_page=1 end
			if reading_page>tonumber(page_max) then reading_page=tonumber(page_max) end
			
			page = reading_page

			for i = ((lpp * page) - lpp) + 1, lpp * page do
				if not lines[i] then break end
				string = string .. lines[i] .. "\n"
			end
		end
	end

	local formspec
	local esc = minetest.formspec_escape
	if owner == player_name and false then
		formspec = "size[8,8]" ..
			"field[0.5,1;7.5,0;title;" .. esc("Title:") .. ";" ..
				esc(title) .. "]" ..
			"textarea[0.5,1.5;7.5,7;text;" .. esc("Contents:") .. ";" ..
				esc(text) .. "]" ..
			"button_exit[2.5,7.5;3,1;save;" .. esc("Save") .. "]"
	else
		formspec = "size[8,8]" ..
			"label[0.5,0.5;" .. esc("by "..owner) .. "]" ..
			"tablecolumns[color;text]" ..
			"tableoptions[background=#00000000;highlight=#00000000;border=false]" ..
			"table[0.4,0;7,0.5;title;#FFFF00," .. esc(title) .. "]" ..
			"textarea[0.5,1.5;7.5,7;;" ..
				minetest.formspec_escape(string ~= "" and string or text) .. ";]" ..
			"button[2.4,7.6;0.8,0.8;book_prev;<]" ..
			"label[3.2,7.7;" .. esc("Page "..page.." of "..page_max) .. "]" ..
			"button[4.9,7.6;0.8,0.8;book_next;>]"
	end

	minetest.show_formspec("preview:book", formspec)
	return itemstack
end

show_player_inventory = true

local function prepare_formspec()

	local formspec = ""
	local formspec_style = "bgcolor[#080808BB;true]" ..
		"background[5,5;1,1;gui_formbg.png;true]" ..
		"listcolors[#00000069;#5A5A5A;#141318;#30434C;#FFF]"

	if show_player_inventory then
		formspec = "size[8,9]" ..
			"list[current_player;main;0,4.85;8,1;]" ..
			"list[current_player;main;0,6.08;8,3;8]"
	else
		formspec = "size[8,4.5]"
	end

	return formspec .. formspec_style

end


local function show_book_inventory(params)

	local formspec = prepare_formspec()

	minetest.show_formspec("preview:node",
		formspec ..
		"label[0,0;"..params["label"].."]" ..
		"list[nodemeta:"..params["spos"]..";books;0,0.55;8,4;]" ..
		"field[0.375,3.25;5.25,0.8;number;Index to read;]" ..
		"button[1.5,3.8;3,0.8;read;Read]" ..
		"field[0.375,5.25;5.25,0.8;position;Postion;"..params["spos"].."]"
	)

end

function tableHasKey(table,key)
    return table[key] ~= nil
end

local current_book_stack

core.register_on_formspec_input(function(formname,fields)
	if formname == "preview:node" then
		if fields.read then --[Read] button
			local num = tonumber(fields.number)
			local _,pos = core.parse_pos(fields.position)
			local meta = minetest.get_meta(pos)
			if not tableHasKey(meta:to_table(),"inventory") then
				return false
			end
			books = meta:to_table()["inventory"]["books"]
			if num>#books then num=#books end
			if num<1 then num=1 end
			itemstack = books[num]
			if itemstack==nil then return end
			if itemstack:is_empty() then return end
			if itemstack:get_name()~="default:book_written" then return end
			reading_page = 1
			current_book_stack = itemstack
			show_book_formspec(itemstack)
		end
	elseif formname == "preview:book" then
		if fields.book_next then
			reading_page = reading_page + 1
		elseif fields.book_prev then
			reading_page = reading_page - 1
		end
		show_book_formspec(current_book_stack)
	end
end)

core.register_on_punchnode(function(pos, node)
	if not show_stuff then return false end
	if string.find(node.name,"bookshelf") then
		local spos = pos["x"]..","..pos["y"]..","..pos["z"]
		show_book_inventory({label="Bookshelf",spos=spos})
	end
	print("The local player punched a node!")
	local itemstack = core.localplayer:get_wielded_item()
	print(dump(itemstack:to_table()))
	print("pos:" .. dump(pos))
	print("node:" .. dump(node))
	local meta = core.get_meta(pos)
	print("punched meta: " .. (meta and adjusted_dump(meta:to_table()) or "(missing)"))
	--local inventory = meta:get_inventory()
	if not meta:to_table() then return false end
	if not tableHasKey(meta:to_table(),"inventory") then
		return false
	end
	print("inventory: {")
	local desc = ""
	for key, list in pairs(meta:to_table()["inventory"]) do
		desc = desc .. indent(1, key .. ": ") .. "\n"
		local size = #list
		for i = 1, size do
			local stack = list[i]
			if not stack:is_empty() then
				desc = desc .. indent(2, "\"" .. stack:get_name() .. "\" - " .. stack:get_count()) .. "\n"
			end
		end
	end
	print(desc)
	print("}")
	return false
end)

core.register_chatcommand("privs", {
	func = function(param)
		return true, core.privs_to_string(minetest.get_privilege_list())
	end,
})

core.register_chatcommand("text", {
	func = function(param)
		return core.localplayer:hud_change(id, "text", param)
	end,
})

core.register_chatcommand("debug", {
	desc="Toggle debug info",
	func = function(param)
		show_stuff = not show_stuff
		if show_stuff then
			return true,"Debugging on."
		else
			return true,"Debugging off."
		end
	end,
})


--[[core.register_on_mods_loaded(function()
	core.log("Yeah preview mod is loaded with other CSM mods.")
end)--]]
