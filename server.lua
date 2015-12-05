

-- USCRIPT_VERSION 1
local list = {
  "Mode", -- (pin, mode)
  "Read", -- (pin)
  "Write", -- (pin, value)
  "Aread", -- (pin)
  "Pwrite", -- (pin, value)
  "Ibegin", -- Wiring.begin(sda, scl)
  "Ifrom", -- Wiring.requestFrom(address, quantity, stop)
  "Istart", -- Wiring.beginTransmission(address)
  "Istop", -- Wiring.endTransmission(stop)
  "Iwrite", -- Wiring.write(byte)
  "Iavailable", -- Wiring.available()
  "Iread", -- Wiring.read()
  "Delay", -- (ms)
  "Gget", -- (index)
  "Gset", -- (index, value)
  "Get", -- (index)
  "Set", -- (index, value)
  "Incr", "Decr", -- (index)
  "IncrMod", "DecrMod", -- (index, mod)
  "Forever", -- (action)
  "While", -- (condition, action)
  "Wait", -- (condition)
  "If", -- (condition, action)
  "ElseIf", -- (condition, action)
  "Else", -- (action)
  "Do", "End", -- do ... end
  "Add", "Sub", "Mul", "Div", "Mod", "Neg",
  "Band", "Bor", "Bxor", "Bnot", "Lshift", "Rshift",
  "And", "Or", "Xor", "Not", "Choose",
  "Gt", "Gte", "Lt", "Lte", "Eq", "Neq",
  "Srand", -- (seed)
  "Rand", -- (modulus)
  "Restart", "ChipId", "FlashChipId", "CycleCount",
}
local op = {};
for i = 1, #list do
  op[list[i]] = i + 127
end

-- GPIO map for nodemcu
local pin = {
  [0] = 16,
  [1] = 5,
  [2] = 4,
  [3] = 0,
  [4] = 2,
  [5] = 14,
  [6] = 12,
  [7] = 13,
  [8] = 15,
  [9] = 3,
  [10] = 1,
  [11] = 9,
  [12] = 10,
}

local codes = {
  -- string.char(op.Do,
  --   op.Mode, 13, 1,
  --   op.Forever, op.Do,
  --     op.Write, 13, 1,
  --     op.Delay, 0x47, 0x68, -- 1000
  --     op.Write, 13, 0,
  --     op.Delay, 0x47, 0x68, -- 1000
  --   op.End,
  -- op.End),
  -- string.char(op.Do,
  --   op.Mode, 13, 1,
  --   op.Forever, op.Do,
  --     op.Write, 13, 1,
  --     op.Delay, 60,
  --     op.Write, 13, 0,
  --     op.Delay, 60,
  --   op.End,
  -- op.End),
  string.char(op.Do,
    -- Set output mode for LED pins
    op.Mode, pin[5], 1, -- blue
    op.Mode, pin[6], 1, -- green
    op.Mode, pin[7], 1, -- red
    -- Use 8 as ground
    op.Mode, pin[8], 1, -- ground
    op.Write, pin[8], 0,

    -- Set input with pull-up for buttons
    op.Mode, pin[1], 0, -- blue button
    op.Write, pin[1], 1,
    op.Mode, pin[2], 0, -- yellow button
    op.Write, pin[2], 1,

    op.Set, 0, 0, -- Set variable to zero
    op.Forever, op.Do,
      op.If, op.Read, pin[1],
        op.IncrMod, 0, 6, -- Increment variable looping around at 6
      op.If, op.Read, pin[2],
        op.DecrMod, 0, 6, -- Decrement variable looping around at 6

      -- i  R G B
      -- 0  1 0 0
      -- 1  1 1 0
      -- 2  0 1 0
      -- 3  0 1 1
      -- 4  0 0 1
      -- 5  1 0 1
      op.Write, pin[7], op.Or,
        op.Lte, op.Get, 0, 1,
        op.Gte, op.Get, 0, 5,
      op.Write, pin[6], op.And,
        op.Gte, op.Get, 0, 1,
        op.Lte, op.Get, 0, 3,
      op.Write, pin[5],
        op.Gte, op.Get, 0, 3,

      op.Delay,
        0x43, 0x74, -- 500
    op.End,
  op.End)
}

local uv = require('uv')

require('coro-net').createServer({
  host = "0.0.0.0",
  port = 1337
}, function (read, write, socket)
  p(socket:getpeername())
  local i = 0
  uv.new_timer():start(0, 10000, function ()
    coroutine.wrap(function ()
      i = (i % #codes) + 1
      local code = codes[i]
      local len = #code
      write(string.char(
        bit.rshift(len, 8),
        bit.band(len, 0xff)
      ) .. code);
    end)()
  end)

  for data in read do
    p(data)
  end
  write()
end)
print("uScript uController endpoint on port 1337")

require('weblit-app')

  .bind({
    host = "0.0.0.0",
    port = 8080
  })

  .use(require('weblit-logger'))
  .use(require('weblit-auto-headers'))
  .use(require('weblit-etag-cache'))

  .use(require('weblit-static')(module.dir .. "/www"))

  .start()
