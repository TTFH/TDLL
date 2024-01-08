# TDLL

## Extended Teardown API - DLL

### What it does:
- It crashes the game.
- That's it.

### What it's supposed to do:
- Provides a DLL that can be used to extend the Teardown API
- Implements the next functions:

```lua
list = GetWater()
Arguments
none
Return value
list (table) – Indexed table with handles to all water entities

list = GetScripts()
Arguments
none
Return value
list (table) – Indexed table with handles to all script entities

list = GetBoundaryVertices()
Arguments
none
Return value
list (table) – Indexed table with vertices of the boundary

list = GetVehicleWheels(vehicle)
Arguments
vehicle (number) – Vehicle handle
Return value
list (table) – Indexed table with handles to all wheels of the vehicle

path = GetScriptPath(handle)
Arguments
handle (number) – Script handle
Return value
path (string) – Path to the script

handle = GetPlayerFlashlight()
Arguments
none
Return value
handle (number) – Handle to the player flashlight, a normal Light entity

transform = GetWaterTransform(water)
Arguments
water (number) – Water handle
Return value
transform (table) – Transform of the water

list = GetWaterVertices(water)
Arguments
water (number) – Water handle
Return value
list (table) – Indexed table with vertices of the water

p1, p2 = GetJointLocalBodyPos(joint)
Arguments
joint (number) – Joint handle
Return value
p1 (table) – Local position of the joint relative to the first attached body
p2 (table) – Local position of the joint relative to the second attached body

texture, weight, blendTexture, blendTextureWeight = GetShapeTexture(shape)
Arguments
shape (number) – Shape handle
Return value
texture (number) – Texture index
weight (number) – Texture weight
blendTexture (number) – Blend texture index
blendTextureWeight (number) – Blend texture weight

x, y, z = GetTextureOffset(shape)
Arguments
shape (number) – Shape handle
Return value
x (number) – X offset
y (number) – Y offset
z (number) – Z offset

SetShapeTexture(shape, texture, weight, blendTexture, blendTextureWeight)
Arguments
shape (number) – Shape handle
texture (number) – Texture index
weight (number) – Texture weight
blendTexture (number) – Blend texture index
blendTextureWeight (number) – Blend texture weight
Return value
none

SetTextureOffset(shape, x, y, z)
Arguments
shape (number) – Shape handle
x (number) – X offset
y (number) – Y offset
z (number) – Z offset
Return value
none

ZlibSaveCompressed(file, str)
Arguments
file (string) – File path
str (string) – String to compress
Return value
none

str = ZlibLoadCompressed(file)
Arguments
file (string) – File path
Return value
str (string) – Decompressed string
```

## TODO:
- Do not crash the game
- Register functions automatically
- Rename GetWater() to FindWater()
- Rename GetScripts() to FindScripts()
- Change return type of GetTextureOffset(shape) to Vec()
- Implement SetRenderDistance(dist)
- Implement ToggleBoundary()
- Implement tr = GetJointWorldTransform(j)
- Does the water/scripts have tags? Can I get local script entities? for example to implement FindWater(tag, global)
