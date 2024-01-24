function FindWater(tag, global)
	if tag == nil then tag = "" end
	if global == nil then global = false end

	if global then
		local waters = GetWaters()
		for i = 1, #waters do
			local water = waters[i]
			if tag == "" or HasTag(water, tag)then
				return water
			end
		end
	else
		DebugPrint("FindWater: local search not implemented yet")
		--[[local script = WhoAmI()
		local entities = GetScriptEntities(script)
		for i = 1, #entities do
			local entity = entities[i]
			if GetEntityType(entity) == "water" and (tag == "" or HasTag(entity, tag)) then
				return entity
			end
		end]]
	end
end

function FindWaters(tag, global)
	if tag == nil then tag = "" end
	if global == nil then global = false end

	local result = {}
	if global then
		local waters = GetWaters()
		for i = 1, #waters do
			local water = waters[i]
			if tag == "" or HasTag(water, tag)then
				table.insert(result, water)
			end
		end
	else
		DebugPrint("FindWaters: local search not implemented yet")
		--[[local script = WhoAmI()
		local entities = GetScriptEntities(script)
		for i = 1, #entities do
			local entity = entities[i]
			if GetEntityType(entity) == "water" and (tag == "" or HasTag(entity, tag)) then
				table.insert(result, entity)
			end
		end]]
	end
	return result
end

function DebugWater(water)
	local water_tr = GetWaterTransform(water)
	local water_vertices = GetWaterVertices(water)
	for j = 1, #water_vertices do
		local v1 = water_vertices[j]
		local v2 = water_vertices[j + 1]
		if v2 == nil then
			v2 = water_vertices[1]
		end
		local p1 = TransformToParentPoint(water_tr, v1)
		local p2 = TransformToParentPoint(water_tr, v2)
		DebugLine(p1, p2, 0, 0, 1, 0.5)
	end
end

function DrawAABB(pmin, pmax)
	local points = {
		Vec(pmax[1], pmax[2], pmax[3]),
		Vec(pmax[1], pmax[2], pmin[3]),
		Vec(pmax[1], pmin[2], pmax[3]),
		Vec(pmax[1], pmin[2], pmin[3]),
		Vec(pmin[1], pmax[2], pmax[3]),
		Vec(pmin[1], pmax[2], pmin[3]),
		Vec(pmin[1], pmin[2], pmax[3]),
		Vec(pmin[1], pmin[2], pmin[3]),
	}
	DrawLine(points[1], points[2], 1, 1, 1)
	DrawLine(points[1], points[3], 1, 1, 1)
	DrawLine(points[1], points[5], 1, 1, 1)
	DrawLine(points[4], points[2], 1, 1, 1)
	DrawLine(points[4], points[3], 1, 1, 1)
	DrawLine(points[4], points[8], 1, 1, 1)
	DrawLine(points[6], points[2], 1, 1, 1)
	DrawLine(points[6], points[5], 1, 1, 1)
	DrawLine(points[6], points[8], 1, 1, 1)
	DrawLine(points[7], points[3], 1, 1, 1)
	DrawLine(points[7], points[5], 1, 1, 1)
	DrawLine(points[7], points[8], 1, 1, 1)
end

function DrawOBB(shape)
	local transf = GetShapeWorldTransform(shape)
	local size = Vec(GetShapeSize(shape))
	local pmin = Vec(0, 0, 0)
	local pmax = VecScale(size, 0.1)
	local points = {
		TransformToParentPoint(transf, Vec(pmax[1], pmax[2], pmax[3])),
		TransformToParentPoint(transf, Vec(pmax[1], pmax[2], pmin[3])),
		TransformToParentPoint(transf, Vec(pmax[1], pmin[2], pmax[3])),
		TransformToParentPoint(transf, Vec(pmax[1], pmin[2], pmin[3])),
		TransformToParentPoint(transf, Vec(pmin[1], pmax[2], pmax[3])),
		TransformToParentPoint(transf, Vec(pmin[1], pmax[2], pmin[3])),
		TransformToParentPoint(transf, Vec(pmin[1], pmin[2], pmax[3])),
		TransformToParentPoint(transf, Vec(pmin[1], pmin[2], pmin[3])),
	}
	DrawLine(points[1], points[2], 1, 1, 1)
	DrawLine(points[1], points[3], 1, 1, 1)
	DrawLine(points[1], points[5], 1, 1, 1)
	DrawLine(points[4], points[2], 1, 1, 1)
	DrawLine(points[4], points[3], 1, 1, 1)
	DrawLine(points[4], points[8], 1, 1, 1)
	DrawLine(points[6], points[2], 1, 1, 1)
	DrawLine(points[6], points[5], 1, 1, 1)
	DrawLine(points[6], points[8], 1, 1, 1)
	DrawLine(points[7], points[3], 1, 1, 1)
	DrawLine(points[7], points[5], 1, 1, 1)
	DrawLine(points[7], points[8], 1, 1, 1)
