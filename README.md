# TDLL

## Extended Teardown API - DLL

### What it does:
- Provides a DLL that can be used to extend the Teardown API
- Implements the functions listed below

### How to install:
Download `pros.sdk.x64.dll` and copy it to the Teardown directory, overwrite the existing file.  
Create a script that contains the text "DLL" on it's path, for example: `DLL Global Mod\main.lua` or `Content Mod\DLL_main.lua` to access the new API functions.  
Check `main.lua` for examples on how to use the new API functions.  
Press 'F1' on the pause menu to access the DLL options.

### How to uninstall:
Verify the game on Steam.
Or restore the original `pros.sdk.x64.dll` file if you made a backup.

### TODO:
- Implement ToggleBoundary()
- Implement SetRenderDistance(dist)
- Get entity tags and local script entities to implement FindWater[s](tag, global), FindScript[s](tag, global), FindWheel[s](tag, global)
- Add ImGui API for Lua

## New API functions:

```lua
list = FindWaters()
Arguments
none
Return value
list (table) – Indexed table with handles to all water entities

list = FindScripts()
Arguments
none
Return value
list (table) – Indexed table with handles to all script entities

list = FindWheels()
Arguments
none
Return value
list (table) – Indexed table with handles to all wheel entities

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

handle = GetWheelVehicle(wheel)
Arguments
wheel (number) – Wheel handle
Return value
handle (number) – Handle to the vehicle the wheel is part of

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

size = GetShadowVolumeSize()
Arguments
none
Return value
size (number) – Vector with the size of the shadow volume

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

## Internal functions:

```lua
fuction ActivityAvailabilityChange(...)
fuction ActivityEnd(...)
fuction ActivityResume(...)
fuction ActivityStart(...)
fuction ActivityTerminate(...)
fuction CompleteAchievement(...)
fuction DeleteShape(...)
fuction explosionDebrisCPP(...)
fuction explosionSparksCPP(...)
fuction fireUpdateCPP(...)
fuction FxEmitSmokeCPP(...)
fuction GetActionByButton(...)
fuction GetBodyHit(...)
fuction GetBodyHitsCount(...)
fuction GetButtonsByAction(...)
fuction GetDisplayCount(...)
fuction GetDisplayName(...)
fuction GetDisplayResolution(...)
fuction GetKeyByAction(...)
fuction GetLayoutActions(...)
fuction GetScriptId(...)
fuction GetShapeStrength(...)
fuction HasInputController(...)
fuction IndicateAchievementProgress(...)
fuction IsAchievementCompleted(...)
fuction IsBodyHitted(...)
fuction IsRunningOnEgs(...)
fuction IsRunningOnSwitch(...)
fuction PermitScreenRecord(...)
fuction ProhibitScreenRecord(...)
fuction ProsBrowseToQRLink(...)
fuction ProsHasActiveQRCode(...)
fuction ProsIsAccountLinked(...)
fuction ProsRequestQRCode(...)
fuction QueryLayerFilter(...)
fuction RadiolinkCheckConnectionCPP(...)
fuction RadiolinkDrawLinesCPP(...)
fuction ResumeLevel(...)
fuction RobotAimUpdateCPP(...)
fuction RobotFootStepCPP(...)
fuction RobotGetBodyParametersCPP(...)
fuction RobotGetTransformAndAxesCPP(...)
fuction RobotHoverGetUpCPP(...)
fuction RobotHoverUprightCPP(...)
fuction RobotProcessSamplesCPP(...)
fuction RobotRejectAllBodiesCPP(...)
fuction RobotRemoveTaggedJointsCPP(...)
fuction RobotSensorGetBlockedCPP(...)
fuction RobotSetBodyCollisionFilterCPP(...)
fuction RobotSetFootConstraintsCPP(...)
fuction SaveShape(...)
fuction SetPresence(...)
fuction SetRopeSlack(rope, slack)
fuction SetShapeStrength(...)
fuction SetVehicleEngineHealth(...)
fuction smokeUpdateCPP(...)
fuction TornadoBodiesSuctionCPP(...)
fuction TornadoSpawnParticlesCPP(...)
fuction trailUpdateCPP(...)
fuction UiAddDrawObject(...)
fuction UiDislikeProsMod(...)
fuction UiDrawLater(...)
fuction UiForceMouse(...)
fuction UiGetFrameNo(...)
fuction UiGetProsModNumber(...)
fuction UiGetProsModShortInfo(...)
fuction UiLikeProsMod(...)
fuction UiRemoveDrawObject(...)
fuction UiSelectedProsModInfo(...)
fuction UiSelectProsMod(...)
fuction UiSetProsModFilter(...)
fuction UiSubscribeToProsMod(...)
fuction UiUnsubscribeFromProsMod(...)
fuction WatchBodyHit(...)
fuction WinddustSpawnParticlesCPP(...)
```
