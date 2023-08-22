
CONFIG_SPACING_CHAR = '  '

UncompletedLine = {
	["Message"] = "",
	["HasBeenInterrupted"] = false,
}

function UncompletedLine:new(Message)
	local instance = {}
	setmetatable(instance, self)
	self.__index = self

	instance.Message = Message
	instance.HasBeenInterrupted = false

	io.write(string.rep(CONFIG_SPACING_CHAR, Logger:GetIndent()) .. Message)

	return instance
end

function UncompletedLine:Continue(message)

	if self.HasBeenInterrupted then
		self.HasBeenInterrupted = false
		io.write(self.Message .. message)
	else
		io.write(message)
	end

	self.Message = self.Message .. message
end

function UncompletedLine:Interrupt()
	if self.HasBeenInterrupted then
		return
	end

	self.HasBeenInterrupted = true
	print()
end

function UncompletedLine:End(message)
	if self.HasBeenInterrupted then
		self.HasBeenInterrupted = false
		print(string.rep(CONFIG_SPACING_CHAR, Logger:GetIndent()) .. self.Message .. message)
	else
		print(message)
	end
end

Logger = {
	["Indent"] = 0,
	["UncompletedLine"] = {},
}

function Logger:Log(message)
	if message ~= nil then
		if self:HasUncompletedLine() and not self:GetLastUncompletedLine().HasBeenInterrupted then
			self:GetLastUncompletedLine():Interrupt()
		end

		print(string.rep(CONFIG_SPACING_CHAR, self.Indent) .. message)
	end
end

function Logger:w(message)
	self:Log(message)
end

function Logger:Push(arg)
	if arg == nil then
		self.Indent = self.Indent + 1
	elseif type(arg) == "number" then
		self.Indent = self.Indent + arg
	else
		self:Log(arg)
		self.Indent = self.Indent + 1
	end
end

function Logger:Pop(arg)
	if arg == nil then
		self.Indent = self.Indent - 1
	elseif type(arg) == "number" then
		self.Indent = self.Indent - arg
	else
		self.Indent = self.Indent - 1
		self:Log(arg)
	end

	if self.Indent < 0 then
		error("Logger: Indent is negative")
	end
end

function Logger:GetIndent()
	return self.Indent
end

function Logger:PrintTable(table)
	for key, value in pairs(table) do
		if type(value) == "table" then
			self:Push(key)
			self:PrintTable(value)
			self:Pop()
		else
			self:Log(key .. ": " .. tostring(value))
		end
	end
end

function Logger:Start(Message)
	if self:HasUncompletedLine() then
		self:GetLastUncompletedLine():Interrupt()
	end

	local newUncompletedLine = UncompletedLine:new(Message)
	table.insert(self.UncompletedLine, newUncompletedLine)
end

function Logger:Continue(Message)
	if self:HasUncompletedLine() then
		self:GetLastUncompletedLine():Continue(Message)
	else
		Logger:Start(Message)
	end
end

function Logger:End(message)
	if self:HasUncompletedLine() then
		self:GetLastUncompletedLine():End(message)
		table.remove(self.UncompletedLine)
	end
end

function Logger:Error(Message)
	if self:HasUncompletedLine() then
		self:GetLastUncompletedLine():Interrupt()
	end
	error(Message)
end

function Logger:HasUncompletedLine()
	return #self.UncompletedLine > 0
end

function Logger:GetLastUncompletedLine()
	return self.UncompletedLine[#self.UncompletedLine]
end