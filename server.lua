

-- USCRIPT_VERSION 1
local list = {
  "Do", "End", -- do ... end
  "Dump", -- (num)
  "Mode", -- pinMode(pin, mode)
  "Read", -- digitalRead(pin)
  "Write", -- digitalWrite(pin, value)
  "Aread", -- analogRead(pin)
  "Pwrite", -- analogWrite(pin, value)
  "Ibegin", -- Wiring.begin(sda, scl)
  "Ifrom", -- Wiring.requestFrom(address, quantity, stop)
  "Istart", -- Wiring.beginTransmission(address)
  "Istop", -- Wiring.endTransmission(stop)
  "Iwrite", -- Wiring.write(byte)
  "Iavailable", -- Wiring.available()
  "Iread", -- Wiring.read()
  "Tone", -- tone(pin, frequency, ms)
  "Delay", -- delay(ms)
  "Call", -- (stackOffset, codeOffset)
  "Gosub", -- (codeOffset)
  "Goto", -- (codeOffset)
  "Gget", -- (var)
  "Gset", -- (var, value)
  "Get", -- (var)
  "Set", -- (var, value)
  "Incr", "Decr", -- (var)
  "IncrMod", "DecrMod", -- (var, mod)
  "Forever", -- (action)
  "While", -- (condition, action)
  "Wait", -- (condition)
  "If", -- (condition, action)
  "ElseIf", -- (condition, action)
  "Else", -- (action)
  "Add", "Sub", "Mul", "Div", "Mod", "Neg",
  "Band", "Bor", "Bxor", "Bnot", "Lshift", "Rshift",
  "And", "Or", "Xor", "Not", "Choose",
  "Gt", "Gte", "Lt", "Lte", "Eq", "Neq",
  "Srand", -- deadbeef_srand(seed)
  "Rand", -- deadbeef_rand(modulus)
  "Restart", "ChipId", "FlashChipId", "CycleCount", "GetFree",
}
local op = {};
for i = 1, #list do
  op[list[i]] = i + 127
end


