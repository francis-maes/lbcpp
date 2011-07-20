local set_from_string = require 'luaanalyze.util' . set_from_string

local tio = {}

local is_io_field = set_from_string[[
  close
  flush
  input
  lines
  open
  output
  popen
  read
  stderr
  stdin
  stdout
  tmpfile
  type
  write
]]


local tio = {}
function tio:__index(k_ast)
  local name = k_ast[1]
  if is_io_field[name] then -- ok
  else
    error{name .. ' not field of io', k_ast}
  end
end
tio.__basetype = 'table'

function tio.import()
  globaltypematch('^io$', tio)
end
return tio

