#include "json_parser.lua"

function init()
	local query = "Respond with a random cat fact."
	local request = [[
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
	local headers = {
		["Content-Type"] = "application/json",
		["Authorization"] = "Bearer ghp_*****",
	}
	local status, response = HttpRequest("POST", "https://models.inference.ai.azure.com/chat/completions", headers, request)
	DebugPrint("Status: " .. tostring(status))

	local json_response = json.parse(response)
	content = json_response.choices[1].message.content
end

function draw()
	UiTranslate(100, 100)
	UiFont("regular.ttf", 26)
	UiText(content)
end
