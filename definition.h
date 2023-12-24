// compiler.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <stdio.h>
#include <malloc.h>
#include <vector>
#include <map>
#include <stack>

using namespace std;

typedef struct Token
{
    Token(string _s, string _type, int _line) {
        s = _s;
        type = _type;
        line = _line;
        next = nullptr;
    }
    string s;
    string type;
    int line;
    struct Token* next;
}TOKEN;

typedef struct CSTNode
{
    CSTNode(string _s, string _type, int _line) {
        s = _s;
        type = _type;
        line = _line;
        next = nullptr;
        first_child = nullptr;
        last_child = nullptr;
    }
    string s;
    string type;
    int line;
    struct CSTNode* next;
    struct CSTNode* first_child;
    struct CSTNode* last_child;
}CSTNode;

extern void lexAnalysis(ifstream *infile, TOKEN **token, ofstream *outfile=nullptr, int DEBUG=0);

extern void grammarAnalysis(TOKEN **token, string type, CSTNode *root, ofstream *outfile=nullptr, int DEBUG=0);

extern void genCSTCppCode(CSTNode *root, string type, ofstream *outfile=nullptr, string prefix="", int DEBUG=0);

class UnaryDataType
{
    public:
        string type;  // "basic", "class", "generic" (generic不参与类型判定)
        string name;  // "int", "float", "long", "bool", "str", "None", Ident, "List"
        UnaryDataType() {}
        UnaryDataType(string _type, string _name) {
            type = _type;
            name = _name;
        }
        UnaryDataType(string _type, string _name, vector<UnaryDataType> _rgenerics) {
            type = _type;
            name = _name;
        }
        string to_string() const {
            return type + " " + name;
        }
        bool equals(UnaryDataType d) {
            if (type != d.type) return false;
            if (name == "List") {
                if (d.name != "List" && d.name != "int") return false;
                return true;
            }
            if (d.name == "List") {
                if (name != "List" && name != "int") return false;
                return true;
            }
            return name == d.name;
        }
        UnaryDataType copy() {
            UnaryDataType d = UnaryDataType(type, name);
            return d;
        }
        void tryAssign(UnaryDataType d, int line) {
            // 注意这里this是左值，d是右值
            if (type == "basic") {
                if (d.type != "basic") {
                    cerr << "[line " << line << "] SemanticError: Basic type " << name << " cannot be assigned type " << d.type << "!" << endl;
                    exit(3);
                }
                if ((d.name == "float" && name != "float")
                    || (d.name == "long" && name != "long")
                    || (d.name == "str" && name != "str")
                    || (d.name == "None" && name != "None")
                    || (d.name != "None" && name == "None")) {
                    cerr << "[line " << line << "] SemanticError: Basic type " << name << " cannot be assigned type " << d.name << "!" << endl;
                }
            }
            else if (type == "class") {
                if (d.type != "class") {
                    cerr << "[line " << line << "] SemanticError: Class " << name << " cannot be assigned type " << d.type << "!" << endl;
                    exit(3);
                }
                if (name != d.name) {
                    cerr << "[line " << line << "] SemanticError: Class " << name << " cannot be assigned different Class" << d.name << "!" << endl;
                    exit(3);
                }
            }
        }
        UnaryDataType oneOp(string op, int line) {
            if (type == "class") {
                cerr << "[line " << line << "] SemanticError: Class " << name << " cannot be operated by " << op << "!" << endl;
                exit(3);
            }
            if (name == "str") {
                cerr << "[line " << line << "] SemanticError: Str cannot be operated by " << op << "!" << endl;
                exit(3);
            }
            if (op == "not") return UnaryDataType("basic", "bool");
            return UnaryDataType("basic", name);
        }
        UnaryDataType twoOp(string op, UnaryDataType d, int line) {
            if (type == "class" || d.type == "class") {
                cerr << "[line " << line << "] SemanticError: Class cannot be operated by " << op << "!" << endl;
                exit(3);
            }
            if (name == "str") {
                if (!equals(d)) {
                    cerr << "[line " << line << "] SemanticError: Str cannot be operated with datatype " << d.name << "!" << endl;
                    exit(3);
                }
                if (op == "!=" || op == "==" || op == "<" || op == ">" || op == "<=" || op == ">=") {
                    return UnaryDataType("basic", "bool");
                } else if (op == "+") {
                    return UnaryDataType("basic", "str");
                } else {
                    cerr << "[line " << line << "] SemanticError: Str cannot be operated by " << op << "!" << endl;
                    exit(3);
                }
            }
            if (type == "float" && d.type == "long" || (type == "long" && d.type == "float")) {
                cerr << "[line " << line << "] SemanticError: Float and Long cannot be operated by " << op << "!" << endl;
            }
            if (op == "!=" || op == "==" || op == "<" || op == ">" || op == "<=" || op == ">=" || op == "and" || op == "or") {
                return UnaryDataType("basic", "bool");
            }
            if (type == "float" || d.type == "float") {
                return UnaryDataType("basic", "float");
            }
            if (type == "long" || d.type == "long") {
                return UnaryDataType("basic", "long");
            }
            if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
                return UnaryDataType("basic", "int");
            }
            return UnaryDataType("basic", name);
        }
};

