VehicleData = {}
ShapeData = {}
REPAIRS_PER_TICK = 10

function GetTags(handle)
	local tags_values = {}
	local tags = ListTags(handle)
	for i = 1, #tags do
		local tag = tags[i]
		local value = GetTagValue(handle, tag)
		table.insert(tags_values, { key = tag, value = value })
	end
	return tags_values
end

function SetTags(handle, tags)
	for i = 1, #tags do
		local tag = tags[i]
		SetTag(handle, tag.key, tag.value)
	end
end

function InitShapeData(vehicle)
	VehicleData[vehicle] = { bodies = {} }
	local vehicle_tr = GetVehicleTransform(vehicle)
	local bodies = GetVehicleBodies(vehicle)
	for i = 1, #bodies do
		local body = bodies[i]
		local body_tr = GetBodyTransform(body)
		local body_local_tr = TransformToLocalTransform(vehicle_tr, body_tr)
		table.insert(VehicleData[vehicle].bodies, {
			handle = body,
			transform = body_local_tr,
			description = GetDescription(body),
			tags = GetTags(body),
			shapes = {}
		})
		local shapes = GetBodyShapes(body)
		for j = 1, #shapes do
			local shape = shapes[j]
			table.insert(VehicleData[vehicle].bodies[i].shapes, shape)
			if not IsShapeBroken(shape) then
				local count = GetShapeVoxelCount(shape)
				local sizex, sizey, sizez = GetShapeSize(shape)
				local shape_tr = GetShapeLocalTransform(shape)

				local density = GetProperty(shape, "density")
				local strength = GetProperty(shape, "strength")
				local properties = { density = density, strength = strength }
				if extended_api then
					local palette = GetShapePaletteIndex(shape)
					local t, tw, bt, bw = GetShapeTexture(shape)
					local offset = GetTextureOffset(shape)
					properties.palette = palette
					properties.t = t
					properties.tw = tw
					properties.bt = bt
					properties.bw = bw
					properties.offset = offset
				end

				local joint_data = {}
				if extended_api then
					local joints = GetShapeJoints(shape)
					for k = 1, #joints do
						local joint = joints[k]
						local joint_shapes = GetJointShapes(joint)
						if shape == joint_shapes[1] then
							local joint_type = GetJointType(joint)
							local pos, axis = GetJointLocalPosAndAxis(joint, 1)
							table.insert(joint_data, {
								handle = joint,
								type = joint_type,
								pos = pos,
								axis = axis,
								tags = GetTags(joint)
							})
							if joint_type == "rope" then
								local r, g, b = GetProperty(joint, "ropecolor")
								local slack = GetProperty(joint, "slack")
								local rope_strength = GetProperty(joint, "strength")
								local maxstretch = GetProperty(joint, "maxstretch")
								local other_shape = GetJointOtherShape(joint, shape)
								local end_pos = GetJointLocalPosAndAxis(joint, 2)
								-- suddenly the JS spread operator looks nice
								joint_data[#joint_data].color = { r = r, g = g, b = b }
								joint_data[#joint_data].slack = slack
								joint_data[#joint_data].strength = rope_strength
								joint_data[#joint_data].maxstretch = maxstretch
								joint_data[#joint_data].shape2 = other_shape
								joint_data[#joint_data].pos2 = end_pos
							else
								local size = GetProperty(joint, "size")
								local rotstrength = GetProperty(joint, "rotstrength")
								local rotspring = GetProperty(joint, "rotspring")
								local min, max = GetJointLimits(joint)
								local collide, sound, autodisable = GetJointParams(joint)
								joint_data[#joint_data].size = size
								joint_data[#joint_data].rotstrength = rotstrength
								joint_data[#joint_data].rotspring = rotspring
								joint_data[#joint_data].limits = { min = min, max = max }
								joint_data[#joint_data].collide = collide
								joint_data[#joint_data].sound = sound
								joint_data[#joint_data].autodisable = autodisable
							end
						end
					end
				end

				ShapeData[shape] = {
					size = { x = sizex, y = sizey, z = sizez },
					count = count,
					voxels = {},
					transform = shape_tr,
					description = GetDescription(shape),
					tags = GetTags(shape),
					properties = properties,
					joints = joint_data
				}
				for z = 0, sizez - 1 do
					for y = 0, sizey - 1 do
						for x = 0, sizex - 1 do
							local _, _, _, _, _, index = GetShapeMaterialAtIndex(shape, x, y, z)
							if index ~= 0 then
								table.insert(ShapeData[shape].voxels, { x = x, y = y, z = z, index = index })
							end
						end
					end
				end
			end
		end
	end
end

function RevertShapeSize(shape, original_size, original_pos)
	local sizex, sizey, sizez = GetShapeSize(shape)
	if sizex ~= original_size.x or sizey ~= original_size.y or sizez ~= original_size.z then
		local shape_tr = GetShapeLocalTransform(shape)
		local min_offset = VecSub(original_pos, shape_tr.pos)
		min_offset = TransformToLocalVec(shape_tr, min_offset)
		min_offset = VecScale(min_offset, 10)
		local max_offset = VecAdd(Vec(original_size.x, original_size.y, original_size.z), min_offset)
		ResizeShape(shape, min_offset[1], min_offset[2], min_offset[3], max_offset[1] - 1, max_offset[2] - 1, max_offset[3] - 1)

		sizex, sizey, sizez = GetShapeSize(shape)
		return sizex == original_size.x and sizey == original_size.y and sizez == original_size.z
	end
	return true
end

function CreateShapeFromData(body, shape)
	local new_shape = CreateShape(body, ShapeData[shape].transform, shape)
	ShapeData[new_shape] = ShapeData[shape]

	local properties = ShapeData[shape].properties
	SetShapeDensity(new_shape, properties.density)
	if extended_api then
		AllowInternalFunctions()
		SetShapeStrength(new_shape, properties.strength)
		SetShapePalette(new_shape, properties.palette)
		SetShapeTexture(new_shape, properties.t, properties.tw, properties.bt, properties.bw)
		SetTextureOffset(new_shape, properties.offset)
	else
		SetProperty(new_shape, "strength", properties.strength)
		-- The shape handle passed to CreateShape() is not valid
		-- TODO: find a shape with the same palette
	end
	return new_shape
end

function GetJointWorldTransform(shape, pos, axis)
	local body = GetShapeBody(shape)
	local body_tr = GetBodyTransform(body)
	local joint_pos = TransformToParentPoint(body_tr, pos)
	local joint_dir = TransformToParentVec(body_tr, axis)
	local joint_rot = QuatLookAt(joint_pos, VecSub(joint_pos, joint_dir))
	return Transform(joint_pos, joint_rot)
end

function GetJointWorldPos(shape, pos)
	local body = GetShapeBody(shape)
	local body_tr = GetBodyTransform(body)
	local joint_pos = TransformToParentPoint(body_tr, pos)
	return joint_pos
end

function GetJointWorldPosAndAxis(joint, index)
	local shapes = GetJointShapes(joint)
	if index == nil then index = 1 end
	if #shapes ~= 2 or index < 1 or index > 2 or not extended_api then return Vec(), Vec() end
	local shape = shapes[index]
	local body = GetShapeBody(shape)
	local body_tr = GetBodyTransform(body)
	local point, axis = GetJointLocalPosAndAxis(joint, index)
	local joint_pos = TransformToParentPoint(body_tr, point)
	local joint_dir = TransformToParentVec(body_tr, axis)
	return joint_pos, joint_dir
end

function VecToStr(vec)
	return "'" .. vec[1] .. " " .. vec[2] .. " " .. vec[3] .. "'"
end

function BoolToStr(bool)
	return bool and "true" or "false"
end

function RepairVehicle(vehicle, repair_paint)
	RemoveDebries(vehicle)
	local count = 0
	local bodies = VehicleData[vehicle].bodies
	for i = 1, #bodies do
		local body = bodies[i].handle
		if not IsHandleValid(body) or not IsBodyDynamic(body) then
			local vehicle_tr = GetVehicleTransform(vehicle)
			local body_tr = TransformToParentTransform(vehicle_tr, bodies[i].transform)
			local entities = Spawn("<body dynamic='true'/>", body_tr)
			local new_body = entities[1]
			SetTags(new_body, bodies[i].tags)
			bodies[i].handle = new_body
			body = new_body
		end
		local shapes = bodies[i].shapes
		for j = 1, #shapes do
			local shape = shapes[j]
			if not IsHandleValid(shape) then
				local new_shape = CreateShapeFromData(body, shape)
				SetTags(new_shape, ShapeData[shape].tags)
				bodies[i].shapes[j] = new_shape
				shape = new_shape
			end
			if GetShapeBody(shape) ~= body then
				SetShapeBody(shape, body, ShapeData[shape].transform)
			end
			if RevertShapeSize(shape, ShapeData[shape].size, ShapeData[shape].transform.pos) then
				local voxels = ShapeData[shape].voxels
				for k = 1, #voxels do
					local voxel = voxels[k]
					if voxel.index ~= 0 then
						local _, _, _, _, _, index = GetShapeMaterialAtIndex(shape, voxel.x, voxel.y, voxel.z)
						if index ~= (repair_paint and voxel.index or 0) then
							SetBrush("cube", 1, voxel.index)
							DrawShapeBox(shape, voxel.x, voxel.y, voxel.z, voxel.x, voxel.y, voxel.z)
							count = count + 1
							if count % REPAIRS_PER_TICK == 0 then
								coroutine.yield()
							end
						end
					end
				end
			end
			if extended_api then
				local joints = ShapeData[shape].joints
				for k = 1, #joints do
					local joint = joints[k]
					if IsJointBroken(joint.handle) then
						if joint.type == "rope" then
							local entities = Spawn([[
								<rope slack=']] .. joint.slack ..
								[[' color=']] .. joint.color.r .. ' ' .. joint.color.g .. ' ' .. joint.color.b ..
								[[' slack=']] .. joint.slack ..
								[[' strength=']] .. joint.strength ..
								[[' maxstretch=']] .. joint.maxstretch ..
								[['>
									<location pos=]] .. VecToStr(GetJointWorldPos(shape, joint.pos)) .. [[/>
									<location pos=]] .. VecToStr(GetJointWorldPos(joint.shape2, joint.pos2)) .. [[/>
								</rope>
							]], Transform(), true, true)
							local new_joint = entities[1]
							SetTags(new_joint, joint.tags)
							joint.handle = new_joint
						else
							local entities = Spawn([[
								<joint type=']] .. joint.type ..
								[[' size=']] .. joint.size ..
								[[' rotstrength=']] .. joint.rotstrength ..
								[[' rotspring=']] .. joint.rotspring ..
								[[' limits=']] .. joint.limits.min .. ' ' .. joint.limits.max ..
								[[' collide=']] .. BoolToStr(joint.collide) ..
								[[' sound=']] .. BoolToStr(joint.sound) ..
								[[' autodisable=']] .. BoolToStr(joint.autodisable) ..
							[['/>]],
								GetJointWorldTransform(shape, joint.pos, joint.axis), true, true)
							local new_joint = entities[1]
							SetTags(new_joint, joint.tags)
							joint.handle = new_joint
						end
					end
				end
			end
		end
		SetBodyDynamic(body, true)
	end
end

function GetHotkey(path, key)
	if GetString(path) == "" then
		SetString(path, key)
	end
	return GetString(path)
end

function RemoveDebries(vehicle)
	local bodies = GetVehicleBodies(vehicle)
	for b = 1, #bodies do
		local body = bodies[b]
		local aabbMin, aabbMax = GetBodyBounds(body)
		QueryRejectVehicle(vehicle)
		QueryRequire("physical dynamic small")
		local debries = QueryAabbBodies(aabbMin, aabbMax)
		for i = 1, #debries do
			Delete(debries[i])
		end
	end
end

initialized = false
function init()
	extended_api = GetDllVersion and true or false
	if initialized then return end
	initialized = true

	local vehicles = FindVehicles("", true)
	for i = 1, #vehicles do
		local vehicle = vehicles[i]
		InitShapeData(vehicle)
	end
	repair = nil
	last_repair_vehicle = 0
	REPAIR_KEY = GetHotkey("savegame.mod.repair", "X")
end

function tick()
	if repair and coroutine.status(repair) ~= "dead" then
		coroutine.resume(repair)
	end
	local vehicle = GetPlayerVehicle()
	if vehicle ~= 0 and InputPressed(REPAIR_KEY) then
		SetString("hud.hintinfo", "Repairing vehicle...")
		repair = coroutine.create(RepairVehicle)
		coroutine.resume(repair, vehicle, true)
		last_repair_vehicle = vehicle
	end
	if extended_api and vehicle ~= 0 and GetVehicleHealth(vehicle) < 0.1 then
		-- When the vehicle health reach zero, it is no longer regenerated when adding voxels, this may fix that
		AllowInternalFunctions()
		SetVehicleHealth(vehicle, 1.0)
		SetVehicleEngineHealth(vehicle, 1.0)
	end
	local vehicles = FindVehicles("", true)
	for i = 1, #vehicles do
		local v = vehicles[i]
		if not VehicleData[v] then
			InitShapeData(v)
		end
	end
end
