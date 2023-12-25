#include "definition.h"

string cppDataType(string s) {
    if (s == "int") return "int";
    if (s == "float") return "float";
    if (s == "str") return "string";
    if (s == "bool") return "bool";
    if (s == "long") return "long";
    if (s == "None") return "void";
    if (s == "True") return "true";
    if (s == "False") return "false";
    // else: str const(int/float/long)
    return s;
}

string cppDataType(NestDataType ds) {
    string s = "";
    for (int i = 0; i < ds.size(); i++) {
        UnaryDataType d = ds.datatype_list[i];
        if (d.name == "List") {
            s += "vector<";
        }
        else if (i < ds.size() - 1) {
            s += "map<" + cppDataType(d.name) + ",";
        }
        else {
            s += cppDataType(d.name);
        }
    }
    for (int i = 0; i < ds.size() - 1; i++) {
        s += ">";
    }
    return s;
}

string cppOneOp(string s) {
    if (s == "+") return "+";
    if (s == "-") return "-";
    if (s == "not") return "!";
    return s;
}

string cppTwoOp(string s) {
    if (s == ">") return ">";
    if (s == ">=") return ">=";
    if (s == "<") return "<";
    if (s == "<=") return "<=";
    if (s == "==") return "==";
    if (s == "!=") return "!=";
    if (s == "and") return "&&";
    if (s == "or") return "||";
    if (s == "+") return "+";
    if (s == "-") return "-";
    if (s == "*") return "*";
    if (s == "/") return "/";
    if (s == "%") return "%";
    return s;
}

int temp_cnt = 0;
string new_temp() {
    return "temp" + to_string(temp_cnt++);
}

string getLVal(vector<string> &pre_lines, ASTNode *root, string prefix, int DEBUG);
string genExpCode(vector<string> &pre_lines, ASTNode *root, string prefix, int DEBUG);

string getLVal(vector<string> &pre_lines, ASTNode *root, string prefix, int DEBUG) {
    // [self] [var/attr {Index}] {FuncCall / attr{Index}}
    ASTNode *p = root->first_child;
    bool needDot = false;
    string s = "";
    if (p->s == "self") {
        s += "this->";
        p = p->next;
    }
    if (p->type == "var" || p->type == "attr") {
        s += p->s;
        p = p->next;
        needDot = true;
    }
    while (p != nullptr && p->type == "Index") {
        s += "[" + genExpCode(pre_lines, p->first_child, prefix, DEBUG) + "]";
        p = p->next;
    }
    while (p != nullptr) {
        if (needDot) s += ".";
        if (p != nullptr && p->type == "FuncCall") {
            s += p->s + "(";
            // FuncRParams: {Exp}
            ASTNode* p2 = p->first_child;
            while (p2 != nullptr && p2->type == "FuncRParam") {
                s += genExpCode(pre_lines, p2->first_child, prefix, DEBUG);
                p2 = p2->next;
                if (p2 != nullptr && p2->type == "FuncRParam") s += ", ";
            }
            s += ")";
            p = p->next;
        }
        else if (p != nullptr && p->type == "attr") {
            s += "." + p->s;
            p = p->next;
            while (p != nullptr && p->type == "Index") {
                s += "[" + genExpCode(pre_lines, p->first_child, prefix, DEBUG) + "]";
                p = p->next;
            }
        }
    }
    return s;
}

