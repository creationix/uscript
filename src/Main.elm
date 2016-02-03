import Graphics.Element exposing (show)

{--

--}


type Opcode
  = Do (List Opcode)
  | Return (Opcode)
  | Dump (Opcode)
  | Mode (Opcode, Opcode)
  | Read (Opcode)
  | Write (Opcode, Opcode)
  | Aread (Opcode)
  | Pwrite (Opcode, Opcode)
  | Ibegin (Opcode, Opcode)
  | Ifrom (Opcode, Opcode, Opcode)
  | Istart (Opcode)
  | Istop (Opcode)
  | Iwrite (Opcode)
  | Iavailable
  | Iread
  | Tone (Opcode, Opcode, Opcode)
  | Delay (Opcode)
  | Call (Opcode, Opcode)
  | Gosub (Opcode)
  | Goto (Opcode)
  | Gget (Opcode)
  | Gset (Opcode, Opcode)
  | Get (Opcode)
  | Set (Opcode, Opcode)
  | Incr (Opcode)
  | Decr (Opcode)
  | IncrMod (Opcode, Opcode)
  | DecrMod (Opcode, Opcode)
  | Forever (Opcode)
  | While (Opcode)
  | Wait (Opcode)
  | If (Opcode, Opcode)
  | ElseIf (Opcode, Opcode)
  | Else (Opcode)
  | Add (Opcode, Opcode)
  | Sub (Opcode, Opcode)
  | Mul (Opcode, Opcode)
  | Div (Opcode, Opcode)
  | Mod (Opcode, Opcode)
  | Neg (Opcode)
  | Band (Opcode, Opcode)
  | Bor (Opcode, Opcode)
  | Bxor (Opcode, Opcode)
  | Bnot (Opcode)
  | Lshift (Opcode, Opcode)
  | Rshift (Opcode, Opcode)
  | And (Opcode, Opcode)
  | Or (Opcode, Opcode)
  | Xor (Opcode, Opcode)
  | Not (Opcode)
  | Choose (Opcode, Opcode, Opcode)
  | Gt (Opcode, Opcode)
  | Gte (Opcode, Opcode)
  | Lt (Opcode, Opcode)
  | Lte (Opcode, Opcode)
  | Eq (Opcode, Opcode)
  | Neq (Opcode, Opcode)
  | Srand (Opcode)
  | Rand (Opcode)
  | Restart
  | ChipId
  | FlashChipId
  | CycleCount
  | GetFree



sparkle = do
  var y 0
  var i 128
  while decr i do
      istart 0x70 -- Wiring.beginTransmission(0x70)
      iwrite incrmod y 16
      iwrite rand 0xa0 -- Wiring.write(0xa0)
      istop 0 -- Wiring.stop(0)
      delay 3
    end


main: do
  mode 5 1 -- orange
  mode 6 1 -- yellow
  mode 7 1 -- green
  mode 8 1 -- blue

  write 5 0
  write 6 0
  write 7 0
  write 8 0

  ibegin 2 1

  -- Turn the oscillator on
  istart 0x70 -- Wiring.beginTransmission(0x70)
  iwrite 0x21 -- Wiring.write(OSCILLATOR_ON)
  istop 0 -- Wiring.stop(0)

  -- Turn off blink
  istart 0x70 -- Wiring.beginTransmission(0x70)
  iwrite 0x81 -- Wiring.write(0x81)
  istop 0 -- Wiring.stop(0)

  -- Set brightness to 15
  istart 0x70 -- Wiring.beginTransmission(0x70)
  iwrite 0xef -- Wiring.write(0xef)
  istop 0 -- Wiring.stop(0)

  forever do

      -- left forward
      write 8 0
      write 5 1
      gosub sparkle

      -- left backwards
      write 5 0
      write 6 1
      gosub sparkle

      -- right forward
      write 6 0
      write 7 1
      gosub sparkle

      -- right backwards
      write 7 0
      write 8 1
      gosub sparkle

    end
end



main: Graphics.Element.Element
main = show 42
