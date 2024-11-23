function GetWheelShape(wheel)
	return wheel + 1
end

function GetWheelRadius(wheel)
	local shape = GetWheelShape(wheel)
	local sizex, sizey, sizez, scale = GetShapeSize(shape)
	local width = math.max(sizex, math.max(sizey, sizez))
	return (width / 2) * scale
end

function ScaleWorld(FACTOR)
	local player_tr = GetPlayerTransform()
	player_tr.pos = VecScale(player_tr.pos, FACTOR)
	SetPlayerTransform(player_tr)
	SetPlayerSpawnTransform(player_tr)
	local waters = FindEntities("", true, "water")
	for i = 1, #waters do
		local water = waters[i]
		local water_tr = GetWaterTransform(water)
		water_tr.pos = VecScale(water_tr.pos, FACTOR)
		local vertices = GetWaterVertices(water)
		local depth = GetProperty(water, "depth")
		local wave = GetProperty(water, "wave")
		local ripple = GetProperty(water, "ripple")
		local motion = GetProperty(water, "motion")
		local foam = GetProperty(water, "foam")
		local color = GetProperty(water, "color")
		local water_xml = string.format("<water type='polygon' depth='%f' wave='%f' ripple='%f' motion='%f' foam='%f' color='%f %f %f'>\n",
			depth, wave, ripple, motion, foam, color[1], color[2], color[3])
		for j = 1, #vertices do
			local vertex = vertices[j]
			vertex = VecScale(vertex, FACTOR)
			water_xml = water_xml .. string.format("\t<vertex pos='%f %f'/>\n", vertex[1], vertex[3])
		end
		water_xml = water_xml .. "</water>"
		local spawned = Spawn(water_xml, water_tr)
		if #spawned == 0 then
			DebugPrint("Failed to spawn water. " .. water_xml)
		end
		Delete(water)
	end
	local joints = FindJoints("", true)
	for i = 1, #joints do
		local joint = joints[i]
		-- Too lazy to implement
		Delete(joint)
	end
	local lights = FindLights("", true)
	for i = 1, #lights do
		local light = lights[i]
		Delete(light)
	end
	local screens = FindScreens("", true)
	for i = 1, #screens do
		local screen = screens[i]
		Delete(screen)
	end
	-- TODO: locations, triggers, vehicle (driver pos, etc.)
	local wheels = FindEntities("", true, "wheel")
	for i = 1, #wheels do
		local wheel = wheels[i]
		local wheel_tr = GetWheelTransform(wheel)
		wheel_tr.pos = VecScale(wheel_tr.pos, FACTOR)
		SetWheelTransform(wheel, wheel_tr)
		local radius = GetWheelRadius(wheel)
		SetWheelRadius(wheel, radius * FACTOR)
	end
	local shapes = FindShapes("", true)
	for i = 1, #shapes do
		local shape = shapes[i]
		local parent = GetEntityParent(shape, "", "animator")
		if parent == 0 then
			local _, _, _, scale = GetShapeSize(shape)
			SetShapeScale(shape, scale * FACTOR)
			local shape_tr = GetShapeLocalTransform(shape)
			shape_tr.pos = VecScale(shape_tr.pos, FACTOR)
			SetShapeLocalTransform(shape, shape_tr)
		end
	end
	local bodies = FindBodies("", true)
	for i = 1, #bodies do
		local body = bodies[i]
		local parent = GetEntityParent(body, "", "animator")
		if parent == 0 then
			local body_tr = GetBodyTransform(body)
			body_tr.pos = VecScale(body_tr.pos, FACTOR)
			SetBodyTransform(body, body_tr)
			SetBodyActive(body, false)
		end
	end
end

function init()
	ScaleWorld(0.5)
end
