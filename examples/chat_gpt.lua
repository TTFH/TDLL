#include "json_parser.lua"

function init()
	local query = "Respond with a random cat fact"
	local request = [[
		{
			"model": "gpt-3.5-turbo-0125",
			"messages": [
				{
					"role": "assistant",
					"content": "]] .. query .. [["
				}
			]
		}
	]]
	local headers = {
		["Content-Type"] = "application/json",
		["Authorization"] = "Bearer sk-*****",
	}
	local status, response = HttpRequest("POST", "https://api.openai.com/v1/chat/completions", headers, request)
	DebugPrint("Status: " .. tostring(status))

	local json_response = json.parse(response)
	content = json_response.choices[1].message.content
end

function draw()
	UiTranslate(100, 100)
	UiFont("regular.ttf", 26)
	UiText(content)
end