class NestDataType
{
    public:
        vector<UnaryDataType> datatype_list;
        bool end = false;
        NestDataType() {}
        NestDataType(UnaryDataType d) {
            datatype_list.push_back(d);
        }
        void push(UnaryDataType d) {
            datatype_list.push_back(d);
        }
        int size() const {
            return datatype_list.size();
        }
        void clear () {
            datatype_list.clear();
            end = false;
        }
        string to_string() const {
            string s = "[";
            for (int i = 0; i < size(); i++) {
                s += datatype_list[i].to_string();
                if (i < size() - 1) s += ", ";
            }
            s += "]";
            return s;
        }
        NestDataType removeFront() {
            NestDataType d = NestDataType();
            for (int i = 1; i < size(); i++) {
                d.push(datatype_list[i]);
            }
            return d;
        }
        void tryAssign(NestDataType d, int line) {
            int len = size();
            if (len != d.size()) {
                cerr <<  "[line " << line << "] SemanticError: NestDataType size " << len << " cannot be assigned NestDataType size " << d.size() << "!" << endl;
                exit(3);
            }
            for (int i = 0; i < len - 1; i++) {
                if (!datatype_list[i].equals(d.datatype_list[i])) {
                    cerr << "[line " << line << "] SemanticError: NestDataType " << i << "th type " << datatype_list[i].name << " cannot be assigned type " << d.datatype_list[i].name << "!" << endl;
                    exit(3);
                }
            }
            datatype_list[len - 1].tryAssign(d.datatype_list[len - 1], line);
        }
        bool equals(vector<UnaryDataType> _datatype_list, int start) {
            if (size() != _datatype_list.size() - start) return false;
            for (int i = 0; i < size(); i++) {
                if (!datatype_list[i].equals(_datatype_list[start + i])) return false;
            }
            return true;
        }

        NestDataType oneOp(string op, int line) {
            if (size() != 1) {
                cerr << "[line " << line << "] SemanticError: NestDataType size of op " << op << " is " << size() << "!" << endl;
                exit(3);
            }
            return NestDataType(datatype_list[0].oneOp(op, line));
        }

        NestDataType twoOp(string op, NestDataType d, int line) {
            if (size() != 1) {
                cerr << to_string() << endl;
                cerr << "[line " << line << "] SemanticError: NestDataType size of op " << op << " is " << size() << "!" << endl;
                exit(3);
            }
            if (d.size() != 1) {
                cerr << d.to_string() << endl;
                cerr << "[line " << line << "] SemanticError: NestDataType size of op " << op << " is " << d.size() << "!" << endl;
                exit(3);
            }
            return NestDataType(datatype_list[0].twoOp(op, d.datatype_list[0], line));
        }
};

