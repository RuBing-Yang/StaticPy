#include "definition.h"

// 注意这里key用的是old_name
map<string, VarStack> varMap;
map<string, FuncList> funcMap;
map<string, ClassList> classMap;

// 注意这里key用的是new_name
map<string, UnaryClass> unaryClassMap;

UnaryClass *nowDefClass = nullptr;
UnaryFunc *nowDefFunc = nullptr;

// ClassDef或者FuncDef前面的泛型列表
map<string, int> generic_name2id;
// 实例化了新的泛型列表，返回去再扫一遍的时候用到
vector<UnaryDataType> generic_real_list;
map<string, UnaryDataType> generic_real_map;

ASTNode *astRoot = nullptr;

ASTNode* creatNode(ASTNode* aroot, string s, string type, string datatype = "")
{
	ASTNode *ap = new ASTNode(s, type, datatype);
	if (aroot->first_child == nullptr) {
		aroot->first_child = aroot->last_child = ap;
	}
	else {
		aroot->last_child->next = ap;
		aroot->last_child = ap;
	}
	return ap;
}

int var_cnt = 0;
int func_cnt = 0;
int class_cnt = 0;

string newSymbolName (string type)
{
    if (type == "var") {
        return "var" + to_string(var_cnt++);
    } else if (type == "func") {
        return "func" + to_string(func_cnt++);
    } else if (type == "class") {
        return "class" + to_string(class_cnt++);
    }
    else return "";
}

void addVar(map<string, VarStack> &varMap, string name, UnaryVar v, int line) {
    if (varMap.find(name) == varMap.end()) {
        varMap[name] = VarStack(v);
    } else {
        VarStack varStack = varMap[name];
        if (varStack.size() > 0 && v.level == varStack.top().level) {
            cerr << "[line " << line << "] SemanticError: Variable name " << name << " duplicate declaration!" << endl;
            exit(3);
        }
        varMap[name].push(v);
    }
}

void addFunc(map<string, FuncList> &targetMap, string name, UnaryFunc f, int line) {
    UnaryFunc f2;
    if (targetMap.find(name) == targetMap.end()) {
        targetMap[name] = FuncList(f);
    } else {
        if (targetMap[name].find(f)) {
            cerr << "[line " << line << "] SemanticError: Function name " << name << " duplicate declaration!" << endl;
            exit(3);
        }
        targetMap[name].push(f);
    }
}

void addClass(map<string, ClassList> &targetMap, string name, UnaryClass c, int line) {
    if (targetMap.find(name) == targetMap.end()) {
        targetMap[name] = ClassList(c);
    } else {
        UnaryClass c2;
        if (targetMap[name].find(c.generics, c2)) {
            cerr << "[line " << line << "] SemanticError: Class name " << name << " duplicate declaration!" << endl;
            exit(3);
        }
        targetMap[name].push(c);
    }
}

map<string, UnaryDataType> genericName2Type(map<string, int> _generic_name2id, vector<UnaryDataType> rgenerics, int line)
{
    map<string, UnaryDataType> res;
    for (auto it = _generic_name2id.begin(); it != _generic_name2id.end(); it++) {
        if (it->second >= rgenerics.size()) {
            cerr << "[line " << line << "] SemanticError: Generic id " << it->second << " exceed rgenerics size" << rgenerics.size() << "!" << endl;
            exit(3);
        }
        res[it->first] = rgenerics[it->second];
    }
    return res;
}

vector<UnaryDataType> readGenericReal(CSTNode* cp)
{
    // GenericReal ::= '<' DataType {',' DataType} '>'
    vector<UnaryDataType> temp_generics;
    cp = cp->next;
    while (cp != nullptr && cp->type == "DataType") {
        if (cp->first_child->type == "IDENFR") {
            cerr << "[line " << cp->line << "] SemanticError: Generic type cannot be identifier!" << endl;
            exit(3);
        } else if (cp->first_child->type == "NONETK") {
            cerr << "[line " << cp->line << "] SemanticError: Generic type cannot be None!" << endl;
            exit(3);
        }
        temp_generics.push_back(UnaryDataType("basic", cp->first_child->s));
        cp = cp->next;
        if (cp != nullptr && cp->type == "COMMA") cp = cp->next;
    }
    return temp_generics;
}

vector<NestDataType> readFuncRParams(CSTNode* cp, ASTNode* aroot, int level) {
    // FuncRParams ::= Exp { ',' Exp }
    vector<NestDataType> res;
    NestDataType expDataType = NestDataType();
    semanticAnalysis(cp, aroot, cp->type, expDataType, level); // Exp
    res.push_back(expDataType);
    cp = cp->next;
    while (cp != nullptr && cp->type == "COMMA") {
        cp = cp->next;
        semanticAnalysis(cp, aroot, cp->type, expDataType, level); // Exp
        res.push_back(expDataType);
        cp = cp->next;
    }
    return res;
}

