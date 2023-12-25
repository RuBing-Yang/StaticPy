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

ASTNode* creatNode(ASTNode* aroot, string s, string type)
{
    if (aroot == nullptr) {
        cerr << "SemanticError: creatNode(s=" << s << ", type=" << type << ")with null root node!" << endl;
        exit(3);
    }
    ASTNode *ap = new ASTNode(s, type);
	if (aroot->first_child == nullptr) {
		aroot->first_child = aroot->last_child = ap;
	}
	else {
		aroot->last_child->next = ap;
		aroot->last_child = ap;
	}
    return ap;
}

ASTNode* creatNode(ASTNode* aroot, string s, string type, NestDataType datatype)
{
    if (aroot == nullptr) {
        cerr << "SemanticError: creatNode(s=" << s << ", type=" << type << ")with null root node!" << endl;
        exit(3);
    }
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

string newSymbolName(string type)
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

vector<NestDataType> readFuncRParams(CSTNode* cp, ASTNode* aroot, int level, int DEBUG=0) {
    // FuncRParams ::= Exp { ',' Exp }
    vector<NestDataType> res;
    NestDataType expDataType = NestDataType();
    ASTNode* ap = creatNode(aroot, "", "FuncRParam", expDataType);
    semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // Exp
    res.push_back(expDataType);
    cp = cp->next;
    while (cp != nullptr && cp->type == "COMMA") {
        cp = cp->next;
        ap = creatNode(aroot, "", "FuncRParam", expDataType);
        semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // Exp
        res.push_back(expDataType);
        cp = cp->next;
    }
    return res;
}

string class_generic_real(CSTNode* cp, string class_name, vector<UnaryDataType> temp_generics, int DEBUG=0)
{
    if (DEBUG) {
        cout << "class_generic_real class_name=" << class_name << ", temp_generics=[";
        for (int i = 0; i < temp_generics.size(); i++) {
            cout << temp_generics[i].name;
            if (i < temp_generics.size() - 1) cout << ", " << endl;
        }
        cout << "]" << endl;
    }
    if (classMap.find(class_name) == classMap.end()) {
        cerr << "[line " << cp->line << "] SemanticError: Undefined class identifier '" << cp->s << "'!" << endl;
        exit(3);
    }
    UnaryClass c;
    if (classMap[class_name].find(temp_generics, c)) {
        // expDataType.push(UnaryDataType("class", c.new_name));
        return c.new_name;
    } else {
        if (!classMap[class_name].findTemp(temp_generics, c)) {
            cerr << "[line " << cp->line << "] SemanticError: Cannot find template class '" << cp->s << "' with generics " << temp_generics.size() << " size !" << endl;
            exit(3);
        }
        string _new_name = newSymbolName("class");
        UnaryClass _c = c.real(_new_name, temp_generics, cp->line);
        // expDataType.push(UnaryDataType("class", _new_name));

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
        semanticAnalysis(c.cst_root, astRoot, "ClassDef", expDataType2, 0, DEBUG);
        
        // 恢复现场
        varMap = _varMap;
        nowDefClass = _nowDefClass;
        nowDefFunc = _nowDefFunc;
        generic_name2id = _generic_name2id;
        generic_real_list = _generic_real_list;
        generic_real_map = _generic_real_map;
        return _new_name;
    }
}

string class_generic_real(CSTNode* cp, int DEBUG=0)
{
    // Ident '<' DataType {',' DataType} '>'
    string class_name = cp->s;
    vector<UnaryDataType> temp_generics = readGenericReal(cp->next->first_child);
    return class_generic_real(cp, class_name, temp_generics, DEBUG);
}

void func_generic_real(CSTNode* cp, ASTNode* aroot, int level, int DEBUG=0)
{
    // LVal[Ident] GenericReal['<' DataType {',' DataType} '>'] '(' [FuncRParams] ')'
    string func_name = cp->first_child->s;
    // ASTNode* ap = creatNode(aroot, func_name, "FuncCall");
    if (funcMap.find(func_name) == funcMap.end()) {
        cerr << "[line " << cp->line << "] SemanticError: Undefined func identifier '" << cp->s << "'!" << endl;
        exit(3);
    }
    vector<UnaryDataType> temp_generics;
    if (cp->next != nullptr && cp->next->type == "GenericReal")
        temp_generics = readGenericReal(cp->next->first_child);
    vector<NestDataType> temp_rparams;
    if (cp->next->next->next != nullptr && cp->next->next->next->type == "FuncRParams")
        temp_rparams = readFuncRParams(cp->next->next->next->first_child, aroot, level, DEBUG);
    
    if (DEBUG) {
        cout << "func_generic_real func_name=" << func_name << ", temp_generics=[";
        for (int i = 0; i < temp_generics.size(); i++) {
            cout << temp_generics[i].name;
            if (i < temp_generics.size() - 1) cout << ", " << endl;
        }
        cout << "], temp_rparams=[";
        for (int i = 0; i < temp_rparams.size(); i++) {
            cout << temp_rparams[i].to_string();
            if (i < temp_rparams.size() - 1) cout << ", " << endl;
        }
        cout << "]" << endl;
    }

    UnaryFunc f;
    if (funcMap[func_name].find(temp_generics, temp_rparams, f)) {
        // expDataType = f.ret;
        aroot->s = f.new_name;
        aroot->datatype = f.ret;
    } else {
        if (!funcMap[func_name].findTemp(temp_generics, temp_rparams, f)) {
            cerr << "[line " << cp->line << "] SemanticError: Cannot find template function " << cp->s << " !" << endl;
            exit(3);
        }
        string _new_name = newSymbolName("func");
        UnaryFunc _f = f.real(_new_name, temp_generics, cp->line);
        aroot->s = _new_name;
        aroot->datatype = _f.ret;

        // expDataType = _f.ret;
        addFunc(funcMap, func_name, _f, cp->line);
        if (DEBUG) cout << "func_generic_real create" << _f.to_string() << endl;

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
        if (f.cst_root == nullptr) {
            cerr << "[line " << cp->line << "] SemanticError: func_generic_real template function cst_root is null!" << endl;
            exit(3);
        }
        NestDataType expDataType2;
        semanticAnalysis(f.cst_root, astRoot, "FuncDef", expDataType2, 0, DEBUG);
        
        // 恢复现场
        varMap = _varMap;
        nowDefClass = _nowDefClass;
        nowDefFunc = _nowDefFunc;
        generic_name2id = _generic_name2id;
        generic_real_list = _generic_real_list;
        generic_real_map = _generic_real_map;
    }
}

void semanticAnalysis(CSTNode *croot, ASTNode *aroot, string type, NestDataType& expDataType, int level, int DEBUG)
{
    if (croot == nullptr) return;
    ASTNode *ap = nullptr;
    CSTNode* cp = croot->first_child;
    if (DEBUG) cout << "[" << type << "]" << "start " << cp->s << endl;
    if (type == "CompUnit") {
        // { [GenericDefs] (ClassDef | FuncDef) }
        astRoot = aroot;
        while (cp != nullptr) {
            nowDefFunc = nullptr;
            nowDefClass = nullptr;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);
            if (cp->type != "GenericDefs") {
                generic_name2id.clear();
            }
            expDataType.clear();
            cp = cp->next;
        }
        cout << "Semantic Analysis & Type Check OK!" << endl;
    }
    else if (type == "GenericDefs") {
        generic_name2id.clear();
        if (cp->type == "GenericDef") {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);
            cp = cp->next;
            while (cp != nullptr && cp->type == "GenericDef") {
                semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);
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
            ap = new ASTNode(new_name, "ClassDef", NestDataType(UnaryDataType("class", new_name)));
            ap->next = nowDefClass->ast_root->next;
            nowDefClass->ast_root->next = ap;
        } else {
            new_name = newSymbolName("class");
            if (generic_name2id.empty())
                ap = creatNode(aroot, new_name, "ClassDef", NestDataType(UnaryDataType("class", new_name)));
            else
                ap = creatNode(aroot, new_name, "template");
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
        if (generic_name2id.size() == 0) {;
            cp = cp->next->next->next;
            while (cp != nullptr && cp->type == "ClassAttrDef") {
                semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
                cp = cp->next;
            }
            while (cp != nullptr && cp->type == "ClassInitDef") {
                semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
                cp = cp->next;
            }
            while (cp != nullptr && cp->type == "ClassFuncDef") {
                semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
                cp = cp->next;
            }
        } else {
            // 模板类，留一个入口等待泛型实例化
            nowDefClass->cst_root = croot;
            nowDefClass->ast_root = ap;
        }
        addClass(classMap, class_name, *nowDefClass, cp->line);
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
        semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);
        string new_name = newSymbolName("var");
        nowDefClass->attrs[attr_name] = UnaryVar(level, attr_name, new_name, expDataType);
        ap = creatNode(aroot, new_name, "ClassAttrDef", expDataType);
        expDataType.clear();
	}
	else if (type == "ClassInitDef") {
		// 'def' 'init' '(' [FuncFParams] ')' Block
        string new_name = newSymbolName("func");
        nowDefFunc = new UnaryFunc(new_name, nowDefClass->new_name, generic_name2id, generic_real_list);
        nowDefFunc->ret = NestDataType(UnaryDataType("class", nowDefClass->new_name));
        ap = creatNode(aroot, new_name, "ClassInitDef", nowDefFunc->ret);
        cp = cp->next->next->next;
        if (cp->type == "FuncFParams") {
            semanticAnalysis(cp, ap, cp->type, expDataType, level + 1, DEBUG);
        }
        addFunc(nowDefClass->funcMap, "init", *nowDefFunc, cp->line);
        addFunc(funcMap, nowDefClass->old_name, *nowDefFunc, cp->line);
        while (cp->type != "Block") cp = cp->next;
		semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
        nowDefFunc = nullptr;
	}
	else if (type == "ClassFuncDef") {
        // 'def' Ident '(' 'self' [',' FuncFParams] ')' '->' FuncType Block
        cp = cp->next;
        ap = aroot;
        string new_name = newSymbolName("func");
        string func_name = cp->s;
        nowDefFunc = new UnaryFunc(new_name, func_name, generic_name2id, generic_real_list);        
        ap = creatNode(aroot, new_name, "ClassFuncDef");

        cp = cp->next->next->next->next;
        if (cp->type == "FuncFParams") {
            semanticAnalysis(cp, ap, cp->type, expDataType, level + 1, DEBUG);
        }

        while (cp->type != "FuncType") cp = cp->next;
        expDataType.clear();
        semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
        ap->datatype = expDataType;
        addFunc(nowDefClass->funcMap, func_name, *nowDefFunc, cp->line);

        while (cp->type != "Block") cp = cp->next;
		semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
        nowDefFunc = nullptr;
	}
    else if (type == "FuncDef") {
        // 'def' Ident '(' [FuncFParams] ')' '->' FuncType Block
        cp = cp->next;
        ap = aroot;
        string func_name = cp->s;
        string new_name = func_name;
        if (func_name != "main") new_name = newSymbolName("func");
        bool isTemplate = false;
        if (generic_name2id.size() != 0)  // 是模板函数
            isTemplate = true;
        if (nowDefFunc != nullptr) {  // 泛型模板函数的实现, UnaryFunc已声明
            ap = new ASTNode(nowDefFunc->new_name, "FuncDef", nowDefFunc->ret);
            ap->next = nowDefFunc->ast_root->next;
            nowDefFunc->ast_root->next = ap;
            // ap = creatNode(aroot, nowDefFunc->new_name, "FuncDef", nowDefFunc->ret);
            for (int i = 0; i < nowDefFunc->fparams.size(); i++) {
                string param_name = nowDefFunc->params_name[i];
                NestDataType d = nowDefFunc->fparams[i];
                string new_name = newSymbolName("var");
                addVar(varMap, param_name, UnaryVar(level, param_name, new_name, d), cp->line);
                creatNode(ap, new_name, "FuncFParam", d);
            }
            addFunc(funcMap, nowDefFunc->new_name, *nowDefFunc, cp->line);
            while (cp->type != "Block") cp = cp->next;
            semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
        }
        else {
            nowDefFunc = new UnaryFunc(new_name, func_name, generic_name2id, generic_real_list);
            if (isTemplate) {
                nowDefFunc->cst_root = croot;
                nowDefFunc->ast_root = creatNode(aroot, new_name, "template");
            }
            if (!isTemplate) ap = creatNode(aroot, new_name, "FuncDef");
            cp = cp->next->next;
            if (cp->type == "FuncFParams") {
                semanticAnalysis(cp, ap, cp->type, expDataType, level + 1, DEBUG);
            }
            expDataType.clear();
            while (cp->type != "FuncType") cp = cp->next;
            semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
            addFunc(funcMap, func_name, *nowDefFunc, cp->line);
            if (!isTemplate) {
                ap->datatype = expDataType;
                while (cp->type != "Block") cp = cp->next;
                semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
            }
        }
        // 泛型模板函数没进block，需要单独清理FuncFParams
        varMap.clear();
        nowDefFunc = nullptr;
    }
    else if (type == "FuncType") {
        if (cp->type == "NONETK") {
			nowDefFunc->ret = expDataType = NestDataType(UnaryDataType("basic", "None"));
		} else {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);
            nowDefFunc->ret = expDataType;
        }
    }
    else if (type == "DataType") {
		if (cp->type == "IDENFR") {
            // Ident [GenericReal]
            expDataType.end = true;
            if (cp->next != nullptr && cp->next->type == "GenericReal") {
                string _new_name = class_generic_real(cp, DEBUG);
                expDataType.push(UnaryDataType("class", _new_name));
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
            expDataType.push(UnaryDataType("basic", "List"));
            cp = cp->next->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);  // DataType
        }
        else if (cp->type == "DICTTK") {
            // 'Dict' '[' DataType ',' DataType ']'
            cp = cp->next->next;
            NestDataType keyDataType;
            semanticAnalysis(cp, aroot, cp->type, keyDataType, level, DEBUG);  // DataType
            if (keyDataType.datatype_list.size() != 1 || keyDataType.datatype_list[0].type != "basic") {
                cerr << "[line " << cp->line << "] SemanticError: Dict key type must be basic type!" << endl;
                exit(3);
            }
            expDataType.push(keyDataType.datatype_list[0]);
            cp = cp->next->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);  // DataType
        }
        else {
            // 'int' | 'float' | 'long' | 'str' | 'bool'
            expDataType.end = true;
            expDataType.push(UnaryDataType("basic", cp->s));
        }
    }
    else if (type == "Block") {
        ap = creatNode(aroot, "", "Block");
        while (cp->type != "BlockItem") cp = cp->next;
        while (cp != nullptr && cp->type == "BlockItem") {
            semanticAnalysis(cp, ap, cp->type, expDataType, level + 1, DEBUG);
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
        semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);
        expDataType.clear();
    }
    else if (type == "Decl") {
        // Decl ::= Ident ':' DataType ['=' InitVal]
        bool isAssign = false;
        while (cp != nullptr) {
            if (cp->type == "ASSIGN") {
                isAssign = true;
                cp = croot->first_child;
                ap = creatNode(aroot, "=", "Decl");
                break;
            }
            cp = cp->next;
        }
        cp = croot->first_child;
        string var_name = cp->s;
        while (cp->type != "DataType") cp = cp->next;
        semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
        string new_name = newSymbolName("var");
        addVar(varMap, var_name, UnaryVar(level, var_name, new_name, expDataType), cp->line);
        if (isAssign)
            creatNode(ap, new_name, "var", expDataType);
        else
            creatNode(aroot, new_name, "Decl", expDataType);
        if (isAssign) {
            NestDataType expDataType2;
            while (cp->type != "InitVal") cp = cp->next;
            semanticAnalysis(cp, ap, cp->type, expDataType2, level, DEBUG); // InitVal
            // 变量初始化静态类型检查
            expDataType.tryAssign(expDataType2, cp->line);
        }
    }
    else if (type == "InitVal") {
		if (cp->type == "LBRACK") {  // List
            // '[' [InitVal {',' InitVal}] ']' 
            cp = cp->next;
            ap = creatNode(aroot, "", "ListInitVal");
            if (!expDataType.end) expDataType.push(UnaryDataType("basic", "List"));
            semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
            cp = cp->next;
            while (cp != nullptr && cp->type == "COMMA") {
                cp = cp->next;
                NestDataType expDataType2;
                semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);  // InitVal
                cp = cp->next;
            }
		}
        else if (cp->type == "LBRACE") {  // Dict
            // '{' [Exp ':' InitVal {',' Exp ':' InitVal}] '}'
            cp = cp->next;
            ap = creatNode(aroot, "", "DictInitVal");
            ASTNode *ap1 = creatNode(ap, "", "DictElement");
            NestDataType expDataType1;
            semanticAnalysis(cp, ap1, cp->type, expDataType1, level, DEBUG);  // key: Exp
            if (expDataType1.datatype_list.size() != 1 || expDataType1.datatype_list[0].type != "basic") {
                cerr << "[line " << cp->line << "] SemanticError: Dict key type must be basic type!" << endl;
                exit(3);
            }
            string basic_type = expDataType1.datatype_list[0].name;
            if (!expDataType.end) expDataType.push(UnaryDataType("basic", basic_type));
            cp = cp->next->next;
            semanticAnalysis(cp, ap1, cp->type, expDataType, level, DEBUG);  // value: InitVal
            cp = cp->next;
            while (cp->type == "COMMA") {
                cp = cp->next;
                ASTNode *ap1 = creatNode(ap, "", "DictElement");
                NestDataType expDataType2;
                semanticAnalysis(cp, ap1, cp->type, expDataType2, level, DEBUG);  // key: Exp
                if (expDataType2.datatype_list.size() != 1 || expDataType2.datatype_list[0].type != "basic") {
                    cerr << "[line " << cp->line << "] SemanticError: Dict key type must be basic type!" << endl;
                    exit(3);
                }
                if (basic_type != expDataType2.datatype_list[0].name) {
                    cerr << "[line " << cp->line << "] SemanticError: Dict key type must be same!" << endl;
                    exit(3);
                }
                cp = cp->next->next;
                semanticAnalysis(cp, ap1, cp->type, expDataType, level, DEBUG);  // value: InitVal
            }
		}
		else {  // Exp
            NestDataType expDataType2;
            semanticAnalysis(cp, aroot, cp->type, expDataType2, level, DEBUG);
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
        semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);
        cp = cp->next;
		while (cp != nullptr && cp->type == "COMMA") {
            cp = cp->next;
			semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);
            cp = cp->next;
		}
    }
    else if (type == "FuncFParam") {
        // Ident ':' DataType
        string fparam_name = cp->s;
        nowDefFunc->params_name.push_back(fparam_name);
        cp = cp->next->next;  // DataType
        semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);
        nowDefFunc->fparams.push_back(expDataType);
        string new_name = newSymbolName("var");
        addVar(varMap, fparam_name, UnaryVar(level, fparam_name, new_name, expDataType), cp->line);
        if (aroot != nullptr && generic_name2id.size() == 0) creatNode(aroot, new_name, "FuncFParam", expDataType);
        expDataType.clear();
    }
    else if (type == "Stmt") {
		if (cp->type == "LVal") {
            if (cp->next != nullptr && cp->next->type == "ASSIGN") {
                //  '=' Exp
                ap = creatNode(aroot, "=", "Stmt");
                semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // LVal
                cp = cp->next->next;
                NestDataType expDataType2;
                semanticAnalysis(cp, ap, cp->type, expDataType2, level, DEBUG); // Exp
                // 赋值语句静态类型检查
                expDataType.tryAssign(expDataType2, cp->line);
            } else {
                // LVal '.' 'append' '(' Exp ')'
                ap = creatNode(aroot, "append", "Stmt");
                semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // LVal
                if (expDataType.size() != 2 || expDataType.datatype_list[0].name != "List") {
                    cerr << "[line " << cp->line << "] SemanticError: Cannot append element to non-list type" << endl;
                    exit(3);
                }
                cp = cp->next->next->next->next;
                NestDataType expDataType2;
                semanticAnalysis(cp, ap, cp->type, expDataType2, level, DEBUG); // Exp
                if (!expDataType2.equals(expDataType.datatype_list, 1)) {
                    cerr << "[line " << cp->line << "] SemanticError: Cannot append element to different type" << endl;
                    exit(3);
                }
            }
		}
		else if (cp->type == "IFTK") {
            // 'if' Exp Block ['else' Block]
            cp = cp->next;
            ap = creatNode(aroot, "if", "Stmt");
            semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // Exp
            if (expDataType.datatype_list[0].name != "bool") {
                cerr << "[line " << cp->line << "] SemanticError: If condition" << expDataType.datatype_list[0].name << "is not bool type" << endl;
                exit(3);
            }
            cp = cp->next;
            semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // Block
            cp = cp->next;
            if (cp != nullptr && cp->type == "ELSETK") {
                cp = cp->next;
                semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // Block
            }
		}
		else if (cp->type == "WHILETK") {
            // 'while' Exp Block
            cp = cp->next;
            ap = creatNode(aroot, "while", "Stmt");
            semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // Exp
            if (expDataType.datatype_list[0].name != "bool") {
                cerr << "[line " << cp->line << "] SemanticError: While condition" << expDataType.datatype_list[0].name << "is not bool type" << endl;
                exit(3);
            }
            cp = cp->next;
            semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // Block
		}
		else if (cp->type == "BREAKTK") {
            creatNode(aroot, "break", "Stmt");
		}
		else if (cp->type == "CONTINUETK") {
            creatNode(aroot, "continue", "Stmt");
		}
		else if (cp->type == "RETURNTK") {
            ap = creatNode(aroot, "return", "Stmt");
            if (nowDefFunc == nullptr) {
                cerr << "[line " << cp->line << "] SemanticError: Return statement must be in a function!" << endl;
                exit(3);
            }
            cp = cp->next;
            if (cp != nullptr && cp->type == "Exp") {
                semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // Exp
            } else {
                // return; 未传递返回值
                expDataType = NestDataType(UnaryDataType("basic", "None"));
            }
            // 函数返回值类型检查
            nowDefFunc->ret.tryAssign(expDataType, cp->line);
		}
        else if (cp->type == "PRINTTK") {
            // 'print' '(' [Exp {',' Exp}] ')'
            ap = creatNode(aroot, "print", "Stmt");
            cp = cp->next->next;
            while (cp != nullptr && cp->type == "Exp") {
                semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // Exp
                cp = cp->next->next;
            }
        }
		else {
            // Exp
            ap = creatNode(aroot, "", "Stmt");
            semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG);
		}
    }
    else if (type == "Exp") {
        // ap = creatNode(aroot, "", "Exp");
        semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG); // LOrExp
	}
	else if (type == "AddExp" || type == "MulExp" || type == "LOrExp" || type == "LAndExp" || type == "RelExp" || type == "EqExp") {
        ap = aroot;
        bool isTwoOp = false;
        if (cp->next != nullptr) {
            isTwoOp = true;
            ap = new ASTNode(cp->next->s, "TwoOp");
        }
        semanticAnalysis(cp, ap, cp->type, expDataType, level, DEBUG); // TwoOpExp
        cp = cp->next;
        while (cp != nullptr) {      
            string op = cp->s;
            cp = cp->next;
            NestDataType expDataType2;
            semanticAnalysis(cp, ap, cp->type, expDataType2, level, DEBUG); // TwoOpExp
            // 运算数据类型检查
            expDataType = expDataType.twoOp(op, expDataType2, cp->line);
            cp = cp->next;
            ap->datatype = expDataType;
            if (cp != nullptr) {
                ASTNode* ap1 = new ASTNode(cp->s, "TwoOp");
                ap1->first_child = ap1->last_child = ap;
                ap = ap1;
            }
        }
        if (isTwoOp) {
            if (aroot->first_child == nullptr) aroot->first_child = aroot->last_child = ap;
            else {
                aroot->last_child->next = ap;
                aroot->last_child = ap;
            }
        }
	}
    else if (type == "UnaryExp") {
        if (cp->type == "UnaryOp") {
            string op = cp->s;
            ap = creatNode(aroot, op, "OneOp");
            cp = cp->next;
            NestDataType expDataType2;
            semanticAnalysis(cp, ap, cp->type, expDataType2, level, DEBUG); // UnaryExp
            // 运算数据类型检查
            expDataType = expDataType.oneOp(op, cp->line);
            ap->datatype = expDataType;
        }
		else if (cp->type == "IdentExp") {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);
		}
		else {
            semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);  // PrimaryExp
		}
	}
    else if (type == "IdentExp") {
        // LVal [[GenericReal] '(' [FuncRParams] ')']
        if (cp->next != nullptr && cp->next->type == "GenericReal") {
            // 有泛型的全局函数或者类的init函数
            if (cp->first_child->type != "IDENFR") { 
                cerr << "[line " << cp->line << "] SemanticError: Only global function or class can have GenericReal!" << endl;
            }
            string func_name = cp->first_child->s;
            ASTNode* ap0 = creatNode(aroot, "", "LVal");
            ap = creatNode(ap0, func_name, "FuncCall");
            if (classMap.find(func_name) != classMap.end()) {
                vector<UnaryDataType> temp_generics = readGenericReal(cp->next->first_child);
                string class_new_name = class_generic_real(cp, func_name, temp_generics, DEBUG);
                ap->s = class_new_name;
                cp = cp->next->next->next;
                vector<NestDataType> temp_rparams;
                if (cp != nullptr && cp->type == "FuncRParams") {
                    temp_rparams = readFuncRParams(cp->first_child, ap, level, DEBUG);
                }
                // UnaryFunc f;
                // if (!funcMap[class_new_name].find(temp_rparams, f)) {
                //     cerr << "[line " << cp->line << "] SemanticError: Cannot find Class " << unaryClassMap[class_new_name].old_name << " init function with params (";
                //     for (int i = 0; i < temp_rparams.size(); i++) {
                //         cerr << temp_rparams[i].to_string();
                //         if (i != temp_rparams.size() - 1) cerr << ", ";
                //     }
                //     cerr << ") after generic realization!" << endl;
                // }
                expDataType = NestDataType(UnaryDataType("class", class_new_name));
            }
            else {
                func_generic_real(cp, ap, level, DEBUG);
                expDataType = ap->datatype;
            }
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
            semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG);  // LVal
        }   
    }
    else if (type == "PrimaryExp") {
		if (cp->type == "LPARENT") {
            cp = cp->next;
            semanticAnalysis(cp, aroot, cp->type, expDataType, level, DEBUG); // Exp
		} else {  // INTCON | FLOATCON | LONGCON | STRCON | 'True' | 'False'
            if (cp->type == "TRUETK" || cp->type == "FALSETK") expDataType = NestDataType(UnaryDataType("basic", "bool"));
            else if (cp->type == "INTCON") expDataType = NestDataType(UnaryDataType("basic", "int"));
            else if (cp->type == "FLOATCON") expDataType = NestDataType(UnaryDataType("basic", "float"));
            else if (cp->type == "LONGCON") expDataType = NestDataType(UnaryDataType("basic", "long"));
            else if (cp->type == "STRCON") expDataType = NestDataType(UnaryDataType("basic", "str"));
            else {
                cerr << "[line " << cp->line << "] SemanticError: Unknown primary expression!" << endl;
                exit(3);
            }
            creatNode(aroot, cp->s, "const", expDataType);
		}
	}
    else if (type == "LVal") {
        // ['self' '.' ] Ident {'[' Exp ']'} {'.' Ident {'[' Exp ']'}}
        ap = creatNode(aroot, "", "LVal");
        bool isClassDef = false;
        if (cp->type == "SELFTK") {
            isClassDef = true;
            cp = cp->next->next;
            creatNode(ap, "self", "self");
            if (DEBUG) cout << "nowDefClass for self: " << nowDefClass->to_string() << endl;
            if (nowDefClass == nullptr) {
                cerr << "[line " << cp->line << "] SemanticError: 'self' can only be used in class definition!" << endl;
                exit(3);
            }
        }
        bool findVar = false;
        string var_name = cp->s;
        if (!isClassDef && varMap.find(var_name) != varMap.end()) {
            expDataType = varMap[var_name].top().nest_datatype;
            creatNode(ap, varMap[var_name].top().new_name, "var", expDataType);
        } else if (isClassDef && nowDefClass->attrs.find(var_name) != nowDefClass->attrs.end()) {
            expDataType = nowDefClass->attrs[var_name].nest_datatype;
            creatNode(ap, nowDefClass->attrs[var_name].new_name, "attr", expDataType);
        } else if (cp->next == nullptr && croot->next != nullptr && croot->next->type == "LPARENT") {
            string func_name = var_name;
            vector<NestDataType> temp_rparams;
            ASTNode* ap1 = creatNode(ap, func_name, "FuncCall");
            if (croot->next->next!= nullptr && croot->next->next->type == "FuncRParams") {
                temp_rparams = readFuncRParams(croot->next->next->first_child, ap1, level, DEBUG);
            }
            UnaryFunc f;
            if (isClassDef && nowDefClass->funcMap[func_name].find(temp_rparams, f)) {
                ap1->datatype = expDataType = f.ret;
                if (func_name == "init") {
                    ap1->s = nowDefClass->new_name;
                } else {
                    ap1->s = f.new_name;    
                }
            }
            else if (!isClassDef && funcMap[func_name].find(temp_rparams, f)) {
                ap1->datatype = expDataType = f.ret;
                if (unaryClassMap.find(f.old_name) != unaryClassMap.end()) {
                    ap1->s = f.old_name; // class的new_name
                } else {
                    ap1->s = f.new_name;
                }
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
            // '[' Exp ']'
            cp = cp->next;
            NestDataType expDataType2 = NestDataType(expDataType.datatype_list[0]);
            if (expDataType2.datatype_list[0].name == "List")
                expDataType2.datatype_list[0].name = "int";
            expDataType = expDataType.removeFront();
            NestDataType expDataType3;
            ASTNode* ap1 = creatNode(ap, "", "Index");
            semanticAnalysis(cp, ap1, cp->type, expDataType3, level, DEBUG); // Exp
            ap1->datatype = expDataType3;
            // 下标类型检查（List的id或者Dict的key）
            expDataType2.tryAssign(expDataType3, cp->line);
            cp = cp->next->next;
		}
        while (cp != nullptr && cp->type == "DOT") {
            if (DEBUG) cout << "LVAL DOT with expDataType:" << expDataType.to_string() << endl;
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
            if (DEBUG) cout << "unaryClassMap[" << class_name << "]: " << unaryClassMap[class_name].to_string() << endl;
            string attr_name = cp->s;
            if (cp->next == nullptr && croot->next != nullptr && croot->next->type == "LPARENT") {
                string func_name = attr_name;
                vector<NestDataType> temp_rparams;
                ASTNode* ap1 = creatNode(ap, func_name, "FuncCall");
                if (croot->next->next!= nullptr && croot->next->next->type == "FuncRParams") {
                    temp_rparams = readFuncRParams(croot->next->next->first_child, ap, level, DEBUG);
                }
                UnaryFunc f;
                if (unaryClassMap[class_name].funcMap[func_name].find(temp_rparams, f)) {
                    expDataType = f.ret;
                    ap1->datatype = f.ret;
                    if (unaryClassMap.find(f.old_name) != unaryClassMap.end()) {
                        ap1->s = f.old_name; // class的new_name
                    } else {
                        ap1->s = f.new_name;
                    }
                }
                else {
                    cerr << "[line " << cp->line << "] SemanticError: Cannot find function " << func_name << " of class " << unaryClassMap[class_name].old_name << " !" << endl;
                    exit(3);
                }
            }
            else if (unaryClassMap[class_name].attrs.find(attr_name) != unaryClassMap[attr_name].attrs.end()) {
                expDataType = unaryClassMap[class_name].attrs[attr_name].nest_datatype;
                creatNode(ap, unaryClassMap[class_name].attrs[attr_name].new_name, "attr", expDataType);
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
                ASTNode* ap1 = creatNode(ap, "", "Index");
                semanticAnalysis(cp, ap1, cp->type, expDataType2, level, DEBUG); // Exp
                ap1->datatype = expDataType2;
                cp = cp->next->next;
            }
        }
	}
    if (DEBUG) cout << "[" << type << "] return expDataType=" << expDataType.to_string() << endl;
}