class UnaryVar
{
    // 示例：List<Map<str, Class1<str, int>>> => ["int", "str", class "Class1" ["str", "int"]]
    public:
        int level;
        string new_name;
        string old_name;
        NestDataType nest_datatype;
        UnaryVar() {}
        UnaryVar(int _level, string _old_name, string _new_name) {
            level = _level;
            old_name = _old_name;
            new_name = _new_name;
        }
        UnaryVar(int _level, string _old_name, string _new_name, UnaryDataType _datatype) {
            level = _level;
            old_name = _old_name;
            new_name = _new_name;
            nest_datatype.push(_datatype);
        }
        UnaryVar(int _level, string _old_name, string _new_name, NestDataType _datatype) {
            level = _level;
            old_name = _old_name;
            new_name = _new_name;
            for (UnaryDataType d : _datatype.datatype_list) {
                nest_datatype.push(d.copy());
            }
        }
        string to_string() const {
           return old_name + " " + new_name + " " + nest_datatype.to_string();
        }
        void push(UnaryDataType d) {
            nest_datatype.push(d);
        }
        int size() const {
            return nest_datatype.size();
        }
        UnaryDataType datatype_list(int i) {
            return nest_datatype.datatype_list[i];
        }
};

class VarStack
{
    public:
        vector<UnaryVar> vars;
        VarStack() {}
        VarStack(UnaryVar v) {
            vars.push_back(v);
        }
        string to_string() const {
            string s = "[";
            for (int i = 0; i < vars.size(); i++) {
                s += vars[i].old_name + ":" + vars[i].nest_datatype.to_string();
                if (i < vars.size() - 1) s += ", ";
            }
            return s + "]";
        }
        void delTab(int parent_level) {
            while (vars.size() > 0 && top().level > parent_level) {
                pop();
            }
        }
        UnaryVar top() {
            return vars.back();
        }
        int size() const {
            return vars.size();
        }
        void push(UnaryVar v) {
            vars.push_back(v);
        }
        void pop() {
            vars.pop_back();
        }
};

