local set_from_string = require 'luaanalyze.util' . set_from_string

local tdebug = {}

local is_debug_field = set_from_string[[
  debug
  getfenv
  gethook
  getinfo
  getlocal
  getmetatable
  getregistry
  getupvalue
  setfenv
  sethook
  setlocal
  setmetatable
  setupvalue
  traceback
]]


local tdebug = {}
function tdebug:__index(k_ast)
  local name = k_ast[1]
  if is_debug_field[name] then -- ok
  else
    error{name .. ' not field of debug', k_ast}
  end
end
tdebug.__basetype = 'table'

function tdebug.import()
  globaltypematch('^debug$', tdebug)
end
return tdebug

