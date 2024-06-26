#include "utils.lua"

Words = {}
InitialWords = {
	{
		index = 1,
		emoji = "💧",
		result = "Water",
		is_new = false,
		last_used = 0,
	},
	{
		index = 2,
		emoji = "🔥",
		result = "Fire",
		is_new = false,
		last_used = 0,
	},
	{
		index = 3,
		emoji = "🌬️",
		result = "Wind",
		is_new = false,
		last_used = 0,
	},
	{
		index = 4,
		emoji = "🌍",
		result = "Earth",
		is_new = false,
		last_used = 0,
	},
}

function SaveProgress()
	for _, element in ipairs(Words) do
		local registry_key = "savegame.mod." .. element.index
		SetString(registry_key .. ".emoji", element.emoji)
		SetString(registry_key .. ".result", element.result)
		if element.is_new then
			SetBool(registry_key .. ".is_new", element.is_new)
		end
	end
end

function LoadProgress()
	local list = ListKeys("savegame.mod")
	if #list == 0 then return end
	Words = {}
	for i = 1, #list do
		local index = list[i]
		local registry_key = "savegame.mod." .. index
		local emoji = GetString(registry_key .. ".emoji")
		local result = GetString(registry_key .. ".result")
		local is_new = GetBool(registry_key .. ".is_new")
		table.insert(Words, {
			index = tonumber(index),
			emoji = emoji,
			result = result,
			is_new = is_new,
			last_used = 0
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
	for _, element in ipairs(InitialWords) do
		table.insert(Words, element)
	end
	LoadProgress()
	anim_timer = 0
	anim_text = ""
	prev_selected = ""

	sorting = 1
	SortingTexts = {
		[1] = "Sort by time",
		[2] = "Sort by name",
		[3] = "Sort by emoji",
		[4] = "Sort by last used"
	}
	filter_new = false
end

function IsDuplicated(word)
	for _, element in ipairs(Words) do
		if element.result == word then
			return true
		end
	end
	return false
end

function HandleCombination(word1, word2)
	local new_combination = CombineWords(word1, word2)
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
				new_combination.index = #Words + 1
				new_combination.last_used = GetTime()
				table.insert(Words, new_combination)
				SaveProgress()
				if sorting ~= 1 then
					SortWords()
				end
			end
		else
			UiSound("MOD/snd/error.ogg")
		end
	end
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
		UiColor(0.1, 0.1, 0.1)
	else
		UiColor(0.9, 0.9, 0.9)
		UiImageBox("ui/common/box-solid-6.png", width + 50, 50, 6, 6)
		UiColor(0, 0, 0)
	end

	UiButtonImageBox("ui/common/box-outline-6.png", 6, 6)
	if UiTextButton(button_text, width + 50, 50) then
		UiSound("MOD/snd/instance.ogg")
		if prev_selected == "" then
			prev_selected = element.result
			element.last_used = GetTime()
		else
			HandleCombination(prev_selected, element.result)
			element.last_used = GetTime()
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

function ToggleDiscoveries()
	UiPush()
	if filter_new then
		UiColor(0.26, 0.23, 0.14)
		UiImageBox("ui/common/box-solid-6.png", 150, 50, 6, 6)
	end
	UiPop()
	UiPush()
	if UiTextButton("Discoveries", 150, 50) then
		filter_new = not filter_new
	end
	UiPop()
end

function SortWords()
	if sorting == 2 then
		table.sort(Words, function(a, b)
			return a.result < b.result
		end)
	elseif sorting == 3 then
		table.sort(Words, function(a, b)
			return a.emoji < b.emoji
		end)
	elseif sorting == 4 then
		table.sort(Words, function(a, b)
			return a.last_used < b.last_used
		end)
	else
		table.sort(Words, function(a, b)
			return a.index < b.index
		end)
	end
end

function SortButton()
	UiPush()
	if UiTextButton(SortingTexts[sorting], 150, 50) then
		sorting = IncrementAndLoop(sorting, #SortingTexts)
		SortWords()
	end
	UiPop()
end

gFilterText = ""
gFocusText = false
function Search()
	UiPush()
	UiImageBox("ui/common/box-outline-4.png", 300, 50, 12, 12)
	local newText = UiTextInput(gFilterText, 300, 50, gFocusText)
	gFocusText = false
	if gFilterText == "" then
		UiColor(1, 1, 1, 0.5)
		UiText("Search items...")
	else
		UiTranslate(125, 0)
		UiColor(1, 1, 1)
		if UiImageButton("ui/common/close.png") then
			newText = ""
			gFocusText = true
		end
	end
	gFilterText = newText
	UiPop()
end

scroll = 0
scroll_speed = 10
scroll_mutiplier = 4
function ScrollBar()
	UiPush()
		local height = UiHeight() - 300
		scroll = clamp(scroll - InputValue("mousewheel") * scroll_speed, 0, height)
		UiTranslate(UiWidth() - 50, UiHeight() / 2 - 50)
		UiImageBox("ui/common/box-solid-6.png", 13, height, 6, 6)
		UiTranslate(0, -height / 2)
		UiColor(1, 1, 0.5, 1)
		scroll = UiSlider("ui/common/dot.png", "y", scroll, 0, height)
	UiPop()
	UiTranslate(0, -scroll * scroll_mutiplier)
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
	UiTranslate(175, 0)
	ToggleDiscoveries()
	UiTranslate(200, 0)
	SortButton()
	UiTranslate(275, 0)
	Search()
	UiPop()

	UiPush()
	UiTranslate(0, 50)
	UiClipRect(UiWidth(), UiHeight() - 150)
	ScrollBar()
	UiTranslate(100, 50)
	for _, element in ipairs(Words) do
		if not filter_new or element.is_new then
			if gFilterText == "" or string.find(string.lower(element.result), string.lower(gFilterText)) then
				local width = DrawElement(element)
				UiTranslate(width / 2 + 75, 0)
				local x = UiGetCursorPos()
				if x > UiWidth() - 250 then
					UiTranslate(-x + 100, 75)
				end
			end
		end
	end
	UiPop()
end
