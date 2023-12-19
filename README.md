# StaticPy

将Python从动态类型的解释语言更改成静态类型的编译语言

目标语言为c++（如果还有时间写代码生成的话）

更改：

- 静态类型：
  - 在使用变量前声明采用[Type Hint](https://www.python.org/dev/peps/pep-0484/)：`x:int = 1`
  - 函数：`def funcname(para: para_type) -> return_type:`
  - 类：不允许通过self.attr动态添加属性，只能在init函数里定义
- 程序入口：由逐行执行改为从入口函数进入
- 高级功能：
  - 泛型：`AnyVar = TypeVar("AnyVar")`
  - 类的方法的重载

## 运行

```
g++ main.cpp lex.cpp grammar.cpp gencode.cpp -o compiler
.\compiler.exe "files/test_class.txt"
g++ files\out\cpp_file.cpp -o cpp_output
.\cpp_output.exe
```

运行效果：

<img src="https://umeta.oss-cn-beijing.aliyuncs.com/wx_program/image-20231210235036412.png" alt="image-20231210235036412" width=80% />

词法分析：

<img src="https://umeta.oss-cn-beijing.aliyuncs.com/wx_program/image-20231210235137259.png" alt="image-20231210235137259" width=15% />

语法树：



## 词法

类别码

| 单词名称   | 类别码     | 单词名称 | 类别码    | 单词名称 | 类别码 | 单词名称 | 类别码  |
| ---------- | ---------- | -------- | --------- | -------- | ------ | -------- | ------- |
| Ident      | IDENFR     | not      | NOTTK     | !        | NOT    | (        | LPARENT |
| IntConst   | INTCON     | and      | ANDTK     | <        | LSS    | )        | RPARENT |
| FloatConst | FLOATCON   | or       | ORTK      | <=       | LEQ    | [        | LBRACK  |
| StrConst   | STRCON     | return   | RETURNTK  | >        | GRE    | ]        | RBRACK  |
| const      | CONSTTK    | None     | NONETK    | >=       | GEQ    | {        | LBRACE  |
| int        | INTTK      | AddTab   | ADDTAB    | ==       | EQL    | }        | RBRACE  |
| break      | BREAK      | DelTab   | DELTAB    | !=       | NEQ    | +        | PLUS    |
| continue   | CONTINUETK | List     | LISTTK    | =        | ASSIGN | -        | MINU    |
| if         | IFTK       | Dict     | DICTTK    | ,        | COMMA  | *        | MULT    |
| else       | ELSETK     | False    | FALSETK   | :        | COLON  | /        | DIV     |
| def        | DEFTK      | True     | TRUETK    | ->       | ARROW  | %        | MOD     |
| class      | CLASSTK    | TypeVar  | TYPEVARTK | .        | DOT    |          |         |
| while      | WHILETK    | self     | SELFTK    |          |        |          |         |
| init       | INITTK     | bool     | BOOLTK    |          |        |          |         |
| long       | LONGTTK    | str      | STRTK     |          |        |          |         |
| append     | APPENDTK   |          |           |          |        |          |         |



## 文法

TODO:
  - 语法结构
    - for循环：`for iter:type in container:`
  - 数据结构
    - ~~Dict声明写得有点问题~~(Done)
    - ~~LVal为List/Dict中元素赋值~~(Done)
    - ~~float数据类型~~
    - ~~List/Dict的嵌套~~
    - list的append
- 代码生成：
  - ~~输出c++代码~~(Done)
- 泛型
  - ~~参考[pep-0484](https://peps.python.org/pep-0484/#user-defined-generic-types)~~
- 类
  - ~~self的使用~~
  - ~~方法重写~~

```python
CompUnit ::= { [GenericDefs] (ClassDef | FuncDef)}
GenericDefs ::= {GenericDef}
GenericDef ::= Ident '=' 'TypeVar' '(' Str ')'
ClassDef ::= 'class' Ident ':' 'AddTab' {ClassAttrDef} [ClassInitDef] {ClassFuncDef} 'DelTab'
ClassAttrDef ::= Ident ':' DataType
ClassInitDef ::= 'def' 'init' '(' [FuncFParams] ')' Block
ClassFuncDef ::= 'def' Ident '(' 'self' [',' FuncFParams] ')' '->' FuncType Block
FuncDef ::= 'def' Ident '(' [FuncFParams] ')' '->' FuncType Block
FuncType ::= 'None' | DataType
DataType ::= 'int' | 'float' | 'long' | 'str' | 'bool'
    | 'List' '[' DataType ']'
    | 'Dict' '[' DataType ',' DataType ']'
    | Ident  #泛型或者类
Block ::= ':' 'AddTab' {BlockItem} 'DelTab'
BlockItem ::= Decl | Stmt
Decl ::= Ident ':' DataType ['=' InitVal]  #静态类型检查
InitVal ::= Exp
    | '[' [InitVal {',' InitVal}] ']' 
    | '{' [InitVal ':' InitVal {',' InitVal ':' InitVal}] '}'
FuncFParams ::= FuncFParam {',' FuncFParam}
FuncFParam ::= Ident ':' DataType
Stmt ::= Exp
    | LVal '=' Exp
    | 'if' Cond Block ['else' Block]
    | 'while' Cond Block
    | 'break' | 'continue'
    | 'return' [Exp]  #返回值类型检查
    | 'print' '(' [(Str | Exp) {',' (Str | Exp)}] ')'
    | LVal '.' 'append' '(' Exp ')'
Exp ::= LOrExp
AddExp ::= MulExp { ('+' | '−') MulExp }
MulExp ::= UnaryExp { ('*' | '/' | '%') UnaryExp }
UnaryExp ::= IdentExp | PrimaryExp | ('+' | '−' | 'not') UnaryExp
IdentExp ：：= LVal [[GenericReal] '(' [FuncRParams] ')'] #函数或类的init
GenericReal ::= '<' DataType {',' DataType} '>'
PrimaryExp ::= '(' Exp ')' | IntConst | FloatConst | StrConst | 'True' | 'False'
FuncRParams ::= Exp { ',' Exp }
LVal ::= ['self' '.' ] Ident {'[' Exp ']'} {'.' Ident {'[' Exp ']'}}
LOrExp ::= LAndExp { 'or' LAndExp }
LAndExp ::= EqExp { 'and' EqExp }
EqExp ::= RelExp { ('==' | '!=') RelExp }
RelExp ::= AddExp { ('<' | '>' | '<=' | '>=') AddExp }
```

