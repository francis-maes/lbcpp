-- Francis Maes, 01/08/2011
-- Data Parsers

require 'Vector'
require 'Dictionary'

Parser = {}

function Parser.libSVMClassification(filename, labels)
  local f,err = io.open(filename)
  if err then 
    error("Could not open file " .. filename)
  end

  while true do
    local line = f:read()
    if line == nil then break end

    label = string.gmatch(line, "[%+%-%w]+")()
    if #label == 0 then print("Oops"); return; end

    x = Vector.newSparse()
    local app = x.append
    for k,v in string.gmatch(line, "(%d+):([%d%.-]+)") do
      app(x, tonumber(k), tonumber(v))
    end
    coroutine.yield({x, labels:add(label)})
  end
  f:close()
end

return Parser