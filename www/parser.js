(function (def) {
  try { module.exports = def(require('./opcodes')); }
  catch (err) { window.parser = def(window.opcodes); }
})(function (opcodes) {
  return function parser(state, type, token) {
    "use strict";
    var stack = state.stack;
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
    if (top.type === "DEF" || top.type === "LET" || top.type === "SET" ||
        top.type === "FOR" || top.type === "EACH") {
      if (type !== "variable") return "error";
    }

    // Handle a block closer
    if (typeof top.left === "number" && token !== top.expectToken) {
      top.left--;
      if (top.left < 0) return "error";
    }

    if (type === "variable") {
      if (top.type === "DEF" || top.type === "LET" || top.type === "FOR" ||
          top.type === "EACH") {
        top.scope[token] = top.scope[token] === undefined ? (
          top.type === "DEF" ? "DEF" : "LET") : "error";
        if (top.type === "DEF" || top.type === "LET") {
          top.name = token;
        }
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
      else if (opcodes[token]) {
        stack.push({
          mode: "fixed",
          scope: cloneScope(),
          left: opcodes[token],
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
  };
});
