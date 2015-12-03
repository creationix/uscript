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
  // I2C operations2
  ISETUP: 3, ISTART: 1, ISTOP: 1, IADDR: 3, IWRITE: 2, IREAD: 1,
  // Local variables
  LET: 2, SET: 2, GET: 1,
  // Pausing statements
  DELAY: 1, UDELAY: 1, WAIT: 1, YIELD: 0,
  // Control Flow
  IF: 2,
  // Loops / Iterators
  WHILE: 2, DOWHILE: 2, FOR: 4, EACH: 2,
  // Loop control flow
  BREAK: 0, CONTINUE: 0,
  // Function calling and definition
  FUN: undefined, RETURN: 1,
  // block
  DO: undefined, END: 0,
  // Macros
  DEF: 2, PRINT: 1,
};

function parser(state2, type, token) {
  "use strict";
  var stack = state2.stack;
  if (!stack.length) return "error";
  var top = stack[stack.length - 1];
  // console.log(type, token, stack.length, top);

  function cloneScope() {
    var scope = {};
    var keys = Object.keys(top.scope);
    for (var i = 0, l = keys.length; i < l; i++) {
      var key = keys[i];
      scope[key] = top.scope[key];
    }
    return scope;
  }

  // Handle the function name token
  if (top.mode === "function") {
    // The token after FUN must be a new, unused variable
    if (type !== "variable" || top.scope[token] !== undefined) return "error";
    // Prepare to consume args list
    top.mode = "args";
    top.name = token;
    top.args = [];
    return "builtin";
  }

  // Handle the function args list and trailing DO
  if (top.mode === "args") {
    if (type === "keyword" && token === "DO") {
      top.scope[top.name] = top.args.length;
      for (var i = 0, l = top.args.length; i < l; i++) {
        top.scope[top.args[i]] = "LET";
      }
      delete top.args;
      top.mode = "block";
      top.expectToken = "END";
      top.expectType = "keyword";
      return type;
    }
    if (type !== "variable" || top.scope[token] !== undefined) return "error";
    top.args.push(token);
    return "variable";
  }

  function pop() {
    // Check for export values
    var name, value;
    if (top.name) {
      name = top.name;
      value = top.scope[name];
    }
    // Clean up the old stack frame
    stack.pop();
    top = stack[stack.length - 1];

    // Export the value if there was one
    if (name) {
      top.scope[name] = value;
    }
  }

  // Ensure the first token after DEF and LET are unused variables.
  if (top.type === "DEF" || top.type === "LET" || top.type === "SET") {
    if (type !== "variable") return "error";
  }

  // Handle a block closer
  if (typeof top.left === "number" && token !== top.expectToken) {
    top.left--;
    if (top.left < 0) return "error";
  }

  if (type === "variable") {
    if (top.type === "DEF" || top.type === "LET") {
      top.scope[token] = top.scope[token] === undefined ? top.type: "error";
      top.name = token;
    }
    var kind = top.scope[token];

    // Figure out which kind of variable this is.
    if (kind === "LET") type = "variable";
    else if (kind === "DEF" && top.type !== "SET") type = "variable-2";
    else if (typeof kind === "number" && top.type !== "SET") {
      type = "builtin";
    }
    else type = "error";

    if (top.type) delete top.type;
    if (type === "builtin" && kind > 0) {
      stack.push({
        mode: "fixed",
        scope: cloneScope(),
        left: kind,
      });
      return type;
    }
  }
  else if (type === "bracket") {
    if (token === "(" || token === "[" || token === "{" || token === "<") {
      stack.push(top = {
        mode: "block",
        scope: cloneScope(),
        expectType: "bracket",
        expectToken: {
          "(":")",
          "[":"]",
          "{":"}",
          "<":">",
        }[token],
      });
      if (token === "(") {
        top.left = 2;
      }
      return type;
    }
    if (top.mode !== "block" || top.expectType !== "bracket") {
      return "error";
    }
  }

  else if (type === "keyword") {
    if (token === "FUN") {
      stack.push({
        mode: "function",
        scope: cloneScope(),
      });
      return type;
    }
    else if (token === "DO") {
      stack.push({
        mode: "block",
        scope: cloneScope(),
        expectToken: "END",
        expectType: "keyword",
      });
      return type;
    }
    else if (BUILTINS[token]) {
      stack.push({
        mode: "fixed",
        scope: cloneScope(),
        left: BUILTINS[token],
        type: token
      });
      return type;
    }
  }

  if (top.mode === "block" && type === top.expectType && token === top.expectToken) {
    if (typeof top.left === "number" && top.left !== 0) {
      type = "error";
    }
    pop();
  }

  // Clean out any filled fixed items.
  while (top.mode === "fixed" && !top.left) {
    pop();
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
        stack: [
          {
            mode: "block",
            scope: {},
          }
        ],
      };
    },
    copyState: function (state) {
      return JSON.parse(JSON.stringify(state));
    },
    lineComment: "--",
    electricInput: /.$/i,
    fold: "uscript",
    indent: function (state, textAfter) {
      var back = 0;
      var match;
      while ((match = textAfter.match(/^\s*([\)\]\}>]|end\b)/i))) {
        back++;
        textAfter = textAfter.substring(match.length);
      }
      return (state.stack.length - 1 - back) * cm.indentUnit;
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

  var pairs = [
    ["do","end"],
    ["{","}"],
    ["[","]"],
    ["(",")"],
    ["<",">"],
  ];
  var startToken, endToken;
  for (var i = 0, l = pairs.length; i < l; i++) {
    var s = findOpening(pairs[i][0]);
    if (s !== undefined && (startCh === undefined || s < startCh)) {
      startCh = s;
      startToken = pairs[i][0];
      endToken = pairs[i][1];
    }
  }

  if (startCh === null) return;
  var count = 1, lastLine = cm.lastLine(), end, endCh;
  outer: for (var i = line; i <= lastLine; ++i) {
    var text = cm.getLine(i), pos = i == line ? startCh : 0;
    for (;;) {
      var nextOpen = text.indexOf(startToken, pos);
      var nextClose = text.indexOf(endToken, pos);
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
