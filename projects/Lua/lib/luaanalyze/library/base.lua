local set_from_string = require 'luaanalyze.util' . set_from_string

local tbase = {}

local is_base_field = set_from_string[[
_G
_VERSION
assert
collectgarbage
dofile
error
getfenv
getmetatable
ipairs
load
loadfile
loadstring
module
next
pairs
pcall
print
rawequal
rawget
rawset
require
select
setfenv
setmetatable
tonumber
tostring
type
unpack
xpcall
]]


local tbase = {}
function tbase:__index(k_ast)
  local name = k_ast[1]
  if is_base_field[name] then -- ok
  else
    error{name .. ' not field of _G', k_ast}
  end
end
tbase.__basetype = 'table'

function tbase.import()
  globaltypematch('^_G$', tos)
end
return tbase

