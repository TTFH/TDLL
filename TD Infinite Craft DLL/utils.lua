#include "json_parser.lua"
#include "emoji_sprites.lua"

function GetIndex(tbl, key)
	for i, v in ipairs(tbl) do
		if v == key then
			return i
		end
	end
	return nil
end

function IncrementAndLoop(i, N)
	return i % N + 1
end

function clamp(value, mi, ma)
	if value < mi then value = mi end
	if value > ma then value = ma end
	return value
end

function utf8_codepoint(byte)
	local n = string.byte(byte)
	if n < 128 then
		return n
	elseif n < 224 then
		local a = string.byte(byte, 1) - 192
		local b = string.byte(byte, 2) - 128
		return a * 64 + b
	elseif n < 240 then
		local a = string.byte(byte, 1) - 224
		local b = string.byte(byte, 2) - 128
		local c = string.byte(byte, 3) - 128
		return a * 4096 + b * 64 + c
	elseif n < 248 then
		local a = string.byte(byte, 1) - 240
		local b = string.byte(byte, 2) - 128
		local c = string.byte(byte, 3) - 128
		local d = string.byte(byte, 4) - 128
		return a * 262144 + b * 4096 + c * 64 + d
	end
	return 0
end

function EmojiToUnicode(emoji)
	local codePoints = {}
	for code in emoji:gmatch('[%z\1-\127\194-\244][\128-\191]*') do
		table.insert(codePoints, string.format('%04X', utf8_codepoint(code)))
	end
	return table.concat(codePoints, "-")
end

function GetSpriteTile(index)
	local x = (index - 1) % EMOJI_ROW
	local y = math.floor((index - 1) / EMOJI_ROW)
	return x, y
end

EmojiCache = {}

function GetImageAndTile(emoji)
	if EmojiCache[emoji] then
		return EmojiCache[emoji]
	end
	local unicode = EmojiToUnicode(emoji)
	for i = 1, #EMOJI_RESOURCES do
		local resource = EMOJI_RESOURCES[i]
		local hexcodes = resource.hexcodes
		local index = GetIndex(hexcodes, unicode)
		if index then
			local x, y = GetSpriteTile(index)
			EmojiCache[emoji] = { image = resource.image, x = x, y = y }
			return EmojiCache[emoji]
		end
	end

	local fallback = unicode:match("([^%-]*)")
	if fallback ~= unicode then
		for i = 1, #EMOJI_RESOURCES do
			local resource = EMOJI_RESOURCES[i]
			local hexcodes = resource.hexcodes
			local index = GetIndex(hexcodes, fallback)
			if index then
				local x, y = GetSpriteTile(index)
				EmojiCache[emoji] = { image = resource.image, x = x, y = y }
				return EmojiCache[emoji]
			end
		end
	end

	return { image = "", x = 0, y = 0 }
end

function EncodeUriComponent(str)
	local res = string.gsub(str, "([^%w])", function(c)
		return string.format("%%%02X", string.byte(c))
	end)
	return res
end

PairsCache = {}

function CombineWords(first, second)
	if PairsCache[first] and PairsCache[first][second] then
		return PairsCache[first][second]
	end
	if PairsCache[second] and PairsCache[second][first] then
		return PairsCache[second][first]
	end
	if not GetDllVersion then
		SetString("hud.hintinfo", "TDLL .dll is not installed")
		return {}
	end
	local headers = {
		["User-Agent"] = "Solver",
		["Referer"] = "https://neal.fun/infinite-craft/",
	}
	local endpoint = "https://neal.fun/api/infinite-craft/pair?first=" .. EncodeUriComponent(first) .. "&second=" .. EncodeUriComponent(second)
	local status, response = HttpRequest("GET", endpoint, headers)
	if status ~= 200 then
		SetString("hud.hintinfo", "Failed to fetch data " .. response)
		return {}
	end
	local json_response = json.parse(response)
	if not json_response then
		return {}
	end
	local result = json_response.result
	local emoji = json_response.emoji
	local is_new = json_response.isNew
	PairsCache[first] = PairsCache[first] or {}
	PairsCache[first][second] = { result = result, emoji = emoji, is_new = is_new }
	PairsCache[second] = PairsCache[second] or {}
	PairsCache[second][first] = PairsCache[first][second]
	return PairsCache[first][second]
end

function DrawEmoji(emoji)
	local data = GetImageAndTile(emoji)
	local image = data.image
	local x = data.x
	local y = data.y
	UiImage(image, x * EMOJI_WIDTH, y * EMOJI_WIDTH, (x + 1) * EMOJI_WIDTH, (y + 1) * EMOJI_WIDTH)
end
