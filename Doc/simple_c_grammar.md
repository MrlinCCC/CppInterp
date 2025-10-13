# EBNF

```
---------------- Grammer Structure ----------------
program ::= { import_stmt | function_definition | statement} ;
import_stmt ::= "import" (STRING_LITERAL | IDENTIFIER) ";" ;
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
if_stmt ::= "if" "(" expression ")" statement [ "else" statement ] ;
switch_stmt ::= "switch" "(" expression ")" "{" { case_clause } [ default_clause ] "}" ;
case_clause ::= "case" literal ":" { statement } ;
default_clause ::= "default" ":" { statement } ;
---------------- Loop ----------------
while_stmt ::= "while" "(" expression ")" statement ;
for_stmt ::= "for" "("  [ variable_declaration without ";" |  expression ] ";"  [ expression ] ";" [ expression ] ")" statement ;
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
          | "(" expression ")"
          | function_literal;
argument_list ::= expression { "," expression } ;
initializer ::= assignment
                     | "." identifier "=" initializer
                     | "{" [ initializer_list ] "}" ;
initializer_list ::= initializer { "," initializer } ;
function_literal ::= "lambda" "(" [ parameter_list ] ")" "->" type compound_stmt ;
---------------- Literals ----------------
literal ::= INT_LITERAL
          | DOUBLE_LITERAL
          | CHAR_LITERAL
          | STRING_LITERAL
          | BOOL_LITERAL
          | NULL_LITERAL ;

---------------- Type ----------------
type ::= builtin_type | identifier | function_type;
builtin_type ::= "int" | "double" | "char" | "string" | "bool" | "void" ;
function_type ::= "(" [ parameter_type_list ] ")" "->" type ;
parameter_type_list ::= type { "," type } ;
```

# Terminal

- Import: "import"
- Identifier: "identifier"
- Keyword: "function", "let", "const", "struct", "if", "else", "switch", "case", "default", "while", "for", "return", "break", "continue", "lambda"
- Symbol: "(", ")", "{", "}", "[", "]", ";", ",", ":", ".", "->"
- Operator: "=", "+=", "-=", "\*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=", "+", "-", "!", "~", "++", "--", "||", "&&", "|", "^", "&", "==", "!=", "<", ">", "<=", ">=", "<<", ">>", "?"
- Literal: INT_LITERAL, DOUBLE_LITERAL, CHAR_LITERAL, STRING_LITERAL, BOOL_LITERAL, NULL_LITERAL
- Type: "int", "double", "char", "string", "bool", "void"

# Non-Terminal

- Program: "program", "function_definition", "import_stmt", "parameter_list", "parameter",
- Statement: "compound_stmt", "expression_stmt", "variable_declaration", "declarator_list", "declarator", "struct_definition", "struct_member_declaration"
- Condition: "if_stmt", "switch_stmt", "case_clause", "default_clause"
- Loop: "while_stmt", "for_stmt", "return_stmt", "break_stmt", "continue_stmt",
- Expressions: "expression", "assignment", "assignment_operator", "conditional", "logical_or", "logical_and", "bit_or", "bit_xor", "bit_and", "equality", "relational", "shift", "additive", "multiplicative", "unary", "postfix", "primary", "argument_list", "initializer", "initializer_list", "function_literal"
- Literals: "literal"
- Type: "type", "builtin_type", "function_type", "parameter_type_list"

# AST Node Classification

- Sequence Chosen Nodes( { ... } or { X | Y | Z } ): program
- Single Chosen Nodes( (X | Y | Z)... ): literal, type, builtin_type, assignment_operator, primary, initializer
- List Nodes( X { "," X } ): parameter_list, declarator_list, parameter_type_list, argument_list, initializer_list, expression
- Sequence Nodes( A B C ... ): function_definition, compound_stmt, declarator, struct_definition, if_stmt, switch_stmt, case_clause, default_clause, for_stmt, return_stmt, assignment, binary_expr, unary, function_type, function_literal
- Fix Nodes( A B C ): import_stmt, parameter, variable_declaration, struct_member_declaration, while_stmt, break_stmt, continue_stmt
- Logic Nodes: statement, expression_stmt
- Expression Nodes ( if only left return left else return new node after built relation )
  - Sequence Relation: conditional
  - Left Associative: binary(logical_or, logical_and, bit_or, bit_xor, bit_and, equality, relational, shift, additive, multiplicative), postfix
  - Right Associative: unary

# Notes

- 该语法非严格 LL1，但可以通过递归下降法解析

- if dangling else 文法歧义上通过以下拆分解决, 但递归下降隐式会优先解析最深的 if, 可以很自然解决

```
  if_stmt ::= matched_if | unmatched_if ;
  matched_if ::= "if" "(" expression ")" matched_stmt "else" matched_stmt ;
  unmatched_if ::= "if" "(" expression ")" statement ;
  matched_stmt ::= matched_if | statement ;
```

- 变量定义时保留 let/const,否则 statement 可以推导为 expression_stmt 或 variable_declaration,二者的 first 集都有 identifier 会存在冲突

- 表达式中根据优先级进行划分,同时要注意运算符的左右结合性
