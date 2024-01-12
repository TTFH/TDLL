function init()
	DebugPrint("coroutine " .. tostring(coroutine ~= nil))
	DebugPrint("debug " .. tostring(debug ~= nil))
	DebugPrint("io " .. tostring(io ~= nil))
	DebugPrint("math " .. tostring(math ~= nil))
	DebugPrint("os " .. tostring(os ~= nil))
	DebugPrint("package " .. tostring(package ~= nil))
	DebugPrint("string " .. tostring(string ~= nil))
	DebugPrint("table " .. tostring(table ~= nil))

	g_shape = 0
	joint_cache = {}
	wheel_cache = {}
	water_cache = {}
	boundary_vertices = {}
	dll_loaded = false
end

function tick(dt)
	if not dll_loaded and GetDllVersion then
		dll_loaded = true

		local version = GetDllVersion()
		DebugPrint(version)

		local flashlight = GetPlayerFlashlight()
		SetLightEnabled(flashlight, true)
		SetLightColor(flashlight, 1, 0, 0)

		boundary_vertices = GetBoundaryVertices()
		waters = GetWater()

		local scripts = GetScripts()
		for i = 1, #scripts do
			local script = scripts[i]
			local script_path = GetScriptPath(script)
			DebugPrint(script .. " | " .. script_path)
		end

		local current_map = GetString("game.levelid")
		DebugPrint("level id: " .. current_map)
		if current_map ~= "" then
			ZlibSaveCompressed(current_map, "Hello, World!")
		end
	end

	if not dll_loaded then return end
	local player_pos = GetPlayerTransform().pos

	for i = 1, #waters do
		local water = waters[i]
		if not water_cache[water] then
			water_cache[water] = {
				tr = GetWaterTransform(water),
				vertices = GetWaterVertices(water)
			}
		end
		local water_tr = water_cache[water].tr
		local water_vertices = water_cache[water].vertices
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
		if not wheel_cache[vehicle] then
			local wheels = GetVehicleWheels(vehicle)
			wheel_cache[vehicle] = wheels
		end
		local wheels = wheel_cache[vehicle]
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
		SetTextureOffset(g_shape, GetTime(), 0, 0)
		local x, y, z = GetTextureOffset(g_shape)
		DebugWatch("texture offset", VecStr(Vec(x, y, z)))
	end

	if InputPressed("T") then
		local current_map = GetString("game.levelid")
		if current_map ~= "" then
			local text = ZlibLoadCompressed(current_map)
			DebugWatch("unzip file", text)
		end
	end
end

function draw()
	if not dll_loaded then return end
	local joints = FindJoints("", true)
	for i = 1, #joints do
		local joint = joints[i]
		local shapes = GetJointShapes(joint)
		if not joint_cache[joint] then
			local point = GetJointLocalBodyPos(joint)
			joint_cache[joint] = point
			return
		end
		for j = 1, #shapes do
			local shape = shapes[j]
			local body = GetShapeBody(shape)
			local body_tr = GetBodyTransform(body)
			local joint_pos = TransformToParentPoint(body_tr, joint_cache[joint])
			DrawPoint(joint_pos, GetJointType(joint))
			break
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
