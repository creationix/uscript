// Mapping from builtin opcode to arity
var BUILTINS = {
  // Integer operations
  NEG: 1, ADD: 2, SUB: 2, MUL: 2, DIV: 2, MOD: 2,
  // Bitwise operations
  BNOT: 1, BXOR: 2, BAND: 2, BOR: 2, LSHIFT: 2, RSHIFT: 2,
  // Comparison operations
  GT: 2, GTE: 2, LT: 2, LTE: 2, EQ: 2, NEQ: 2,
  // Logical operations
  NOT: 1, AND: 2, OR: 2, XOR: 2, CHOOSE: 3,
  // Pseudo random number generator
  SRAND: 1, RAND: 1,
  // Garbage Collection
  GC: 1,
  // Buffer operations
  LEN: 1, PEEK: 2, POKE: 3, FILL: 4, COPY: 5, INDEX: 2, FIND: 2,
  // Pair operations
  HEAD: 1, TAIL: 1, SETHEAD: 2, SETTAIL: 2,
  // List operations
  LLEN: 1, LIDX: 2, LGET: 2, LSET: 3, LDEL: 2,
  // Set operations
  SHAS: 2, SDEL: 2, SADD: 2,
  // Map operations
  MSET: 3, MGET: 2, MDEL: 2,
  // GPIO operations
  MODE: 2, WRITE: 2, READ: 1, PWRITE: 2, AREAD: 1,
  // I2C operations
  ISETUP: 3, ISTART: 1, ISTOP: 1, IADDR: 3, IWRITE: 2, IREAD: 1,
  // Local variables
  LET: 2, SET: 2, GET: 1,
  // Pausing statements
  DELAY: 1, UDELAY: 1, WAIT: 1, YIELD: 0,
  // Control Flow
  IF: 0, THEN: 0, ELSE: 0, ELIF: 0,
  // Loops / Iterators
  WHILE: 1, DOWHILE: 1, FOR: 3, EACH: 1,
  // Loop control flow
  BREAK: 0, CONTINUE: 0,
  // Function calling and definition
  FUN: 1, DO: 0, RETURN: 1, END: 0,
  // Macros
  DEF: 2, PRINT: 1,
};

function parser(state, type, token) {
  "use strict";
  if (state.depth < 0) return "error";
  if (type === "keyword" && (token === "DEF" || token === "FUN")) {
    state.mode = token;
    return type;
  }
  if (type === "keyword") {
    if (token === "DO" || token === "THEN") {
      console.log(token);
      state.depth++;
    }
    else if (token === "ELIF" || token === "END") {
      console.log(token);
      state.depth--;
    }
  }

  if (type === "keyword" && token === "END") {
    console.log("END", state.depth);
    if (!state.depth) state.params = {};
    if (state.depth < 0) return "error";
    return type;
  }
  if (state.mode === "DEF") {
    state.mode = undefined;
    if (type !== "variable") return "error";
    if (state.defs[token] || state.params[token] || state.fns[token]) return "error";
    state.defs[token] = true;
    return type;
  }
  if (state.mode === "FUN") {
    state.mode = "ARGS";
    state.depth = 0;
    if (type !== "variable") return "error";
    if (state.defs[token] || state.params[token] || state.fns[token]) return "error";
    state.fns[token] = true;
    return "builtin";
  }
  if (state.mode === "ARGS") {
    if (type === "keyword" && token === "DO") {
      state.mode = undefined;
      return type;
    }
    if (type !== "variable") return "error";
    if (state.defs[token] || state.params[token] || state.fns[token]) return "error";
    state.params[token] = true;
    return "variable-2";
  }
  if (type === "variable") {
    if (state.params[token]) return "variable-2";
    if (state.fns[token]) return "builtin";
    if (state.defs[token]) return "variable";
    return "error";
  }
  return type;
}