class UnaryFunc
{
    public:
        string new_name;
        string old_name;
        CSTNode* cst_root = nullptr;
        map<string, int> generic_name2id;
        vector<UnaryDataType> generics;  // 泛型取值
        vector<NestDataType> fparams;
        vector<string> params_name;
        NestDataType ret;
        string to_string() const
        {
            string s;
            s += old_name + "(";
            for (int i = 0; i < fparams.size(); i++) {
                s += fparams[i].to_string();
                if (i < fparams.size() - 1) s += ", ";
            }
            return s + ")->" + ret.to_string();
        }
        UnaryFunc () {}
        UnaryFunc (string _new_name, string _old_name) {
            new_name = _new_name;
            old_name = _old_name;
        }
        UnaryFunc (string _new_name, string _old_name, map<string, int> _generic_name2id, vector<UnaryDataType> _generics) {
            new_name = _new_name;
            old_name = _old_name;
            for (UnaryDataType d : _generics) {
                generics.push_back(d);
            }
            for (const auto& it : _generic_name2id) {
                generic_name2id[it.first] = it.second;
            }
        }
        UnaryFunc copy(string type) {
            if (type == "template")
            return UnaryFunc(new_name, old_name, generic_name2id, generics);
            cout << "你要复制什么玩意？" << endl;
            return UnaryFunc();
        }
        NestDataType returnType(vector<UnaryDataType> rgenerics, int line)
        {
            NestDataType ret_real;
            for (int i = 0; i < ret.size(); i++) {
                UnaryDataType fdatatype = ret.datatype_list[i];
                if (fdatatype.type == "generic") {
                    if (generic_name2id.find(fdatatype.name) == generic_name2id.end()) {
                        cerr << "[line " << line << "] SemanticError: Generic name " << fdatatype.name << " has not been defined!" << endl;
                        exit(3);
                    }
                    UnaryDataType basic_type = rgenerics[generic_name2id[fdatatype.name]];
                    if (basic_type.type != "basic") {
                        cerr << "[line " << line << "] SemanticError: Generic " << fdatatype.name << " real type " << basic_type.type << " is not basic!" << endl;
                        exit(3);
                    }
                    ret_real.push(basic_type);
                } else {
                    ret_real.push(fdatatype);
                }
            }
            return ret_real;
        }
        bool findTemp(vector<UnaryDataType> rgenerics, vector<NestDataType> rparams, int line) {
            if (fparams.size() != rparams.size()) return false;
            if (generic_name2id.size() != rgenerics.size()) return false;
            for (int i = 0; i < fparams.size(); i++) {
                if (fparams[i].size() != rparams[i].size()) return false;
                for (int j = 0; j < fparams[i].size(); j++) {
                    UnaryDataType fdatatype = fparams[i].datatype_list[j];
                    UnaryDataType rdatatype = rparams[i].datatype_list[j];
                    if (fdatatype.type == "generic") {
                        if (generic_name2id.find(fdatatype.name) == generic_name2id.end()) {
                            cerr << "[line " << line << "] SemanticError: Generic name " << fdatatype.name << " of function " << old_name << " has not been defined!" << endl;
                            exit(3);
                        }
                        int generic_id = generic_name2id[fdatatype.name];
                        if (generic_id >= generic_name2id.size()) {
                            cerr << "[line " << line << "] SemanticError: Generic id " << generic_id<< " of function " << old_name << " exceed generic_name2id size" << generic_name2id.size() << "!" << endl;
                            exit(3);
                        }
                        UnaryDataType basic_type = rgenerics[generic_id];
                        if (basic_type.type != "basic") {
                            cerr << "[line " << line << "] SemanticError: Generic " << fdatatype.name << " real type " << basic_type.type << " is not basic!" << endl;
                            exit(3);
                        }
                        if (rdatatype.type != "basic") return false;
                        if (basic_type.name != rdatatype.name) return false;
                    }
                    else if (!fdatatype.equals(rdatatype)) return false;
                }
            }
            return true;
        }
        bool equals(vector<NestDataType> rparams, int line) {
            if (fparams.size() != rparams.size()) return false;
            for (int i = 0; i < fparams.size(); i++) {
                if (fparams[i].size() != rparams[i].size()) return false;
                for (int j = 0; j < fparams[i].size(); j++) {
                    UnaryDataType fdatatype = fparams[i].datatype_list[j];
                    UnaryDataType rdatatype = rparams[i].datatype_list[j];
                    if (!fdatatype.equals(rdatatype)) return false;
                }
            }
            return true;
        }
        bool equals(vector<UnaryDataType> rgenerics, vector<NestDataType> rparams, int line) {
            if (generics.size() != rgenerics.size()) return false;
            for (int i = 0; i < generics.size(); i++) {
                if (generics[i].name != rgenerics[i].name) return false;
            }
            if (fparams.size() != rparams.size()) return false;
            for (int i = 0; i < fparams.size(); i++) {
                if (fparams[i].size() != rparams[i].size()) return false;
                for (int j = 0; j < fparams[i].size(); j++) {
                    UnaryDataType fdatatype = fparams[i].datatype_list[j];
                    UnaryDataType rdatatype = rparams[i].datatype_list[j];
                    if (!fdatatype.equals(rdatatype)) return false;
                }
            }
            return true;
        }