void class_generic_real(CSTNode* cp, NestDataType& expDataType)
{
    // Ident '<' DataType {',' DataType} '>'
    string class_name = cp->s;
    if (classMap.find(class_name) == classMap.end()) {
        cerr << "[line " << cp->line << "] SemanticError: Undefined class identifier '" << cp->s << "'!" << endl;
        exit(3);
    }
    vector<UnaryDataType> temp_generics = readGenericReal(cp->next->first_child);
    UnaryClass c;
    if (classMap[class_name].find(temp_generics, c)) {
        expDataType.push(UnaryDataType("class", c.new_name));
    } else {
        if (!classMap[class_name].findTemp(temp_generics, c)) {
            cerr << "[line " << cp->line << "] SemanticError: Cannot find template class '" << cp->s << "' with generics " << temp_generics.size() << " size !" << endl;
            exit(3);
        }
        string _new_name = newSymbolName("class");
        UnaryClass _c = c.real(_new_name, temp_generics, cp->line);
        expDataType.push(UnaryDataType("class", _new_name));
        cout << "class_generic_real " << _c.to_string() << endl;

        // 保存现场
        map<string, VarStack> _varMap = map<string, VarStack>(varMap);
        varMap.clear();
        UnaryClass *_nowDefClass = nowDefClass;
        nowDefClass = &_c;
        UnaryFunc *_nowDefFunc = nowDefFunc;
        nowDefFunc = nullptr;
        map<string, int> _generic_name2id = map<string, int>(generic_name2id);
        generic_name2id.clear();
        vector<UnaryDataType> _generic_real_list = vector<UnaryDataType>(generic_real_list);
        generic_real_list = temp_generics;
        map<string, UnaryDataType> _generic_real_map = map<string, UnaryDataType>(generic_real_map);
        generic_real_map = genericName2Type(c.generic_name2id, temp_generics, cp->line);
        
        //开始重新过一遍类的定义
        NestDataType expDataType2;
        semanticAnalysis(c.cst_root, astRoot, "ClassDef", expDataType2, 0);
        
        // 恢复现场
        varMap = _varMap;
        nowDefClass = _nowDefClass;
        nowDefFunc = _nowDefFunc;
        generic_name2id = _generic_name2id;
        generic_real_list = _generic_real_list;
        generic_real_map = _generic_real_map;
    }
}

void func_generic_real(CSTNode* cp, NestDataType& expDataType, ASTNode* aroot, int level)
{
    // LVal[Ident] GenericReal['<' DataType {',' DataType} '>'] '(' [FuncRParams] ')'
    string func_name = cp->first_child->s;
    if (funcMap.find(func_name) == funcMap.end()) {
        cerr << "[line " << cp->line << "] SemanticError: Undefined func identifier '" << cp->s << "'!" << endl;
        exit(3);
    }
    vector<UnaryDataType> temp_generics;
    if (cp->next != nullptr && cp->next->type == "GenericReal")
        temp_generics = readGenericReal(cp->next->first_child);
    vector<NestDataType> temp_rparams;
    if (cp->next->next->next != nullptr && cp->next->next->next->type == "FuncRParams")
        temp_rparams = readFuncRParams(cp->next->next->next->first_child, aroot, level);
    UnaryFunc f;
    if (funcMap[func_name].find(temp_generics, temp_rparams, f)) {
        expDataType = f.ret;
    } else {
        if (!funcMap[func_name].findTemp(temp_generics, temp_rparams, f)) {
            cerr << "[line " << cp->line << "] SemanticError: Cannot find template function " << cp->s << " !" << endl;
            exit(3);
        }
        string _new_name = newSymbolName("func");
        UnaryFunc _f = f.real(_new_name, temp_generics, cp->line);
        expDataType = _f.ret;
        addFunc(funcMap, func_name, _f, cp->line);
        cout << "func_generic_real " << _f.to_string() << endl;

        // 保存现场
        map<string, VarStack> _varMap = map<string, VarStack>(varMap);
        varMap.clear();
        UnaryClass *_nowDefClass = nowDefClass;
        nowDefClass = nullptr;
        UnaryFunc *_nowDefFunc = nowDefFunc;
        nowDefFunc = &_f;
        map<string, int> _generic_name2id = map<string, int>(generic_name2id);
        generic_name2id.clear();
        vector<UnaryDataType> _generic_real_list = vector<UnaryDataType>(generic_real_list);
        generic_real_list = temp_generics;
        map<string, UnaryDataType> _generic_real_map = map<string, UnaryDataType>(generic_real_map);
        generic_real_map = genericName2Type(f.generic_name2id, temp_generics, cp->line);
        
        //开始重新过一遍函数的定义
        NestDataType expDataType2;
        semanticAnalysis(f.cst_root, astRoot, "FuncDef", expDataType2, 0);
        
        // 恢复现场
        varMap = _varMap;
        nowDefClass = _nowDefClass;
        nowDefFunc = _nowDefFunc;
        generic_name2id = _generic_name2id;
        generic_real_list = _generic_real_list;
        generic_real_map = _generic_real_map;
    }
}

