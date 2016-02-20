#include "helpers.c"

static state_t* S;

void checkStr(value_t v, int max, const char* expected) {
  uint8_t* end = writeValue(buffer, buffer + max, S, v);
  int len = (int)(end - buffer);
  printf("%.*s =? %s\n", len, buffer, expected);
  assert((int)strlen(expected) == len);
  assert(strncmp(expected, (char*)buffer, (size_t)len) == 0);
}

int main() {
  S = State();
  value_t v;
  checkStr(Integer(S, 1234567890123), 10, "1234567890");
  checkStr(Integer(S, -1234567890123), 10, "-123456789");
  checkStr(Int(0), 10, "0");
  checkStr(Integer(S, 12345), 10, "12345");
  checkStr(Integer(S, -12345), 10, "-12345");
  v = Bool(true);
  checkStr(v, 10, "true");
  checkStr(v, 2, "tr");
  v = Bool(false);
  checkStr(v, 10, "false");
  checkStr(v, 2, "fa");
  checkStr(Rational(S, 1, 2), 10, "1/2");
  checkStr(Rational(S, 1, 0), 10, "Inf");
  checkStr(Rational(S, -1, 0), 10, "-Inf");
  checkStr(Rational(S, 0, 0), 10, "NaN");
  checkStr(Rational(S, 4321, 8765), 10, "4321/8765");
  checkStr(Char(0), 10, "'\\0'");
  checkStr(Char(1), 10, "'\\1'");
  checkStr(Char('\a'), 10, "'\\a'");
  checkStr(Char('\b'), 10, "'\\b'");
  checkStr(Char('\f'), 10, "'\\f'");
  checkStr(Char('\r'), 10, "'\\r'");
  checkStr(Char('\n'), 10, "'\\n'");
  checkStr(Char('\t'), 10, "'\\t'");
  checkStr(Char('\v'), 10, "'\\v'");
  checkStr(Char('a'), 10, "'a'");
  checkStr(Char('A'), 10, "'A'");
  checkStr(Char('5'), 10, "'5'");
  checkStr(Char(0x0278), 10, "'ɸ'");
  checkStr(Char(0x05d4), 10, "'ה'");
  checkStr(Char(0x232c), 10, "'⌬'");
  checkStr(Char(0x25b6), 10, "'▶'");
  checkStr(Char(0x27f3), 10, "'⟳'");
  checkStr(Char(0x1f604), 10, "'😄'");
  checkStr(Char(0x1f63c), 10, "'😼'");
  checkStr(Char(0x1f074), 10, "'🁴'");
  checkStr(Char(0x1f78b), 10, "'🞋'");
  checkStr(String(S, -1, (uint8_t*)"Hello World\n"), 20, "\"Hello World\\n\"");
  checkStr(String(S, 3, (uint8_t*)"*\0*"), 20, "\"*\\0*\"");
  checkStr(String(S, -1, (uint8_t*)"🞋🁴ɸ⌬▶😼"), 30, "\"🞋🁴ɸ⌬▶😼\"");
  checkStr(Symbol(S, -1, (uint8_t*)"name"), 30, ":name");
  checkStr(Buffer(S, 3, (uint8_t[]){1,2,3}), 30, "<01 02 03>");
  checkStr(Pixels(S, 3, (uint32_t[]){1,2,3}), 30, "<00000001 00000002 00000003>");
  checkStr(Pair(S, Int(1), Int(2)), 10, "(1 2)");
  v = Stack(S);
  checkStr(v, 20, "[]");
  stackPush(S, v, Char('A'));
  checkStr(v, 20, "['A']");
  stackPush(S, v, Char('B'));
  checkStr(v, 20, "['B' 'A']");
  stackPush(S, v, Char('C'));
  checkStr(v, 20, "['C' 'B' 'A']");
  v = stackReverse(S, v);
  checkStr(v, 20, "['A' 'B' 'C']");
  v = Set(S);
  checkStr(v, 20, "||");
  setAdd(S, v, Int(10));
  checkStr(v, 20, "|10|");
  setAdd(S, v, Int(20));
  checkStr(v, 20, "|10 20|");
  v = Map(S);
  checkStr(v, 20, "{}");
  mapSet(S, v, Int(10), Bool(true));
  checkStr(v, 20, "{10:true}");
  mapSet(S, v, Int(20), Bool(false));
  checkStr(v, 20, "{10:true 20:false}");


  freeState(S);
  return 0;
}