CodeMirror.defineMode("uscript-asm", function (cm) {
  "use strict";
  var opcode = new RegExp("^(" + Object.keys(BUILTINS).join("|") + ")\\b", "i");
  var number = /^([-]?0[bB][01]+|[-]?0[oO][07]+|[-]?0[xX][0-9a-fA-f]+|[-]?[1-9][0-9]*|0)\b/;
  var boolean = /^(true|false)\b/;
  var comment = /^[-][-]/;
  var symbol = /^@[a-z_]\w*/i;
  var ident = /^[a-z_]\w*/i;
  var bracket = /^[(){}\[\]<>:]/i;
  var string = /^"(?:[^"]|\\.)*"/;
  var char = /^'(?:[^']|\\.)'/;

  return {
    startState: function () {
      return {
        defs: {},
        params: {},
        fns: {},
        depth: 0,
      };
    },
    copyState: function (state) {
      var key;
      var defs = {};
      for (key in state.defs) defs[key] = state.defs;
      var fns = {};
      for (key in state.fns) fns[key] = state.fns;
      var params = {};
      for (key in state.params) params[key] = state.params;
      return {
        defs: defs,
        fns: fns,
        params: params,
        depth: state.depth,
      };
    },
    lineComment: "--",
    electricInput: /end$/i,
    fold: "uscript",
    indent: function (state, textAfter) {
      var depth = state.depth;
      var match;
      while ((match = textAfter.match(/^[^"]*(do|then|end|[-][-].*|"([^"]|\\.)*")/))) {
        match = match[0].toUpperCase();
        if (match == "DO" || match == "THEN") depth++;
        else if (match == "END") depth--;
        textAfter = textAfter.substring(match.length);
      }
      return depth * cm.indentUnit;
    },
    token: function (stream, state) {
      if (stream.eatSpace()) return;
      if (stream.match(comment)) {
        stream.skipToEnd();
        return "comment";
      }
      if (stream.match(opcode)) {
        return parser(state, "keyword", stream.current().toUpperCase());
      }
      if (stream.match(number)) {
        return parser(state, "number", stream.current());
      }
      if (stream.match(boolean)) {
        return parser(state, "atom", stream.current());
      }
      if (stream.match(symbol)) {
        return parser(state, "property", stream.current());
      }
      if (stream.match(ident)) {
        return parser(state, "variable", stream.current());
      }
      if (stream.match(bracket)) {
        return parser(state, "bracket", stream.current());
      }
      if (stream.match(char)) {
        return parser(state, "string-2", stream.current());
      }
      if (stream.match(string)) {
        return parser(state, "string", stream.current());
      }
      if (stream.match(/\S+/)) {
        return "error";
      }
      stream.next();
    }
  };
});

CodeMirror.registerHelper("fold", "uscript", function(cm, start) {
  var line = start.line, lineText = cm.getLine(line);
  var startCh, tokenType;

  function findOpening(openCh) {
    for (var at = start.ch, pass = 0;;) {
      var found = at <= 0 ? -1 : lineText.lastIndexOf(openCh, at - 1);
      if (found == -1) {
        if (pass == 1) break;
        pass = 1;
        at = lineText.length;
        continue;
      }
      if (pass == 1 && found < start.ch) break;
      tokenType = cm.getTokenTypeAt(CodeMirror.Pos(line, found + 1));
      if (!/^(comment|string)/.test(tokenType)) return found + 1;
      at = found - 1;
    }
  }

  var startToken = "{", endToken = "}", startCh = findOpening("{");
  if (startCh == null) {
    startToken = "[", endToken = "]";
    startCh = findOpening("[");
  }
  if (startCh == null) {
    startToken = "(", endToken = ")";
    startCh = findOpening("(");
  }
  var skipToken;
  if (startCh == null) {
    startToken = "do", endToken = "end";
    skipToken = "then";
    startCh = findOpening("do");
  }
  if (startCh == null) {
    startToken = "then", endToken = "end";
    skipToken = "do";
    startCh = findOpening("then");
  }

  if (startCh == null) return;
  var count = 1, lastLine = cm.lastLine(), end, endCh;
  outer: for (var i = line; i <= lastLine; ++i) {
    var text = cm.getLine(i), pos = i == line ? startCh : 0;
    for (;;) {
      var nextOpen = text.indexOf(startToken, pos), nextClose = text.indexOf(endToken, pos);
      if (nextOpen < 0) nextOpen = text.indexOf(skipToken, pos);

      if (nextOpen < 0) nextOpen = text.length;
      if (nextClose < 0) nextClose = text.length;
      pos = Math.min(nextOpen, nextClose);
      if (pos == text.length) break;
      if (cm.getTokenTypeAt(CodeMirror.Pos(i, pos + 1)) == tokenType) {
        if (pos == nextOpen) ++count;
        else if (!--count) { end = i; endCh = pos; break outer; }
      }
      ++pos;
    }
  }
  if (end == null || line == end && endCh == startCh) return;
  return {from: CodeMirror.Pos(line, startCh + startToken.length - 1),
          to: CodeMirror.Pos(end, endCh)};
});
