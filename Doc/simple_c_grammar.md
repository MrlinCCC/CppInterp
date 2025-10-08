<!-- # Grammar Definition

```ebnf
program ::= { declaration | function_definition } ;
declaration ::= type init_declarator_list ";" ;
init_declarator_list ::= init_declarator { "," init_declarator } ;
init_declarator ::= identifier [ "=" expression ] ;
function_definition ::= type identifier "(" [ parameter_list ] ")" compound_stmt ;
parameter_list ::= parameter { "," parameter } ;
parameter ::= type identifier ;
compound_stmt ::= "{" { statement } "}" ;
statement ::= compound_stmt | declaration | expression_stmt | if_stmt | while_stmt | for_stmt | return_stmt ;
expression_stmt ::= [ expression ] ";" ;
if_stmt ::= "if" "(" expression ")" statement { "else" "if" "(" expression ")" statement } [ "else" statement ] ;
while_stmt ::= "while" "(" expression ")" statement ;
for_stmt ::= "for" "(" [expression_stmt] [expression_stmt] [expression] ")" statement ;
return_stmt ::= "return" [ expression ] ";" ;
expression ::= assignment { "," assignment } ;
assignment ::= conditional [ assignment_operator assignment ] ;
assignment_operator ::= "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>=" ;
conditional ::= logical_or [ "?" expression ":" conditional ] ;
logical_or ::= logical_and { "||" logical_and } ;
logical_and ::= bit_or { "&&" bit_or } ;
bit_or ::= bit_xor { "|" bit_xor } ;
bit_xor ::= bit_and { "^" bit_and } ;
bit_and ::= equality { "&" equality } ;
equality ::= relational { ("==" | "!=") relational } ;
relational ::= shift { ("<" | ">" | "<=" | ">=") shift } ;
shift ::= additive { ("<<" | ">>") additive } ;
additive ::= multiplicative { ("+" | "-") multiplicative } ;
multiplicative ::= unary { ("*" | "/" | "%") unary } ;
unary ::= [("+" | "-" | "!" | "~" | "++" | "--")] postfix ;
postfix ::= primary [("++" | "--")] ;
primary ::= identifier | literal | "(" expression ")" | function_call ;
function_call ::= identifier "(" [ argument_list ] ")" ;
argument_list ::= expression { "," expression } ;
literal ::= INT_LITERAL | DOUBLE_LITERAL | CHAR_LITERAL | STRING_LITERAL | BOOL_LITERAL ;
type ::= "int" | "double" | "char" | "string" | "bool" ;
``` -->

# EBNF

