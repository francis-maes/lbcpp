-- Francis Maes, 02/08/2011
-- Pure Lua dependencies


__linesMaps = {}

local function parseErrorMessage(str)
  local chunkName = string.gmatch(str, '%[string ".*"%]')()
  chunkName = string.sub(chunkName, 10, #chunkName - 2)
  local lineNumber = string.gmatch(str, ":%d*:")()
  lineNumber = string.sub(lineNumber, 2, #lineNumber - 1)
  local lineNumberIndex = string.find(str, ":%d*:")
  lineNumberIndex = string.find(str, ":", lineNumberIndex + 1)
  local message = string.sub(str, lineNumberIndex + 2)
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
  context:error(message, chunkName .. ":" .. lineNumber)
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
  --print ("inteluaLoader: " .. name)
  return interpreter:loadFile("C:/Projets/lbcpp/projects/Lua/lib/" .. name .. ".lua")
end

package.loaders = {inteluaLoader}