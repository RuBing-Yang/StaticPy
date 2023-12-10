#include "definition.h"

string cpp_datatype(string type) {
    if (type == "INTTK") return "int";
    if (type == "NONETK") return "void";
    if (type == "LISTTK") return "vector<int>";
    if (type == "DICTTK") return "map<int, int>";
    return "";
}

string cpp_unaryop(string s) {
    if (s == "+") return "+";
    if (s == "-") return "-";
    if (s == "not") return "!";
    return "";
}

void genCppCode(ASTNODE *root, string type, ofstream *outfile){
    if (root == nullptr) return;
    cout << "genCppCode " << type << " begin" << endl;
    ASTNODE* p = root->first_child;
    if (type == "CompUnit") {
        (*outfile) << "#include <map>" << endl;
        (*outfile) << "#include <vector>" << endl;
        (*outfile) << "using namespace std;" << endl;
        while (p != nullptr) {
            (*outfile) << endl;
            cout <<  "[CompUnit] " << p->type << endl;
            genCppCode(p, p->type, outfile);
            p = p->next;
        }
    }
    else if (type == "FuncDef") {
        // 'def' Ident '(' [FuncFParams] ')' '->' FuncType Block
        while (p->type != "ARROW") p = p->next;
        string func_type = cpp_datatype(p->next->type);

        cout <<  "[FuncDef] " << func_type << endl;

        p = root->first_child->next;
        string func_name = p->s;
        cout <<  "[FuncDef] " << func_name << endl;
        (*outfile) << func_type << " " << func_name << "(";

        p = p->next->next;
        if (p->type == "FuncFParams") {
            genCppCode(p, p->type, outfile);
        }
        (*outfile) << ")" << endl;

        while (p->type != "Block") p = p->next;
        cout <<  "[FuncDef] " << func_name << " start Block" << endl;
		genCppCode(p, p->type, outfile);
        cout <<  "[FuncDef] " << func_name << " end Block" << endl;
        cout << "FuncDef end" << endl;
    }
    else if (type == "Block") {
        (*outfile) << "{" << endl;
        while (p->type != "BlockItem") p = p->next;
        while (p != nullptr && p->type == "BlockItem") {
            genCppCode(p, p->type, outfile);
            p = p->next;
        }
        (*outfile) << "}" << endl;
    }
    else if (type == "BlockItem") {
        // Decl | Stmt
        genCppCode(p, p->type, outfile);
        (*outfile) << ";" << endl;
    }
    else if (type == "Decl") {
        (*outfile) << cpp_datatype(p->next->next->type) << " " << p->s;
        p = p->next->next->next;
        if (p != nullptr && p->type == "ASSIGN") {
            (*outfile) << " = ";
            p = p->next;
            genCppCode(p, p->type, outfile); // InitVal
        }
    }
    else if (type == "InitVal") {
		if (p->type == "LBRACK") {  // List
            (*outfile) << "[";
            p = p->next;
            if (p->type == "InitVal") {
                genCppCode(p, p->type, outfile);
                p = p->next;
                while (p->type == "COMMA") {
                    p = p->next;
                    (*outfile) << ", ";
                    genCppCode(p, p->type, outfile);  // InitVal
                    p = p->next;
                }
            }
			(*outfile) << "]";
		}
        else if (p->type == "LBRACE") {  // Dict
            (*outfile) << "{";
            p = p->next;
            if (p->type == "InitVal") {
                (*outfile) << "{";
                genCppCode(p, p->type, outfile);  // InitVal
                p = p->next;
                (*outfile) << ", ";  // COLON
                p = p->next;
                genCppCode(p, p->type, outfile);  // InitVal
                p = p->next;
                (*outfile) << "}";
                while (p->type == "COMMA") {
                    p = p->next;
                    (*outfile) << ", {";
                    genCppCode(p, p->type, outfile);  // InitVal
                    p = p->next;
                    (*outfile) << ", ";  // COLON
                    p = p->next;
                    genCppCode(p, p->type, outfile);  // InitVal
                    p = p->next;
                    (*outfile) << "}";
                }
            }
			(*outfile) << "}";
		}
		else {  // Exp
            genCppCode(p, p->type, outfile);
		}
    }
    else if (type == "FuncFParams") {
        genCppCode(p, p->type, outfile);
        p = p->next;
		while (p != nullptr && p->type == "COMMA") {
            (*outfile) << ", ";
            p = p->next;
			genCppCode(p, p->type, outfile);
            p = p->next;
		}
    }
    else if (type == "FuncFParam") {
        // Ident ':' DataType
        (*outfile) << cpp_datatype(p->next->next->type) << " " << p->s;
    }
    else if (type == "Stmt") {
		if (p->type == "LVal") {
            // TODO: 数组/字典中元素赋值
            genCppCode(p, p->type, outfile); // LVal
            (*outfile) << " = ";
            p = p->next->next;
            genCppCode(p, p->type, outfile); // Exp
		}
		else if (p->type == "IFTK") {
            // 'if' '(' Cond ')' Block ['else' Block]
            (*outfile) << "if (";
            p = p->next->next;
            genCppCode(p, p->type, outfile); // Cond
            (*outfile) << ") ";
            p = p->next->next;
            genCppCode(p, p->type, outfile); // Block
            p = p->next;
            if (p != nullptr && p->type == "ELSETK") {
                (*outfile) << "else ";
                p = p->next;
                genCppCode(p, p->type, outfile); // Block
            }
		}
		else if (p->type == "WHILETK") {
            // 'while' '(' Cond ')' Block
            (*outfile) << "while (";
            p = p->next->next;
            genCppCode(p, p->type, outfile); // Cond
            (*outfile) << ") ";
            p = p->next->next;
            genCppCode(p, p->type, outfile); // Block
		}
		else if (p->type == "BREAKTK") {
            (*outfile) << "break";
		}
		else if (p->type == "CONTINUETK") {
            (*outfile) << "continue";
		}
		else if (p->type == "RETURNTK") {
            (*outfile) << "return";
            p = p->next;
            if (p != nullptr && p->type == "Exp") {
                (*outfile) << " ";
                genCppCode(p, p->type, outfile); // Exp
            }
		}
        else if (p->type == "PRINTTK") {
            // 'print' '(' [Exp {',' Exp}] ')'
            (*outfile) << "cout ";
            p = p->next->next;
            if (p->type == "STRCON") {
                (*outfile) << "<< \"" << p->s << "\"";
                p = p->next;
            }
            else if (p->type == "Exp") {
                (*outfile) << "<< ";
                genCppCode(p, p->type, outfile);
                p = p->next;
            }
            while (p != nullptr && p->type == "COMMA") {
                (*outfile) << " << ";
                p = p->next;
                if (p->type == "STRCON") {
                    (*outfile) << "<< \"" << p->s << "\"";
                    p = p->next;
                }
                else if (p->type == "Exp") {
                    (*outfile) << "<< ";
                    genCppCode(p, p->type, outfile); // Exp
                    p = p->next;
                }
            }
            (*outfile) << "<< endl";
        }
		else {
			genCppCode(p, p->type, outfile); // Exp
		}
    }
    else if (type == "Exp") { 
        genCppCode(p, p->type, outfile); // AddExp
	}
	else if (type == "AddExp") {
        genCppCode(p, p->type, outfile); // MulExp
        p = p->next;
        while (p != nullptr && (p->type == "PLUS" || p->type == "MINU")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            genCppCode(p, p->type, outfile); // MulExp
            p = p->next;
        }
	}
    else if (type == "MulExp") { 
        genCppCode(p, p->type, outfile); // UnaryExp
        p = p->next;
        while (p != nullptr && (p->type == "MULT" || p->type == "DIV" || p->type == "MOD")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            genCppCode(p, p->type, outfile); // UnaryExp
            p = p->next;
        }
	}
    else if (type == "UnaryExp") {
        if (p->type == "UnaryOp") {
            (*outfile) << cpp_unaryop(p->s);
            p = p->next;
            genCppCode(p, p->type, outfile); // UnaryExp
        }
		else if (p->type == "IDENFR" && p->next != nullptr && p->next->type == "LPARENT") {
            // 'IDENFR' '(' [FuncRParams] ')'
            (*outfile) << p->s << "(";
            p = p->next->next;
            if (p != nullptr && p->type == "FuncRParams") {
                genCppCode(p, p->type, outfile); // FuncRParams
            }
            (*outfile) << ")";
		}
		else {
            genCppCode(p, p->type, outfile); // PrimaryExp
		}
	}
    else if (type == "PrimaryExp") {
		if (p->type == "LPARENT") {
            (*outfile) << "(";
            p = p->next;
            genCppCode(p, p->type, outfile); // Exp
            (*outfile) << ")";
		} else if (p->type == "INTCON") {
            (*outfile) << p->s;
		} else {
            genCppCode(p, p->type, outfile); // LVal
		}
	}
    else if (type == "FuncRParams") {
        if (p->type == "Exp") {
            genCppCode(p, p->type, outfile); // Exp
            p = p->next;
            while (p != nullptr && p->type == "COMMA") {
                (*outfile) << ", ";
                p = p->next;
                genCppCode(p, p->type, outfile); // Exp
                p = p->next;
            }
        }
	}
    else if (type == "LVal") {
		if (p->type == "IDENFR") {
			(*outfile) << p->s;
            p = p->next;
		}
        // IDENFR '[' Exp ']'
		if (p != nullptr && p->type == "LBRACK") {
            p = p->next;
            (*outfile) << "[";
            genCppCode(p, p->type, outfile); // Exp
            (*outfile) << "]";
		}
	}
    else if (type == "Cond") {
        genCppCode(p, p->type, outfile); // LOrExp
	}
	else if (type == "LOrExp") {
        genCppCode(p, p->type, outfile); // LAndExp
        p = p->next;
        while (p != nullptr && p->type == "ORTK") {
            (*outfile) << " || ";
            p = p->next;
            genCppCode(p, p->type, outfile); // LAndExp
            p = p->next;
        }
	}
	else if (type == "LAndExp") { 
        genCppCode(p, p->type, outfile); // EqExp
        p = p->next;
        while (p != nullptr && p->type == "ANDTK") {
            (*outfile) << " && ";
            p = p->next;
            genCppCode(p, p->type, outfile); // EqExp
            p = p->next;
        }
	}
	else if (type == "EqExp") { 
        genCppCode(p, p->type, outfile); // RelExp
        p = p->next;
        while (p != nullptr && (p->type == "EQL" || p->type == "NEQ")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            genCppCode(p, p->type, outfile); // RelExp
            p = p->next;
        }
	}
	else if (type == "RelExp") { 
        genCppCode(p, p->type, outfile); // AddExp
        p = p->next;
        while (p != nullptr && (p->type == "GRE" || p->type == "LSS" || p->type == "GEQ" || p->type == "LEQ")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            genCppCode(p, p->type, outfile); // AddExp
            p = p->next;
        }
	}
    cout << "genCppCode " << type << " done" << endl;
}