```
keyword_constant:
    true
    | false
    | null
    
factor:
    INT_CONST
    | CHAR_CONST
    | STRING_CONST
    | keyword_constant
    | ID 
    | "(" expression ")"
    | "[" expression_list "]"

positive_factor:
    factor { ( [ "[" expression "]" ] | [ "(" expression_list ")" ] ) }
    
not_factor:
    [ "!" ] positive_factor

term:
    [ "-" ] not_factor

additive_expression:
    term { ( "*" | "/" ) term }

relational_expression:
    additive_expression { ( "+" | "-" ) additive_expression }

and_expression:
    relational_expression [ ( "<" | ">" | ">=" | "<=" | "==" | "!=" ) relational_expression ]

or_expression:
    and_expression { "&&" and_expression } 

expression:
    or_expression { "||" or_expression } 

assign_expression:
    expression { "=" expression }

var_decl:
    "let" ID "=" assign_expression ";" 

expression_list:
    expression { "," expression }

statement:
    block
    | "if" "(" assign_expression ")" statement [ "else" statement ]
    | "while" "(" assign_expression ")" statement
    | "return" [ assign_expression ] ";"
    | "break" ";"
    | "continue" ";"
    | assign_expression ";"
    | var_decl

block:
    "{" { statement } "}"

param_list:
    ID { ","  ID }

function_decl:
    "function" ID "(" [ param_list ] ")" block

program: 
    { function_decl }
```