function init()
--[[
	DebugPrint("coroutine " .. tostring(coroutine ~= nil))
	DebugPrint("debug " .. tostring(debug ~= nil))
	DebugPrint("io " .. tostring(io ~= nil))
	DebugPrint("math " .. tostring(math ~= nil))
	DebugPrint("os " .. tostring(os ~= nil))
	DebugPrint("package " .. tostring(package ~= nil))
	DebugPrint("string " .. tostring(string ~= nil))
	DebugPrint("table " .. tostring(table ~= nil))
]]
	local version = GetDllVersion()
	DebugPrint(version)

	local flashlight = GetPlayerFlashlight()
	SetLightEnabled(flashlight, true)
	SetLightColor(flashlight, 1, 0, 0)

	local scripts = GetScripts()
	for i = 1, #scripts do
		local script = scripts[i]
		local script_path = GetScriptPath(script)
		DebugPrint(script .. " | " .. script_path)
	end
--[[
	local current_map = GetString("game.levelid")
	if current_map ~= "" then
		ZlibSaveCompressed(current_map .. ".z", "Hello, World!")
	end
]]
	g_shape = 0
	extended_api = false
end

function tick(dt)
	if not extended_api then
		extended_api = true
		AllowInternalFunctions() -- Removes "function is only for internal usage" for all already loaded mods
	end

	local player_pos = GetPlayerTransform().pos

	local waters = GetWaters()
	for i = 1, #waters do
		local water = waters[i]
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
			p1[2] = player_pos[2]
			p2[2] = player_pos[2]
			DebugLine(p1, p2, 0, 0, 1)
		end
	end

	local triggers = FindTriggers("", true)
	for i = 1, #triggers do
		local trigger = triggers[i]
		local trigger_tr = GetTriggerTransform(trigger)
		local tr_type = GetTriggerType(trigger)
		DebugWatch("trigger " .. trigger, tr_type)
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
				DebugLine(p1, p2, 1, 0, 0) -- bottom
				DebugLine(p3, p4, 1, 0, 0) -- top
				DebugLine(p1, p3, 1, 0, 0) -- side
			end
		end
	end

	local boundary_vertices = GetBoundaryVertices()
	for j = -5, 20 do
		for i = 1, #boundary_vertices do
			local v1 = boundary_vertices[i]
			local v2 = boundary_vertices[i + 1]
			if v2 == nil then
				v2 = boundary_vertices[1]
			end
			v1[2] = player_pos[2] + j
			v2[2] = player_pos[2] + j
			DrawLine(v1, v2, 1, 1, 0)
		end
	end

	local vehicle = GetPlayerVehicle()
	if vehicle ~= 0 then
		local wheels = GetVehicleWheels(vehicle)
		for i = 1, #wheels do
			local wheel_shape = wheels[i] + 1
			DrawOBB(wheel_shape)
		end
	end

	if InputPressed("rmb") then
		local camera_tr = GetCameraTransform()
		local camera_fwd = TransformToParentVec(camera_tr, Vec(0, 0, -1))
		local hit, _, _, shape = QueryRaycast(camera_tr.pos, camera_fwd, 100, 0, false)
		if hit then
			g_shape = shape
			SetShapeTexture(shape, 8, 0.5, 4, 0.7)
			local t, tw, b, bw = GetShapeTexture(shape)
			DebugWatch("texture", t .. " " .. tw)
			DebugWatch("blend texture", b .. " " .. bw)
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

function draw()
	local joints = FindJoints("", true)
	for i = 1, #joints do
		local joint = joints[i]
		local shapes = GetJointShapes(joint)
		for j = 1, #shapes do
			local shape = shapes[j]
			local body = GetShapeBody(shape)
			local body_tr = GetBodyTransform(body)
			local joint_type = GetJointType(joint)

			local point, axis = GetJointLocalPosAndAxis(joint, j)
			local joint_pos = TransformToParentPoint(body_tr, point)
			local joint_dir = TransformToParentVec(body_tr, axis)

			if joint_type == "ball" then
				if j == 1 then
					DrawPoint(joint_pos, joint_type)
				end
			elseif joint_type == "hinge" then
				if j == 1 then
					DrawPoint(joint_pos, joint_type)
					DebugLine(joint_pos, VecAdd(joint_pos, joint_dir), 1, 1, 0)
				end
			elseif joint_type == "prismatic" then
				if j == 1 then
					DrawPoint(joint_pos, joint_type .. "-1")
				else
					DrawPoint(joint_pos, joint_type .. "-2")
					DebugLine(joint_pos, VecAdd(joint_pos, joint_dir), 1, 0, 0)
				end
			elseif joint_type == "rope" then
				if j == 1 then
					DrawPoint(joint_pos, joint_type .. "-1")
				else
					DrawPoint(joint_pos, joint_type .. "-2")
				end
			end
		end
	end
end

function DrawPoint(point, name)
	local x, y, d = UiWorldToPixel(point)
	if d > 0 then
	UiPush()
		UiTranslate(x, y)
		UiAlign("center middle")
		UiImage("ui/common/dot.png")
		UiTranslate(0, 20)
		UiFont("regular.ttf", 20)
		UiText(name)
	UiPop()
	end
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
	DrawLine(points[6], points[2], 0, 0, 1)
	DrawLine(points[6], points[5], 1)
	DrawLine(points[6], points[8], 0, 1)
	DrawLine(points[7], points[3], 1, 1, 1)
	DrawLine(points[7], points[5], 1, 1, 1)
	DrawLine(points[7], points[8], 1, 1, 1)
end
