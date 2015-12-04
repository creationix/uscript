/*jshint esnext: true*/
var inspect = require('util').inspect;
function p(val) {
  console.log(inspect(val, {colors:true,depth:null}));
}
global.p = p;

require('./lang.js');
//
// require('gen-run')(function* () {
//   "use strict";
//   var symbols = {};
//   var loader = require('./loader.js');
//
//   var parser = require('./parser.js').parser;
//   function parse(code) {
//     return parser.parse(code);
//   }
//   var extract = require('./extract.js')(symbols, loader, { jack: parse });
//   yield* extract("creationix/test-ops.jack");
// });
