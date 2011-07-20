local set_from_string = require 'luaanalyze.util' . set_from_string

local ttable = {}

local is_table_field = set_from_string[[
  concat
  insert
  maxn
  remove
  sort
]]


local ttable = {}
function ttable:__index(k_ast)
  local name = k_ast[1]
  if is_table_field[name] then -- ok
  else
    error{name .. ' not field of table', k_ast}
  end
end
ttable.__basetype = 'table'

function ttable.import()
  globaltypematch('^table$', ttable)
end
return ttable

