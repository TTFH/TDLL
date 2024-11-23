function init()
	voxels = {}
	palette = {}
	shape_tr = Transform()
end

function tick(dt)
	if InputPressed("X") then
		local camera_tr = GetPlayerCameraTransform()
		local camera_fwd = TransformToParentVec(camera_tr, Vec(0, 0, -1))
		local hit, dist, _, shape = QueryRaycast(camera_tr.pos, camera_fwd, 100)
		if hit then
			--local hit_pos = VecAdd(camera_tr.pos, VecScale(camera_fwd, dist))
			local voxel_count = GetShapeVoxelCount(shape)
			if voxel_count < 256 * 256 * 10 then
				shape_tr = GetShapeWorldTransform(shape)

				local palette_id = GetShapePaletteId(shape)
				voxels = GetShapeVoxelMatrix(shape)
				for i = 1, 255 do
					local _, r, g, b = GetPaletteMaterial(palette_id, i)
					palette[i] = { r = r, g = g, b = b }
				end
			end
		end
	end

	for x = 1, #voxels do
		for y = 1, #voxels[x] do
			for z = 1, #voxels[x][y] do
				local index = voxels[x][y][z]
				if index ~= 0 then
					local offset = VecScale(Vec(x, y, z), 0.1)
					local global_pos = TransformToParentPoint(shape_tr, offset)
					local color = palette[index]
					DebugCross(global_pos, color.r, color.g, color.b)
				end
			end
		end
	end

end
