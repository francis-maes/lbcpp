-- Francis Maes, 02/08/2011
-- Pure Lua dependencies

function __errorHandler(msg)
  context:error(msg)
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
  print ("inteluaLoader: " .. name)
  return interpreter:loadFile("C:/Projets/lbcpp/projects/Lua/lib/" .. name .. ".lua")
end

package.loaders = {inteluaLoader}
