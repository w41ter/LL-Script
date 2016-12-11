## EBNF

```
keyword_constant:
    true
    | false
    | null

constant:
    INT_CONST
    | CHAR_CONST
    | STRING_CONST

variable:
    ID 
    | "(" right_hand_expression ")"
    | lambda_decl

expression_list:
    right_hand_expression { "," right_hand_expression }

variable_suffix_common:
    "[" expression "]" 
    | "(" [ expression_list ] ")" 
    | "." ID

variable_suffix:
    variable { variable_suffix_common }

table_decl_line:
    ID [ "=" right_hand_expression ]
    | constant [ "=" right_hand_expression ]

table_decl:
    "[" table_decl_line { "," table_decl_line } "," "]"

value:
    constant
    | keyword_constant
    | table_decl
    | variable_suffix

not_expr:
    [ "!" ] value

neg_expr:
    [ "-" ] not_expr

mul_and_div_expr:
    neg_expr { ( "*" | "/" ) neg_expr }

add_and_sub_expr:
    mul_and_div_expr { ( "+" | "-" ) mul_and_div_expr }

realtion_expr:
    add_and_sub_expr {  ( "<" | ">" | ">=" | "<=" | "==" | "!=" )  add_and_sub_expr }

and_expr:
    realtion_expr {  "&"  realtion_expr } 

or_expr:
    and_expr { "|" and_expr } 

right_hand_expression:
    or_expr

assignment_expression:
    or_expr { variable_suffix_common } "=" right_hand_expression

expression:
    assign_expression ";"

statement:
    block
    | "if" "(" right_hand_expression ")" statement [ "else" statement ]
    | "while" "(" right_hand_expression ")" statement
    | "return" [ right_hand_expression ] ";"
    | "break" ";"
    | "continue" ";"
    | expression
    | variable_decl
    | ";"

block:
    "{" { statement } "}"

param_list:
    ID { ","  ID }

function_common:
    "(" [ param_list ] ")" block

function_decl:
    "function" function_common

lambda_decl:
    "lambda" function_common

variable_decl:
    "define" ID "=" expression 
    | "let" ID "=" expression

program: 
    { statement | function_decl | EOF }
```

## Example

```
let lib = require("other.ll");

function main() {
    lib.test();

    let table = [ 
        "string",
        1,
        call = lambda () {
            println("here");
        }
    ];

    table.call();
    println(table[0]);

    return;
}


