function init()
	width, height, pixels = LoadImagePixels("preview.png")
	--SaveImageToFile("output.jpg", width, height, pixels)
end

function draw()
	UiTranslate(50, 50)
	-- Iteration example, for drawing use UiImage instead
	for i = 1, height do
		for j = 1, width do
			local index = (i - 1) * width + j
			local pixel = pixels[index]
			UiColor(pixel.r / 255, pixel.g / 255, pixel.b / 255)
			UiRect(1, 1)
			UiTranslate(1, 0)
		end
		UiTranslate(-width, 1)
	end

	UiColor(1, 1, 1)
	UiTranslate(width + 50, -height)
	UiImage("preview.png")
end
