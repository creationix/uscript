
%lex

%%
\s+                   /* skip whitespace */
[-][-].*              /* skip comments */
[-]?0[bB][01]+        return 'NUMBER';
[-]?0[oO][07]+        return 'NUMBER';
[-]?0[xX][0-9a-fA-f]+ return 'NUMBER';
[-]?[1-9][0-9]*       return 'NUMBER';
0                     return 'NUMBER';
"*"                   return '*';
"/"                   return '/';
"-"                   return '-';
"+"                   return '+';
"%"                   return '%';
"("                   return '(';
")"                   return ')';
"!"                   return '!';
"~"                   return '~';
"<<<"                 return '<<<';
">>>"                 return '>>>';
"<<"                  return '<<';
">>"                  return '>>';
"||"                  return '||';
"^^"                  return '^^';
"&&"                  return '&&';
"|"                   return '|';
"^"                   return '^';
"&"                   return '&';
"=="                  return '==';
"!="                  return '!=';
"<="                  return '<=';
">="                  return '>=';
"<"                   return '<';
">"                   return '>';
"?"                   return '?';
":"                   return ':';
"="                   return '=';
"{"                   return '{';
"}"                   return '}';
"["                   return '[';
"]"                   return ']';
","                   return ',';
export\b              return 'EXPORT';
true\b                return 'TRUE';
false\b               return 'FALSE';
if\b                  return 'IF';
import\b              return 'IMPORT';
namespace\b           return 'NAMESPACE';
([a-zA-Z][a-zA-Z0-9_]*[.])*[a-zA-Z][a-zA-Z0-9_]* return 'IDENT';
["]([^"]|\\.)*["]      return 'STRING';
<<EOF>>               return 'EOF';

/lex

%left ','
%left ':'
%left '='
%right '?'
%left '||'
%left '^^'
%left '&&'
%left '|'
%left '^'
%left '&'
%left '==' '!='
%left '<' '<=' '>' '>='
%left '<<' '>>'
%left '+' '-'
%left '*' '/' '%'
%right '!' '~'
%left UMINUS

%start file

%% /* language grammar */

file
    : declarations EOF
      {return $1;}
    ;

declarations
    : declaration
        {$$ = [$1];}
    | declarations declaration
        {$$ = $1;$1.push($2);}
    ;

declaration
    : EXPORT IDENT '=' e
        {$$ = ["CONSTANT",true,$2,$4];}
    | IDENT '=' e
        {$$ = ["CONSTANT",false,$1,$3];}
    | EXPORT e
        {$$ = ["CONSTANT",true,null,$2];}
    | EXPORT IDENT arghead statementBlock
        {$$ = ["FUNCTION",true,$2,$3,$4];}
    | IDENT arghead statementBlock
        {$$ = ["FUNCTION",false,$1,$2,$3];}
    | IMPORT IDENT
        {$$ = ["IMPORT",$2];}
    | NAMESPACE IDENT declarationBlock
        {$$ = ["NAMESPACE",$2,$3];}
    ;

declarationBlock
    : '{' '}'
        {$$ = [];}
    | '{' declarations '}'
        {$$ = $2;}
    ;

statementBlock
    : '{' '}'
        {$$ = [];}
    | '{' statements '}'
        {$$ = $2;}
    ;

arghead
    : '(' ')'
        {$$ = [];}
    | '(' args ')'
        {$$ = $2;}
    ;

args
    : IDENT
        {$$ = [$1];}
    | args ',' IDENT
        {$$ = $1;$1.push($3);}
    ;

statements
    : statement
        {$$ = [$1];}
    | statements statement
        {$$ = $1;$1.push($2);}
    ;

statement
    : IDENT '=' e
        {$$ = ["ASSIGN",$1,$3];}
    | IDENT '=' IDENT '(' expressions ')'
        {$$ = ["CALL",$3,$5,$1];}
    | IDENT '(' expressions ')'
        {$$ = ["CALL",$1,$3];}
    | IF e statementBlock
        {$$ = ["IF",$2,$3];}
    ;

expressions
    : e
        {$$ = [$1];}
    | expressions ',' e
        {$$ = $1;$1.push($3);}
    ;

numbers
    : NUMBER
        {$$ = [$1];}
    | numbers NUMBER
        {$$ = $1;$1.push($2);}
    ;

e
    : e '+' e
        {$$ = ["ADD",$1,$3];}
    | e '-' e
        {$$ = ["SUB",$1,$3];}
    | e '*' e
        {$$ = ["MUL",$1,$3];}
    | e '/' e
        {$$ = ["DIV",$1,$3];}
    | e '%' e
        {$$ = ["MOD",$1,$3];}
    | '-' e %prec UMINUS
        {$$ = ["NEG", $2];}

    | '~' e
        {$$ = ["BNOT",$2];}
    | e '^' e
        {$$ = ["BXOR",$1,$3];}
    | e '&' e
        {$$ = ["BAND",$1,$3];}
    | e '|' e
        {$$ = ["BOR",$1,$3];}
    | e '<<' e
        {$$ = ["LSHIFT",$1,$3];}
    | e '>>' e
        {$$ = ["RSHIFT",$1,$3];}

    | e '>' e
        {$$ = ["GT",$1,$3];}
    | e '>=' e
        {$$ = ["GTE",$1,$3];}
    | e '<' e
        {$$ = ["LT",$1,$3];}
    | e '<=' e
        {$$ = ["LTE",$1,$3];}
    | e '==' e
        {$$ = ["EQ",$1,$3];}
    | e '!=' e
        {$$ = ["NEQ",$1,$3];}

    | '&&' e
        {$$ = ["AND",$1,$3];}
    | '||' e
        {$$ = ["OR",$1,$3];}
    | '^^' e
        {$$ = ["XOR",$1,$3];}
    | '!' e
        {$$ = ["NOT", $2];}

    | '(' e ')'
        {$$ = $2;}
    | NUMBER
        {$$ = Number(yytext);}
    | TRUE
        {$$ = true;}
    | FALSE
        {$$ = false;}
    | '<<<' numbers '>>>'
        {$$ = new Buffer($2);}
    | STRING
        {$$ = new Buffer(JSON.parse($1));}
    | IDENT
        {$$ = ["IDENT", $1];}
    | '[' expressions ']'
        {$$ = ["LIST"].concat($2);}
    | '[' expressions ',' ']'
        {$$ = ["LIST"].concat($2);}
    | e ':' e
        {$$ = ["PAIR",$1,$3];}
    ;