        UnaryFunc real(string _new_name, vector<UnaryDataType> rgenerics, int line) {
            UnaryFunc real_f = copy("template");
            real_f.new_name = _new_name;
            for (UnaryDataType d : rgenerics) {
                real_f.generics.push_back(d);
            }
            for (int i = 0; i < fparams.size(); i++) {
                real_f.fparams.push_back(NestDataType());
                real_f.params_name.push_back(params_name[i]);
                for (int j = 0; j < fparams[i].size(); j++) {
                    UnaryDataType fdatatype = fparams[i].datatype_list[j];
                    if (fdatatype.type == "generic") {
                        if (generic_name2id.find(fdatatype.name) == generic_name2id.end()) {
                            cerr << "[line " << line << "] SemanticError: Generic name " << fdatatype.name << " of function " << old_name << " has not been defined!" << endl;
                            exit(3);
                        }
                        int generic_id = generic_name2id[fdatatype.name];
                        if (generic_id >= generic_name2id.size()) {
                            cerr << "[line " << line << "] SemanticError: Generic id " << generic_id<< " of function " << old_name << " exceed generic_name2id size" << generic_name2id.size() << "!" << endl;
                            exit(3);
                        }
                        UnaryDataType basic_type = rgenerics[generic_id];
                        if (basic_type.type != "basic") {
                            cerr << "[line " << line << "] SemanticError: Generic " << fdatatype.name << " real type " << basic_type.type << " is not basic!" << endl;
                            exit(3);
                        }
                        real_f.fparams[i].push(basic_type);
                    }
                    else {
                        real_f.fparams[i].push(fdatatype);
                    }
                }
            }
            for (int i = 0; i < ret.size(); i++) {
                if (ret.datatype_list[i].type == "generic") {
                    if (generic_name2id.find(ret.datatype_list[i].name) == generic_name2id.end()) {
                        cerr << "[line " << line << "] SemanticError: Generic name " << ret.datatype_list[i].name << " of function " << old_name << " has not been defined!" << endl;
                        exit(3);
                    }
                    int generic_id = generic_name2id[ret.datatype_list[i].name];
                    if (generic_id >= generic_name2id.size()) {
                        cerr << "[line " << line << "] SemanticError: Generic id " << generic_id << " of function " << old_name << " exceed generic_name2id size" << generic_name2id.size() << "!" << endl;
                        exit(3);
                    }
                    UnaryDataType basic_type = rgenerics[generic_id];
                    if (basic_type.type != "basic") {
                        cerr << "[line " << line << "] SemanticError: Generic " << ret.datatype_list[i].name << " real type " << basic_type.type << " is not basic!" << endl;
                        exit(3);
                    }
                    real_f.ret.datatype_list.push_back(basic_type);
                }
                else {
                    real_f.ret.datatype_list.push_back(ret.datatype_list[i]);
                }
            }
            return real_f;
        }
};

class FuncList
{
    public:
        vector<UnaryFunc> funcs;
        FuncList() {}
        FuncList(UnaryFunc f) {
            funcs.push_back(f);
        }
        string to_string() const
        {
            string s = "[";
            for (int i = 0; i < funcs.size(); i++) {
                s += funcs[i].to_string();
                if (i < funcs.size() - 1) s += ", ";
            }
            return s + "]";
        }
        void push(UnaryFunc f) {
            funcs.push_back(f);
        }
        bool find(vector<UnaryDataType> rgenerics, vector<NestDataType> rparams, UnaryFunc &f) {
            for (int i = 0; i < funcs.size(); i++) {
                if (funcs[i].equals(rgenerics, rparams, 0)) {
                    f = funcs[i];
                    return true;
                }
            }
            return false;
        }
        bool find(vector<NestDataType> rparams, UnaryFunc &f) {
            for (int i = 0; i < funcs.size(); i++) {
                if (funcs[i].equals(rparams, 0)) {
                    f = funcs[i];
                    return true;
                }
            }
            return false;
        }
        bool findTemp(vector<UnaryDataType> rgenerics, vector<NestDataType> rparams, UnaryFunc &f) {
            for (int i = 0; i < funcs.size(); i++) {
                if (funcs[i].findTemp(rgenerics, rparams, 0)) {
                    f = funcs[i];
                    return true;
                }
            }
            return false;
        }
        bool find(UnaryFunc f) {
            vector<UnaryDataType> empty_generics;
            for (int i = 0; i < funcs.size(); i++) {
                if (funcs[i].equals(empty_generics, f.fparams, 0)) {
                    return true;
                }
            }
            return false;
        }
};

