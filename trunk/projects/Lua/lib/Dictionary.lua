-- Francis Maes, 01/08/2011
-- Dictionary of strings

Dictionary = {}

--Dictionary = {}
Dictionary.mt = {__index = Dictionary}

function Dictionary.mt.__tostring(dict)
  local res = ""
  for i,v in ipairs(dict.content) do
    if #res > 0 then
      res = res .. " "
    end
    res = res .. v
  end
  return res
end

function Dictionary.mt.__len(dict)
  print ("pouet: " .. #dict.content)
  return #dict.content
end

function Dictionary.get(self, stringOrIndex)
  return self.content[stringOrIndex]
end

function Dictionary.add(self, string)
  --if not isstring(string) then
  --  error("Not a string")
  --end
  local index = self:get(string)
  if not index then
    table.insert(self.content, string)
    index = #self.content
    self.content[string] = index
  end
  return index
end

function Dictionary.size(self)
  return #self.content
end

function Dictionary.new()
  local res = { content = {} }
  return setmetatable(res, Dictionary.mt)
end

return Dictionary