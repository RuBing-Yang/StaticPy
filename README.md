# StaticPy

将Python从动态类型的解释型语言更改成静态类型的编译型语言

对Python进行的更改：

- 静态类型：
  - 在使用变量前声明采用[Type Hint](https://www.python.org/dev/peps/pep-0484/)：`x:int = 1`，严格类型检查
  - 函数：`def funcname(para: para_type) -> return_type:`
  - 类：不允许通过self.attr动态添加属性
  - 允许精度不降低的隐式类型转换 `bool->int->float/long`
- 程序入口：由逐行执行改为从入口函数进入
- 高级功能：
  - 为类和函数提供泛型定义：`AnyVar = TypeVar("AnyVar")`
  - 运行函数和类的方法的重载

## 运行

```bash
g++ main.cpp lex.cpp grammar.cpp semantic.cpp gencodeCST.cpp -o compiler
# DEBUG模式后面加1
.\compiler.exe "files/test_mini.txt"
.\compiler.exe "files/test_example.txt"
.\compiler.exe "files/test_func.txt"
.\compiler.exe "files/test_class.txt"
.\compiler.exe "files/test_stl.txt"
.\compiler.exe "files/test_generic.txt"
g++ files\out\cpp_file.cpp -o cpp_output
.\cpp_output.exe
```

运行效果：

<img src="https://umeta.oss-cn-beijing.aliyuncs.com/wx_program/image-20231210235036412.png" alt="image-20231210235036412" width=80% />

词法分析：

<img src="https://umeta.oss-cn-beijing.aliyuncs.com/wx_program/image-20231210235137259.png" alt="image-20231210235137259" width=15% />


TODO:
  - 数据结构
    - ~~Dict声明写得有点问题~~
    - ~~LVal为List/Dict中元素赋值~~
    - ~~float数据类型~~
    - ~~List/Dict的嵌套~~
    - ~~list的append~~
- 代码生成：
  - ~~输出c++代码~~
- 泛型
  - ~~参考[pep-0484](https://peps.python.org/pep-0484/#user-defined-generic-types)~~
- 类
  - ~~self的使用~~
  - ~~方法重写~~
- 错误处理
  - ~~词法分析~~
    - ~~缩进不是4个空格的整数倍~~
    - ~~代码块缩进超过一个制表符~~
    - ~~标识符以数字开头~~
    - ~~long长整型常数字面量超过64位~~
    - ~~缺少匹配的闭合引号/多行注释三引号~~
    - ~~发现未定义的字符~~
  - ~~语法分析~~
    - ~~字符匹配~~
  - ~~语义分析~~：
    - ~~重定义：~~
      - ~~同级变量重定义~~
      - ~~类重名，泛型重名，类和泛型重名~~
      - ~~函数名相同且参数表相同~~
    - ~~未定义：~~
      - ~~使用了未声明的变量~~
      - ~~数据类型名称未定义(泛型/类)~~
      - ~~函数未定义~~
      - ~~不在类的声明中使用了self~~
    - ~~静态类型检查：~~
      - ~~赋值语句~~
      - ~~函数返回值~~
      - ~~运算（以及是否允许隐式类型转换）~~
      - ~~下标类型检查（List的id或者Dict的key）~~

## 词法分析

类别码

| 单词名称   | 类别码     | 单词名称 | 类别码    | 单词名称 | 类别码 | 单词名称 | 类别码  |
| ---------- | ---------- | -------- | --------- | -------- | ------ | -------- | ------- |
| Ident      | IDENFR     | not      | NOTTK     | .        | DOT    | (        | LPARENT |
| IntConst   | INTCON     | and      | ANDTK     | <        | LSS    | )        | RPARENT |
| FloatConst | FLOATCON   | or       | ORTK      | <=       | LEQ    | [        | LBRACK  |
| LongConst  | LONGCON    | return   | RETURNTK  | >        | GRE    | ]        | RBRACK  |
| StrConst   | STRCON     | None     | NONETK    | >=       | GEQ    | {        | LBRACE  |
| int        | INTTK      | AddTab   | ADDTAB    | ==       | EQL    | }        | RBRACE  |
| break      | BREAK      | DelTab   | DELTAB    | !=       | NEQ    | +        | PLUS    |
| continue   | CONTINUETK | List     | LISTTK    | =        | ASSIGN | -        | MINU    |
| if         | IFTK       | Dict     | DICTTK    | ,        | COMMA  | *        | MULT    |
| else       | ELSETK     | False    | FALSETK   | :        | COLON  | /        | DIV     |
| def        | DEFTK      | True     | TRUETK    | ->       | ARROW  | %        | MOD     |
| class      | CLASSTK    | TypeVar  | TYPEVARTK |          |        |          |         |
| while      | WHILETK    | self     | SELFTK    |          |        |          |         |
| init       | INITTK     | bool     | BOOLTK    |          |        |          |         |
| long       | LONGTTK    | str      | STRTK     |          |        |          |         |
| append     | APPENDTK   |          |           |          |        |          |         |



## 语法分析

具体语法树：

```python
CompUnit ::= { [GenericDefs] (ClassDef | FuncDef)}
GenericDefs ::= {GenericDef}
GenericDef ::= Ident '=' 'TypeVar' '(' Str ')'
ClassDef ::= 'class' Ident ':' 'AddTab' {ClassAttrDef} {ClassInitDef} {ClassFuncDef} 'DelTab'
ClassAttrDef ::= Ident ':' DataType
ClassInitDef ::= 'def' 'init' '(' [FuncFParams] ')' Block
ClassFuncDef ::= 'def' Ident '(' 'self' [',' FuncFParams] ')' '->' FuncType Block
FuncDef ::= 'def' Ident '(' [FuncFParams] ')' '->' FuncType Block
FuncType ::= 'None' | DataType
DataType ::= 'int' | 'float' | 'long' | 'str' | 'bool'
    | 'List' '[' DataType ']'
    | 'Dict' '[' DataType ',' DataType ']'
    | Ident [GenericReal]  #泛型或者类
Block ::= ':' 'AddTab' {BlockItem} 'DelTab'
BlockItem ::= Decl | Stmt
Decl ::= Ident ':' DataType ['=' InitVal]  #静态类型检查
InitVal ::= Exp
    | '[' [InitVal {',' InitVal}] ']' 
    | '{' [Exp ':' InitVal {',' Exp ':' InitVal}] '}'
FuncFParams ::= FuncFParam {',' FuncFParam}
FuncFParam ::= Ident ':' DataType
Stmt ::= Exp
    | LVal '=' Exp
    | 'if' Cond Block ['else' Block]
    | 'while' Cond Block
    | 'break' | 'continue'
    | 'return' [Exp]  #返回值类型检查
    | 'print' '(' [Exp {',' Exp}] ')'
    | LVal '.' 'append' '(' Exp ')'
Exp ::= LOrExp
AddExp ::= MulExp { ('+' | '−') MulExp }
MulExp ::= UnaryExp { ('*' | '/' | '%') UnaryExp }
UnaryExp ::= IdentExp | PrimaryExp | ('+' | '−' | 'not') UnaryExp
IdentExp ：：= LVal [[GenericReal] '(' [FuncRParams] ')'] #函数或类的构造方法
GenericReal ::= '<' DataType {',' DataType} '>'
PrimaryExp ::= '(' Exp ')' | IntConst | FloatConst | LongConst | StrConst | 'True' | 'False'
FuncRParams ::= Exp { ',' Exp }
LVal ::= ['self' '.' ] Ident {'[' Exp ']'} {'.' Ident {'[' Exp ']'}}
LOrExp ::= LAndExp { 'or' LAndExp }
LAndExp ::= EqExp { 'and' EqExp }
EqExp ::= RelExp { ('==' | '!=') RelExp }
RelExp ::= AddExp { ('<' | '>' | '<=' | '>=') AddExp }
```

## 语义分析

### 符号表

| 类型    | 组织形式                   |
| ------- | ------------------------- |
| class   | 原名 -> List [UnaryClass] |
|         | 新名 -> UnaryClass        |
| func    | 原名 -> List [UnaryFunc]  |
| var     | 原名 -> Stack [UnaryVar]  |
| generic | 名称->id                  |
|         | List [UnaryDataType]      |

### 数据结构

```c++
class UnaryDataType {
    string type;  // "basic", "class", "generic" (generic不参与类型判定)
    string name;  // "int", "float", "long", "bool", "str", "None", Ident
}
class NestDataType {  // 用于List和Map嵌套
    vector<UnaryDataType> datatype_list;
}
class UnaryVar {
    int level;  // 用于block结束弹栈
    string new_name;  // 唯一确定变量
    string old_name;  // 原程序命名
    NestDataType nest_datatype;  // 数据类型
}
class VarStack {
    stack<UnaryVar> vars;
}

class UnaryFunc {
    string new_name;  // 唯一确定函数
    string old_name;  // 原程序命名
    CSTNode* cst_root;  // 模板函数泛型实现入口
    map<string, int> generic_name2id;  // 泛型
    vector<UnaryDataType> generics;  // 泛型取值
    vector<NestDataType> fparams;  // 形参数据类型
    vector<string> params_name;  // 形参名，与block内部同级
    NestDataType ret;  // 返回值数据类型
}
class FuncList {
    vector<UnaryFunc> funcs;  // 允许函数重载
}

class UnaryClass {
    string new_name;  // 唯一确定类
    string old_name;  // 原程序命名
    CSTNode* cst_root;  // 模板类泛型实现入口
    map<string, int> generic_name2id;  // 泛型
    vector<UnaryDataType> generics;  // 泛型取值
    map<string, NestDataType> attrs;  // 属性数据类型
    map<string, FuncList> funcMap;  // 允许方法重载
}
class ClassList {
    vector<UnaryClass> classes;  // 模板函数和它的每个泛型实现
}
```

### 抽象语义树

命名格式：函数名为func_id，类名为class_id，变量名、类属性名、函数形参名为var_id

| type         | s                                                | datatype                      | 子节点                                                 | 描述                                  |
| ------------ | ------------------------------------------------ | ----------------------------- | ------------------------------------------------------ | ------------------------------------- |
| CompUnit     |                                                  |                               | {ClassDef}, {FuncDef}                                  |                                       |
| ClassDef     | 类名class_id                                     | class class_name              | {ClassAttrDef}, {ClassInitDef}, {ClassFuncDef}, Block  | 类定义                                |
| ClassAttrDef | 类属性名var_id                                   | 属性数据类型                  |                                                        |                                       |
| ClassInitDef | 类函数名func_id                                  | class class_name              | Block                                                  |                                       |
| ClassFuncDef | 类函数名func_id                                  | 函数返回数据类型              | Block                                                  |                                       |
| FuncDef      | 函数名func_id                                    | 函数返回数据类                | {FuncFParam}, Block                                    | 函数定义                              |
| FuncFParam   | 函数形参名var_id                                 | 形参数据类型                  |                                                        |                                       |
| Block        |                                                  |                               |                                                        | 代码块                                |
| Decl         | =                                                |                               | 变量var，赋值InitVal (ListInitVal / DictInitVal / Exp) |                                       |
|              | 变量名                                           | 变量数据类型                  |                                                        |                                       |
| ListInitVal  |                                                  |                               | {列表元素}                                             | 每个列表元素是InitVal，允许嵌套       |
| DictInitVal  |                                                  |                               | {DictElement}                                          |                                       |
| DictElement  |                                                  |                               | Exp, Exp / InitVal                                     | key - value，value可以继续嵌套InitVal |
| Stmt         | =                                                |                               | LVal, Exp                                              |                                       |
|              | append                                           |                               | Exp                                                    |                                       |
|              | if                                               |                               | Exp, Block[, Block]                                    | 可能有else对应的Block                 |
|              | while                                            |                               | Exp, Block                                             |                                       |
|              | break/continue                                   |                               |                                                        |                                       |
|              | return                                           |                               | [Exp]                                                  | 无返回值函数没有子节点                |
|              | print                                            |                               | {Exp}                                                  |                                       |
| Exp          |                                                  | 由运算结果决定                | TwoOp/OneOp/const/LVal                                 |                                       |
| TwoOp        | 二元运算符+-*/% > < == != >= <= and or           | 由运算结果决定                | 左右两个子运算树，中缀表达式                           |                                       |
| OneOp        | 一元运算符 +-not                                 | 由运算结果决定                | 一个子运算树                                           |                                       |
| const        | int/float/long数字 <br />字符串 布尔值True/False | basic int/float/long/str/bool |                                                        |                                       |
| LVal         |                                                  |                               | [self] var/attr {index} {FuncCall/attr {index}}        |                                       |
| self         | self                                             |                               |                                                        |                                       |
| var          | 变量名var_id                                     | 变量数据类型                  |                                                        |                                       |
| attr         | 类属性名var_id                                   | 属性数据类型                  |                                                        |                                       |
| FuncCall     | 函数名                                           | 函数返回数据类型              | {FuncRParam}                                           |                                       |
| FuncRParam   |                                                  |                               | Exp                                                    | 函数实参                              |
| Index        |                                                  | 索引数据类型                  | Exp                                                    |                                       |




## 示例

语法错误:
```python
class Position
    x : int
    y : int
    def init(x1 : int, y1 : int):
        self.x = x1
        self.y = y1
```
```bash
[line 2] SyntaxError: <ClassDef> Expect COLON but get ADDTAB
```

示例：
```python
# 泛型定义
T1 = TypeVar("T1")
T2 = TypeVar("T2")
class Exp:
    x : T1
    y : T2
    def init(x : T1, y : T2):
        self.x = x
        self.y = y
    def getList(self) -> List[T1]:
        l : List[T1] = [self.x, self.y]
        return l

# 递归函数
def factorial(n: int) -> int:
    if n == 0 or n == 1:
        return 1
    else:
        return n * factorial(n - 1)
# 字符串参与逻辑运算
def isLetter(c: str) -> bool:
    if c >= "a" and c <= "z":
        return True
    if c >= "A" and c <= "Z":
        return True
    return False

# 函数入口
def main() -> int:
    exp: Exp<str, str> = Exp<str, str>("abc", "a")
    l: List[str] = exp.getList()
    print("class getList: <", l[0], ", ", l[1], ">")
    
    fact_result: int = factorial(4)
    print("Factorial:", fact_result)

    c: str = "b"
    print(c, " isLetter = ", isLetter(c))

    return 0
```