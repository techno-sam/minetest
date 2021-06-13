core.client_nodes = {}

function core.get_client_node(pos)
	local name
	local info = core.client_nodes[pos]
	if info then
		return table.copy(info)
	end
end

function core.set_client_node(pos,node)
	if not minetest.localplayer then return false end
	if type(node)~="table" then
		node = {name=node}
	end
	if not node.param2 then node.param2 = (core.get_client_node(pos) and core.get_client_node(pos).param2) or 0 end
	if core.get_client_node(pos) and (core.get_client_node(pos).name==node.name and core.get_client_node(pos).param2==node.param2) then return end
	if not core.get_client_node(pos) then--and node.name~="air" then
		local num = minetest.localplayer:hud_add({
			hud_elem_type = "node",
			node=node.name,
			world_pos = pos,
			number=node.param2
		})
		core.client_nodes[pos] = {name=node.name,number=num}
	else--if node.name~="air" then
		if not core.client_nodes[pos] then core.client_nodes[pos]={} end
		core.client_nodes[pos].name = node.name
		core.client_nodes[pos].param2 = node.param2
		return minetest.localplayer:hud_change(core.client_nodes[pos].number,"node",node.name) and minetest.localplayer:hud_change(core.client_nodes[pos].number,"number",node.param2)
	--[[elseif core.get_client_node(pos) and node.name=="air" then
		minetest.localplayer:hud_remove(core.client_nodes[pos].number)
		core.client_nodes[pos] = nil--]]
	end
end
