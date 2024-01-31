---@return string version Version of the DLL
function GetDllVersion() return "" end

--- Enables all functions that are "only for internal usage"
function AllowInternalFunctions() end


---@return any size Vector with the size of the shadow volume
function GetShadowVolumeSize() return nil end

---@return any list Indexed table with vertices of the boundary
function GetBoundaryVertices() return nil end

---@return number handle Handle to the player flashlight, a normal Light entity
function GetPlayerFlashlight() return 0 end


---@param joint number Joint handle
---@param index number Index of the body (1 or 2)
---@return any pos Vector with the position of the joint relative to the index-th body
---@return any axis Vector with the axis of the joint relative to the index-th body
function GetJointLocalPosAndAxis(joint, index) return nil, nil end

---@param joint number Joint handle
---@return boolean collide If the jointed bodies collide
---@return boolean sound If the joint makes sound
---@return boolean autodisable Whatever this does
function GetJointParams(joint) return false, false, false end

---@param rope number Joint handle
---@return number r Red component of the color
---@return number g Green component of the color
---@return number b Blue component of the color
function GetRopeColor(rope) return 0, 0, 0 end


---@return any list Indexed table with handles to all water entities
function GetWaters() return nil end

---@param water number Water handle
---@return any transform Transform of the water
function GetWaterTransform(water) return nil end

---@param water number Water handle
---@return any list Indexed table with vertices of the water
function GetWaterVertices(water) return nil end


---@return any list Indexed table with handles to all script entities
function GetScripts() return nil end

---@param script number Script handle
---@return string path Path to the script
function GetScriptPath(script) return "" end

---@param script number Script handle
---@return any list Indexed table with handles to all entities referenced by the script
function GetScriptEntities(script) return nil end


---@return any list Indexed table with handles to all wheel entities
function GetWheels() return nil end

---@param wheel number Wheel handle
---@return number vehicle Vehicle handle
function GetWheelVehicle(wheel) return 0 end

---@param vehicle number Vehicle handle
---@return any list Indexed table with handles to all wheels of the vehicle
function GetVehicleWheels(vehicle) return nil end


---@param trigger number Trigger handle
---@return string type Type of the trigger, can be box, sphere or polygon
function GetTriggerType(trigger) return "" end

---@param trigger number Trigger handle
---@return any size Vector with the size of the trigger if type is box, number if type is sphere or polygon
function GetTriggerSize(trigger) return nil end

---@param trigger number Trigger handle
---@return any list Indexed table with the vertices of the trigger
function GetTriggerVertices(trigger) return nil end


---@param shape number Shape handle
---@param palette number Palette index
function SetShapePalette(shape, palette) end

---@param shape number Shape handle
---@return number palette Palette index
function GetShapePaletteIndex(shape) return 0 end

---@param shape number Shape handle
---@return number texture Texture index
---@return number weight Texture weight
---@return number blendTexture Blend texture index
---@return number blendTextureWeight Blend texture weight
function GetShapeTexture(shape) return 0, 1, 0, 1 end

---@param shape number Shape handle
---@return any offset Vector with the texture offset
function GetTextureOffset(shape) return nil end

---@param shape number Shape handle
---@param texture number Texture index
---@param weight number Texture weight
---@param blendTexture number Blend texture index
---@param blendTextureWeight number Blend texture weight
function SetShapeTexture(shape, texture, weight, blendTexture, blendTextureWeight) end

---@param shape number Shape handle
---@param offset any Vector with the texture offset
function SetTextureOffset(shape, offset) end


---@param file string File path
---@param str string String to compress
function ZlibSaveCompressed(file, str) end

---@param file string File path
---@return any str Decompressed string or nil if file does not exist
function ZlibLoadCompressed(file) return nil end