end

function DrawOBB2(transf, size)
	local points = {
		TransformToParentPoint(transf, Vec(size[1] / 2, size[2] / 2, size[3] / 2)),
		TransformToParentPoint(transf, Vec(size[1] / 2, size[2] / 2, -size[3] / 2)),
		TransformToParentPoint(transf, Vec(size[1] / 2, -size[2] / 2, size[3] / 2)),
		TransformToParentPoint(transf, Vec(size[1] / 2, -size[2] / 2, -size[3] / 2)),
		TransformToParentPoint(transf, Vec(-size[1] / 2, size[2] / 2, size[3] / 2)),
		TransformToParentPoint(transf, Vec(-size[1] / 2, size[2] / 2, -size[3] / 2)),
		TransformToParentPoint(transf, Vec(-size[1] / 2, -size[2] / 2, size[3] / 2)),
		TransformToParentPoint(transf, Vec(-size[1] / 2, -size[2] / 2, -size[3] / 2)),
	}
	DrawLine(points[1], points[2], 1, 1, 1)
	DrawLine(points[1], points[3], 1, 1, 1)
	DrawLine(points[1], points[5], 1, 1, 1)
	DrawLine(points[4], points[2], 1, 1, 1)
	DrawLine(points[4], points[3], 1, 1, 1)
	DrawLine(points[4], points[8], 1, 1, 1)
	DrawLine(points[6], points[2], 1, 1, 1)
	DrawLine(points[6], points[5], 1, 1, 1)
	DrawLine(points[6], points[8], 1, 1, 1)
	DrawLine(points[7], points[3], 1, 1, 1)
	DrawLine(points[7], points[5], 1, 1, 1)
	DrawLine(points[7], points[8], 1, 1, 1)
end

function DrawGridAux(pmin, pmax)
	local points = {
		Vec(pmax[1], pmax[2], pmax[3]),
		Vec(pmax[1], pmax[2], pmin[3]),
		Vec(pmax[1], pmin[2], pmax[3]),
		Vec(pmax[1], pmin[2], pmin[3]),
		Vec(pmin[1], pmax[2], pmax[3]),
		Vec(pmin[1], pmax[2], pmin[3]),
		Vec(pmin[1], pmin[2], pmax[3]),
		Vec(pmin[1], pmin[2], pmin[3]),
	}
	DrawLine(points[1], points[2], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[1], points[3], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[1], points[5], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[4], points[2], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[4], points[3], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[4], points[8], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[6], points[2], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[6], points[5], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[6], points[8], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[7], points[3], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[7], points[5], 0.9, 0.9, 0.1, 0.25)
	DrawLine(points[7], points[8], 0.9, 0.9, 0.1, 0.25)
end

function DrawGrid(pmin, pmax)
	DrawAABB(pmin, pmax)
	local step = 10
	for x = pmin[1] + step, pmax[1] - step, step do
		DrawGridAux(Vec(x, pmin[2], pmin[3]), Vec(x, pmax[2], pmax[3]))
	end
	for y = pmin[2] + step, pmax[2] - step, step do
		DrawGridAux(Vec(pmin[1], y, pmin[3]), Vec(pmax[1], y, pmax[3]))
	end
	for z = pmin[3] + step, pmax[3] - step, step do
		DrawGridAux(Vec(pmin[1], pmin[2], z), Vec(pmax[1], pmax[2], z))
	end
end

