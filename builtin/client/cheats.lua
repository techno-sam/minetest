local function none() end

core.cheats = {
	--[[["Combat"] = {
		["Killaura"] = "killaura",
		["Forcefield"] = "forcefield",
	},--]]
	["Movement"] = {
		["Freecam"] = "freecam",
		["AutoForward"] = "continuous_forward",
		["PitchMove"] = "pitch_move",
		["AutoJump"] = "autojump",
		--["Jesus"] = "jesus",
		--["NoSlow"] = "no_slow",
		["JetPack"] = "jetpack",
		--["AntiSlip"] = "antislip",
	},
	["Render"] = {
		--["Xray"] = "xray",
		["Fullbright"] = "fullbright",
		--["HUDBypass"] = "hud_flags_bypass",
		--["NoHurtCam"] = "no_hurt_cam",
		["BrightNight"] = "no_night",
		--["Coords"] = "coords",
		["CheatHUD"] = "cheat_hud",
		["EntityESP"] = "enable_esp",
		["EntityTracers"] = "enable_tracers",
		["PlayerOnlyESP"] = "player_only_esp",
		["PlayerOnlyTracers"] = "player_only_tracers",
		["NodeESP"] = "enable_node_esp",
		["NodeTracers"] = "enable_node_tracers",
		["Revert Airlike Item Fix"] = "revert_airlike_item_fix",
		["NametagOverrides"] = "nametag_overrides",
		["ShowEmptyNametags"] = "empty_nametag_overrides",
		["ShowPosInNametags"] = "show_pos_nametag",
		--["ChunkBounds"] = "enable_chunk_bounds",
		--["SectorBounds"] = "enable_sector_bounds",
	},
	["Interact"] = {
		--["FastDig"] = "fastdig",
		--["FastPlace"] = "fastplace",
		--["AutoDig"] = "autodig",
		--["AutoPlace"] = "autoplace",
		["InstantBreak"] = "instamine",
		["NothingElse"] = none,
		--["FastHit"] = "spamclick",
		--["AutoHit"] = "autohit",
	},
	--[[["Exploit"] = {
		--["EntitySpeed"] = "entity_speed",
		["Empty"] = none,
		["StillEmpty"] = none,
	},--]]
	["Player"] = {
		["NoFallDamage"] = "prevent_falldamage",
		--["NoForceRotate"] = "no_force_rotate",
		--["Reach"] = "reach",
		["PointLiquids"] = "pointall",
		["PrivBypass"] = "privilege_override",
		--["AutoRespawn"] = "autorespawn",
		--["ThroughWalls"] = "dont_point_nodes",
	},
}

function core.register_cheat(cheatname, category, func)
	core.cheats[category] = core.cheats[category] or {}
	core.cheats[category][cheatname] = func
end
