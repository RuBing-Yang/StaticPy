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
g++ main.cpp lex.cpp grammar.cpp semantic.cpp gencode.cpp -o compiler
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

语法树：



## 词法

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



## 文法

TODO:
  - 语法结构
    - for循环：`for iter:type in container:`
  - 数据结构
    - ~~Dict声明写得有点问题~~(Done)
    - ~~LVal为List/Dict中元素赋值~~(Done)
    - ~~float数据类型~~
    - ~~List/Dict的嵌套~~
    - ~~list的append~~
- 代码生成：
  - ~~输出c++代码~~(Done)
- 泛型
  - ~~参考[pep-0484](https://peps.python.org/pep-0484/#user-defined-generic-types)~~
- 类
  - ~~self的使用~~
  - ~~方法重写~~
- 错误处理
  - ~~词法分析~~
  - 语法分析
    - ~~字符匹配~~
    - 重定义：
      - 同级变量重定义
      - 类重名，泛型重名，类和泛型重名
      - 函数名相同且参数表相同
    - 未定义：
      - 使用了未声明的变量
      - 数据类型名称未定义(泛型/类)
      - 函数未定义
    - 静态类型检查：
      变量赋值和函数返回值需要进行静态类型检查，规则如下：
      - 2是int，2.2是float，x:long=2是long
      - int和float/long加减乘除结果是float/long，不允许long+float
      - 泛型只能和同名泛型加减乘除
      - str只能+str或和str比较
      - 逻辑运算结果是bool
      - bool加减乘除int/float/long结果是int/float/long

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
    | 'print' '(' [(Str | Exp) {',' (Str | Exp)}] ')'
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

## 语义

符号表

| 类型    | 组织形式                   |
| ------- | ------------------------- |
| class   | 原名 -> List [UnaryClass] |
|         | 新名 -> UnaryClass        |
| func    | 原名 -> List [UnaryFunc]  |
| var     | 原名 -> Stack [UnaryVar]  |
| generic | 名称->id                  |
|         | List [UnaryDataType]      |

数据结构

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