string genExpCode(vector<string> &pre_lines, ASTNode *root, string prefix, int DEBUG) {
    if (root == nullptr) return "";
    if (root->type == "LVal") {
        return getLVal(pre_lines, root, prefix, DEBUG);
    }
    else if (root->type == "TwoOp") {
        string t1 = genExpCode(pre_lines, root->first_child, prefix, DEBUG);
        string t2 = genExpCode(pre_lines, root->first_child->next, prefix, DEBUG);
        string t = new_temp();
        string datatype = cppDataType(root->datatype.datatype_list[0].name);
        string op = cppTwoOp(root->s);
        pre_lines.push_back(prefix + datatype + " " + t + " = (" + t1 + " " + op + " " + t2 + ");");
        return t;
    }
    else if (root->type == "OneOp") {
        string t1 = genExpCode(pre_lines, root->first_child, prefix, DEBUG);
        string t = new_temp();
        string datatype = cppDataType(root->datatype.datatype_list[0].name);
        string op = cppOneOp(root->s);
        pre_lines.push_back(prefix + datatype + " " + t + " = (" + op + t1 + ");");
        return t;
    }
    else if (root->type == "const") {
        return cppDataType(root->s);
    }
    return "";
}

string getInitVal(vector<string> &pre_lines, ASTNode *root, string prefix, int DEBUG) {
    string s;
    ASTNODE* p = root->first_child;
    if (root->type == "ListInitVal") {
        s += "{";
        while (p != nullptr) {
            s += getInitVal(pre_lines, p, prefix, DEBUG);
            p = p->next;
            if (p != nullptr) s += ", ";
        }
        s += "}";
    }
    else if (root->type == "DictInitVal") {
        s += "{";
        while (p != nullptr) {
            string key = genExpCode(pre_lines, p->first_child, prefix, DEBUG);
            string value = getInitVal(pre_lines, p->first_child->next, prefix, DEBUG);
            s += "{" + key + ", " + value + "}";
            p = p->next;
            if (p != nullptr) s += ", ";
        }
        s += "}";
    }
    else {
       return genExpCode(pre_lines, root, prefix, DEBUG);
    }
    return s;
}

