#include "utils.lua"

InitialWords = {
	{
		result = "Water",
		emoji = "💧",
		is_new = false
	},
	{
		result = "Fire",
		emoji = "🔥",
		is_new = false
	},
	{
		result = "Wind",
		emoji = "💨",
		is_new = false
	},
	{
		result = "Earth",
		emoji = "🌍",
		is_new = false
	},
}

function SaveProgress()
	for i, element in ipairs(Words) do
		local registry_key = "savegame.mod." .. i
		SetString(registry_key .. ".result", element.result)
		SetString(registry_key .. ".emoji", element.emoji)
		SetBool(registry_key .. ".is_new", element.is_new)
	end
end

function LoadProgress()
	local list = ListKeys("savegame.mod")
	if #list == 0 then return end
	Words = {}
	for i = 1, #list do
		local index = list[i]
		local registry_key = "savegame.mod." .. index
		local result = GetString(registry_key .. ".result")
		local emoji = GetString(registry_key .. ".emoji")
		local is_new = GetBool(registry_key .. ".is_new")
		table.insert(Words, {
			result = result,
			emoji = emoji,
			is_new = is_new
		})
	end
end

function ClearProgress()
	Words = {}
	for _, element in ipairs(InitialWords) do
		table.insert(Words, element)
	end
	ClearKey("savegame.mod")
end

function init()
	Words = {}
	for _, element in ipairs(InitialWords) do
		table.insert(Words, element)
	end
	LoadProgress()
	anim_timer = 0
	anim_text = ""
	prev_selected = ""
end

function IsDuplicated(word)
	for _, element in ipairs(Words) do
		if element.result == word then
			return true
		end
	end
	return false
end

rotation = 0
function DrawElement(element)
	local selected = element.result == prev_selected
	local button_text = "    " .. element.result
	local width = UiGetTextSize(button_text)
	UiTranslate(width / 2, 0)

	UiPush()
	if element.is_new or (anim_text == element.result and anim_timer > 0) then
		anim_timer = anim_timer - GetTimeStep()
		UiScale(0.5)
		rotation = rotation + 5.0 * GetTimeStep()
		UiRotate(rotation)
		UiImage("MOD/pinwheel.png")
	end
	UiPop()

	UiPush()
	if selected then
		UiColor(1, 0.98, 0.9)
		UiImageBox("ui/common/box-solid-6.png", width + 50, 50, 6, 6)
		UiColor(1, 1, 1)
	end

	UiButtonImageBox("ui/common/box-outline-6.png", 6, 6)
	if UiTextButton(button_text, width + 50, 50) then
		UiSound("MOD/snd/instance.ogg")
		if prev_selected == "" then
			prev_selected = element.result
		else
			local new_combination = CombineWords(prev_selected, element.result)
			if new_combination and new_combination.result then
				if new_combination.result ~= "Nothing" then
					anim_text = new_combination.result
					anim_timer = 2
					if not IsDuplicated(new_combination.result) then
						if new_combination.is_new then
							UiSound("MOD/snd/discovery.ogg")
						else
							UiSound("MOD/snd/reward.ogg")
						end
						table.insert(Words, new_combination)
						SaveProgress()
					end
				else
					UiSound("MOD/snd/error.ogg")
				end
			end
			prev_selected = ""
		end
	end
	UiPop()

	UiPush()
	UiTranslate(-width / 2, 0)
	UiScale(26 / 72)
	DrawEmoji(element.emoji)
	UiPop()

	return width
end

function draw()
	UiAlign("center middle")
	UiFont("bold.ttf", 30)
	UiButtonHoverColor(0.7, 0.7, 0.7)

	UiPush()
	UiTranslate(100, UiHeight() - 50)
	UiButtonImageBox("ui/common/box-outline-6.png", 6, 6)
	if UiTextButton("Reset", 100, 50) then
		ClearProgress()
	end
	UiPop()

	UiTranslate(200, 100)
	for _, element in ipairs(Words) do
		local width = DrawElement(element)
		UiTranslate(width / 2 + 75, 0)
		local x = UiGetCursorPos()
		if x > UiWidth() - 200 then
			UiTranslate(-x + 200, 75)
		end
	end
end
