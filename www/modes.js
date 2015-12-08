
CodeMirror.defineMode("uscript-asm", function (cm) {
  "use strict";
  var opcodes = window.opcodes;
  var parser = window.parser;
  var patterns = window.patterns;
  var special = {
    do: "keyword",
    end: "keyword",
    return: "keyword",
    func: "keyword",
    call: "keyword",
    forever: "keyword",
    while: "keyword",
    wait: "keyword",
    if: "keyword",
    elseif: "keyword",
    else: "keyword",
    def: "keyword",
    const: "keyword",
    var: "keyword",
    set: "special",
    incr: "special",
    decr: "special",
    incrmod: "special",
    decrmod: "special",
    add: "operator",
    sub: "operator",
    mul: "operator",
    div: "operator",
    mod: "operator",
    neg: "operator",
    band: "operator",
    bor: "operator",
    bxor: "operator",
    bnot: "operator",
    lshift: "operator",
    rshift: "operator",
    and: "operator",
    or: "operator",
    xor: "operator",
    not: "operator",
    choose: "operator",
    gt: "operator",
    gte: "operator",
    lt: "operator",
    lte: "operator",
    eq: "operator",
    neq: "operator",

  };
  var typeMap = {
    opcode: "builtin",
    number: "number",
    boolean: "atom",
    label: "def",
    variable: "variable",
    definition: "variable-2",
    bracket: "bracket",
    char: "string-2",
    string: "string",
  };

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
      while ((match = textAfter.match(patterns.closers))) {
        back++;
        textAfter = textAfter.substring(match.length);
      }
      return (state.stack.length - 1 - back) * cm.indentUnit;
    },
    token: function (stream, state) {
      if (stream.eatSpace()) return;
      if (stream.match(patterns.comment)) {
        stream.skipToEnd();
        return "comment";
      }
      var names = Object.keys(patterns);
      var type;
      var name;
      var token;
      for (var i = 0, l = names.length; i < l; i++) {
        name = names[i];
        if (stream.match(patterns[name])) {
          token = stream.current();
          type = parser(state, name, token);
          break;
        }
      }
      if (!type) {
        if (stream.match(/\S+/)) {
          return "error";
        }
        return stream.next();
      }
      type = typeMap[type] || type;
      if (type === "builtin") {
        type = special[token] || type;
      }
      return type;
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
  var i, l;
  for (i = 0, l = pairs.length; i < l; i++) {
    var s = findOpening(pairs[i][0]);
    if (s !== undefined && (startCh === undefined || s < startCh)) {
      startCh = s;
      startToken = pairs[i][0];
      endToken = pairs[i][1];
    }
  }

  if (startCh === null) return;
  var count = 1, lastLine = cm.lastLine(), end, endCh;
  outer: for (i = line; i <= lastLine; ++i) {
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
  if (end === undefined || line == end && endCh == startCh) return;
  return {from: CodeMirror.Pos(line, startCh + startToken.length - 1),
          to: CodeMirror.Pos(end, endCh)};
});
