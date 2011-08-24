-- Francis Maes, 02/08/2011
-- Pure Lua dependencies


__linesMaps = {}

local function parseErrorMessage(str)
  local chunkName = string.gmatch(str, '%[string ".*"%]')()
  if chunkName then
    chunkName = string.sub(chunkName, 10, #chunkName - 2)
  end
  local lineNumber = string.gmatch(str, ":%d*:")()
  if lineNumber then
    lineNumber = string.sub(lineNumber, 2, #lineNumber - 1)
  end
  local lineNumberIndex = string.find(str, ":%d*:")
  local message
  if lineNumberIndex then
    lineNumberIndex = string.find(str, ":", lineNumberIndex + 1)
    message = string.sub(str, lineNumberIndex + 2)
  else
    message = str
  end
  return chunkName, lineNumber, message
end

function __errorHandler(msg)
  local chunkName, lineNumber, message = parseErrorMessage(msg)
  --print ("ChunkName: ", chunkName, " linesMaps: ", __linesMaps[chunkName])
  local map = __linesMaps[chunkName]
  if map then
    --for i,v in ipairs(map) do print (i,v) end
    --print (lineNumber,map[lineNumber-1])
    local ln = map[lineNumber - 1]
    if ln then
      --print ("Youpi: " .. lineNumber .. " => " .. ln)
      lineNumber = ln
    end
  end
  if message == nil then
    message = "Error"
  end
  local where = chunkName or ""
  if lineNumber then
    where = (where and (where .. ":") or "") .. lineNumber
  end
  context:error(message or "Error", where)
  return msg
end

function print(...)
  context:information(...)
end

require 'Context'
require 'Subspecified'
require 'Derivable'
require 'Stochastic'

local function inteluaLoader(name)
  local path
  if package.searchpath then
    path = package.searchpath(name, package.path)
  else
    name = string.gsub(name, "%.", "/")
    path = package.inteluaPath .. "/" .. name .. ".lua"
  end
  --print ("inteluaLoader: " .. name, path)  
  return path and interpreter:loadFile(path)
end

package.loaders[2] = inteluaLoader