function DrawCircleX(pos, radius)
	local steps = 32
	local angle_inc = (2 * math.pi) / steps
	local points = {}

	for i = 1, steps do
		local angle = i * angle_inc
		local x = pos[1]
		local y = pos[2] + radius * math.cos(angle)
		local z = pos[3] + radius * math.sin(angle)
		table.insert(points, Vec(x, y, z))
	end

	for i = 1, steps do
		local j = i % steps + 1
		DrawLine(points[i], points[j])
	end
end

function DrawCircleY(pos, radius)
	local steps = 32
	local angle_inc = (2 * math.pi) / steps
	local points = {}

	for i = 1, steps do
		local angle = i * angle_inc
		local x = pos[1] + radius * math.cos(angle)
		local y = pos[2]
		local z = pos[3] + radius * math.sin(angle)
		table.insert(points, Vec(x, y, z))
	end

	for i = 1, steps do
		local j = i % steps + 1
		DrawLine(points[i], points[j])
	end
end

function DrawCircleZ(pos, radius)
	local steps = 32
	local angle_inc = (2 * math.pi) / steps
	local points = {}

	for i = 1, steps do
		local angle = i * angle_inc
		local x = pos[1] + radius * math.cos(angle)
		local y = pos[2] + radius * math.sin(angle)
		local z = pos[3]
		table.insert(points, Vec(x, y, z))
	end

	for i = 1, steps do
		local j = i % steps + 1
		DrawLine(points[i], points[j])
	end
end

function DrawCircle(center, radius)
	DrawCircleX(center, radius)
	DrawCircleY(center, radius)
	DrawCircleZ(center, radius)
end

function HSVtoRGBA(h, s, v, a)
	local i = math.floor(h / 60) % 6
	local f = h / 60 - math.floor(h / 60)
	local p = v * (1 - s)
	local q = v * (1 - s * f)
	local t = v * (1 - (1 - f) * s)

	if i == 0 then return v, t, p, a end
	if i == 1 then return q, v, p, a end
	if i == 2 then return p, v, t, a end
	if i == 3 then return p, q, v, a end
	if i == 4 then return t, p, v, a end
	if i == 5 then return v, p, q, a end
	return 0, 0, 0, 1
end

function GetJointWorldPosAndAxis(joint, index)
	local shapes = GetJointShapes(joint)
	if index == nil then index = 1 end
	if #shapes ~= 2 or index < 1 or index > 2 then return Vec(), Vec() end
	local shape = shapes[index]
	local body = GetShapeBody(shape)
	local body_tr = GetBodyTransform(body)
	local point, axis = GetJointLocalPosAndAxis(joint, index)
	local joint_pos = TransformToParentPoint(body_tr, point)
	local joint_dir = TransformToParentVec(body_tr, axis)
	return joint_pos, joint_dir
end

function GetWheelShape(wheel)
	return wheel + 1 -- magic
end

function init()
	local version = GetDllVersion()
	DebugPrint(version)
	AllowInternalFunctions()
--[[
	local current_map = GetString("game.levelid")
	if current_map ~= "" then
		ZlibSaveCompressed(current_map .. ".z", "Hello, World!")
	end
]]
	rainbow = 0
	g_shape = 0
end