void semanticAnalysis(CSTNode *croot, ASTNode *aroot, string type, NestDataType& expDataType, int level)
{
    if (croot == nullptr) return;
    ASTNode *ap;
    CSTNode* cp = croot->first_child;
    if (type == "CompUnit") {
        // { [GenericDefs] (ClassDef | FuncDef) }
        astRoot = aroot;
        while (cp != nullptr) {
            nowDefFunc = nullptr;
            nowDefClass = nullptr;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);
            if (cp->type != "GenericDefs") {
                generic_name2id.clear();
            }
            expDataType.clear();
            cp = cp->next;
        }
    }
    else if (type == "GenericDefs") {
        generic_name2id.clear();
        if (cp->type == "GenericDef") {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);
            cp = cp->next;
            while (cp != nullptr && cp->type == "GenericDef") {
                semanticAnalysis(cp, aroot, cp->type, expDataType, level);
                cp = cp->next;
            }
        }
    }
    else if (type == "GenericDef") {
        // Ident '=' 'TypeVar' '(' Str ')'
        if (generic_name2id.find(cp->s) != generic_name2id.end()) {
            cerr << "[line " << cp->line << "] SemanticError: Generic name '" << cp->s << "' has been defined!" << endl;
            exit(3);
        }
        if (classMap.find(cp->s) != classMap.end()) {
            cerr << "[line " << cp->line << "] SemanticError: Generic name '" << cp->s << "' has been defined as a class name!" << endl;
            exit(3);
        }
        generic_name2id[cp->s] = generic_name2id.size();
    }
	else if (type == "ClassDef") {
        // 'class' Ident ':' 'AddTab' {ClassAttrDef} [ClassInitDef] {ClassFuncDef} 'DelTab'
        cp = cp->next;
        string class_name = cp->s;
        string new_name;
        bool isExist = (nowDefClass != nullptr);
        if (isExist) {
            // 泛型模板类的实例化, UnaryClass已声明
            new_name = nowDefClass->new_name;
        } else {
            new_name = newSymbolName("class");
            nowDefClass = new UnaryClass(new_name, class_name, generic_name2id, generic_real_list);
            if (classMap.find(class_name) != classMap.end()) {
                cerr << "[line " << cp->line << "] SemanticError: Class name '" << class_name << "' has been defined!" << endl;
                exit(3);
            }
            if (generic_name2id.find(class_name) != generic_name2id.end()) {
                cerr << "[line " << cp->line << "] SemanticError: Class name '" << class_name << "' has been defined as a generic name!" << endl;
                exit(3);
            }
        }
        if (generic_name2id.size() == 0) {
            cp = cp->next->next->next;
            while (cp != nullptr && cp->type == "ClassAttrDef") {
                semanticAnalysis(cp, ap, cp->type, expDataType, level);
                cp = cp->next;
            }
            while (cp != nullptr && cp->type == "ClassInitDef") {
                semanticAnalysis(cp, ap, cp->type, expDataType, level);
                cp = cp->next;
            }
            while (cp != nullptr && cp->type == "ClassFuncDef") {
                semanticAnalysis(cp, ap, cp->type, expDataType, level);
                cp = cp->next;
            }
        } else {
            // 模板类，留一个入口等待泛型实例化
            nowDefClass->cst_root = croot;
        }
        if (!isExist) {
            addClass(classMap, class_name, *nowDefClass, cp->line);
        }
        unaryClassMap[new_name] = *nowDefClass;
        nowDefClass = nullptr;
	}
	else if (type == "ClassAttrDef") {
        // Ident ':' DataType
        string attr_name = cp->s;      
        if (nowDefClass->attrs.find(attr_name) != nowDefClass->attrs.end()) {
            cerr << "[line " << cp->line << "] SemanticError: Attribute name '" << attr_name << "' of class has been defined!" << endl;
            exit(3);
        }
        while (cp->type != "DataType") cp = cp->next;
        semanticAnalysis(cp, aroot, cp->type, expDataType, level);
        nowDefClass->attrs[attr_name] = expDataType;
        expDataType.clear();
	}
	else if (type == "ClassInitDef") {
		// 'def' 'init' '(' [FuncFParams] ')' Block
        string new_name = newSymbolName("func");
        nowDefFunc = new UnaryFunc(new_name, nowDefClass->old_name, generic_name2id, generic_real_list);
        nowDefFunc->ret = NestDataType(UnaryDataType("class", nowDefClass->new_name));
        cp = cp->next->next->next;
        if (cp->type == "FuncFParams") {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level + 1);
        }
        addFunc(nowDefClass->funcMap, "init", *nowDefFunc, cp->line);
        addFunc(funcMap, nowDefClass->old_name, *nowDefFunc, cp->line);
        while (cp->type != "Block") cp = cp->next;
		semanticAnalysis(cp, aroot, cp->type, expDataType, level);
        nowDefFunc = nullptr;
	}
	else if (type == "ClassFuncDef") {
        // 'def' Ident '(' 'self' [',' FuncFParams] ')' '->' FuncType Block
        cp = cp->next;
        string new_name = newSymbolName("func");
        string func_name = cp->s;
        nowDefFunc = new UnaryFunc(new_name, func_name, generic_name2id, generic_real_list);
        cp = cp->next->next->next->next;
        if (cp->type == "FuncFParams") {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level + 1);
        }
        while (cp->type != "FuncType") cp = cp->next;
        semanticAnalysis(cp, aroot, cp->type, expDataType, level);
        cout << "[ClassFuncDef] " << nowDefFunc->to_string() << endl;
        addFunc(nowDefClass->funcMap, func_name, *nowDefFunc, cp->line);
        while (cp->type != "Block") cp = cp->next;
		semanticAnalysis(cp, aroot, cp->type, expDataType, level);
        nowDefFunc = nullptr;
	}
    else if (type == "FuncDef") {
        // 'def' Ident '(' [FuncFParams] ')' '->' FuncType Block
        cp = cp->next;
        string new_name = newSymbolName("func");
        string func_name = cp->s;
        if (nowDefFunc != nullptr) {  // 泛型模板函数的实例化, UnaryFunc已声明
            for (int i = 0; i < nowDefFunc->fparams.size(); i++) {
                string param_name = nowDefFunc->params_name[i];
                NestDataType d = nowDefFunc->fparams[i];
                addVar(varMap, param_name, UnaryVar(level, param_name, d), cp->line);
            }
            while (cp->type != "Block") cp = cp->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);
        }
        else {
            nowDefFunc = new UnaryFunc(new_name, func_name, generic_name2id, generic_real_list);
            cp = cp->next->next;
            if (cp->type == "FuncFParams") {
                semanticAnalysis(cp, aroot, cp->type, expDataType, level + 1);
            }
            while (cp->type != "FuncType") cp = cp->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);
            for (const auto& it : funcMap) {
                cout << "funcMap[" << it.first << "] " << it.second.funcs.size() << endl;
            }
            if (generic_name2id.size() == 0) {
                while (cp->type != "Block") cp = cp->next;
                semanticAnalysis(cp, aroot, cp->type, expDataType, level);
            } else {
                nowDefFunc->cst_root = croot;
            }
            addFunc(funcMap, func_name, *nowDefFunc, cp->line);
        }
        // 泛型模板函数没进block，需要单独清理FuncFParams
        varMap.clear();
        nowDefFunc = nullptr;
    }
    else if (type == "FuncType") {
        if (cp->type == "NONETK") {
			nowDefFunc->ret = NestDataType(UnaryDataType("basic", "None"));
		} else {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);
            nowDefFunc->ret = expDataType;
        }
    }
    else if (type == "DataType") {
		if (cp->type == "IDENFR") {
            // Ident [GenericReal]
            expDataType.end = true;
            if (cp->next != nullptr && cp->next->type == "GenericReal") {
                class_generic_real(cp, expDataType);
            }
            else {
                if (classMap.find(cp->s) != classMap.end()) {
                    UnaryClass unaryClass =  classMap[cp->s].classes[0];
                    if (unaryClass.generic_name2id.size() > 0) {
                        cerr << "[line " << cp->line << "] SemanticError: Not found generics realization list for class identifier '" << cp->s << "'!" << endl;
                        exit(3);
                    }
                    expDataType.push(UnaryDataType("class", unaryClass.new_name));
                }
                else if (generic_real_map.find(cp->s) != generic_real_map.end()) {
                    // 类或函数的泛型实例化
                    expDataType.push(generic_real_map[cp->s]);
                }
                else if (generic_name2id.find(cp->s) != generic_name2id.end() ) {
                    // 函数的参数或者返回值类型
                    expDataType.push(UnaryDataType("generic", cp->s));
                }
                else {
                    cerr << "[line " << cp->line << "] SemanticError: Undefined class or generic identifier '" << cp->s << "'!" << endl;
                    exit(3);
                }
            }
		}
        else if (cp->type == "LISTTK") {
            // 'List' '[' DataType ']'
            expDataType.push(UnaryDataType("basic", "int"));
            cp = cp->next->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);  // DataType
        }
        else if (cp->type == "DICTTK") {
            // 'Dict' '[' DataType ',' DataType ']'
            cp = cp->next->next;
            NestDataType keyDataType;
            semanticAnalysis(cp, aroot, cp->type, keyDataType, level);  // DataType
            if (keyDataType.datatype_list.size() != 1 || keyDataType.datatype_list[0].type != "basic") {
                cerr << "[line " << cp->line << "] SemanticError: Dict key type must be basic type!" << endl;
                exit(3);
            }
            expDataType.push(keyDataType.datatype_list[0]);
            cp = cp->next->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);  // DataType
        }
        else {
            // 'int' | 'float' | 'long' | 'str' | 'bool'
            expDataType.end = true;
            expDataType.push(UnaryDataType("basic", cp->s));
        }
    }
    else if (type == "Block") {
        while (cp->type != "BlockItem") cp = cp->next;
        while (cp != nullptr && cp->type == "BlockItem") {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level + 1);
            cp = cp->next;
        }
        // Block内部作用域的变量出栈
        vector<string> delVarList;
        for (auto it = varMap.begin(); it != varMap.end(); it++) {
            it->second.delTab(level);
            if (it->second.size() == 0) {
                delVarList.push_back(it->first);
            }
        }
        for (string s : delVarList) {
            varMap.erase(s);
        }
    }
    else if (type == "BlockItem") {
        // Decl | Stmt
        expDataType.clear();
        semanticAnalysis(cp, aroot, cp->type, expDataType, level);
        expDataType.clear();
    }
    else if (type == "Decl") {
        // Decl ::= Ident ':' DataType ['=' InitVal]
        string var_name = cp->s;
        while (cp->type != "DataType") cp = cp->next;
        semanticAnalysis(cp, aroot, cp->type, expDataType, level);
        addVar(varMap, var_name, UnaryVar(level, var_name, expDataType), cp->line);
        cp = cp->next;
        if (cp != nullptr && cp->type == "ASSIGN") {
            NestDataType expDataType2;
            cp = cp->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // InitVal
            // 变量初始化静态类型检查
            expDataType.tryAssign(expDataType2, cp->line);
        }
    }
    else if (type == "InitVal") {
		if (cp->type == "LBRACK") {  // List
            // '[' [InitVal {',' InitVal}] ']' 
            cp = cp->next;
            if (!expDataType.end) expDataType.push(UnaryDataType("basic", "int"));
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);
            cp = cp->next;
            while (cp != nullptr && cp->type == "COMMA") {
                cp = cp->next;
                NestDataType expDataType2;
                semanticAnalysis(cp, aroot, cp->type, expDataType, level);  // InitVal
                cp = cp->next;
            }
		}
        else if (cp->type == "LBRACE") {  // Dict
            // '{' [Exp ':' InitVal {',' Exp ':' InitVal}] '}'
            cp = cp->next;
            NestDataType expDataType1;
            semanticAnalysis(cp, aroot, cp->type, expDataType1, level);  // key: Exp
            if (expDataType1.datatype_list.size() != 1 || expDataType1.datatype_list[0].type != "basic") {
                cerr << "[line " << cp->line << "] SemanticError: Dict key type must be basic type!" << endl;
                exit(3);
            }
            string basic_type = expDataType1.datatype_list[0].name;
            if (!expDataType.end) expDataType.push(UnaryDataType("basic", basic_type));
            cp = cp->next->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);  // value: InitVal
            cp = cp->next;
            while (cp->type == "COMMA") {
                cp = cp->next;
                NestDataType expDataType2;
                semanticAnalysis(cp, aroot, cp->type, expDataType2, level);  // key: Exp
                if (expDataType2.datatype_list.size() != 1 || expDataType2.datatype_list[0].type != "basic") {
                    cerr << "[line " << cp->line << "] SemanticError: Dict key type must be basic type!" << endl;
                    exit(3);
                }
                if (basic_type != expDataType2.datatype_list[0].name) {
                    cerr << "[line " << cp->line << "] SemanticError: Dict key type must be same!" << endl;
                    exit(3);
                }
                cp = cp->next->next;
                semanticAnalysis(cp, aroot, cp->type, expDataType, level);  // value: InitVal
            }
		}
		else {  // Exp
            NestDataType expDataType2;
            semanticAnalysis(cp, aroot, cp->type, expDataType2, level);
            // if (expDataType2.datatype_list.size() != 1 && expDataType2.datatype_list[0].type != "basic") {
            //     cerr << "[line " << cp->line << "] SemanticError: InitVal element must be basic type!" << endl;
            //     exit(3);
            // }
            // string basic_type = expDataType2.datatype_list[0].name;
            if (!expDataType.end) {
                for (UnaryDataType d : expDataType2.datatype_list) {
                    expDataType.push(d);
                }
                expDataType.end = true;
            }
            else {
                if (expDataType.datatype_list.back().name != expDataType2.datatype_list[0].name) {
                    cerr << "[line " << cp->line << "] SemanticError: InitVal element data type must be same!" << endl;
                    exit(3);
                }
            }
		}
    }
    else if (type == "FuncFParams") {
        semanticAnalysis(cp, aroot, cp->type, expDataType, level);
        cp = cp->next;
		while (cp != nullptr && cp->type == "COMMA") {
            cp = cp->next;
			semanticAnalysis(cp, aroot, cp->type, expDataType, level);
            cp = cp->next;
		}
    }
    else if (type == "FuncFParam") {
        // Ident ':' DataType
        string fparam_name = cp->s;
        nowDefFunc->params_name.push_back(fparam_name);
        cp = cp->next->next;  // DataType
        semanticAnalysis(cp, aroot, cp->type, expDataType, level);
        nowDefFunc->fparams.push_back(expDataType);
        addVar(varMap, fparam_name, UnaryVar(level, fparam_name, expDataType), cp->line);
        expDataType.clear();
    }
    else if (type == "Stmt") {
		if (cp->type == "LVal") {
            if (cp->next != nullptr && cp->next->type == "ASSIGN") {
                //  '=' Exp
                semanticAnalysis(cp, aroot, cp->type, expDataType, level); // LVal
                cout << endl;
                cp = cp->next->next;
                NestDataType expDataType2;
                semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // Exp
                // 赋值语句静态类型检查
                expDataType.tryAssign(expDataType2, cp->line);
            } else {
                // LVal '.' 'append' '(' Exp ')'
                semanticAnalysis(cp, aroot, cp->type, expDataType, level); // LVal
                if (expDataType.datatype_list[0].name != "int") {
                    cerr << "[line " << cp->line << "] SemanticError: Cannot append element to non-list type" << endl;
                    exit(3);
                }
                cp = cp->next->next->next->next;
                NestDataType expDataType2;
                semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // Exp
                if (!expDataType2.equals(expDataType.datatype_list, 1)) {
                    cerr << "[line " << cp->line << "] SemanticError: Cannot append element to different type" << endl;
                    exit(3);
                }
            }
		}
		else if (cp->type == "IFTK") {
            // 'if' Exp Block ['else' Block]
            cp = cp->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level); // Exp
            if (expDataType.datatype_list[0].name != "bool") {
                cerr << "[line " << cp->line << "] SemanticError: If condition" << expDataType.datatype_list[0].name << "is not bool type" << endl;
                exit(3);
            }
            cp = cp->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level); // Block
            cp = cp->next;
            if (cp != nullptr && cp->type == "ELSETK") {
                cp = cp->next;
                semanticAnalysis(cp, aroot, cp->type, expDataType, level); // Block
            }
		}
		else if (cp->type == "WHILETK") {
            // 'while' Exp Block
            cp = cp->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level); // Exp
            if (expDataType.datatype_list[0].name != "bool") {
                cerr << "[line " << cp->line << "] SemanticError: While condition" << expDataType.datatype_list[0].name << "is not bool type" << endl;
                exit(3);
            }
            cp = cp->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level); // Block
		}
		else if (cp->type == "BREAKTK") {
            // do nothing
		}
		else if (cp->type == "CONTINUETK") {
            // do nothing
		}
		else if (cp->type == "RETURNTK") {
            if (nowDefFunc == nullptr) {
                cerr << "[line " << cp->line << "] SemanticError: Return statement must be in a function!" << endl;
                exit(3);
            }
            cp = cp->next;
            if (cp != nullptr && cp->type == "Exp") {
                semanticAnalysis(cp, aroot, cp->type, expDataType, level); // Exp
            } else {
                // return; 未传递返回值
                expDataType = NestDataType(UnaryDataType("basic", "None"));
            }
            // 函数返回值类型检查
            nowDefFunc->ret.tryAssign(expDataType, cp->line);
		}
        else if (cp->type == "PRINTTK") {
            // do nothing
        }
		else {
            // Exp
			// do nothing
		}
    }
    else if (type == "Exp") {
        semanticAnalysis(cp, aroot, cp->type, expDataType, level); // LOrExp
	}
	else if (type == "AddExp") {
        semanticAnalysis(cp, aroot, cp->type, expDataType, level); // MulExp
        cp = cp->next;
        while (cp != nullptr && (cp->type == "PLUS" || cp->type == "MINU")) {
            string op = cp->s;
            cp = cp->next;
            NestDataType expDataType2;
            semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // MulExp
            expDataType = expDataType.twoOp(op, expDataType2, cp->line);
            cp = cp->next;
        }
	}
    else if (type == "MulExp") { 
        semanticAnalysis(cp, aroot, cp->type, expDataType, level); // UnaryExp
        cp = cp->next;
        while (cp != nullptr && (cp->type == "MULT" || cp->type == "DIV" || cp->type == "MOD")) {
            string op = cp->s;
            cp = cp->next;
            NestDataType expDataType2;
            semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // UnaryExp
            expDataType = expDataType.twoOp(op, expDataType2, cp->line);
            cp = cp->next;
        }
	}
    else if (type == "UnaryExp") {
        if (cp->type == "UnaryOp") {
            string op = cp->s;
            cp = cp->next;
            NestDataType expDataType2;
            semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // UnaryExp
            expDataType = expDataType.oneOp(op, cp->line);
        }
		else if (cp->type == "IdentExp") {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);
		}
		else {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);  // PrimaryExp
		}
	}
    else if (type == "IdentExp") {
        // LVal [[GenericReal] '(' [FuncRParams] ')']
        if (cp->next != nullptr && cp->next->type == "GenericReal") {
            // 只有全局函数才有泛型
            func_generic_real(cp, expDataType, aroot, level);
        }
        // else if (cp->next != nullptr && cp->next->type == "LPARENT") {
        //     string func_name = cp->first_child->s;
        //     bool isClassDef = false;
        //     if (cp->first_child->type == "SELFTK") {
        //         isClassDef = true;
        //         func_name = cp->first_child->next->next->s;
        //     }
        //     cp = cp->next->next;
        //     if (cp->type == "FuncRParams") {
        //         vector<NestDataType> temp_rparams = readFuncRParams(cp->next->next->next->first_child, aroot, level);
        //         UnaryFunc f;
        //         if (isClassDef && nowDefClass->funcMap[func_name].find(temp_rparams, f)) {
        //             expDataType = f.ret;
        //         }
        //         else if (!isClassDef && funcMap[func_name].find(temp_rparams, f)) {
        //             expDataType = f.ret;
        //         }
        //         else {
        //             cerr << "[line " << cp->line << "] SemanticError: Cannot find function " << func_name << " with params!" << endl;
        //             exit(3);
        //         }
        //     }
        // }
        else {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level);  // LVal
        }   
    }
    else if (type == "PrimaryExp") {
		if (cp->type == "LPARENT") {
            cp = cp->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level); // Exp
		} else {  // INTCON | FLOATCON | LONGCON | STRCON | 'True' | 'False'
            if (cp->type == "TRUETK" || cp->type == "FALSETK") expDataType = NestDataType(UnaryDataType("basic", "bool"));
            else if (cp->type == "INTCON") expDataType = NestDataType(UnaryDataType("basic", "int"));
            else if (cp->type == "FLOATCON") expDataType = NestDataType(UnaryDataType("basic", "float"));
            else if (cp->type == "LONGCON") expDataType = NestDataType(UnaryDataType("basic", "long"));
            else if (cp->type == "STRCON") expDataType = NestDataType(UnaryDataType("basic", "str"));
		}
	}
    else if (type == "LVal") {
        // ['self' '.' ] Ident {'[' Exp ']'} {'.' Ident {'[' Exp ']'}}
        bool isClassDef = false;
        if (cp->type == "SELFTK") {
            isClassDef = true;
            cp = cp->next->next;
            if (nowDefClass == nullptr) {
                cerr << "[line " << cp->line << "] SemanticError: 'self' can only be used in class definition!" << endl;
                exit(3);
            }
        }
        bool findVar = false;
        string var_name = cp->s;
        if (!isClassDef && varMap.find(var_name) != varMap.end()) {
            expDataType = varMap[var_name].top().nest_datatype;
        } else if (isClassDef && nowDefClass->attrs.find(var_name) != nowDefClass->attrs.end()) {
            expDataType = nowDefClass->attrs[var_name];
        } else if (cp->next == nullptr && croot->next != nullptr && croot->next->type == "LPARENT") {
            string func_name = var_name;
            vector<NestDataType> temp_rparams;
            if (croot->next->next!= nullptr && croot->next->next->type == "FuncRParams") {
                temp_rparams = readFuncRParams(croot->next->next->first_child, aroot, level);
            }
            UnaryFunc f;
            cout << "funcMap: {";
            int cnt = 0;
            for (const auto& it : funcMap) {
                cout << it.first + ":" + it.second.to_string();
                if (cnt++ < funcMap.size() - 1) cout << ", ";
            }
            cout << "}" << endl;
            if (isClassDef && nowDefClass->funcMap[func_name].find(temp_rparams, f)) {
                expDataType = f.ret;
            }
            else if (!isClassDef && funcMap[func_name].find(temp_rparams, f)) {
                expDataType = f.ret;
            }
            else {
                cerr << "[line " << cp->line << "] SemanticError: Cannot find function " << func_name << " with params (";
                for (int i = 0; i < temp_rparams.size(); i++) {
                    cerr << temp_rparams[i].to_string();
                    if (i != temp_rparams.size() - 1) cerr << ", ";
                }
                cerr << ") !" << endl;
                exit(3);
            }
        } else {
            cerr << "[line " << cp->line << "] SemanticError: Undefined variable identifier '" << var_name << "'!" << endl;
            exit(3);
        }
        cp = cp->next;
		while (cp != nullptr && cp->type == "LBRACK") {
            cp = cp->next;
            expDataType = expDataType.removeFront();
            NestDataType expDataType2;
            semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // Exp
            cp = cp->next->next;
		}
        while (cp != nullptr && cp->type == "DOT") {
            cout << "LVAL DOT with expDataType:" << expDataType.to_string() << endl;
            if (expDataType.datatype_list.size() != 1 || expDataType.datatype_list[0].type != "class") {
                cerr << "[line " << cp->line << "] SemanticError: Cannot find attribute of non-class type!" << endl;
                exit(3);
            }
            string class_name = expDataType.datatype_list[0].name;
            if (unaryClassMap.find(class_name) == unaryClassMap.end()) {
                cerr << "[line " << cp->line << "] SemanticError: Cannot find class with name " << class_name << "!" << endl;
                exit(3);
            }
            cp = cp->next;
            cout << "unaryClassMap[" << class_name << "]: " << unaryClassMap[class_name].to_string() << endl;
            string attr_name = cp->s;
            if (cp->next == nullptr && croot->next != nullptr && croot->next->type == "LPARENT") {
                string func_name = attr_name;
                vector<NestDataType> temp_rparams;
                if (croot->next->next!= nullptr && croot->next->next->type == "FuncRParams") {
                    temp_rparams = readFuncRParams(croot->next->next->first_child, aroot, level);
                }
                UnaryFunc f;
                if (unaryClassMap[class_name].funcMap[func_name].find(temp_rparams, f)) {
                    expDataType = f.ret;
                }
                else {
                    cerr << "[line " << cp->line << "] SemanticError: Cannot find function " << func_name << " of class " << unaryClassMap[class_name].old_name << " !" << endl;
                    exit(3);
                }
            }
            else if (unaryClassMap[class_name].attrs.find(attr_name) != unaryClassMap[attr_name].attrs.end()) {
                expDataType = unaryClassMap[class_name].attrs[attr_name];
            } 
            else {
                cerr << "[line " << cp->line << "] SemanticError: Undefined attribute identifier '" << var_name << "' of class " << unaryClassMap[class_name].old_name << " !" << endl;
                exit(3);
            }
            cp = cp->next;
            while (cp != nullptr && cp->type == "LBRACK") {
                cp = cp->next;
                expDataType = expDataType.removeFront();
                NestDataType expDataType2;
                semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // Exp
                cp = cp->next->next;
            }
        }
	}
	else if (type == "LOrExp") {
        semanticAnalysis(cp, aroot, cp->type, expDataType, level); // LAndExp
        cp = cp->next;
        while (cp != nullptr && cp->type == "ORTK") {
            cp = cp->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level); // LAndExp
            cp = cp->next;
        }
	}
	else if (type == "LAndExp") { 
        semanticAnalysis(cp, aroot, cp->type, expDataType, level); // EqExp
        cp = cp->next;
        while (cp != nullptr && cp->type == "ANDTK") {
            string op = cp->s;
            cp = cp->next;
            NestDataType expDataType2;
            semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // EqExp
            expDataType = expDataType.twoOp(op, expDataType2, cp->line);
            cp = cp->next;
        }
	}
	else if (type == "EqExp") { 
        semanticAnalysis(cp, aroot, cp->type, expDataType, level); // RelExp
        cp = cp->next;
        while (cp != nullptr && (cp->type == "EQL" || cp->type == "NEQ")) {
            string op = cp->s;
            cp = cp->next;
            NestDataType expDataType2;
            semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // RelExp
            expDataType = expDataType.twoOp(op, expDataType2, cp->line);
            cp = cp->next;
        }
	}
	else if (type == "RelExp") { 
        semanticAnalysis(cp, aroot, cp->type, expDataType, level); // AddExp
        cp = cp->next;
        while (cp != nullptr && (cp->type == "GRE" || cp->type == "LSS" || cp->type == "GEQ" || cp->type == "LEQ")) {      
            string op = cp->s;
            cp = cp->next;
            NestDataType expDataType2;
            semanticAnalysis(cp, aroot, cp->type, expDataType2, level); // AddExp
            expDataType = expDataType.twoOp(op, expDataType2, cp->line);
            cp = cp->next;
        }
	}
    cout << "[" << type << "] return expDataType=" << expDataType.to_string() << endl;
}