#include "definition.h"

string cpp_datatype(string type) {
    if (type == "INTTK") return "int";
    if (type == "FLOATTK") return "float";
    if (type == "NONETK") return "void";
    // if (type == "LISTTK") return "vector<int>";
    // if (type == "DICTTK") return "map<int, int>";
    return "";
}

string cpp_unaryop(string s) {
    if (s == "+") return "+";
    if (s == "-") return "-";
    if (s == "not") return "!";
    return "";
}

void genCppCode(ASTNODE *root, string type, ofstream *outfile, string prefix){
    if (root == nullptr) return;
    ASTNODE* p = root->first_child;
    if (type == "CompUnit") {
        (*outfile) << "#include <iostream>" << endl;
        (*outfile) << "#include <map>" << endl;
        (*outfile) << "#include <vector>" << endl;
        (*outfile) << "using namespace std;" << endl;
        while (p != nullptr) {
            (*outfile) << endl;
            genCppCode(p, p->type, outfile, prefix);
            p = p->next;
        }
    }
    else if (type == "FuncDef") {
        // 'def' Ident '(' [FuncFParams] ')' '->' FuncType Block
        while (p->type != "FuncType") p = p->next;
        genCppCode(p, p->type, outfile, prefix);
        p = root->first_child->next;
        string func_name = p->s;
        (*outfile) << " " << func_name << "(";
        p = p->next->next;
        if (p->type == "FuncFParams") {
            genCppCode(p, p->type, outfile, prefix);
        }
        (*outfile) << ")" << endl;
        while (p->type != "Block") p = p->next;
		genCppCode(p, p->type, outfile, prefix);
    }
    else if (type == "FuncType") {
        if (p->type == "NONETK") {
            (*outfile) << "void";
        }
        else {  // DataType
            genCppCode(p, p->type, outfile, prefix);
        }
    }
    else if (type == "DataType") {
        if (p->type == "LISTTK") {
            // 'List' '[' DataType ']'
            (*outfile) << "vector<";
            p = p->next->next;
            genCppCode(p, p->type, outfile, prefix);  // DataType
            (*outfile) << ">";
        }
        else if (p->type == "DICTTK") {
            // 'Dict' '[' DataType ',' DataType ']'
            (*outfile) << "map<";
            p = p->next->next;
            genCppCode(p, p->type, outfile, prefix);  // DataType
            (*outfile) << ",";
            p = p->next->next;
            genCppCode(p, p->type, outfile, prefix);  // DataType
            (*outfile) << ">";
        }
        else {
            (*outfile) << cpp_datatype(p->type);
        }
    }
    else if (type == "Block") {
        (*outfile) << prefix << "{" << endl;
        while (p->type != "BlockItem") p = p->next;
        while (p != nullptr && p->type == "BlockItem") {
            genCppCode(p, p->type, outfile, prefix + "    ");
            p = p->next;
        }
        (*outfile) << prefix << "}" << endl;
    }
    else if (type == "BlockItem") {
        // Decl | Stmt
        genCppCode(p, p->type, outfile, prefix);
    }
    else if (type == "Decl") {
        // Decl ::= Ident ':' DataType ['=' InitVal]
        (*outfile) << prefix;
        while (p->type != "DataType") p = p->next;
        genCppCode(p, p->type, outfile, prefix);
        p = root->first_child;
        (*outfile) << " " << p->s;
        while (p->type != "DataType") p = p->next;
        p = p->next;
        if (p != nullptr && p->type == "ASSIGN") {
            (*outfile) << " = ";
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // InitVal
        }
        (*outfile) << ";" << endl;
    }
    else if (type == "InitVal") {
		if (p->type == "LBRACK") {  // List
            (*outfile) << "{";
            p = p->next;
            if (p->type == "InitVal") {
                genCppCode(p, p->type, outfile, prefix);
                p = p->next;
                while (p->type == "COMMA") {
                    p = p->next;
                    (*outfile) << ", ";
                    genCppCode(p, p->type, outfile, prefix);  // InitVal
                    p = p->next;
                }
            }
			(*outfile) << "}";
		}
        else if (p->type == "LBRACE") {  // Dict
            (*outfile) << "{";
            p = p->next;
            if (p->type == "InitVal") {
                (*outfile) << "{";
                genCppCode(p, p->type, outfile, prefix);  // InitVal
                p = p->next;
                (*outfile) << ", ";  // COLON
                p = p->next;
                genCppCode(p, p->type, outfile, prefix);  // InitVal
                p = p->next;
                (*outfile) << "}";
                while (p->type == "COMMA") {
                    p = p->next;
                    (*outfile) << ", {";
                    genCppCode(p, p->type, outfile, prefix);  // InitVal
                    p = p->next;
                    (*outfile) << ", ";  // COLON
                    p = p->next;
                    genCppCode(p, p->type, outfile, prefix);  // InitVal
                    p = p->next;
                    (*outfile) << "}";
                }
            }
			(*outfile) << "}";
		}
		else {  // Exp
            genCppCode(p, p->type, outfile, prefix);
		}
    }
    else if (type == "FuncFParams") {
        genCppCode(p, p->type, outfile, prefix);
        p = p->next;
		while (p != nullptr && p->type == "COMMA") {
            (*outfile) << ", ";
            p = p->next;
			genCppCode(p, p->type, outfile, prefix);
            p = p->next;
		}
    }
    else if (type == "FuncFParam") {
        // Ident ':' DataType
        p = p->next->next;  // DataType
        genCppCode(p, p->type, outfile, prefix);
        p = root->first_child;
        (*outfile) << " " << p->s;
    }
    else if (type == "Stmt") {
		if (p->type == "LVal") {
            (*outfile) << prefix;
            genCppCode(p, p->type, outfile, prefix); // LVal
            (*outfile) << " = ";
            p = p->next->next;
            genCppCode(p, p->type, outfile, prefix); // Exp
            (*outfile) << ";" << endl;
		}
		else if (p->type == "IFTK") {
            // 'if' Exp Block ['else' Block]
            (*outfile) << prefix << "if (";
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // Exp
            (*outfile) << ")" << endl;
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // Block
            p = p->next;
            if (p != nullptr && p->type == "ELSETK") {
                (*outfile) << prefix << "else" << endl;
                p = p->next;
                genCppCode(p, p->type, outfile, prefix); // Block
            }
		}
		else if (p->type == "WHILETK") {
            // 'while' Exp Block
            (*outfile) << prefix << "while (";
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // Exp
            (*outfile) << ")" << endl;
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // Block
		}
		else if (p->type == "BREAKTK") {
            (*outfile) << prefix << "break;" << endl;
		}
		else if (p->type == "CONTINUETK") {
            (*outfile) << prefix << "continue;" << endl;
		}
		else if (p->type == "RETURNTK") {
            (*outfile) << prefix << "return";
            p = p->next;
            if (p != nullptr && p->type == "Exp") {
                (*outfile) << " ";
                genCppCode(p, p->type, outfile, prefix); // Exp
            }
            (*outfile) << ";" << endl;
		}
        else if (p->type == "PRINTTK") {
            // 'print' '(' [Exp {',' Exp}] ')'
            (*outfile) << prefix << "cout ";
            p = p->next->next;
            if (p->type == "STRCON") {
                (*outfile) << "<< " << p->s << " ";
                p = p->next;
            }
            else if (p->type == "Exp") {
                (*outfile) << "<< ";
                genCppCode(p, p->type, outfile, prefix);
                (*outfile) << " ";
                p = p->next;
            }
            while (p != nullptr && p->type == "COMMA") {
                p = p->next;
                if (p->type == "STRCON") {
                    (*outfile) << "<< " << p->s << " ";
                    p = p->next;
                }
                else if (p->type == "Exp") {
                    (*outfile) << "<< ";
                    genCppCode(p, p->type, outfile, prefix); // Exp
                    (*outfile) << " ";
                    p = p->next;
                }
            }
            (*outfile) << "<< endl;" << endl;
        }
		else {
            (*outfile) << prefix;
			genCppCode(p, p->type, outfile, prefix); // Exp
            (*outfile) << ";" << endl;
		}
    }
    else if (type == "Exp") { 
        genCppCode(p, p->type, outfile, prefix); // LOrExp
	}
	else if (type == "AddExp") {
        genCppCode(p, p->type, outfile, prefix); // MulExp
        p = p->next;
        while (p != nullptr && (p->type == "PLUS" || p->type == "MINU")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // MulExp
            p = p->next;
        }
	}
    else if (type == "MulExp") { 
        genCppCode(p, p->type, outfile, prefix); // UnaryExp
        p = p->next;
        while (p != nullptr && (p->type == "MULT" || p->type == "DIV" || p->type == "MOD")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // UnaryExp
            p = p->next;
        }
	}
    else if (type == "UnaryExp") {
        if (p->type == "UnaryOp") {
            (*outfile) << cpp_unaryop(p->s);
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // UnaryExp
        }
		else if (p->type == "IDENFR" && p->next != nullptr && p->next->type == "LPARENT") {
            // 'IDENFR' '(' [FuncRParams] ')'
            (*outfile) << p->s << "(";
            p = p->next->next;
            if (p != nullptr && p->type == "FuncRParams") {
                genCppCode(p, p->type, outfile, prefix); // FuncRParams
            }
            (*outfile) << ")";
		}
		else {
            genCppCode(p, p->type, outfile, prefix); // PrimaryExp
		}
	}
    else if (type == "PrimaryExp") {
		if (p->type == "LPARENT") {
            (*outfile) << "(";
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // Exp
            (*outfile) << ")";
		} else if (p->type == "INTCON" || p->type == "FLOATCON") {
            (*outfile) << p->s;
		} else {
            genCppCode(p, p->type, outfile, prefix); // LVal
		}
	}
    else if (type == "FuncRParams") {
        if (p->type == "Exp") {
            genCppCode(p, p->type, outfile, prefix); // Exp
            p = p->next;
            while (p != nullptr && p->type == "COMMA") {
                (*outfile) << ", ";
                p = p->next;
                genCppCode(p, p->type, outfile, prefix); // Exp
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
		while (p != nullptr && p->type == "LBRACK") {
            p = p->next;
            (*outfile) << "[";
            genCppCode(p, p->type, outfile, prefix); // Exp
            (*outfile) << "]";
            p = p->next->next;
		}
	}
	else if (type == "LOrExp") {
        genCppCode(p, p->type, outfile, prefix); // LAndExp
        p = p->next;
        while (p != nullptr && p->type == "ORTK") {
            (*outfile) << " || ";
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // LAndExp
            p = p->next;
        }
	}
	else if (type == "LAndExp") { 
        genCppCode(p, p->type, outfile, prefix); // EqExp
        p = p->next;
        while (p != nullptr && p->type == "ANDTK") {
            (*outfile) << " && ";
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // EqExp
            p = p->next;
        }
	}
	else if (type == "EqExp") { 
        genCppCode(p, p->type, outfile, prefix); // RelExp
        p = p->next;
        while (p != nullptr && (p->type == "EQL" || p->type == "NEQ")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // RelExp
            p = p->next;
        }
	}
	else if (type == "RelExp") { 
        genCppCode(p, p->type, outfile, prefix); // AddExp
        p = p->next;
        while (p != nullptr && (p->type == "GRE" || p->type == "LSS" || p->type == "GEQ" || p->type == "LEQ")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            genCppCode(p, p->type, outfile, prefix); // AddExp
            p = p->next;
        }
	}
}