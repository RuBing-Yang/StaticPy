#include "definition.h"
#include "datatype.h"

map<string, VarStack> varMap;
map<string, FuncList> funcMap;
map<string, UnaryClass> classMap;

UnaryFunc *nowDefFunc = nullptr;

map<string, int> generic_name2id;

void semanticAnalysis(ASTNODE *root, string type, NestDataType* expDataType, int level)
{
    if (root == nullptr) return;
    ASTNODE* p = root->first_child;if (root == nullptr) return;
    ASTNODE* p = root->first_child;
    if (type == "CompUnit") {
        // { [GenericDefs] (ClassDef | FuncDef) }
        while (p != nullptr) {
            semanticAnalysis(p, p->type, expDataType, level);
            if (p->type != "GereralDefs") {
                generic_name2id.clear();
                varMap.erase("self");
            }
            p = p->next;
        }
    }
    else if (type == "GenericDefs") {
        generic_name2id.clear();
        if (p->type == "GenericDef") {
            semanticAnalysis(p, p->type, expDataType, level);
            p = p->next;
            while (p != nullptr && p->type == "GenericDef") {
                semanticAnalysis(p, p->type, expDataType, level);
                p = p->next;
            }
        }
    }
    else if (type == "GenericDef") {
        // Ident '=' 'TypeVar' '(' Str ')'
        if (generic_name2id.find(p->s) != generic_name2id.end()) {
            cerr << "[line " << p->line << "] SemanticError: Generic name '" << p->s << "' has been defined!" << endl;
            exit(3);
        }
        if (classMap.find(p->s) != classMap.end()) {
            cerr << "[line " << p->line << "] SemanticError: Generic name '" << p->s << "' has been defined as a class name!" << endl;
            exit(3);
        }
        generic_name2id[p->s] = generic_name2id.size();
    }
	else if (type == "ClassDef") {
        // 'class' Ident ':' 'AddTab' {ClassAttrDef} [ClassInitDef] {ClassFuncDef} 'DelTab'
        p = p->next;
        string class_name = p->s;
        if (classMap.find(class_name) != classMap.end()) {
            cerr << "[line " << p->line << "] SemanticError: Class name '" << class_name << "' has been defined!" << endl;
            exit(3);
        }
        if (generic_name2id.find(class_name) != generic_name2id.end()) {
            cerr << "[line " << p->line << "] SemanticError: Class name '" << class_name << "' has been defined as a generic name!" << endl;
            exit(3);
        }
        UnaryClass unaryClass(generic_name2id);
        classMap[class_name] = unaryClass;
        UnaryDataType unaryDataType("class", class_name);
        varMap["self"] = VarStack(UnaryVar(level, unaryDataType));
        p = p->next->next->next;
        while (p != nullptr && p->type == "ClassAttrDef") {
            semanticAnalysis(p, p->type, expDataType, level);
            p = p->next;
        }
        while (p != nullptr && p->type == "ClassInitDef") {
            semanticAnalysis(p, p->type, expDataType, level);
            p = p->next;
        }
	}
	else if (type == "ClassAttrDef") {
        // Ident ':' DataType
        string attr_name = p->s;
        string class_name = varMap["self"].top().datatype_list(0).name;
        if (classMap[class_name].attrs.find(attr_name) != classMap[class_name].attrs.end()) {
            cerr << "[line " << p->line << "] SemanticError: Attribute name '" << attr_name << "' of class <" << class_name << "> has been defined!" << endl;
            exit(3);
        }
        while (p->type != "DataType") p = p->next;
        semanticAnalysis(p, p->type, expDataType, level);
        classMap[class_name].attrs[p->s] = *expDataType;
	}
	else if (type == "ClassInitDef") {
		// 'def' 'init' '(' [FuncFParams] ')' Block
        *nowDefFunc = UnaryFunc();
        p = p->next->next->next;
        if (p->type == "FuncFParams") {
            semanticAnalysis(p, p->type, expDataType, level + 1);
        }
        string class_name = varMap["self"].top().datatype_list(0).name;
        addFunc(classMap[class_name].funcMap, "init", *nowDefFunc, p->line);
        addFunc(funcMap, class_name, *nowDefFunc, p->line);
        while (p->type != "Block") p = p->next;
		semanticAnalysis(p, p->type, expDataType, level);
        *nowDefFunc = UnaryFunc();
	}
	else if (type == "ClassFuncDef") {
        // 'def' Ident '(' 'self' [',' FuncFParams] ')' '->' FuncType Block
        p = p->next;
        *nowDefFunc = UnaryFunc();
        string func_name = p->s;
        p = p->next->next->next->next;
        if (p->type == "FuncFParams") {
            semanticAnalysis(p, p->type, expDataType, level + 1);
        }
        while (p->type != "FuncType") p = p->next;
        semanticAnalysis(p, p->type, expDataType, level);
        string class_name = varMap["self"].top().datatype_list(0).name;
        addFunc(classMap[class_name].funcMap, func_name, *nowDefFunc, p->line);
        while (p->type != "Block") p = p->next;
		semanticAnalysis(p, p->type, expDataType, level);
        *nowDefFunc = UnaryFunc();
	}
    else if (type == "FuncDef") {
        // 'def' Ident '(' [FuncFParams] ')' '->' FuncType Block
        p = p->next;
        *nowDefFunc = UnaryFunc();
        string func_name = p->s;
        p = p->next->next;
        if (p->type == "FuncFParams") {
            semanticAnalysis(p, p->type, expDataType, level + 1);
        }
        while (p->type != "FuncType") p = p->next;
        semanticAnalysis(p, p->type, expDataType, level);
        addFunc(funcMap, func_name, *nowDefFunc, p->line);
        while (p->type != "Block") p = p->next;
		semanticAnalysis(p, p->type, expDataType, level);
        *nowDefFunc = UnaryFunc();
    }
    else if (type == "FuncType") {
        semanticAnalysis(p, p->type, expDataType, level);
        nowDefFunc->ret = *expDataType;
    }
    else if (type == "DataType") {
        if (expDataType == nullptr) {
            *expDataType = NestDataType();
        }
		if (p->type == "IDENFR") {
            // Ident [GenericReal]
            if (p->next != nullptr && p->next->type == "GenericReal") {
                expDataType->push(UnaryDataType("class", p->s));
                semanticAnalysis(p, p->type, expDataType, level);
            } else {
                if (generic_name2id.find(p->s) != generic_name2id.end()) {
                    vector<UnaryDataType> generics;
                    if (varMap.find("self") != varMap.end()) {
                        string class_name = varMap["self"].top().datatype_list(0).name;
                        generics = classMap[class_name].generics;
                    } else if (nowDefFunc != nullptr) {
                        generics = nowDefFunc->generics;
                    }
                    expDataType->push(generics[generic_name2id[p->s]]);
                } else if (classMap.find(p->s) != classMap.end()) {
                    expDataType->push(UnaryDataType("class", p->s));
                } else {
                    cerr << "[line " << p->line << "] SemanticError: Undefined class or generic identifier '" << p->s << "'!" << endl;
                    exit(3);
                }
            }
		}
        else if (p->type == "LISTTK") {
            // 'List' '[' DataType ']'
            expDataType->push(UnaryDataType("basic", "int"));
            p = p->next->next;
            semanticAnalysis(p, p->type, expDataType, level);  // DataType
        }
        else if (p->type == "DICTTK") {
            // 'Dict' '[' DataType ',' DataType ']'
            p = p->next->next;
            NestDataType *keyDataType;
            semanticAnalysis(p, p->type, keyDataType, level);  // DataType
            if (keyDataType->datatype_list.size() != 1 || keyDataType->datatype_list[0].type != "basic") {
                cerr << "[line " << p->line << "] SemanticError: Dict key type must be basic type!" << endl;
                exit(3);
            }
            expDataType->push(keyDataType->datatype_list[0]);
            p = p->next->next;
            semanticAnalysis(p, p->type, expDataType, level);  // DataType
        }
        else {
            // 'int' | 'float' | 'long' | 'str' | 'bool'
            expDataType->push(UnaryDataType("basic", p->s));
        }
    }
    else if (type == "Block") {
        while (p->type != "BlockItem") p = p->next;
        while (p != nullptr && p->type == "BlockItem") {
            semanticAnalysis(p, p->type, expDataType, level + 1);
            p = p->next;
        }
    }
    else if (type == "BlockItem") {
        // Decl | Stmt
        semanticAnalysis(p, p->type, expDataType, level);
    }
    else if (type == "Decl") {
        // Decl ::= Ident ':' DataType ['=' InitVal]
        string var_name = p->s;
        while (p->type != "DataType") p = p->next;
        semanticAnalysis(p, p->type, expDataType, level);
        addVar(varMap, var_name, UnaryVar(level, *expDataType), p->line);
        p = p->next;
        if (p != nullptr && p->type == "ASSIGN") {
            NestDataType *expDataType2;
            p = p->next;
            semanticAnalysis(p, p->type, expDataType2, level); // InitVal
            if ()
        }
        (*outfile) << ";" << endl;
    }
    else if (type == "InitVal") {
		if (p->type == "LBRACK") {  // List
            (*outfile) << "{";
            p = p->next;
            if (p->type == "InitVal") {
                semanticAnalysis(p, p->type, expDataType, level);
                p = p->next;
                while (p->type == "COMMA") {
                    p = p->next;
                    (*outfile) << ", ";
                    semanticAnalysis(p, p->type, expDataType, level);  // InitVal
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
                semanticAnalysis(p, p->type, expDataType, level);  // InitVal
                p = p->next;
                (*outfile) << ", ";  // COLON
                p = p->next;
                semanticAnalysis(p, p->type, expDataType, level);  // InitVal
                p = p->next;
                (*outfile) << "}";
                while (p->type == "COMMA") {
                    p = p->next;
                    (*outfile) << ", {";
                    semanticAnalysis(p, p->type, expDataType, level);  // InitVal
                    p = p->next;
                    (*outfile) << ", ";  // COLON
                    p = p->next;
                    semanticAnalysis(p, p->type, expDataType, level);  // InitVal
                    p = p->next;
                    (*outfile) << "}";
                }
            }
			(*outfile) << "}";
		}
		else {  // Exp
            semanticAnalysis(p, p->type, expDataType, level);
		}
    }
    else if (type == "FuncFParams") {
        semanticAnalysis(p, p->type, expDataType, level);
        p = p->next;
		while (p != nullptr && p->type == "COMMA") {
            p = p->next;
			semanticAnalysis(p, p->type, expDataType, level);
            p = p->next;
		}
    }
    else if (type == "FuncFParam") {
        // Ident ':' DataType
        string fparam_name = p->s;
        p = p->next->next;  // DataType
        semanticAnalysis(p, p->type, expDataType, level);
        nowDefFunc.fparams.push_back(*expDataType);
        addVar(varMap, p->s, UnaryVar(level, *expDataType), p->line);
    }
    else if (type == "Stmt") {
		if (p->type == "LVal") {
            if (p->next->type == "ASSIGN") {
                // LVal '=' Exp
                semanticAnalysis(p, p->type, expDataType, level); // LVal
                p = p->next->next;
                UnaryDataType *expDataType2;
                semanticAnalysis(p, p->type, expDataType2, level); // Exp
                (*outfile) << ";" << endl;
            } else {
                // LVal '.' 'append' '(' Exp ')'
                semanticAnalysis(p, p->type, expDataType, level); // LVal
                (*outfile) << ".push_back(";  // append
                p = p->next->next->next->next;
                semanticAnalysis(p, p->type, expDataType, level); // Exp
                (*outfile) << ");" << endl;
            }
		}
		else if (p->type == "IFTK") {
            // 'if' Exp Block ['else' Block]
            (*outfile) << prefix << "if (";
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // Exp
            (*outfile) << ")" << endl;
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // Block
            p = p->next;
            if (p != nullptr && p->type == "ELSETK") {
                (*outfile) << prefix << "else" << endl;
                p = p->next;
                semanticAnalysis(p, p->type, expDataType, level); // Block
            }
		}
		else if (p->type == "WHILETK") {
            // 'while' Exp Block
            (*outfile) << prefix << "while (";
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // Exp
            (*outfile) << ")" << endl;
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // Block
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
                semanticAnalysis(p, p->type, expDataType, level); // Exp
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
                semanticAnalysis(p, p->type, expDataType, level);
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
                    semanticAnalysis(p, p->type, expDataType, level); // Exp
                    (*outfile) << " ";
                    p = p->next;
                }
            }
            (*outfile) << "<< endl;" << endl;
        }
		else {
            (*outfile) << prefix;
			semanticAnalysis(p, p->type, expDataType, level); // Exp
            (*outfile) << ";" << endl;
		}
    }
    else if (type == "Exp") {
        semanticAnalysis(p, p->type, expDataType, level); // LOrExp

	}
	else if (type == "AddExp") {
        semanticAnalysis(p, p->type, expDataType, level); // MulExp
        p = p->next;
        while (p != nullptr && (p->type == "PLUS" || p->type == "MINU")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // MulExp
            p = p->next;
        }
	}
    else if (type == "MulExp") { 
        semanticAnalysis(p, p->type, expDataType, level); // UnaryExp
        p = p->next;
        while (p != nullptr && (p->type == "MULT" || p->type == "DIV" || p->type == "MOD")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // UnaryExp
            p = p->next;
        }
	}
    else if (type == "UnaryExp") {
        if (p->type == "UnaryOp") {
            (*outfile) << cpp_unaryop(p->s);
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // UnaryExp
        }
		else if (p->type == "IdentExp") {
            semanticAnalysis(p, p->type, expDataType, level);
		}
		else {
            semanticAnalysis(p, p->type, expDataType, level);  // PrimaryExp
		}
	}
    else if (type == "IdentExp") {
        // LVal [[GenericReal] '(' [FuncRParams] ')']
        semanticAnalysis(p, p->type, expDataType, level); // LVal
        p = p->next;
        if (p != nullptr && p->type == "GenericReal") {
            semanticAnalysis(p, p->type, expDataType, level);
            p = p->next;
        }
        if (p != nullptr && p->type == "LPARENT") {
            (*outfile) << "(";
            p = p->next;
            if (p->type == "FuncRParams") {
                semanticAnalysis(p, p->type, expDataType, level);  // FuncRParams
            }
            (*outfile) << ")";
        }
    }
    else if (type == "GenericReal") {
        // '<' DataType {',' DataType} '>'
        p = p->next;
        UnaryDataType generic_datatype = expDataType->datatype_list.back();
        while (p != nullptr && p->type == "DataType") {
            NestDataType *expDataType2;
            semanticAnalysis(p, p->type, expDataType2, level); // basic DataType
            if (expDataType2->datatype_list.size() != 1) {
                cerr << "[line " << p->line << "] SemanticError: Generic " << generic_datatype.name << " real datatype size " << expDataType2->datatype_list.size() << " is not 1!" << endl;
                exit(3);
            }
            UnaryDataType basic_type = expDataType2->datatype_list[0];
            if (basic_type.type != "basic") {
                cerr << "[line " << p->line << "] SemanticError: Generic " << generic_datatype.name << " real type " << basic_type.type << " is not basic!" << endl;
                exit(3);
            }
            generic_datatype.rgenerics.push_back(basic_type.name);
            p = p->next->next;
        }
    }
    else if (type == "PrimaryExp") {
		if (p->type == "LPARENT") {
            (*outfile) << "(";
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // Exp
            (*outfile) << ")";
		} else {  // INTCON | FLOATCON | StrConst | 'True' | 'False'
            if (p->type == "TRUETK") (*outfile) << "true";
            else if (p->type == "FALSETK") (*outfile) << "false";
            else (*outfile) << p->s;
		}
	}
    else if (type == "FuncRParams") {
        if (p->type == "Exp") {
            semanticAnalysis(p, p->type, expDataType, level); // Exp
            p = p->next;
            while (p != nullptr && p->type == "COMMA") {
                (*outfile) << ", ";
                p = p->next;
                semanticAnalysis(p, p->type, expDataType, level); // Exp
                p = p->next;
            }
        }
	}
    else if (type == "LVal") {
        // ['self' '.' ] Ident {'[' Exp ']'} {'.' Ident {'[' Exp ']'}}
        if (p->type == "SELFTK") {
            p = p->next;
            if (p->type == "DOT") {
                p = p->next;
            }
        }
        (*outfile) << p->s;
        p = p->next;
		while (p != nullptr && p->type == "LBRACK") {
            p = p->next;
            (*outfile) << "[";
            semanticAnalysis(p, p->type, expDataType, level); // Exp
            (*outfile) << "]";
            p = p->next->next;
		}
        while (p != nullptr && p->type == "DOT") {
            p = p->next;
            (*outfile) << "." << p->s;
            p = p->next;
            while (p != nullptr && p->type == "LBRACK") {
                p = p->next;
                (*outfile) << "[";
                semanticAnalysis(p, p->type, expDataType, level); // Exp
                (*outfile) << "]";
                p = p->next->next;
            }
        }
	}
	else if (type == "LOrExp") {
        semanticAnalysis(p, p->type, expDataType, level); // LAndExp
        p = p->next;
        while (p != nullptr && p->type == "ORTK") {
            (*outfile) << " || ";
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // LAndExp
            p = p->next;
        }
	}
	else if (type == "LAndExp") { 
        semanticAnalysis(p, p->type, expDataType, level); // EqExp
        p = p->next;
        while (p != nullptr && p->type == "ANDTK") {
            (*outfile) << " && ";
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // EqExp
            p = p->next;
        }
	}
	else if (type == "EqExp") { 
        semanticAnalysis(p, p->type, expDataType, level); // RelExp
        p = p->next;
        while (p != nullptr && (p->type == "EQL" || p->type == "NEQ")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // RelExp
            p = p->next;
        }
	}
	else if (type == "RelExp") { 
        semanticAnalysis(p, p->type, expDataType, level); // AddExp
        p = p->next;
        while (p != nullptr && (p->type == "GRE" || p->type == "LSS" || p->type == "GEQ" || p->type == "LEQ")) {
            (*outfile) << " " << p->s << " ";
            p = p->next;
            semanticAnalysis(p, p->type, expDataType, level); // AddExp
            p = p->next;
        }
	}
}