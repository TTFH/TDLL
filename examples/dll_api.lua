---@return string version Version of the DLL
function GetDllVersion() return "" end

--- Enables all functions that are "only for internal usage"
function AllowInternalFunctions() end

---@param index any Index of the clock, between 0 and 15, default is 0 if not provided
function Tick(index) end

---@param index any Index of the clock, between 0 and 15, default is 0 if not provided
---@return number elapsed Time elapsed since Tick was called in nanoseconds
function Tock(index) return 0 end

---@return number hour Current hour
---@return number minute Current minute
---@return number second Current second
function GetSystemTime() return 0, 0, 0 end

---@return number year Current year
---@return number month Current month
---@return number day Current day
function GetSystemDate() return 0, 0, 0 end

---@param method string HTTP method to use
---@param endpoint string URL to send the request
---@param headers any Table with headers to send
---@param request any Request body
---@param cookies any Filename where to store and read cookies from
---@return number status HTTP status code
---@return string response Response body
function HttpRequest(method, endpoint, headers, request, cookies) return 0, "" end

---@return number scale Current time scale
function GetTimeScale() return 0 end

---@return any size Vector with the size of the shadow volume
function GetShadowVolumeSize() return nil end

---@return any list Indexed table with vertices of the boundary
function GetBoundaryVertices() return nil end

function RemoveBoundary() end

---@param index number Index of the vertex
---@param vertex any Vector with the new position of the vertex
function SetBoundaryVertex(index, vertex) end

---@param joint number Joint handle
---@param index number Index of the body (1 or 2)
---@return any pos Vector with the position of the joint relative to the index-th body
---@return any axis Vector with the axis of the joint relative to the index-th body
function GetJointLocalPosAndAxis(joint, index) return nil, nil end

---@param joint number Joint handle
---@return number size Joint size
function GetJointSize(joint) return 0 end

---@param joint number Joint handle
---@return boolean collide If the jointed bodies collide
---@return boolean sound If the joint makes sound
---@return boolean autodisable Whatever this does
function GetJointParams(joint) return false, false, false end

---@param water number Water handle
---@return any transform Transform of the water
function GetWaterTransform(water) return nil end

---@param water number Water handle
---@return any list Indexed table with vertices of the water
function GetWaterVertices(water) return nil end

---@param water number Water handle
---@param index number Index of the vertex
---@param vertex any Vector with the new position of the vertex
function SetWaterVertex(water, index, vertex) end

---@param script number Script handle
---@return string path Path to the script
function GetScriptPath(script) return "" end

---@param script number Script handle
---@return any list Indexed table with handles to all entities referenced by the script
function GetScriptEntities(script) return nil end

---@param wheel number Wheel handle
---@return any transform Transform of the wheel
function GetWheelTransform(wheel) return nil end

---@param wheel number Wheel handle
---@param transform any Transform of the wheel
function SetWheelTransform(wheel, transform) end

---@param wheel number Wheel handle
---@param radius number Radius of the wheel
function SetWheelRadius(wheel, radius) end

---@param trigger number Trigger handle
---@return any list Indexed table with the vertices of the trigger
function GetTriggerVertices(trigger) return nil end

---@param light number Light handle
---@return number val1 Light radius or width
---@return number val2 Light length, angle or height
function GetLightSize(light) return 0, 0 end

---@param palette number Palette index
---@param index number Material index
---@return string type Type
---@return number r Red value
---@return number g Green value
---@return number b Blue value
---@return number a Alpha value
---@return number reflectivity Range 0 to 1
---@return number shininess Range 0 to 1
---@return number metallic Range 0 to 1
---@return number emissive Range 0 to 32
function GetPaletteMaterial(palette, index) return "", 0, 0, 0, 0, 0, 0, 0, 0 end

---@param shape number Shape handle
---@param scale number Scale
function SetShapeScale(shape, scale) end

---@param shape number Shape handle
---@param palette number Palette index
function SetShapePalette(shape, palette) end

---@param shape number Shape handle
---@return number density Density
function GetShapeDensity(shape) return 0 end

---@param shape number Shape handle
---@return number palette Palette index
function GetShapePaletteId(shape) return 0 end

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

---@return string align UI alignment
function UiGetAlign() return "" end

---@param file string File path
---@param str string String to save
function SaveToFile(file, str) end

---@param file string File path
---@param str string String to compress
function ZlibSaveCompressed(file, str) end

---@param file string File path
---@return any str Decompressed string or nil if file does not exist
function ZlibLoadCompressed(file) return nil end
