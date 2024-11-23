# TDLL

## Extended Teardown API - DLL

# This DLL only works on Teardown 1.6.2 Build 16262651 from 06/November/2024 and will stop working after the next game update, make sure to remove it by verifying the game files on Steam.

### What it does:
- Provides a DLL that can be used to extend the Teardown API
- Implements the functions listed below

### How to install:
Download `pros.sdk.x64.dll` and copy it to the Teardown directory, overwrite the existing file.  
Check the `examples` folder for examples.
Press 'F1' on the pause menu to access the DLL menu (only on OpenGL), it contains options to change the render distance and remove the boundary.

### How to uninstall:
Verify the game on Steam.
Or restore the original `pros.sdk.x64.dll` file if you have made a backup.

### Dependencies:
ImGui Docking branch is requiered for compiling the DLL.

## Warning: Some functions are untested and may not work or crash the game, if that happens please report it.

## New API functions:
```lua
version = GetDllVersion()
Arguments
none
Return value
version (string) – Version of the DLL

Tick(index)
Arguments
index (number) – Index of the clock, between 0 and 15, default is 0 if not provided
Return value
none -- Starts a clock

elapsed = Tock(index)
Arguments
index (number) – Index of the clock, between 0 and 15, default is 0 if not provided
Return value
elapsed (number) – Time elapsed since Tick was called in nanoseconds

hour, minute, second = GetSystemTime()
Arguments
none
Return value
hour (number) – Current hour
minute (number) – Current minute
second (number) – Current second

year, month, day = GetSystemDate()
Arguments
none
Return value
year (number) – Current year
month (number) – Current month
day (number) – Current day

status, response = HttpRequest(method, endpoint, headers, body, cookies)
Arguments
method (string) – HTTP method to use
endpoint (string) – URL to send the request
headers (table) – Table with headers to send
body (string) – Request body
cookies (string) – Filename where to store and read cookies from
Return value
status (number) – HTTP status code
response (string) – Response body

id = HttpAsyncRequest(method, endpoint, headers, body)
Arguments
method (string) – HTTP method to use
endpoint (string) – URL to send the request
headers (table) – Table with headers to send
body (string) – Request body
Return value
id (number) – Request id

responses = FetchHttpResponses()
Arguments
none
Return value
responses (table) – Table with responses [{id, url, status, body}]

width, height, pixels = LoadImagePixels(path)
Arguments
path (string) – Path to the image (relative to the game directory or absolute)
Return value
width (number) – Image width
height (number) – Image height
pixels (table) – Table with the image pixels [{r, g, b}]

SaveImageToFile(path, width, height, pixels)
Arguments
path (string) – Path to save the image (relative to the game directory or absolute)
width (number) – Image width
height (number) – Image height
pixels (table) – Table with the image pixels [{r, g, b}]
Return value
none

SendDatagram(message)
Arguments
message (string) – Message to send
Return value
none

messages = FetchDatagrams()
Arguments
none
Return value
messages (table) – Table with received messages. Note: outgoing messages are also included

scale = GetTimeScale()
Arguments
none
Return value
scale (number) – Current time scale

size = GetShadowVolumeSize()
Arguments
none
Return value
size (table) – Vector with the size of the shadow volume

list = GetBoundaryVertices()
Arguments
none
Return value
list (table) – Indexed table with vertices of the boundary

RemoveBoundary()
Arguments
none
Return value
none

SetBoundaryVertex(index, vertex)
Arguments
index (number) – Index of the vertex
vertex (table) – Vector with the new position of the vertex
Return value
none

ragdoll = IsRagdoll(animator)
Arguments
animator (number) – Animator handle
Return value
ragdoll (bool) – If the animator is a ragdoll

count = GetHeatCount()
Arguments
none
Return value
count (number) – Number of heats (blowtorch, AddHeat())

shape, pos, amount = GetHeatInfo(index)
Arguments
index (number) – Index of the heat
Return value
shape (number) – Shape handle
pos (table) – Local position of the heat in voxels
amount (number) – Amount of heat

SetSunLength(length)
Arguments
length (number) – Environment sun length
Return value
none

shape, pos = GetFireInfo(index)
Arguments
index (number) – Index of the fire
Return value
shape (number) – Shape handle
pos (table) – Local position of the fire

SetJointStrength(joint, strength, size)
Arguments
joint (number) – Joint handle
strength (number) – Joint strength for detaching (default 3000)
size (number) – Joint distance for detaching (default 0.8)
Return value
none

list = GetPaletteTintArray(palette, tint, strength)
Arguments
palette (number) – Palette index
tint (string) – Tint type ("black", "yellow", "rgba")
strength (number) – Tint strength (1 to 4)
Return value
list (table) – Array with the palette index asocition for each of the 255 indexes

AllowInternalFunctions(script)
Arguments
script (number) – Script handle
Return value

SetVehicleMaxSteerAngle(vehicle, angle)
Arguments
vehicle (number) – Vehicle handle
angle (number) – Maximum steering angle in degrees (default 30)
Return value
none

pos, axis = GetJointLocalPosAndAxis(joint, index)
Arguments
joint (number) – Joint handle
index (number) – Index of the body (1 or 2)
Return value
pos (table) – Vector with the position of the joint relative to the index-th body
axis (table) – Vector with the axis of the joint relative to the index-th body

size = GetJointSize(joint)
Arguments
joint (number) – Joint handle
Return value
size (number) – Joint size

collide, sound, autodisable = GetJointParams(joint)
Arguments
joint (number) – Joint handle
Return value
collide (bool) – If the jointed bodies collide
sound (bool) – If the joint makes sound
autodisable (bool) – Whatever this does

pos, rot = GetWaterTransform(water)
Arguments
water (number) – Water handle
Return value
transform (table) – Transform of the water

list = GetWaterVertices(water)
Arguments
water (number) – Water handle
Return value
list (table) – Indexed table with vertices of the water

SetWaterVertex(water, index, vertex)
Arguments
water (number) – Water handle
index (number) – Index of the vertex
vertex (table) – Vector with the new position of the vertex
Return value
none

path = GetScriptPath(script)
Arguments
script (number) – Script handle
Return value
path (string) – Path to the script

transform = GetWheelTransform(wheel)
Arguments
wheel (number) – Wheel handle
Return value
transform (table) – Transform of the wheel

SetWheelTransform(wheel, transform)
Arguments
wheel (number) – Wheel handle
transform (table) – Transform of the wheel
Return value
none

SetWheelRadius(wheel, radius)
Arguments
wheel (number) – Wheel handle
radius (number) – Radius of the wheel
Return value
none

list = GetTriggerVertices(trigger)
Arguments
trigger (number) – Trigger handle
Return value
list (table) – Indexed table with the vertices of the trigger

val1, val2 = GetLightSize(light)
Arguments
light (number) – Light handle
Return value
val1 (number) – Light radius or width
val2 (number) – Light length, angle or height

type, r, g, b, a, reflectivity, shininess, metallic, emissive = GetPaletteMaterial(palette, index)
Arguments
palette (number) – Palette index
index (number) – Material index
Return value
type (string) – Type
red (number) – Red value
green (number) – Green value
blue (number) – Blue value
alpha (number) – Alpha value
reflectivity (number) – Range 0 to 1
shininess (number) – Range 0 to 1
metallic (number) – Range 0 to 1
emissive (number) – Range 0 to 32

SetPaletteMaterialType(palette, index, type)
Arguments
palette (number) – Palette index
index (number) – Material index
type (string) – Material type
Return value
none

density = GetShapeDensity(shape)
Arguments
shape (number) – Shape handle
Return value
density (number) – Density

palette = GetShapePaletteId(shape)
Arguments
shape (number) – Shape handle
Return value
palette (number) – Palette index

voxels = GetShapeVoxelMatrix(shape)
Arguments
shape (number) – Shape handle
Return value
voxels (table) – Table with the voxel matrix [x][y][z]

SetShapeScale(shape, scale)
Arguments
shape (number) – Shape handle
scale (number) – Scale
Return value
none

SetShapePalette(shape, palette)
Arguments
shape (number) – Shape handle
palette (number) – Palette index
Return value
none

HasCollision(shape)
Arguments
shape (number) – Shape handle
Return value
collision (bool) – If the shape has collision

SetCollision(shape, collision)
Arguments
shape (number) – Shape handle
collision (bool) – If the shape has collision

texture, weight, blendTexture, blendTextureWeight = GetShapeTexture(shape)
Arguments
shape (number) – Shape handle
Return value
texture (number) – Texture index
weight (number) – Texture weight
blendTexture (number) – Blend texture index
blendTextureWeight (number) – Blend texture weight

offset = GetTextureOffset(shape)
Arguments
shape (number) – Shape handle
Return value
offset (table) – Vector with the texture offset

SetShapeTexture(shape, texture, weight, blendTexture, blendTextureWeight)
Arguments
shape (number) – Shape handle
texture (number) – Texture index
weight (number) – Texture weight
blendTexture (number) – Blend texture index
blendTextureWeight (number) – Blend texture weight
Return value
none

SetTextureOffset(shape, offset)
Arguments
shape (number) – Shape handle
offset (table) – Vector with the texture offset
Return value
none

align = UiGetAlign()
Arguments
none
Return value
align (string) – UI alignment

SaveToFile(file, str)
Arguments
file (string) – File path
str (string) – String to save
Return value

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
str (string) – Decompressed string or nil if file does not exist
```

