/* parser generated by jison 0.4.15 */
/*
  Returns a Parser object of the following structure:

  Parser: {
    yy: {}
  }

  Parser.prototype: {
    yy: {},
    trace: function(),
    symbols_: {associative list: name ==> number},
    terminals_: {associative list: number ==> name},
    productions_: [...],
    performAction: function anonymous(yytext, yyleng, yylineno, yy, yystate, $$, _$),
    table: [...],
    defaultActions: {...},
    parseError: function(str, hash),
    parse: function(input),

    lexer: {
        EOF: 1,
        parseError: function(str, hash),
        setInput: function(input),
        input: function(),
        unput: function(str),
        more: function(),
        less: function(n),
        pastInput: function(),
        upcomingInput: function(),
        showPosition: function(),
        test_match: function(regex_match_array, rule_index),
        next: function(),
        lex: function(),
        begin: function(condition),
        popState: function(),
        _currentRules: function(),
        topState: function(),
        pushState: function(condition),

        options: {
            ranges: boolean           (optional: true ==> token location info will include a .range[] member)
            flex: boolean             (optional: true ==> flex-like lexing behaviour where the rules are tested exhaustively to find the longest match)
            backtrack_lexer: boolean  (optional: true ==> lexer regexes are tested in order and for each matching regex the action code is invoked; the lexer terminates the scan when a token is returned by the action code)
        },

        performAction: function(yy, yy_, $avoiding_name_collisions, YY_START),
        rules: [...],
        conditions: {associative list: name ==> set},
    }
  }


  token location info (@$, _$, etc.): {
    first_line: n,
    last_line: n,
    first_column: n,
    last_column: n,
    range: [start_number, end_number]       (where the numbers are indexes into the input string, regular zero-based)
  }


  the parseError function receives a 'hash' object with these members for lexer and parser errors: {
    text:        (matched text)
    token:       (the produced terminal token, if any)
    line:        (yylineno)
  }
  while parser (grammar) errors will also provide these members, i.e. parser errors deliver a superset of attributes: {
    loc:         (yylloc)
    expected:    (string describing the set of expected tokens)
    recoverable: (boolean: TRUE when the parser has a error recovery rule available for this particular error)
  }
*/
var parser = (function(){
var o=function(k,v,o,l){for(o=o||{},l=k.length;l--;o[k[l]]=v);return o},$V0=[1,4],$V1=[1,5],$V2=[1,6],$V3=[1,7],$V4=[5,7,8,13,14,17],$V5=[1,18],$V6=[1,19],$V7=[1,12],$V8=[1,13],$V9=[1,14],$Va=[1,15],$Vb=[1,16],$Vc=[1,17],$Vd=[1,20],$Ve=[1,21],$Vf=[1,22],$Vg=[1,23],$Vh=[1,24],$Vi=[1,27],$Vj=[2,57],$Vk=[1,32],$Vl=[1,33],$Vm=[1,34],$Vn=[1,35],$Vo=[1,36],$Vp=[1,37],$Vq=[1,38],$Vr=[1,39],$Vs=[1,40],$Vt=[1,41],$Vu=[1,42],$Vv=[1,43],$Vw=[1,44],$Vx=[1,45],$Vy=[1,46],$Vz=[1,47],$VA=[1,48],$VB=[1,50],$VC=[5,7,8,13,14,16,17,20,22,25,28,29,30,31,32,34,35,36,37,38,39,40,41,42,43,44,55,56],$VD=[1,63],$VE=[5,7,8,13,14,16,17,20,22,25,55,56],$VF=[27,52],$VG=[20,22,55],$VH=[1,96],$VI=[1,97],$VJ=[20,22],$VK=[5,7,8,13,14,16,17,20,22,25,28,29,34,35,36,37,38,39,40,41,42,43,44,55,56],$VL=[5,7,8,13,14,16,17,20,22,25,34,35,36,37,38,39,40,41,42,43,44,55,56],$VM=[5,7,8,13,14,16,17,20,22,25,34,35,36,39,40,41,42,43,44,55,56],$VN=[5,7,8,13,14,16,17,20,22,25,34,35,36,43,44,55,56],$VO=[5,7,8,13,14,17,25],$VP=[8,17,25],$VQ=[1,117];
var parser = {trace: function trace() { },
yy: {},
symbols_: {"error":2,"file":3,"declarations":4,"EOF":5,"declaration":6,"EXPORT":7,"IDENT":8,"=":9,"e":10,"arghead":11,"statementBlock":12,"IMPORT":13,"NAMESPACE":14,"declarationBlock":15,"{":16,"}":17,"statements":18,"(":19,")":20,"args":21,",":22,"statement":23,"expressions":24,"IF":25,"numbers":26,"NUMBER":27,"+":28,"-":29,"*":30,"/":31,"%":32,"~":33,"^":34,"&":35,"|":36,"<<":37,">>":38,">":39,">=":40,"<":41,"<=":42,"==":43,"!=":44,"&&":45,"||":46,"^^":47,"!":48,"TRUE":49,"FALSE":50,"<<<":51,">>>":52,"STRING":53,"[":54,"]":55,":":56,"$accept":0,"$end":1},
terminals_: {2:"error",5:"EOF",7:"EXPORT",8:"IDENT",9:"=",13:"IMPORT",14:"NAMESPACE",16:"{",17:"}",19:"(",20:")",22:",",25:"IF",27:"NUMBER",28:"+",29:"-",30:"*",31:"/",32:"%",33:"~",34:"^",35:"&",36:"|",37:"<<",38:">>",39:">",40:">=",41:"<",42:"<=",43:"==",44:"!=",45:"&&",46:"||",47:"^^",48:"!",49:"TRUE",50:"FALSE",51:"<<<",52:">>>",53:"STRING",54:"[",55:"]",56:":"},
productions_: [0,[3,2],[4,1],[4,2],[6,4],[6,3],[6,2],[6,4],[6,3],[6,2],[6,3],[15,2],[15,3],[12,2],[12,3],[11,2],[11,3],[21,1],[21,3],[18,1],[18,2],[23,3],[23,6],[23,4],[23,3],[24,1],[24,3],[26,1],[26,2],[10,3],[10,3],[10,3],[10,3],[10,3],[10,2],[10,2],[10,3],[10,3],[10,3],[10,3],[10,3],[10,3],[10,3],[10,3],[10,3],[10,3],[10,3],[10,2],[10,2],[10,2],[10,2],[10,3],[10,1],[10,1],[10,1],[10,3],[10,1],[10,1],[10,3],[10,4],[10,3]],
performAction: function anonymous(yytext, yyleng, yylineno, yy, yystate /* action[1] */, $$ /* vstack */, _$ /* lstack */) {
/* this == yyval */

var $0 = $$.length - 1;
switch (yystate) {
case 1:
return $$[$0-1];
break;
case 2: case 17: case 19: case 25: case 27:
this.$ = [$$[$0]];
break;
case 3: case 20: case 28:
this.$ = $$[$0-1];$$[$0-1].push($$[$0]);
break;
case 4:
this.$ = ["CONSTANT",true,$$[$0-2],$$[$0]];
break;
case 5:
this.$ = ["CONSTANT",false,$$[$0-2],$$[$0]];
break;
case 6:
this.$ = ["CONSTANT",true,null,$$[$0]];
break;
case 7:
this.$ = ["FUNCTION",true,$$[$0-2],$$[$0-1],$$[$0]];
break;
case 8:
this.$ = ["FUNCTION",false,$$[$0-2],$$[$0-1],$$[$0]];
break;
case 9:
this.$ = ["IMPORT",$$[$0]];
break;
case 10:
this.$ = ["NAMESPACE",$$[$0-1],$$[$0]];
break;
case 11: case 13: case 15:
this.$ = [];
break;
case 12: case 14: case 16: case 51:
this.$ = $$[$0-1];
break;
case 18: case 26:
this.$ = $$[$0-2];$$[$0-2].push($$[$0]);
break;
case 21:
this.$ = ["ASSIGN",$$[$0-2],$$[$0]];
break;
case 22:
this.$ = ["CALL",$$[$0-3],$$[$0-1],$$[$0-5]];
break;
case 23:
this.$ = ["CALL",$$[$0-3],$$[$0-1]];
break;
case 24:
this.$ = ["IF",$$[$0-1],$$[$0]];
break;
case 29:
this.$ = ["ADD",$$[$0-2],$$[$0]];
break;
case 30:
this.$ = ["SUB",$$[$0-2],$$[$0]];
break;
case 31:
this.$ = ["MUL",$$[$0-2],$$[$0]];
break;
case 32:
this.$ = ["DIV",$$[$0-2],$$[$0]];
break;
case 33:
this.$ = ["MOD",$$[$0-2],$$[$0]];
break;
case 34:
this.$ = ["NEG", $$[$0]];
break;
case 35:
this.$ = ["BNOT",$$[$0]];
break;
case 36:
this.$ = ["BXOR",$$[$0-2],$$[$0]];
break;
case 37:
this.$ = ["BAND",$$[$0-2],$$[$0]];
break;
case 38:
this.$ = ["BOR",$$[$0-2],$$[$0]];
break;
case 39:
this.$ = ["LSHIFT",$$[$0-2],$$[$0]];
break;
case 40:
this.$ = ["RSHIFT",$$[$0-2],$$[$0]];
break;
case 41:
this.$ = ["GT",$$[$0-2],$$[$0]];
break;
case 42:
this.$ = ["GTE",$$[$0-2],$$[$0]];
break;
case 43:
this.$ = ["LT",$$[$0-2],$$[$0]];
break;
case 44:
this.$ = ["LTE",$$[$0-2],$$[$0]];
break;
case 45:
this.$ = ["EQ",$$[$0-2],$$[$0]];
break;
case 46:
this.$ = ["NEQ",$$[$0-2],$$[$0]];
break;
case 47:
this.$ = ["AND",$$[$0-1],$$[$01]];
break;
case 48:
this.$ = ["OR",$$[$0-1],$$[$01]];
break;
case 49:
this.$ = ["XOR",$$[$0-1],$$[$01]];
break;
case 50:
this.$ = ["NOT", $$[$0]];
break;
case 52:
this.$ = Number(yytext);
break;
case 53:
this.$ = true;
break;
case 54:
this.$ = false;
break;
case 55:
this.$ = new Buffer($$[$0-1]);
break;
case 56:
this.$ = new Buffer(JSON.parse($$[$0]));
break;
case 57:
this.$ = ["IDENT", $$[$0]];
break;
case 58:
this.$ = ["LIST"].concat($$[$0-1]);
break;
case 59:
this.$ = ["LIST"].concat($$[$0-2]);
break;
case 60:
this.$ = ["PAIR",$$[$0-2],$$[$0]];
break;
}
},
table: [{3:1,4:2,6:3,7:$V0,8:$V1,13:$V2,14:$V3},{1:[3]},{5:[1,8],6:9,7:$V0,8:$V1,13:$V2,14:$V3},o($V4,[2,2]),{8:[1,10],10:11,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{9:[1,25],11:26,19:$Vi},{8:[1,28]},{8:[1,29]},{1:[2,1]},o($V4,[2,3]),o([5,7,8,13,14,17,28,29,30,31,32,34,35,36,37,38,39,40,41,42,43,44,56],$Vj,{11:31,9:[1,30],19:$Vi}),o($V4,[2,6],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz,56:$VA}),{8:$VB,10:49,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:51,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:52,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:53,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:54,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:55,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:56,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},o($VC,[2,52]),o($VC,[2,53]),o($VC,[2,54]),{26:57,27:[1,58]},o($VC,[2,56]),{8:$VB,10:60,19:$V5,24:59,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:61,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{12:62,16:$VD},{8:[1,66],20:[1,64],21:65},o($V4,[2,9]),{15:67,16:[1,68]},{8:$VB,10:69,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{12:70,16:$VD},{8:$VB,10:71,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:72,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:73,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:74,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:75,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:76,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:77,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:78,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:79,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:80,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:81,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:82,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:83,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:84,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:85,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:86,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:87,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},o($VC,[2,34]),o($VC,$Vj),o($VC,[2,35]),o($VE,[2,47],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz}),o($VE,[2,48],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz}),o($VE,[2,49],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz}),o($VC,[2,50]),{20:[1,88],28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz,56:$VA},{27:[1,90],52:[1,89]},o($VF,[2,27]),{22:[1,92],55:[1,91]},o($VG,[2,25],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz,56:$VA}),o($V4,[2,5],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz,56:$VA}),o($V4,[2,8]),{8:$VH,17:[1,93],18:94,23:95,25:$VI},{16:[2,15]},{20:[1,98],22:[1,99]},o($VJ,[2,17]),o($V4,[2,10]),{4:101,6:3,7:$V0,8:$V1,13:$V2,14:$V3,17:[1,100]},o($V4,[2,4],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz,56:$VA}),o($V4,[2,7]),o($VK,[2,29],{30:$Vm,31:$Vn,32:$Vo}),o($VK,[2,30],{30:$Vm,31:$Vn,32:$Vo}),o($VC,[2,31]),o($VC,[2,32]),o($VC,[2,33]),o([5,7,8,13,14,16,17,20,22,25,34,36,55,56],[2,36],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,35:$Vq,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz}),o([5,7,8,13,14,16,17,20,22,25,34,35,36,55,56],[2,37],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz}),o([5,7,8,13,14,16,17,20,22,25,36,55,56],[2,38],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz}),o($VL,[2,39],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo}),o($VL,[2,40],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo}),o($VM,[2,41],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,37:$Vs,38:$Vt}),o($VM,[2,42],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,37:$Vs,38:$Vt}),o($VM,[2,43],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,37:$Vs,38:$Vt}),o($VM,[2,44],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,37:$Vs,38:$Vt}),o($VN,[2,45],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx}),o($VN,[2,46],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx}),o($VE,[2,60],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz}),o($VC,[2,51]),o($VC,[2,55]),o($VF,[2,28]),o($VC,[2,58]),{8:$VB,10:103,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh,55:[1,102]},o($VO,[2,13]),{8:$VH,17:[1,104],23:105,25:$VI},o($VP,[2,19]),{9:[1,106],19:[1,107]},{8:$VB,10:108,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{16:[2,16]},{8:[1,109]},o($V4,[2,11]),{6:9,7:$V0,8:$V1,13:$V2,14:$V3,17:[1,110]},o($VC,[2,59]),o($VG,[2,26],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz,56:$VA}),o($VO,[2,14]),o($VP,[2,20]),{8:[1,112],10:111,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{8:$VB,10:60,19:$V5,24:113,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{12:114,16:$VD,28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz,56:$VA},o($VJ,[2,18]),o($V4,[2,12]),o($VP,[2,21],{28:$Vk,29:$Vl,30:$Vm,31:$Vn,32:$Vo,34:$Vp,35:$Vq,36:$Vr,37:$Vs,38:$Vt,39:$Vu,40:$Vv,41:$Vw,42:$Vx,43:$Vy,44:$Vz,56:$VA}),o([8,17,25,28,29,30,31,32,34,35,36,37,38,39,40,41,42,43,44,56],$Vj,{19:[1,115]}),{20:[1,116],22:$VQ},o($VP,[2,24]),{8:$VB,10:60,19:$V5,24:118,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},o($VP,[2,23]),{8:$VB,10:103,19:$V5,27:$V6,29:$V7,33:$V8,45:$V9,46:$Va,47:$Vb,48:$Vc,49:$Vd,50:$Ve,51:$Vf,53:$Vg,54:$Vh},{20:[1,119],22:$VQ},o($VP,[2,22])],
defaultActions: {8:[2,1],64:[2,15],98:[2,16]},
parseError: function parseError(str, hash) {
    if (hash.recoverable) {
        this.trace(str);
    } else {
        throw new Error(str);
    }
},
parse: function parse(input) {
    var self = this, stack = [0], tstack = [], vstack = [null], lstack = [], table = this.table, yytext = '', yylineno = 0, yyleng = 0, recovering = 0, TERROR = 2, EOF = 1;
    var args = lstack.slice.call(arguments, 1);
    var lexer = Object.create(this.lexer);
    var sharedState = { yy: {} };
    for (var k in this.yy) {
        if (Object.prototype.hasOwnProperty.call(this.yy, k)) {
            sharedState.yy[k] = this.yy[k];
        }
    }
    lexer.setInput(input, sharedState.yy);
    sharedState.yy.lexer = lexer;
    sharedState.yy.parser = this;
    if (typeof lexer.yylloc == 'undefined') {
        lexer.yylloc = {};
    }
    var yyloc = lexer.yylloc;
    lstack.push(yyloc);
    var ranges = lexer.options && lexer.options.ranges;
    if (typeof sharedState.yy.parseError === 'function') {
        this.parseError = sharedState.yy.parseError;
    } else {
        this.parseError = Object.getPrototypeOf(this).parseError;
    }
    function popStack(n) {
        stack.length = stack.length - 2 * n;
        vstack.length = vstack.length - n;
        lstack.length = lstack.length - n;
    }
    _token_stack:
        function lex() {
            var token;
            token = lexer.lex() || EOF;
            if (typeof token !== 'number') {
                token = self.symbols_[token] || token;
            }
            return token;
        }
    var symbol, preErrorSymbol, state, action, a, r, yyval = {}, p, len, newState, expected;
    while (true) {
        state = stack[stack.length - 1];
        if (this.defaultActions[state]) {
            action = this.defaultActions[state];
        } else {
            if (symbol === null || typeof symbol == 'undefined') {
                symbol = lex();
            }
            action = table[state] && table[state][symbol];
        }
                    if (typeof action === 'undefined' || !action.length || !action[0]) {
                var errStr = '';
                expected = [];
                for (p in table[state]) {
                    if (this.terminals_[p] && p > TERROR) {
                        expected.push('\'' + this.terminals_[p] + '\'');
                    }
                }
                if (lexer.showPosition) {
                    errStr = 'Parse error on line ' + (yylineno + 1) + ':\n' + lexer.showPosition() + '\nExpecting ' + expected.join(', ') + ', got \'' + (this.terminals_[symbol] || symbol) + '\'';
                } else {
                    errStr = 'Parse error on line ' + (yylineno + 1) + ': Unexpected ' + (symbol == EOF ? 'end of input' : '\'' + (this.terminals_[symbol] || symbol) + '\'');
                }
                this.parseError(errStr, {
                    text: lexer.match,
                    token: this.terminals_[symbol] || symbol,
                    line: lexer.yylineno,
                    loc: yyloc,
                    expected: expected
                });
            }
        if (action[0] instanceof Array && action.length > 1) {
            throw new Error('Parse Error: multiple actions possible at state: ' + state + ', token: ' + symbol);
        }
        switch (action[0]) {
        case 1:
            stack.push(symbol);
            vstack.push(lexer.yytext);
            lstack.push(lexer.yylloc);
            stack.push(action[1]);
            symbol = null;
            if (!preErrorSymbol) {
                yyleng = lexer.yyleng;
                yytext = lexer.yytext;
                yylineno = lexer.yylineno;
                yyloc = lexer.yylloc;
                if (recovering > 0) {
                    recovering--;
                }
            } else {
                symbol = preErrorSymbol;
                preErrorSymbol = null;
            }
            break;
        case 2:
            len = this.productions_[action[1]][1];
            yyval.$ = vstack[vstack.length - len];
            yyval._$ = {
                first_line: lstack[lstack.length - (len || 1)].first_line,
                last_line: lstack[lstack.length - 1].last_line,
                first_column: lstack[lstack.length - (len || 1)].first_column,
                last_column: lstack[lstack.length - 1].last_column
            };
            if (ranges) {
                yyval._$.range = [
                    lstack[lstack.length - (len || 1)].range[0],
                    lstack[lstack.length - 1].range[1]
                ];
            }
            r = this.performAction.apply(yyval, [
                yytext,
                yyleng,
                yylineno,
                sharedState.yy,
                action[1],
                vstack,
                lstack
            ].concat(args));
            if (typeof r !== 'undefined') {
                return r;
            }
            if (len) {
                stack = stack.slice(0, -1 * len * 2);
                vstack = vstack.slice(0, -1 * len);
                lstack = lstack.slice(0, -1 * len);
            }
            stack.push(this.productions_[action[1]][0]);
            vstack.push(yyval.$);
            lstack.push(yyval._$);
            newState = table[stack[stack.length - 2]][stack[stack.length - 1]];
            stack.push(newState);
            break;
        case 3:
            return true;
        }
    }
    return true;
}};
/* generated by jison-lex 0.3.4 */
var lexer = (function(){
var lexer = ({

EOF:1,

parseError:function parseError(str, hash) {
        if (this.yy.parser) {
            this.yy.parser.parseError(str, hash);
        } else {
            throw new Error(str);
        }
    },

// resets the lexer, sets new input
setInput:function (input, yy) {
        this.yy = yy || this.yy || {};
        this._input = input;
        this._more = this._backtrack = this.done = false;
        this.yylineno = this.yyleng = 0;
        this.yytext = this.matched = this.match = '';
        this.conditionStack = ['INITIAL'];
        this.yylloc = {
            first_line: 1,
            first_column: 0,
            last_line: 1,
            last_column: 0
        };
        if (this.options.ranges) {
            this.yylloc.range = [0,0];
        }
        this.offset = 0;
        return this;
    },

// consumes and returns one char from the input
input:function () {
        var ch = this._input[0];
        this.yytext += ch;
        this.yyleng++;
        this.offset++;
        this.match += ch;
        this.matched += ch;
        var lines = ch.match(/(?:\r\n?|\n).*/g);
        if (lines) {
            this.yylineno++;
            this.yylloc.last_line++;
        } else {
            this.yylloc.last_column++;
        }
        if (this.options.ranges) {
            this.yylloc.range[1]++;
        }

        this._input = this._input.slice(1);
        return ch;
    },

// unshifts one char (or a string) into the input
unput:function (ch) {
        var len = ch.length;
        var lines = ch.split(/(?:\r\n?|\n)/g);

        this._input = ch + this._input;
        this.yytext = this.yytext.substr(0, this.yytext.length - len);
        //this.yyleng -= len;
        this.offset -= len;
        var oldLines = this.match.split(/(?:\r\n?|\n)/g);
        this.match = this.match.substr(0, this.match.length - 1);
        this.matched = this.matched.substr(0, this.matched.length - 1);

        if (lines.length - 1) {
            this.yylineno -= lines.length - 1;
        }
        var r = this.yylloc.range;

        this.yylloc = {
            first_line: this.yylloc.first_line,
            last_line: this.yylineno + 1,
            first_column: this.yylloc.first_column,
            last_column: lines ?
                (lines.length === oldLines.length ? this.yylloc.first_column : 0)
                 + oldLines[oldLines.length - lines.length].length - lines[0].length :
              this.yylloc.first_column - len
        };

        if (this.options.ranges) {
            this.yylloc.range = [r[0], r[0] + this.yyleng - len];
        }
        this.yyleng = this.yytext.length;
        return this;
    },

// When called from action, caches matched text and appends it on next action
more:function () {
        this._more = true;
        return this;
    },

// When called from action, signals the lexer that this rule fails to match the input, so the next matching rule (regex) should be tested instead.
reject:function () {
        if (this.options.backtrack_lexer) {
            this._backtrack = true;
        } else {
            return this.parseError('Lexical error on line ' + (this.yylineno + 1) + '. You can only invoke reject() in the lexer when the lexer is of the backtracking persuasion (options.backtrack_lexer = true).\n' + this.showPosition(), {
                text: "",
                token: null,
                line: this.yylineno
            });

        }
        return this;
    },

// retain first n characters of the match
less:function (n) {
        this.unput(this.match.slice(n));
    },

// displays already matched input, i.e. for error messages
pastInput:function () {
        var past = this.matched.substr(0, this.matched.length - this.match.length);
        return (past.length > 20 ? '...':'') + past.substr(-20).replace(/\n/g, "");
    },

// displays upcoming input, i.e. for error messages
upcomingInput:function () {
        var next = this.match;
        if (next.length < 20) {
            next += this._input.substr(0, 20-next.length);
        }
        return (next.substr(0,20) + (next.length > 20 ? '...' : '')).replace(/\n/g, "");
    },

// displays the character position where the lexing error occurred, i.e. for error messages
showPosition:function () {
        var pre = this.pastInput();
        var c = new Array(pre.length + 1).join("-");
        return pre + this.upcomingInput() + "\n" + c + "^";
    },

// test the lexed token: return FALSE when not a match, otherwise return token
test_match:function (match, indexed_rule) {
        var token,
            lines,
            backup;

        if (this.options.backtrack_lexer) {
            // save context
            backup = {
                yylineno: this.yylineno,
                yylloc: {
                    first_line: this.yylloc.first_line,
                    last_line: this.last_line,
                    first_column: this.yylloc.first_column,
                    last_column: this.yylloc.last_column
                },
                yytext: this.yytext,
                match: this.match,
                matches: this.matches,
                matched: this.matched,
                yyleng: this.yyleng,
                offset: this.offset,
                _more: this._more,
                _input: this._input,
                yy: this.yy,
                conditionStack: this.conditionStack.slice(0),
                done: this.done
            };
            if (this.options.ranges) {
                backup.yylloc.range = this.yylloc.range.slice(0);
            }
        }

        lines = match[0].match(/(?:\r\n?|\n).*/g);
        if (lines) {
            this.yylineno += lines.length;
        }
        this.yylloc = {
            first_line: this.yylloc.last_line,
            last_line: this.yylineno + 1,
            first_column: this.yylloc.last_column,
            last_column: lines ?
                         lines[lines.length - 1].length - lines[lines.length - 1].match(/\r?\n?/)[0].length :
                         this.yylloc.last_column + match[0].length
        };
        this.yytext += match[0];
        this.match += match[0];
        this.matches = match;
        this.yyleng = this.yytext.length;
        if (this.options.ranges) {
            this.yylloc.range = [this.offset, this.offset += this.yyleng];
        }
        this._more = false;
        this._backtrack = false;
        this._input = this._input.slice(match[0].length);
        this.matched += match[0];
        token = this.performAction.call(this, this.yy, this, indexed_rule, this.conditionStack[this.conditionStack.length - 1]);
        if (this.done && this._input) {
            this.done = false;
        }
        if (token) {
            return token;
        } else if (this._backtrack) {
            // recover context
            for (var k in backup) {
                this[k] = backup[k];
            }
            return false; // rule action called reject() implying the next rule should be tested instead.
        }
        return false;
    },

// return next match in input
next:function () {
        if (this.done) {
            return this.EOF;
        }
        if (!this._input) {
            this.done = true;
        }

        var token,
            match,
            tempMatch,
            index;
        if (!this._more) {
            this.yytext = '';
            this.match = '';
        }
        var rules = this._currentRules();
        for (var i = 0; i < rules.length; i++) {
            tempMatch = this._input.match(this.rules[rules[i]]);
            if (tempMatch && (!match || tempMatch[0].length > match[0].length)) {
                match = tempMatch;
                index = i;
                if (this.options.backtrack_lexer) {
                    token = this.test_match(tempMatch, rules[i]);
                    if (token !== false) {
                        return token;
                    } else if (this._backtrack) {
                        match = false;
                        continue; // rule action called reject() implying a rule MISmatch.
                    } else {
                        // else: this is a lexer rule which consumes input without producing a token (e.g. whitespace)
                        return false;
                    }
                } else if (!this.options.flex) {
                    break;
                }
            }
        }
        if (match) {
            token = this.test_match(match, rules[index]);
            if (token !== false) {
                return token;
            }
            // else: this is a lexer rule which consumes input without producing a token (e.g. whitespace)
            return false;
        }
        if (this._input === "") {
            return this.EOF;
        } else {
            return this.parseError('Lexical error on line ' + (this.yylineno + 1) + '. Unrecognized text.\n' + this.showPosition(), {
                text: "",
                token: null,
                line: this.yylineno
            });
        }
    },

// return next match that has a token
lex:function lex() {
        var r = this.next();
        if (r) {
            return r;
        } else {
            return this.lex();
        }
    },

// activates a new lexer condition state (pushes the new lexer condition state onto the condition stack)
begin:function begin(condition) {
        this.conditionStack.push(condition);
    },

// pop the previously active lexer condition state off the condition stack
popState:function popState() {
        var n = this.conditionStack.length - 1;
        if (n > 0) {
            return this.conditionStack.pop();
        } else {
            return this.conditionStack[0];
        }
    },

// produce the lexer rule set which is active for the currently active lexer condition state
_currentRules:function _currentRules() {
        if (this.conditionStack.length && this.conditionStack[this.conditionStack.length - 1]) {
            return this.conditions[this.conditionStack[this.conditionStack.length - 1]].rules;
        } else {
            return this.conditions["INITIAL"].rules;
        }
    },

// return the currently active lexer condition state; when an index argument is provided it produces the N-th previous condition state, if available
topState:function topState(n) {
        n = this.conditionStack.length - 1 - Math.abs(n || 0);
        if (n >= 0) {
            return this.conditionStack[n];
        } else {
            return "INITIAL";
        }
    },

// alias for begin(condition)
pushState:function pushState(condition) {
        this.begin(condition);
    },

// return the number of states currently on the stack
stateStackSize:function stateStackSize() {
        return this.conditionStack.length;
    },
options: {},
performAction: function anonymous(yy,yy_,$avoiding_name_collisions,YY_START) {
var YYSTATE=YY_START;
switch($avoiding_name_collisions) {
case 0:/* skip whitespace */
break;
case 1:/* skip comments */
break;
case 2:return 27;
break;
case 3:return 27;
break;
case 4:return 27;
break;
case 5:return 27;
break;
case 6:return 27;
break;
case 7:return 30;
break;
case 8:return 31;
break;
case 9:return 29;
break;
case 10:return 28;
break;
case 11:return 32;
break;
case 12:return 19;
break;
case 13:return 20;
break;
case 14:return 48;
break;
case 15:return 33;
break;
case 16:return 51;
break;
case 17:return 52;
break;
case 18:return 37;
break;
case 19:return 38;
break;
case 20:return 46;
break;
case 21:return 47;
break;
case 22:return 45;
break;
case 23:return 36;
break;
case 24:return 34;
break;
case 25:return 35;
break;
case 26:return 43;
break;
case 27:return 44;
break;
case 28:return 42;
break;
case 29:return 40;
break;
case 30:return 41;
break;
case 31:return 39;
break;
case 32:return '?';
break;
case 33:return 56;
break;
case 34:return 9;
break;
case 35:return 16;
break;
case 36:return 17;
break;
case 37:return 54;
break;
case 38:return 55;
break;
case 39:return 22;
break;
case 40:return 7;
break;
case 41:return 49;
break;
case 42:return 50;
break;
case 43:return 25;
break;
case 44:return 13;
break;
case 45:return 14;
break;
case 46:return 8;
break;
case 47:return 53;
break;
case 48:return 5;
break;
}
},
rules: [/^(?:\s+)/,/^(?:[-][-].*)/,/^(?:[-]?0[bB][01]+)/,/^(?:[-]?0[oO][07]+)/,/^(?:[-]?0[xX][0-9a-fA-f]+)/,/^(?:[-]?[1-9][0-9]*)/,/^(?:0\b)/,/^(?:\*)/,/^(?:\/)/,/^(?:-)/,/^(?:\+)/,/^(?:%)/,/^(?:\()/,/^(?:\))/,/^(?:!)/,/^(?:~)/,/^(?:<<<)/,/^(?:>>>)/,/^(?:<<)/,/^(?:>>)/,/^(?:\|\|)/,/^(?:\^\^)/,/^(?:&&)/,/^(?:\|)/,/^(?:\^)/,/^(?:&)/,/^(?:==)/,/^(?:!=)/,/^(?:<=)/,/^(?:>=)/,/^(?:<)/,/^(?:>)/,/^(?:\?)/,/^(?::)/,/^(?:=)/,/^(?:\{)/,/^(?:\})/,/^(?:\[)/,/^(?:\])/,/^(?:,)/,/^(?:export\b)/,/^(?:true\b)/,/^(?:false\b)/,/^(?:if\b)/,/^(?:import\b)/,/^(?:namespace\b)/,/^(?:([a-zA-Z][a-zA-Z0-9_]*[.])*[a-zA-Z][a-zA-Z0-9_]*)/,/^(?:["]([^"]|\\.)*["])/,/^(?:$)/],
conditions: {"INITIAL":{"rules":[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48],"inclusive":true}}
});
return lexer;
})();
parser.lexer = lexer;
function Parser () {
  this.yy = {};
}
Parser.prototype = parser;parser.Parser = Parser;
return new Parser;
})();


if (typeof require !== 'undefined' && typeof exports !== 'undefined') {
exports.parser = parser;
exports.Parser = parser.Parser;
exports.parse = function () { return parser.parse.apply(parser, arguments); };
exports.main = function commonjsMain(args) {
    if (!args[1]) {
        console.log('Usage: '+args[0]+' FILE');
        process.exit(1);
    }
    var source = require('fs').readFileSync(require('path').normalize(args[1]), "utf8");
    return exports.parser.parse(source);
};
if (typeof module !== 'undefined' && require.main === module) {
  exports.main(process.argv.slice(1));
}
}