local codes = {
  ["\001\000\241\207\157"] = string.char(op.Do,
    op.Mode, 5, 1, -- orange
    op.Mode, 6, 1, -- yellow
    op.Mode, 7, 1, -- green
    op.Mode, 8, 1, -- blue

    op.Write, 5, 0,
    op.Write, 6, 0,
    op.Write, 7, 0,
    op.Write, 8, 0,

    op.Ibegin, 2, 1,

    -- Turn the oscillator on
    op.Istart, 0x40, 0x70, -- Wiring.beginTransmission(0x70)
    op.Iwrite, 0x21, -- Wiring.write(OSCILLATOR_ON)
    op.Istop, 0, -- Wiring.stop(0)

    -- Turn off blink
    op.Istart, 0x40, 0x70, -- Wiring.beginTransmission(0x70)
    op.Iwrite, 0x41, 0x01, -- Wiring.write(0x81)
    op.Istop, 0, -- Wiring.stop(0)

    -- Set brightness to 15
    op.Istart, 0x40, 0x70, -- Wiring.beginTransmission(0x70)
    op.Iwrite, 0x41, 0x6f, -- Wiring.write(0xef)
    op.Istop, 0, -- Wiring.stop(0)

    op.Set, 0, 0,

    op.Forever, op.Do,

      -- left forward
      op.Write, 8, 0,
      op.Write, 5, 1,
      op.Gosub, 26,

      -- left backwards
      op.Write, 5, 0,
      op.Write, 6, 1,
      op.Gosub, 18,

      -- left forward
      op.Write, 6, 0,
      op.Write, 7, 1,
      op.Gosub, 10,

      -- left backwards
      op.Write, 7, 0,
      op.Write, 8, 1,
      op.Gosub, 2,

    op.End,
  op.End,

  op.Do,
    op.Set, 3, 0x41, 0x00,
    op.While, op.Decr, 3, op.Do,
      op.Istart, 0x40, 0x70, -- Wiring.beginTransmission(0x70)
      op.Iwrite, op.IncrMod, 0, 16,
      op.Iwrite, op.Rand, 0x42, 0x00, -- Wiring.write(0xa0)
      op.Istop, 0, -- Wiring.stop(0)
      op.Delay, 3,
    op.End,
  op.End),
  ["\001\000\166?="] = string.char(op.Do,

    op.Mode, 7, 1,

    op.Ibegin, 2, 1,

    -- Turn the oscillator on
    op.Istart, 0x40, 0x70, -- Wiring.beginTransmission(0x70)
    op.Iwrite, 0x21, -- Wiring.write(OSCILLATOR_ON)
    op.Istop, 0, -- Wiring.stop(0)

    -- Turn off blink
    op.Istart, 0x40, 0x70, -- Wiring.beginTransmission(0x70)
    op.Iwrite, 0x41, 0x01, -- Wiring.write(0x81)
    op.Istop, 0, -- Wiring.stop(0)

    -- Set brightness to 15
    op.Istart, 0x40, 0x70, -- Wiring.beginTransmission(0x70)
    op.Iwrite, 0x41, 0x6f, -- Wiring.write(0xef)
    op.Istop, 0, -- Wiring.stop(0)

    op.Set, 0, 0, -- x offset of start

    op.Forever, op.Do,
      op.Set, 1, 8,
      op.While, op.Get, 1, op.Do,
        op.Decr, 1,
        op.Istart, 0x40, 0x70,
        op.Iwrite, op.Mul, op.Get, 1, 2,
        op.Iwrite, op.Lshift, 1, op.Get, 0,
        op.Istop, 0,
        op.Set, 0, op.Mod, op.Add, op.Get, 0, op.Rand, 2, 8,
      op.End,
      op.Set, 0, op.Mod, op.Add, op.Get, 0, 4, 8,
      op.Write, 7, op.Rand, 2,
      op.Delay, 30,
    op.End,

  op.End),

  ["\001\000\166@\169"] = string.char(op.Do,
    -- Set output mode for LED pins
    op.Mode, 5, 1, -- blue
    op.Mode, 6, 1, -- green
    op.Mode, 7, 1, -- red
    -- Use 8 as ground
    op.Mode, 8, 1, -- ground
    op.Write, 8, 0,

    -- Set input with pull-up for buttons
    op.Mode, 1, 2, -- blue button, pull-up input
    op.Mode, 2, 2, -- yellow button, pull-up input
    -- op.Write, 2, 1,

    op.Set, 2, 2,
    op.Set, 3, 1,

    op.Set, 0, 0, -- Set variable to zero
    op.Forever, op.Do,
      op.If, op.Not, op.Read, 1, op.Do,
        op.IncrMod, 0, 6, -- Increment variable looping around at 6
        op.Gosub, 61,
        --op.Call, 0, op.Gget, 0,
        op.Delay, 0x41, 0x00,
        op.Wait, op.Read, 1,
        op.Delay, 0x41, 0x00,
      op.End,
      op.ElseIf, op.Not, op.Read, 2, op.Do,
        op.DecrMod, 0, 6, -- Decrement variable looping around at 6
        op.Gosub, 41,
        op.Delay, 0x41, 0x00,
        op.Wait, op.Read, 2,
        op.Delay, 0x41, 0x00,
      op.End,
      -- b += c
      op.Set, 2, op.Add, op.Get, 2, op.Get, 3,
      -- if b > 100 { c = -1 }
      -- if b < 2 { c = 1 }
      op.If, op.Gte, op.Get, 2, 0x41, 0x00,
        op.Set, 3, op.Neg, 1,
      op.ElseIf, op.Lte, op.Get, 2, 1,
        op.Set, 3, 1,

      --
      op.Gosub, 4,

      op.Delay, 10,
    op.End,
  op.End,

  -- i  R G B
  -- 0  1 0 0
  -- 1  1 1 0
  -- 2  0 1 0
  -- 3  0 1 1
  -- 4  0 0 1
  -- 5  1 0 1
  op.Do,
    op.Pwrite, 7, op.Mul, op.Get, 2,
      op.Or,
        op.Lte, op.Get, 0, 1,
        op.Gte, op.Get, 0, 5,
    op.Pwrite, 6, op.Mul, op.Get, 2,
      op.And,
        op.Gte, op.Get, 0, 1,
        op.Lte, op.Get, 0, 3,
    op.Pwrite, 5, op.Mul, op.Get, 2,
      op.Gte, op.Get, 0, 3,
  op.End)

}
require('coro-net').createServer({
  host = "0.0.0.0",
  port = 1337
}, function (read, write, socket)
  p(socket:getpeername())
  local data = ""
  for chunk in read do
    data = data .. chunk
    if #data >= 5 then
      p(data)
      local code = codes[data:sub(1, 5)]
      if not code then return end
      local len = #code
      write(string.char(
        bit.rshift(len, 8),
        bit.band(len, 0xff)
      ) .. code);
      break
    end
  end
  for chunk in read do
    p(chunk)
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
