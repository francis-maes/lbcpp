local set_from_string = require 'luaanalyze.util' . set_from_string

local tcoroutine = {}

local is_coroutine_field = set_from_string[[
  create
  resume
  running
  status
  wrap
  yield
]]


local tcoroutine = {}
function tcoroutine:__index(k_ast)
  local name = k_ast[1]
  if is_coroutine_field[name] then -- ok
  else
    error{name .. ' not field of coroutine', k_ast}
  end
end
tcoroutine.__basetype = 'table'

function tcoroutine.import()
  globaltypematch('^coroutine$', tcoroutine)
end
return tcoroutine

