# TDLL

## Extended Teardown API - DLL

### What it does:
- Provides a DLL that can be used to extend the Teardown API
- Implements the functions listed below

### How to install:
Download `pros.sdk.x64.dll` and copy it to the Teardown directory, overwrite the existing file.  
Create a script that contains the text "DLL" on it's path, for example: `DLL Global Mod\main.lua` or `Content Mod\DLL_main.lua` to access the new API functions. Check the `examples` folder for examples.
Press 'F1' on the pause menu to access the DLL menu (only on OpenGL), it contains options to change the render distance and remove the boundary.

### How to uninstall:
Verify the game on Steam.
Or restore the original `pros.sdk.x64.dll` file if you have made a backup.

## New API functions:
```lua
version = GetDllVersion()
Arguments
none
Return value
version (string) – Version of the DLL

AllowInternalFunctions()
Arguments
none
Return value
none -- Allows all already loaded mods to access internal funtions

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

status, response = HttpRequest(method, endpoint, headers, request, cookies)
Arguments
method (string) – HTTP method to use
endpoint (string) – URL to send the request
headers (table) – Table with headers to send
request (string) – Request body
cookies (string) – Filename where to store and read cookies from
Return value
status (number) – HTTP status code
response (string) – Response body

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

path = GetScriptPath(script)
Arguments
script (number) – Script handle
Return value
path (string) – Path to the script

list = GetScriptEntities(script)
Arguments
script (number) – Script handle
Return value
list (table) – Indexed table with handles to all entities referenced by the script

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
GetDisplayCount
GetDisplayName
GetDisplayResolution
GetKeyByAction
GetLayoutActions
GetScriptId
GetShapeStrength
HasInputController
IndicateAchievementProgress
IsAchievementCompleted
IsBodyHitted
IsRunningOnApple
IsRunningOnEgs
IsRunningOnIOS
IsRunningOnMac
IsRunningOnSwitch
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
UiSelectProsMod
UiSelectedProsModInfo
UiSetProsModFilter
UiSubscribeToProsMod
UiUnsubscribeFromProsMod
WatchBodyHit
WinddustSpawnParticlesCPP
explosionDebrisCPP
explosionSparksCPP
fireUpdateCPP
smokeUpdateCPP
trailUpdateCPP
```