## Internal functions:
```lua
ActivityAvailabilityChange
ActivityEnd
ActivityResume
ActivityStart
ActivityTerminate
CompleteAchievement
DeleteShape
FxEmitSmokeCPP
GetActionByButton
GetBodyHit
GetBodyHitsCount
GetButtonsByAction
GetClipboardText
GetDisplayCount
GetDisplayName
GetDisplayResolution
GetKeyByAction
GetLayoutActions
GetPlayerToolRecoil
GetShapeStrength
HasInputController
IndicateAchievementProgress
IsAchievementCompleted
IsBodyHitted
IsRunningOnApple
IsRunningOnEgs
IsRunningOnIOS
IsRunningOnMac
IsRunningOnPC
IsRunningOnPlaystation
IsRunningOnPlaystation5
IsRunningOnSteam
IsRunningOnSteamDeck
IsRunningOnSwitch
IsRunningOnXbox
IsRunningOnXboxS
IsRunningOnXboxX
PermitScreenRecord
ProhibitScreenRecord
ProsBrowseToQRLink
ProsHasActiveQRCode
ProsIsAccountLinked
ProsRequestQRCode
QueryLayerFilter
RadiolinkCheckConnectionCPP
RadiolinkDrawLinesCPP
ResumeLevel
RobotAimUpdateCPP
RobotFootStepCPP
RobotGetBodyParametersCPP
RobotGetTransformAndAxesCPP
RobotHoverGetUpCPP
RobotHoverUprightCPP
RobotProcessSamplesCPP
RobotRejectAllBodiesCPP
RobotRemoveTaggedJointsCPP
RobotSensorGetBlockedCPP
RobotSetBodyCollisionFilterCPP
RobotSetFootConstraintsCPP
SaveCameraOverrideTransform
SaveShape
SetClipboardText
SetPresence
SetRopeSlack
SetShapeStrength
SetVehicleEngineHealth
TornadoBodiesSuctionCPP
TornadoSpawnParticlesCPP
UiAddDrawObject
UiDislikeProsMod
UiDrawLater
UiForceMouse
UiGetModById
UiGetModsList
UiGetModsNumber
UiGetProsModAt
UiGetProsModNumber
UiGetProsModRange
UiLikeProsMod
UiLinkColor
UiRemoveDrawObject
UiRichTextSplitByWords
UiSelectProsMod
UiSelectedProsModInfo
UiSetProsModFilter
UiSubscribeToProsMod
UiTextToLower
UiTextToUpper
UiUnsubscribeFromProsMod
WatchBodyHit
WinddustSpawnParticlesCPP
explosionDebrisCPP
explosionSparksCPP
fireUpdateCPP
smokeUpdateCPP
trailUpdateCPP
```