class UnaryClass
{
    public:
        string new_name;
        string old_name;
        CSTNode* cst_root = nullptr;
        map<string, int> generic_name2id;
        vector<UnaryDataType> generics;  // 泛型取值
        map<string, UnaryVar> attrs;
        map<string, FuncList> funcMap;

        UnaryClass() {}
        UnaryClass(string _new_name, string _old_name, map<string, int> _generic_name2id, vector<UnaryDataType> _generics) {
            new_name = _new_name;
            old_name = _old_name;
            for (const auto& it : _generic_name2id) {
                generic_name2id[it.first] = it.second;
            }
            for (UnaryDataType d : _generics) {
                generics.push_back(d);
            }
        }
        UnaryClass copy(string type) {
            if (type == "template")
            return UnaryClass(new_name, old_name, generic_name2id, generics);
            cout << "你要复制什么玩意？" << endl;
            return UnaryClass();
        }

        string to_string () const {
            string s = old_name + " { attrs:{";
            int cnt = 0;
            for (const auto& it : attrs) {
                s += it.first + ":" + it.second.to_string();
                if (cnt++ < attrs.size() - 1) s += ", ";
            }
            s += "} funcs:{";
            cnt = 0;
            for (const auto& it : funcMap) {
                s += it.first + ":" + it.second.to_string();
                if (cnt++ < attrs.size() - 1) s += ", ";
            }
            return s + "} }";
        }

        // old_name相同才需要比较泛型，而new_name和UnaryClass唯一对应
        bool equals(vector<UnaryDataType> rgenerics, int line) {
            if (generics.size() != rgenerics.size()) return false;
            for (int i = 0; i < rgenerics.size(); i++) {
                if (!generics[i].equals(rgenerics[i])) return false;
            }
            return true;
        }

        UnaryClass real(string _new_name, vector<UnaryDataType> rgenerics, int line) {
            UnaryClass real_c = copy("template");
            for (UnaryDataType d : rgenerics) {
                real_c.generics.push_back(d);
            }
            real_c.new_name = _new_name;
            return real_c;
        }
};

class ClassList
{
    public:
        vector<UnaryClass> classes;
        ClassList() {}
        ClassList(UnaryClass c) {
            classes.push_back(c);
        }
        void push(UnaryClass c) {
            classes.push_back(c);
        }
        bool find(vector<UnaryDataType> rgenerics, UnaryClass &c) {
            for (int i = 0; i < classes.size(); i++) {
                if (classes[i].equals(rgenerics, 0)) {
                    c = classes[i];
                    return true;
                }
            }
            return false;
        }
        bool findTemp(vector<UnaryDataType> rgenerics, UnaryClass &c) {
            for (int i = 0; i < classes.size(); i++) {
                if (classes[i].generics.empty() && classes[i].generic_name2id.size() == rgenerics.size()) {
                    c = classes[i];
                    return true;
                }
            }
            return false;
        }
};

typedef struct ASTNode
{
    ASTNode(string _s, string _type) {
        s = _s;
        type = _type;
        next = nullptr;
        first_child = nullptr;
        last_child = nullptr;
    }
    ASTNode(string _s, string _type, NestDataType _datatype) {
        s = _s;
        type = _type;
        datatype = _datatype;
        next = nullptr;
        first_child = nullptr;
        last_child = nullptr;
    }
    string s;
    string type; // 非终结符, op
    NestDataType datatype;
    struct ASTNode* next;
    struct ASTNode* first_child;
    struct ASTNode* last_child;
}ASTNODE;

extern void semanticAnalysis(CSTNode *croot, ASTNode *aroot, string type, NestDataType& expDataType, int level, int DEBUG=0);
