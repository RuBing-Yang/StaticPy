class UnaryDataType
{
    public:
        string type;  // "basic", "class", "generic", "func"
        string name;  // "int", "float", "long", "bool", "str", Ident
        vector<string> rgenerics;  // 类的实例的泛型列表, 或者generic的可取值范围, 取值范围"int", "float", "long", "bool", "str"
        UnaryDataType() {}
        UnaryDataType(string _type, string _name) {
            type = _type;
            name = _name;
        }
        UnaryDataType(string _type, string _name, vector<string> _rgenerics) {
            type = _type;
            name = _name;
            rgenerics = _rgenerics;
        }
        bool equals(UnaryDataType d) {
            if (type != d.type) return false;
            if (type == "basic") return name == d.name;
            if (type == "class") {
                if (name != d.name) return false;
                if (rgenerics.size() != d.rgenerics.size()) return false;
                for (int i = 0; i < rgenerics.size(); i++) {
                    if (rgenerics[i] != d.rgenerics[i]) return false;
                }
                return true;
            }
            if (type == "generic") return name == d.name;
        }
        UnaryDataType copy() {
            UnaryDataType d = UnaryDataType(type, name);
            for (string s : rgenerics) {
                d.rgenerics.push_back(s);
            }
            return d;
        }
        void tryAssignTo(UnaryDataType d, int line) {
            // 注意这里this是左值，d是右值
            if (type == "basic") return name == d.name;
            else if (type == "class") {
                if (d.type != "class") {
                    cerr << "[line " << line << "] SemanticError: Class " << name << " cannot be assign to type " << d.type << "!" << endl;
                    exit(3);
                }
                if (name != d.name) {
                    cerr << "[line " << line << "] SemanticError: Class " << name << " cannot be assign to different Class" << d.name << "!" << endl;
                    exit(3);
                }
                if (rgenerics.size() != d.rgenerics.size()) {
                    cerr << "[line " << line << "] SemanticError: Class " << name << " cannot be assign to Class with different rgenerics size!" << endl;
                    exit(3);
                }
                for (int i = 0; i < rgenerics.size(); i++) {
                    if (rgenerics[i] != d.rgenerics[i]) {
                        cerr << "[line " << line << "] SemanticError: Class " << name << " cannot be assign to Class with different rgenerics type!" << endl;
                        exit(3);
                    }
                }
            }
            else if (type == "generic") return name == d.name;
        }
};

class NestDataType
{
    public:
        vector<UnaryDataType> datatype_list;
        NestDataType() {}
        NestDataType(UnaryDataType d) {
            datatype_list.push_back(d);
        }
        void push(UnaryDataType d) {
            datatype_list.push_back(d);
        }
        int size() {
            return datatype_list.size();
        }
};

class UnaryVar
{
    // 示例：List<Map<str, Class1<str, int>>> => ["int", "str", class "Class1" ["str", "int"]]
    public:
        int level;
        NestDataType nest_datatype;
        UnaryVar() {}
        UnaryVar(int _level) {
            level = _level;
        }
        UnaryVar(int _level, UnaryDataType _datatype) {
            level = _level;
            nest_datatype.push(_datatype);
        }
        UnaryVar(int _level, NestDataType _datatype) {
            level = _level;
            for (UnaryDataType d : _datatype.datatype_list) {
                nest_datatype.push(d.copy());
            }
        }
        void push(UnaryDataType d) {
            nest_datatype.push(d);
        }
        int size() {
            return nest_datatype.size();
        }
        UnaryDataType datatype_list(int i) {
            return nest_datatype.datatype_list[i];
        }
};

class VarStack
{
    public:
        stack<UnaryVar> vars;
        VarStack() {}
        VarStack(UnaryVar v) {
            vars.push(v);
        }
        void delTab(int parent_level) {
            while (vars.size() > 0 && vars.top().level > parent_level) {
                vars.pop();
            }
        }
        UnaryVar top() {
            return vars.top();
        }
        int size() {
            return vars.size();
        }
        void push(UnaryVar v) {
            vars.push(v);
        }
};

class UnaryFunc
{
    public:
        map<string, int> generic_name2id;
        vector<UnaryDataType> generics;  // 泛型取值范围
        vector<NestDataType> fparams;
        NestDataType ret;
        UnaryFunc () {}
        UnaryFunc (map<string, int> _generic_name2id) {
            for (auto it = _generic_name2id.begin(); it != _generic_name2id.end(); it++) {
                generic_name2id[it->first] = it->second;
                generics.push_back(UnaryDataType("generic", it->first, {"int", "float", "long", "bool", "str"}));
            }
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
        bool equals(vector<UnaryDataType> rgenerics, vector<NestDataType> rparams, int line) {
            if (fparams.size() != rparams.size()) return false;
            for (int i = 0; i < fparams.size(); i++) {
                if (fparams[i].size() != rparams[i].size()) return false;
                for (int j = 0; j < fparams[i].size(); j++) {
                    UnaryDataType fdatatype = fparams[i].datatype_list[j];
                    UnaryDataType rdatatype = rparams[i].datatype_list[j];
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
                        if (rdatatype.type != "basic") return false;
                        if (basic_type.name != rdatatype.name) return false;
                    }
                    else if (fdatatype.equals(rdatatype)) return false;
                }
            }
            return true;
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
        map<string, int> generic_name2id;
        vector<UnaryDataType> generics;  // 泛型取值范围
        map<string, NestDataType> attrs;
        map<string, FuncList> funcMap;
        UnaryClass() {}
        UnaryClass(map<string, int> _generic_name2id) {
            for (auto it = _generic_name2id.begin(); it != _generic_name2id.end(); it++) {
                generic_name2id[it->first] = it->second;
                generics.push_back(UnaryDataType("generic", it->first, {"int", "float", "long", "bool", "str"}));
            }
        }
};

void addVar(map<string, VarStack>varMap, string name, UnaryVar v, int line) {
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

void addFunc(map<string, FuncList> targetMap, string name, UnaryFunc f, int line) {
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
