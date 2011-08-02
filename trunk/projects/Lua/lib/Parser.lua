-- Francis Maes, 01/08/2011
-- Data Parsers

module("Parser", package.seeall)

function libSVMClassification(filename, labels)
  local f,err = io.open(filename)
  if err then print("OOps"); return; end

  while true do
    local line = f:read()
    if line == nil then break end

    label = string.gmatch(line, "[%+%-%w]+")()
    if #label == 0 then print("Oops"); return; end

    x = Vector.newSparse()
    for k,v in string.gmatch(line, "(%d+):(%d+)") do
      x:append(tonumber(k), tonumber(v))
    end
    coroutine.yield({x, labels:add(label)})
  end
  f:close()
end
