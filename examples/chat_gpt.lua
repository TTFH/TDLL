#include "json_parser.lua"

function init()
	local query = "Respond with a random cat fact."
	body = [[
		{
			"model": "gpt-4o",
			"messages": [
				{
					"role": "user",
					"content": "]] .. query .. [["
				}
			]
		}
	]]
	headers = {
		["User-Agent"] = "Mozilla/5.0",
		["Content-Type"] = "application/json",
		["Authorization"] = "Bearer ghp_*****",
	}
	content = ""
end

frame = 0
function tick(dt)
	frame = frame + 1
	if InputPressed("X") then
		gpt_handle = HttpAsyncRequest("POST", "https://models.inference.ai.azure.com/chat/completions", headers, body)
		DebugPrint("Request sent with handle " .. gpt_handle)
		HttpAsyncRequest("GET", "https://www.google.com")
	end
	if frame % 120 == 0 then
		responses = FetchHttpResponses()
		DebugWatch("fetching...", "")
		for _, response in ipairs(responses) do
			DebugPrint("ID " .. response.id)
			DebugPrint("Status " .. response.status)
			DebugPrint("URL " .. response.url)
			if response.id == gpt_handle then
				local json_response = json.parse(response.body)
				content = json_response.choices[1].message.content
			else
				DebugPrint("Body " .. response.body)
			end
		end
	end
end

function draw()
	UiTranslate(100, 100)
	UiFont("regular.ttf", 26)
	UiText(content)
end
