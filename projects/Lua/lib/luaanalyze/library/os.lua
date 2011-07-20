local set_from_string = require 'luaanalyze.util' . set_from_string

local tos = {}

local is_os_field = set_from_string[[
  clock
  date
  difftime
  execute
  exit
  getenv
  remove
  rename
  setlocale
  time
  tmpname
]]


local tos = {}
function tos:__index(k_ast)
  local name = k_ast[1]
  if is_os_field[name] then -- ok
  else
    error{name .. ' not field of os', k_ast}
  end
end
tos.__basetype = 'table'

function tos.import()
  globaltypematch('^os$', tos)
end
return tos