function tick(dt)
	local flashlight = GetPlayerFlashlight()
	rainbow = math.fmod(rainbow + 120 * dt, 360)
	local r, g, b = HSVtoRGBA(rainbow, 0.9, 0.9, 1.0)
	SetLightColor(flashlight, r, g, b)

	local shadow_volume = GetShadowVolumeSize()
	local sv_min = Vec(-shadow_volume[1] / 2, 0, -shadow_volume[3] / 2)
	local sv_max = Vec(shadow_volume[1] / 2, shadow_volume[2], shadow_volume[3] / 2)
	DrawGrid(sv_min, sv_max)

	local boundary_vertices = GetBoundaryVertices()
	for i = 1, #boundary_vertices do
		local v1 = boundary_vertices[i]
		local v2 = boundary_vertices[i + 1]
		if v2 == nil then
			v2 = boundary_vertices[1]
		end
		for j = -5, 20 do
			DrawLine(VecAdd(v1, Vec(0, j, 0)), VecAdd(v2, Vec(0, j, 0)), 1, 1, 0)
		end
		DrawLine(VecAdd(v1, Vec(0, -5, 0)), VecAdd(v1, Vec(0, 20, 0)), 1, 1, 0)
	end

	local waters = FindWaters("", true)
	for i = 1, #waters do
		DebugWater(waters[i])
	end

	local scripts = GetScripts()
	for i = 1, #scripts do
		local script = scripts[i]
		local script_path = GetScriptPath(script)
		DebugWatch(script, script_path)
		local entities = GetScriptEntities(script)
		for j = 1, #entities do
			local entity = entities[j]
			local entity_type = GetEntityType(entity)
			if entity_type == "shape" then
				DrawOBB(entity)
			end
		end
	end

	local joints = FindJoints("", true)
	for i = 1, #joints do
		local joint = joints[i]
		local joint_type = GetJointType(joint)
		local pos1, dir1 = GetJointWorldPosAndAxis(joint)
		local pos2, _ = GetJointWorldPosAndAxis(joint, 2)
		if joint_type == "ball" then
			DebugCross(pos1, 0, 0, 1)
		elseif joint_type == "hinge" then
			DebugLine(pos1, VecAdd(pos1, dir1), 0, 0, 1)
		elseif joint_type == "prismatic" then
			DebugLine(pos1, VecAdd(pos1, dir1), 0, 0, 1)
			DebugCross(pos2, 0, 0, 1)
		elseif joint_type == "rope" then
			DrawLine(pos1, pos2, 0, 0, 1)
		end
	end

	local vehicle = GetPlayerVehicle()
	if vehicle ~= 0 then
		local wheels = GetVehicleWheels(vehicle)
		for i = 1, #wheels do
			local wheel = wheels[i]
			local wheel_shape = GetWheelShape(wheel)
			DrawOBB(wheel_shape)
		end

		wheels = GetWheels()
		for i = 1, #wheels do
			local wheel = wheels[i]
			if GetWheelVehicle(wheel) == vehicle then
				local wheel_shape = GetWheelShape(wheel)
				DrawAABB(GetShapeBounds(wheel_shape))
			end
		end
	end

	local triggers = FindTriggers("", true)
	for i = 1, #triggers do
		local trigger = triggers[i]
		local trigger_tr = GetTriggerTransform(trigger)
		local tr_type = GetTriggerType(trigger)
		if tr_type == "polygon" then
			local trigger_vertices = GetTriggerVertices(trigger)
			local height = GetTriggerSize(trigger)
			for j = 1, #trigger_vertices do
				local v1 = trigger_vertices[j]
				local v2 = trigger_vertices[j + 1]
				if v2 == nil then
					v2 = trigger_vertices[1]
				end
				local p1 = TransformToParentPoint(trigger_tr, v1)
				local p2 = TransformToParentPoint(trigger_tr, v2)
				local p3 = VecAdd(p1, Vec(0, height, 0))
				local p4 = VecAdd(p2, Vec(0, height, 0))
				DrawLine(p1, p2) -- bottom
				DrawLine(p3, p4) -- top
				DrawLine(p1, p3) -- side
			end
		elseif tr_type == "box" then
			local size = GetTriggerSize(trigger)
			DrawOBB2(trigger_tr, size)
		else
			local radius = GetTriggerSize(trigger)
			DrawCircle(trigger_tr.pos, radius)
		end
	end

	if InputPressed("rmb") then
		local camera_tr = GetCameraTransform()
		local camera_fwd = TransformToParentVec(camera_tr, Vec(0, 0, -1))
		local hit, _, _, shape = QueryRaycast(camera_tr.pos, camera_fwd, 100, 0, false)
		if hit then
			g_shape = shape
			SetShapeTexture(shape, 8, 0.5, 4, 0.7)
			local t, tw, bt, bw = GetShapeTexture(shape)
			DebugWatch("texture", t .. " " .. tw)
			DebugWatch("blend texture", bt .. " " .. bw)
		end
	end

	if g_shape ~= 0 then
		SetTextureOffset(g_shape, Vec(GetTime(), 0, 0))
		local offset = GetTextureOffset(g_shape)
		DebugWatch("texture offset", VecStr(offset))
	end

	if InputPressed("T") then
		local current_map = GetString("game.levelid")
		if current_map ~= "" then
			local text = ZlibLoadCompressed(current_map .. ".z")
			DebugWatch("unzipped file content", text)
		end
	end
end
