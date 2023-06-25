
function CreateNewBuffer()
	local buffer = {}
	buffer.indentationLevel = 0
	buffer.string = ""

	-- Write a formatted string to the exported file, and increase the indentation level by one
	function buffer.push(input, ...)
		if input == nil or type(input) == "number"then
			buffer.indentationLevel = buffer.indentationLevel + (input or 1)
		else
			buffer.w(input, ...)
			buffer.indentationLevel = buffer.indentationLevel + 1
		end
	end

	-- Write a new line with the current indentation level to the string buffer
	function buffer.w(...)
		if select("#", ...) > 0 then
			buffer.string = buffer.string .. string.rep("\t", buffer.indentationLevel) .. string.format(...) .. '\n'
		else
			buffer.string = buffer.string .. '\n'
		end
	end


	-- Write a formatted string to the exported file, after decreasing the indentation level by one
	function buffer.pop(input, ...)
		if input == nil or type(input) == "number"then
			buffer.indentationLevel = buffer.indentationLevel - (input or 1)
		else
			buffer.indentationLevel = buffer.indentationLevel - 1
			buffer.w(input, ...)
		end
	end

	return buffer
end