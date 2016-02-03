import Graphics.Element exposing (show)

-- Hardware config

type alias Pin = Int -- index for GPIO pin
type alias Color = Int -- Color as 24-bit integer
type alias Label = String -- labels in source code.

type Side
  = Cathode -- Will serialize to 0
  | Anode   -- Will serialize to 1

type Device
  = GlobalDevice
  | LedDevice (Pin, Side)   -- Pin for GPIO and which side of LED we're on.
  | RgbDevice (Pin, Pin, Pin, Side) --  R, G, B, and side for GPIOs.
  | ButtonDevice (Pin)      -- GPIO is pullup, button connects to ground.
  | SpeakerDevice (Pin)     -- GPIO is high-side, speaker connects to ground.
  | I2CDevice (Pin, Pin)    -- SDA and SDC pins.
  | WS2812Device (Pin, Int) -- Pin and number of LEDs in string.

type LedAction
  = Off
  | On
  | SetBrightness (Int) -- Brightness 0-255

type RgbAction
  = SetColor (Int) -- RGB value as 24-bit integer

type ButtonAction
  = DigitalRead -- returns true if pressed
  | WaitPress   -- blocks waiting for press (delay, wait up, delay, wait down)
  | OnPress (Label) -- gosub label when pressed.

type SpeakerAction
  = Sound (Int, Int) -- Frequency (in hz), duration (in ms)
  | Play String -- Play song in MML format. (see mml.txt)
  | RTTTL String -- Play song in RTTTL format.

type I2CAction
  = RequestFrom (Int, Int, Bool) -- address, quantity, stop
  | BeginTransmission (Int) -- address
  | EndTransmission (Bool) -- stop
  | Write (Int) -- byte
  | Available
  | Read

type WS2812Action
  = Set (Int, Color) -- Set value as 24-bit RGB integer
  | Send Color -- Raw direct write
  | Update -- flush buffer to device

type GlobalAction
  = Delay (Int) -- Pause for ms
  | OnTimeout (Int, Label) -- Wait ms and then gosub at label.
  | RGB (Int, Int, Int) -- Red, Green, Blue return 24-bit color.
  | HSL (Int, Int, Int) -- Hue, Saturation, Lightness -> 24-color.
  | Srand (Int) -- Seed the random
  | Rand (Int) -- Get random value modulo int
  | Restart -- restart microcontroller
  | ChipId -- read chip id
  | FlashChipId -- read flash chip id
  | CycleCount -- read cycle count
  | GetFree -- read free memory

type Action
  = Global GlobalAction
  | Led (Device, LedAction)
  | Rgb (Device, RgbAction)
  | Button (Device, ButtonAction)
  | Speaker (Device, SpeakerAction)
  | I2C (Device, I2CAction)
  | WS2812 (Device, WS2812Action)

--
--
-- type Opcode
--   = Empty
--   | Return (Opcode)
--   | Call (Opcode, Opcode)
--   | Gosub (Opcode)
--   | Goto (Opcode)
--   | Gget (Opcode)
--   | Gset (Opcode, Opcode)
--   | Get (Opcode)
--   | Set (Opcode, Opcode)
--   | Incr (Opcode)
--   | Decr (Opcode)
--   | IncrMod (Opcode, Opcode)
--   | DecrMod (Opcode, Opcode)
--   | Forever (Opcode)
--   | While (Opcode)
--   | Wait (Opcode)
--   | If (Opcode, Opcode)
--   | ElseIf (Opcode, Opcode)
--   | Else (Opcode)
--   | Add (Opcode, Opcode)
--   | Sub (Opcode, Opcode)
--   | Mul (Opcode, Opcode)
--   | Div (Opcode, Opcode)
--   | Mod (Opcode, Opcode)
--   | Neg (Opcode)
--   | Band (Opcode, Opcode)
--   | Bor (Opcode, Opcode)
--   | Bxor (Opcode, Opcode)
--   | Bnot (Opcode)
--   | Lshift (Opcode, Opcode)
--   | Rshift (Opcode, Opcode)
--   | And (Opcode, Opcode)
--   | Or (Opcode, Opcode)
--   | Xor (Opcode, Opcode)
--   | Not (Opcode)
--   | Choose (Opcode, Opcode, Opcode)
--   | Gt (Opcode, Opcode)
--   | Gte (Opcode, Opcode)
--   | Lt (Opcode, Opcode)
--   | Lte (Opcode, Opcode)
--   | Eq (Opcode, Opcode)
--   | Neq (Opcode, Opcode)
--
--
--
-- sparkle = do
--   var y 0
--   var i 128
--   while decr i do
--       istart 0x70 -- Wiring.beginTransmission(0x70)
--       iwrite incrmod y 16
--       iwrite rand 0xa0 -- Wiring.write(0xa0)
--       istop 0 -- Wiring.stop(0)
--       delay 3
--     end
--
--
-- main: do
--   mode 5 1 -- orange
--   mode 6 1 -- yellow
--   mode 7 1 -- green
--   mode 8 1 -- blue
--
--   write 5 0
--   write 6 0
--   write 7 0
--   write 8 0
--
--   ibegin 2 1
--
--   -- Turn the oscillator on
--   istart 0x70 -- Wiring.beginTransmission(0x70)
--   iwrite 0x21 -- Wiring.write(OSCILLATOR_ON)
--   istop 0 -- Wiring.stop(0)
--
--   -- Turn off blink
--   istart 0x70 -- Wiring.beginTransmission(0x70)
--   iwrite 0x81 -- Wiring.write(0x81)
--   istop 0 -- Wiring.stop(0)
--
--   -- Set brightness to 15
--   istart 0x70 -- Wiring.beginTransmission(0x70)
--   iwrite 0xef -- Wiring.write(0xef)
--   istop 0 -- Wiring.stop(0)
--
--   forever do
--
--       -- left forward
--       write 8 0
--       write 5 1
--       gosub sparkle
--
--       -- left backwards
--       write 5 0
--       write 6 1
--       gosub sparkle
--
--       -- right forward
--       write 6 0
--       write 7 1
--       gosub sparkle
--
--       -- right backwards
--       write 7 0
--       write 8 1
--       gosub sparkle
--
--     end
-- end
--
--
--
-- main: Graphics.Element.Element
-- main = show 42
