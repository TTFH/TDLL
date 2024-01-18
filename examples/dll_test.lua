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

function init()
	local version = GetDllVersion()
	DebugPrint(version)
	AllowInternalFunctions()

	local scripts = GetScripts()
	for i = 1, #scripts do
		local script = scripts[i]
		local script_path = GetScriptPath(script)
		DebugPrint(script .. " | " .. script_path)
	end
end

function tick(dt)
	local waters = FindWaters("", true)
	for i = 1, #waters do
		DebugWater(waters[i])
	end

	local scripts = GetScripts()
	for i = 1, #scripts do
		local script = scripts[i]
		local entities = GetScriptEntities(script)
		for j = 1, #entities do
			local entity = entities[j]
			local entity_type = GetEntityType(entity)
			if entity_type == "shape" then
				DrawOBB(entity)
			end
		end
	end
end