void genASTCppCode(ASTNode *root, string type, vector<string> &output_lines, string prefix, int DEBUG){
    if (root == nullptr) return;
    ASTNode* p = root->first_child;
    if (DEBUG) cout << "[" << type << "]" << root->s << endl;
    if (type == "CompUnit") {
        cout << "Generating C++ code from AST..." << endl;
        output_lines.push_back("#include <iostream>");
        output_lines.push_back("#include <map>");
        output_lines.push_back("#include <vector>");
        output_lines.push_back("using namespace std;");
        while (p != nullptr) {
            // {ClassDef} {FuncDef}
            if (p->type == "template") {
                // 占位用的泛型模板
                p = p->next;
                continue;
            }
            genASTCppCode(p, p->type, output_lines, prefix, DEBUG);
            p = p->next;
        }
    }
    else if (type == "ClassDef") {
        output_lines.push_back("class " + root->s);
        output_lines.push_back("{");
        output_lines.push_back("    public:");
        while (p != nullptr) {
            // {ClassAttrDef}, {ClassInitDef}, {ClassFuncDef}
            genASTCppCode(p, p->type, output_lines, prefix + "    ", DEBUG);
            p = p->next;
        }
        output_lines.push_back("};");
    }
    else if (type == "ClassAttrDef") {
        // {ClassAttrDef}, {ClassInitDef}, {ClassFuncDef}
        string datatype = cppDataType(root->datatype);
        string varname = root->s;
        output_lines.push_back(prefix + datatype + " " + varname + ";");
    }
    else if (type == "ClassInitDef") {
        string class_name = root->datatype.datatype_list[0].name;
        string s = prefix + class_name + "(";
        while (p != nullptr && p->type == "FuncFParam") {
            string datatype = cppDataType(p->datatype);
            string varname = p->s;
            s += datatype + " " + varname;
            p = p->next;
            if (p != nullptr && p->type == "FuncFParam")
                s += ", ";
        }
        output_lines.push_back(s + ")");
        if (p != nullptr && p->type == "Block")
            genASTCppCode(p, p->type, output_lines, prefix, DEBUG);
    }
    else if (type == "ClassFuncDef" || type == "FuncDef") {
        // {FuncFParam}, Block
        string func_name = root->s;
        string ret = cppDataType(root->datatype);
        string s = prefix + ret + " " + func_name + "(";
        while (p != nullptr && p->type == "FuncFParam") {
            string datatype = cppDataType(p->datatype);
            string varname = p->s;
            s += datatype + " " + varname;
            p = p->next;
            if (p != nullptr && p->type == "FuncFParam")
                s +=", ";
        }
        output_lines.push_back(s + ")");
        if (p != nullptr && p->type == "Block")
            genASTCppCode(p, p->type, output_lines, prefix, DEBUG);
    }
    else if (type == "Block") {
        output_lines.push_back(prefix + "{");
        while (p != nullptr) {
            // {Decl}, {Stmt}
            genASTCppCode(p, p->type, output_lines, prefix + "    ", DEBUG);
            p = p->next;
        }
        output_lines.push_back(prefix + "}");
    }
    else if (type == "Decl") {
        if (root->s == "=") {
            //变量var，赋值InitVal (ListInitVal / DictInitVal / Exp)
            string datatype = cppDataType(p->datatype);
            string varname = p->s;
            string initval = getInitVal(output_lines, p->next, prefix, DEBUG);
            output_lines.push_back(prefix + datatype + " " + varname + " = " + initval + ";");
        }
        else {
            string datatype = cppDataType(root->datatype);
            string varname = root->s;
            output_lines.push_back(prefix + datatype + " " + varname + ";");
        }
    }
    else if (type == "Stmt") {
        if (root->s == "=") {
            // LVal, Exp
            string lval = getLVal(output_lines, p, prefix, DEBUG);
            string exp = genExpCode(output_lines, p->next, prefix, DEBUG);
            output_lines.push_back(prefix + lval + " = " + exp + ";");
        }
        else if (root->s == "append") {
            string lval = getLVal(output_lines, p, prefix, DEBUG);
            string exp = genExpCode(output_lines, p->next, prefix, DEBUG);
            output_lines.push_back(prefix + lval + ".push_back(" + exp + ");");            
        }
        else if (root->s == "if") {
            // Exp, Block[, Block]
            string exp = genExpCode(output_lines, p, prefix, DEBUG);
            output_lines.push_back(prefix + "if (" + exp + ")");
            genASTCppCode(p->next, p->next->type, output_lines, prefix, DEBUG);
            if (p->next->next != nullptr) {
                output_lines.push_back(prefix + "else");
                genASTCppCode(p->next->next, p->next->next->type, output_lines, prefix, DEBUG);
            }
        }
        else if (root->s == "while") {
            // Exp, Block
            vector<string> condition_lines;
            string exp = genExpCode(condition_lines, p, prefix, DEBUG);
            output_lines.insert(output_lines.end(), condition_lines.begin(), condition_lines.end());
            output_lines.push_back(prefix + "while (" + exp + ")");

            vector<string> block_lines;
            genASTCppCode(p->next, p->next->type, block_lines, prefix, DEBUG);
            // 每次循环后重新计算condition
            condition_lines.push_back(prefix + "    if (!" + exp + ") break;");
            block_lines.insert(block_lines.begin() + 1, condition_lines.begin(), condition_lines.end());
            output_lines.insert(output_lines.end(), block_lines.begin(), block_lines.end());
        }
        else if (root->s == "break" || root->s == "continue") {
            output_lines.push_back(prefix + root->s + ";");
        }
        else if (root->s == "return") {
            string exp = genExpCode(output_lines, p, prefix, DEBUG);
            output_lines.push_back(prefix + "return " + exp + ";");
        }
        else if (root->s == "print") {
            string s = "cout";
            while (p != nullptr) {
                string exp = genExpCode(output_lines, p, prefix, DEBUG);
                s += " << " + exp;
                p = p->next;
            }
            output_lines.push_back(prefix + s + " << endl;");
        }
        else { // Exp
            string exp = genExpCode(output_lines, p, prefix, DEBUG);
            output_lines.push_back(prefix + exp + ";");
        }
    }
    if (DEBUG) cout << "[" << type << "] end" << endl;
}