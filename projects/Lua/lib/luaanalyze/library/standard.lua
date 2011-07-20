local set_from_string = require 'luaanalyze.util' . set_from_string

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

local packages = {}
packages.math      = typeimport "luaanalyze.library.math"
packages.string    = typeimport "luaanalyze.library.string"
packages.io        =  typeimport "luaanalyze.library.io"
packages.os        = typeimport "luaanalyze.library.os"
packages.table     =  typeimport "luaanalyze.library.table"
packages.package   =  typeimport "luaanalyze.library.package"
packages.coroutine = typeimport "luaanalyze.library.coroutine"
packages.debug     = typeimport "luaanalyze.library.debug"

local is_writable_global = {}
local is_readable_global = {}

local tg = {}
function tg:__index(k_ast)
  local name = k_ast[1]
  if is_base_field[name] then --ok
  elseif packages[name] then
    return packages[name]
  elseif is_readable_global[name] then
    -- allow
  elseif is_writable_global[name] then --:FIX:improve
    -- allow
  else
    error{tostring(name) .. ' not in _G', k_ast}
  end
end
function tg.allow_write(name)
  is_writable_global[name] = true
end
function tg.add_field(name)
  is_readable_global[name] = true
end

function tg.import()
  for k in pairs(is_base_field) do
    globaltypematch('^' .. k .. '$', tg)
  end
  typeimport "luaanalyze.library.math"
  typeimport "luaanalyze.library.string"
  typeimport "luaanalyze.library.io"
  typeimport "luaanalyze.library.os"
  typeimport "luaanalyze.library.table"
  typeimport "luaanalyze.library.package"
  typeimport "luaanalyze.library.coroutine"
  typeimport "luaanalyze.library.debug"
end
return tg
