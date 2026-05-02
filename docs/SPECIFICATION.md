# 🔤 VYRONIX Language Specification v1.0

## 1. Lexical Structure
VYRONIX is whitespace-sensitive. Indentation (spaces or tabs) is used to denote block levels.

### Keywords
`let`, `set`, `drop`, `echo`, `ask`, `unit`, `give`, `when`, `elif`, `otherwise`, `loop`, `while`, `class`, `self`, `init`, `new`, `yes`, `no`, `null`, `spawn`, `async`, `await`, `alloc`, `free`, `copy`, `move`, `import`, `from`, `as`, `try`, `catch`, `finally`, `raise`, `panic`.

---

## 2. EBNF Grammar
```ebnf
program        = { statement } ;
statement      = var_decl | assignment | echo_stmt | if_stmt | while_stmt 
               | func_decl | class_decl | memory_stmt | concurrency_stmt ;

block          = INDENT { statement } DEDENT ;

expression     = logical_or ;
logical_or     = logical_and { "or" logical_and } ;
logical_and    = equality { "and" equality } ;
equality       = comparison { ("==" | "!=") comparison } ;
comparison     = term { (">" | ">=" | "<" | "<=") term } ;
term           = factor { ("+" | "-") factor } ;
factor         = primary { ("*" | "/") primary } ;
primary        = NUMBER | STRING | IDENTIFIER | "(" expression ")" | call_expr | "yes" | "no" | "null" ;

var_decl       = "let" IDENTIFIER [ ":" IDENTIFIER ] [ "=" expression ] ;
assignment     = "set" IDENTIFIER "=" expression ;
echo_stmt      = "echo" expression ;

if_stmt        = "when" expression ":" block { "elif" expression ":" block } [ "otherwise" ":" block ] ;
while_stmt     = "while" expression ":" block ;

func_decl      = "unit" IDENTIFIER "(" [params] ")" [ ":" IDENTIFIER ] ":" block ;
params         = IDENTIFIER [ ":" IDENTIFIER ] { "," IDENTIFIER [ ":" IDENTIFIER ] } ;

memory_stmt    = "alloc" IDENTIFIER expression 
               | "free" IDENTIFIER 
               | "copy" IDENTIFIER IDENTIFIER 
               | "move" IDENTIFIER IDENTIFIER ;

concurrency_stmt = "spawn" call_expr | "await" IDENTIFIER ;
```

---

## 3. Type System
### Built-in Types
- `int`: 64-bit signed integer.
- `float`: 64-bit floating point.
- `str`: UTF-8 encoded string.
- `bool`: Boolean (`yes` or `no`).
- `void`: Empty return type.
- `ptr`: Raw memory address.
- `byte`: 8-bit unsigned integer.

---

## 4. Memory Model
VYRONIX implements a deterministic memory model:
1.  **Stack Allocation**: Local variables are automatically reclaimed.
2.  **Manual Heap**: `alloc` and `free` provide explicit control.
3.  **Ownership Semantics**: `move` transfers resource ownership, preventing double-free and use-after-free bugs at the compiler level.

---

## 5. Error Handling
VYRONIX uses a structured exception model:
```python
try:
    # Risky operation
    let file = open("data.txt")
catch error:
    echo "Failed to open file"
finally:
    echo "Cleanup complete"
```
