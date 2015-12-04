(function (def) {
  try { module.exports = def(require('./opcodes')); }
  catch (err) { window.patterns = def(window.opcodes); }
})(function (opcodes) {
  return {
    opcode: new RegExp("^(" + Object.keys(opcodes).join("|") + ")\\b", "i"),
    number: /^([-]?0[bB][01]+|[-]?0[oO][07]+|[-]?0[xX][0-9a-fA-f]+|[-]?[1-9][0-9]*|0)\b/,
    boolean: /^(true|false)\b/,
    comment: /^[-][-]/,
    symbol: /^@[a-z_]\w*/i,
    ident: /^[a-z_]\w*/i,
    bracket: /^[(){}\[\]<>:]/i,
    string: /^"(?:[^"]|\\.)*"/,
    char: /^'(?:[^']|\\.)'/,
    closers: /^\s*([\)\]\}>]|end\b)/i,
  };
});
