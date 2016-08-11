```
keyword_constant:
    true
    | false
    | null

variable:
    ID 
    | "(" expression ")"
    | table_decl
    | lambda_decl

variable_suffix:
    variable { ( "[" expression "]" | "(" expression_list ")" | "." ID )}

value:
    INT_CONST
    | CHAR_CONST
    | STRING_CONST
    | keyword_constant
    | variable_suffix

not_expr:
    [ "!" ] value

neg_expr:
    [ "-" ] not_expr

mul_and_div_expr:
    neg_expr { ( "*" | "/" ) neg_expr }

add_and_sub_expr:
    mul_and_div_expr { ( "+" | "-" ) mul_and_div_expr }

real_expr:
    add_and_sub_expr [add_and_sub_expr ]

and_expr:
    real_expr {  ( "<" | ">" | ">=" | "<=" | "==" | "!=" )  real_expr } 

or_expr:
    and_expr { "&" and_expr } 

expression:
    or_expr { "|" or_expr }

expression_list:
    expression { "," expression }

statement:
    block
    | "if" "(" assign_expression ")" statement [ "else" statement ]
    | "while" "(" assign_expression ")" statement
    | "return" [ assign_expression ] ";"
    | "break" ";"
    | "continue" ";"
    | expression ";"
    | decl

block:
    "{" { statement } "}"

param_list:
    ID { ","  ID }

function_decl:
    "function" "(" [ param_list ] ")" block

variable_decl:
    define_decl
    | let_decl

define_decl:
    "define" ID "=" expression 
    
let_decl:
    "let" ID "=" expression

decl:
    variable_decl
    | function_decl

program: 
    { ( decl | expression ) }
```