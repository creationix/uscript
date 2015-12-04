(function (def) {
  try { module.exports = def(require('./opcodes')); }
  catch (err) { window.patterns = def(window.opcodes); }
})(function (opcodes) {
  return {
    opcode: new RegExp("^(" + Object.keys(opcodes).sort().join("|") + ")\\b"),
    number: /^[-]?(?:0b[01]+|0o[07]+|0x[0-9a-f]+|[1-9][0-9]*|0)\b/i,
    boolean: /^(true|false)\b/,
    comment: /^[-][-]/,
    symbol: /^@[a-z_$]\w*/i,
    ident: /^[a-z_$]\w*/i,
    bracket: /^[(){}\[\]<>:]/,
    string: /^"(?:[^"]|\\.)*"/,
    char: /^'(?:[^']|\\.)'/,
    closers: /^\s*([\)\]\}>]|end\b)/,
  };
});
