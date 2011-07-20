--! typeimport 'luaanalyze.library.standard'
--! checkglobals()
--! checktypes()

local M = {}

local function set_from_string(s)
  local set = {}
  for name in s:gmatch('%S+') do
    set[name] = true
  end
  return set
end
M.set_from_string = set_from_string

return M
