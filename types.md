strongly typed language

Every word has type signature showing the values it inputs and outputs.

compiler enforces types

word definitions (procedures) must explicitly define their interface.

boolean    - int8_t
integer    - intptr_t
byte array - struct { intptr_t* length; uint8_t data[] }
word array - struct { intptr_t* length; uint32_t data[] }
pin        - int8_t
pinMode    - enum {INPUT, OUTPUT, INPUT_PULLUP}