```
---------------- Grammer Structure ----------------
program ::= { import_stmt | function_definition | statement} ;
function_definition ::= "function" type identifier "(" [ parameter_list ] ")" compound_stmt ;
parameter_list ::= parameter { "," parameter } ;
parameter ::= type declarator ;

---------------- Statement ----------------
statement ::=
      compound_stmt
    | expression_stmt
    | variable_declaration
    | struct_definition
    | if_stmt
    | switch_stmt
    | while_stmt
    | for_stmt
    | return_stmt
    | break_stmt
    | continue_stmt ;
compound_stmt ::= "{" { statement } "}" ;
expression_stmt ::= [ expression ] ";" ;
variable_declaration ::= ("let" | "const") type declarator_list ";" ;
declarator_list ::= declarator { "," declarator } ;
declarator ::= identifier { "[" [ expression ] "]" } [ "=" initializer ] ;
struct_definition ::= "struct" identifier "{" { struct_member_declaration } "}" ";" ;
struct_member_declaration ::= type declarator_list ";" ;
------------- Condition --------------
if_stmt       ::= matched_if | unmatched_if ;
matched_if    ::= "if" "(" expression ")" matched_stmt "else" matched_stmt ;
unmatched_if  ::= "if" "(" expression ")" statement ;
matched_stmt  ::= matched_if | statement ;
switch_stmt ::= "switch" "(" expression ")" "{" { case_clause } [ default_clause ] "}" ;
case_clause ::= "case" literal ":" { statement } ;
default_clause ::= "default" ":" { statement } ;
---------------- Loop ----------------
while_stmt ::= "while" "(" expression ")" statement ;
for_stmt ::= "for" "(" [ expression ] ";" [ expression ] ";" [ expression ] ")" statement ;
return_stmt ::= "return" [ expression ] ";" ;
break_stmt ::= "break" ";" ;
continue_stmt ::= "continue" ";" ;

---------------- Expressions ----------------
expression ::= assignment { "," assignment } ;
assignment ::= conditional [ assignment_operator assignment ] ;
assignment_operator ::= "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>=" ;
conditional ::= logical_or [ "?" expression ":" conditional ] ;
logical_or ::= logical_and { "||" logical_and } ;
logical_and ::= bit_or { "&&" bit_or } ;
bit_or ::= bit_xor { "|" bit_xor } ;
bit_xor ::= bit_and { "^" bit_and } ;
bit_and ::= equality { "&" equality } ;
equality ::= relational { ("==" | "!=") relational } ;
relational ::= shift { ("<" | ">" | "<=" | ">=") shift } ;
shift ::= additive { ("<<" | ">>") additive } ;
additive ::= multiplicative { ("+" | "-") multiplicative } ;
multiplicative ::= unary { ("*" | "/" | "%") unary } ;
unary ::= {("+" | "-" | "!" | "~" | "++" | "--")} postfix ;
postfix ::= primary { ("++" | "--" | "." identifier | "[" expression "]" | "(" [ argument_list ] ")") } ;
primary ::= identifier
          | literal
          | "(" expression ")";
argument_list ::= expression { "," expression } ;
initializer ::= expression
                     | "." identifier "=" expression
                     | "{" [ initializer_list ] "}" ;
initializer_list ::= initializer { "," initializer } ;

---------------- Literals ----------------
literal ::= INT_LITERAL
          | DOUBLE_LITERAL
          | CHAR_LITERAL
          | STRING_LITERAL
          | BOOL_LITERAL
          | NULL_LITERAL ;

---------------- Type ----------------
type ::= builtin_type | identifier ;
builtin_type ::= "int" | "double" | "char" | "string" | "bool" | "void" ;

---------------- Import ----------------
import_stmt ::= "import" import_path ";" ;
import_path ::= STRING_LITERAL | identifier ;
```

# Terminal

- Identifier: "identifier"
- Keyword: "function", "let", "const", "struct", "if", "else", "switch", "case", "default", "while", "for", "return", "break", "continue"
- Symbol: "(", ")", "{", "}", "[", "]", ";", ",", ":", "."
- Operator: "=", "+=", "-=", "\*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=", "+", "-", "!", "~", "++", "--", "||", "&&", "|", "^", "&", "==", "!=", "<", ">", "<=", ">=", "<<", ">>", "?"
- Literal: INT_LITERAL, DOUBLE_LITERAL, CHAR_LITERAL, STRING_LITERAL, BOOL_LITERAL, NULL_LITERAL
- Type: "int", "double", "char", "string", "bool", "void"
- Import: "import"

# Non-Terminal

- Program: "program", "function_definition", "parameter_list", "parameter",
- Statement: "statement", "compound_stmt", "expression_stmt", "variable_declaration", "declarator_list", "declarator", "struct_definition", "struct_member_declaration"
- Condition: "if_stmt", "matched_if", "unmatched_if", "matched_stmt", "switch_stmt", "case_clause", "default_clause"
- Loop: "while_stmt", "for_stmt", "return_stmt", "break_stmt", "continue_stmt",
- Expressions: "expression", "assignment", "assignment_operator", "conditional", "logical_or", "logical_and", "bit_or", "bit_xor", "bit_and", "equality", "relational", "shift", "additive", "multiplicative", "unary", "postfix", "primary", "argument_list", "initializer", "initializer_list"
- Literals: "literal"
- Type: "type", "builtin_type"
- Import: "import_stmt", "import_path"

# Notes

该语法非严格 LL1，但可以通过递归下降法解析
