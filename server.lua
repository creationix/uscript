

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
  "Forever", -- (action)
  "While", -- (condition, action)
  "Wait", -- (condition)
  "If", -- (condition, action)
  "ElseIf", -- (condition, action)
  "Else", -- (action)
  "Do", "End", -- do ... end
  "Add", "Sub", "Mul", "Div", "Mod", "Neg",
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

local code1 = string.char(
  op.Do,
  op.Mode, 13, 1,
  op.Forever, op.Do,
    op.Write, 13, 1,
    op.Delay, 0x47, 0x68, -- 1000
    op.Write, 13, 0,
    op.Delay, 0x47, 0x68, -- 1000
  op.End,
op.End)

local code2 = string.char(
  op.Do,
  op.Mode, 13, 1,
  op.Forever, op.Do,
    op.Write, 13, 1,
    op.Delay, 60,
    op.Write, 13, 0,
    op.Delay, 60,
  op.End,
op.End)

local uv = require('uv')

require('coro-net').createServer({
  host = "0.0.0.0",
  port = 1337
}, function (read, write, socket)
  p(socket:getpeername())
  local codes = {code1, code2}
  local i = 1
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
