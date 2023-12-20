#include "definition.h"

ASTNODE* creatNode(ASTNODE* root, string s, string type) {
	ASTNODE *p = new ASTNode(s, type);
	if (root->first_child == nullptr) {
		root->first_child = root->last_child = p;
	}
	else {
		root->last_child->next = p;
		root->last_child = p;
	}
	return p;
}

int nextToken(TOKEN **token, ofstream* outfile) {
	if (outfile != nullptr)
		(*outfile) << (*token)->type << " " << (*token)->s << endl;
	*token = (*token)->next;
	if ((*token) == nullptr) return 1;
	else return 0;
}

bool isDataType(string type) {
	if (type == "INTTK" || type == "FLOATTK" || type == "LISTTK" || type == "DICTTK" || type == "STRTK" || type == "BOOLTK" || type == "LONGTK") return true;
	return false;
}

bool checkToken(string token_type, string target_type, string grammar_type) {
	if (token_type == target_type) return true;
	cerr << "SyntaxError: <" << grammar_type << "> Expect " << target_type << " but get " << token_type << endl;
	exit(2);
	return false;
}

void grammarAnalysis(TOKEN **token, string type, ASTNODE *root, ofstream *outfile)
{
    ASTNODE *p;
	TOKEN *t;
	if ((*token) == nullptr) return;
    if (type == "CompUnit") {
		while ((*token) != nullptr) {
			if ((*token)->type == "IDENFR") {
				p = creatNode(root, "", "GenericDefs");
				grammarAnalysis(token, "GenericDefs", p, outfile);
			}
			else if ((*token)->type == "DEFTK") {
				p = creatNode(root, "", "FuncDef");
				grammarAnalysis(token, "FuncDef", p, outfile);
			}
			else if ((*token)->type == "CLASSTK") {
				p = creatNode(root, "", "ClassDef");
				grammarAnalysis(token, "ClassDef", p, outfile);
			}
		}
    }
	else if (type == "GenericDefs") {
		while ((*token) != nullptr && (*token)->type == "IDENFR") {
			p = creatNode(root, "", "GenericDef");
			grammarAnalysis(token, "GenericDef", p, outfile);
		}
	}
	else if (type == "GenericDef") {
		if (checkToken((*token)->type, "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "ASSIGN", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "TYPEVARTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "LPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "STRCON", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "RPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
	}
	else if (type == "ClassDef") {
		// 'class' Ident ':' 'AddTab' {ClassAttrDef} [ClassInitDef] {ClassFuncDef} 'DelTab'
		if (checkToken((*token)->type, "CLASSTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "COLON", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "ADDTAB", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
		}
		while ((*token)->type == "IDENFR") {
			p = creatNode(root, "", "ClassAttrDef");
			grammarAnalysis(token, "ClassAttrDef", p, outfile);
		}
		while ((*token)->type == "DEFTK") {
			if ((*token)->next->type == "INITTK") {
				p = creatNode(root, "", "ClassInitDef");
				grammarAnalysis(token, "ClassInitDef", p, outfile);
			} else {
				p = creatNode(root, "", "ClassFuncDef");
				grammarAnalysis(token, "ClassFuncDef", p, outfile);
			}
		}
		if (checkToken((*token)->type, "DELTAB", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
		}
	}
	else if (type == "ClassAttrDef") {
		if (checkToken((*token)->type, "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "COLON") {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "DataType");
			grammarAnalysis(token, "DataType", p, outfile);
		}
	}
	else if (type == "ClassInitDef") {
		// 'def' 'init' '(' [FuncFParams] ')' Block
		if (checkToken((*token)->type, "DEFTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
		if (checkToken((*token)->type, "INITTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
		if (checkToken((*token)->type, "LPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "RPARENT") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		else {
			p = creatNode(root, "", "FuncFParams");
			grammarAnalysis(token, "FuncFParams", p, outfile);
			if (checkToken((*token)->type, "RPARENT", type)) {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		p = creatNode(root, "", "Block");
		grammarAnalysis(token, "Block", p, outfile);
	}
	else if (type == "ClassFuncDef") {
		// 'def' Ident '(' 'self' [',' FuncFParams] ')' '->' FuncType Block
        if (checkToken((*token)->type, "DEFTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
		if (checkToken((*token)->type, "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "LPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "SELFTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "COMMA") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "FuncFParams");
			grammarAnalysis(token, "FuncFParams", p, outfile);
		}
		if (checkToken((*token)->type, "RPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
        if (checkToken((*token)->type, "ARROW", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
        if ((*token)->type == "NONETK" || isDataType((*token)->type) || (*token)->type == "IDENFR") {
			p = creatNode(root, "", "FuncType");
			grammarAnalysis(token, "FuncType", p, outfile);
        }
		p = creatNode(root, "", "Block");
		grammarAnalysis(token, "Block", p, outfile);
	}
    else if (type == "FuncDef") {
        if (checkToken((*token)->type, "DEFTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
		if (checkToken((*token)->type, "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "LPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "RPARENT") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		else {
			p = creatNode(root, "", "FuncFParams");
			grammarAnalysis(token, "FuncFParams", p, outfile);
			if (checkToken((*token)->type, "RPARENT", type)) {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
        if (checkToken((*token)->type, "ARROW", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
        if ((*token)->type == "NONETK" || isDataType((*token)->type) || (*token)->type == "IDENFR") {
			p = creatNode(root, "", "FuncType");
			grammarAnalysis(token, "FuncType", p, outfile);
        }
		p = creatNode(root, "", "Block");
		grammarAnalysis(token, "Block", p, outfile);
    }
    else if (type == "FuncType") {
        if ((*token)->type == "NONETK") {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
		} else {
			p = creatNode(root, "", "DataType");
			grammarAnalysis(token, "DataType", p, outfile);
		}
    }
    else if (type == "DataType") {
		if ((*token)->type == "IDENFR") {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
		}
        else if ((*token)->type == "LISTTK") {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
			if ((*token)->type == "LBRACK") {
				p = creatNode(root, (*token)->s, (*token)->type);
            	if (nextToken(&(*token), outfile)) return;
			}
			p = creatNode(root, "", "DataType");
			grammarAnalysis(token, "DataType", p, outfile);
			if (checkToken((*token)->type, "RBRACK", type)) {
				p = creatNode(root, (*token)->s, (*token)->type);
            	if (nextToken(&(*token), outfile)) return;
			}
		}
		else if ((*token)->type == "DICTTK") {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
			if ((*token)->type == "LBRACK") {
				p = creatNode(root, (*token)->s, (*token)->type);
            	if (nextToken(&(*token), outfile)) return;
			}
			p = creatNode(root, "", "DataType");
			grammarAnalysis(token, "DataType", p, outfile);
			if ((*token)->type == "COMMA") {
				p = creatNode(root, (*token)->s, (*token)->type);
            	if (nextToken(&(*token), outfile)) return;
				p = creatNode(root, "", "DataType");
				grammarAnalysis(token, "DataType", p, outfile);
			}
			if ((*token)->type == "RBRACK") {
				p = creatNode(root, (*token)->s, (*token)->type);
            	if (nextToken(&(*token), outfile)) return;
			}
		}
		else {
			// INTTK | FLOATTK
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
    }
    else if (type == "Block") {
		if (checkToken((*token)->type, "COLON", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "ADDTAB", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		while ((*token) != nullptr && (*token)->type != "DELTAB") {
			p = creatNode(root, "", "BlockItem");
			grammarAnalysis(token, "BlockItem", p, outfile);
		}
		if (checkToken((*token)->type, "DELTAB", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			nextToken(&(*token), outfile);
		}
    }
    else if (type == "BlockItem") {
		if ((*token)->type == "IDENFR" && (*token)->next != nullptr && (*token)->next->type == "COLON") {
			p = creatNode(root, "", "Decl");
			grammarAnalysis(token, "Decl", p, outfile);
		}
		else {
			p = creatNode(root, "", "Stmt");
			grammarAnalysis(token, "Stmt", p, outfile);
		}
    }
    else if (type == "Decl") {
		if (checkToken((*token)->type, "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "COLON", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "DataType");
			grammarAnalysis(token, "DataType", p, outfile);
		}
		if (checkToken((*token)->type, "ASSIGN", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
            p = creatNode(root, "", "InitVal");
            grammarAnalysis(token, "InitVal", p, outfile);
		}
    }
    else if (type == "InitVal") {
		if ((*token)->type == "LBRACK") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
            if ((*token)->type != "RBRACK") {
                p = creatNode(root, "", "InitVal");
                grammarAnalysis(token, "InitVal", p, outfile);
                while ((*token) != nullptr && (*token)->type == "COMMA") {
                    p = creatNode(root, (*token)->s, (*token)->type);
                    if (nextToken(&(*token), outfile)) return; 
                    p = creatNode(root, "", "InitVal");
                    grammarAnalysis(token, "InitVal", p, outfile);
                }
            }
			if (checkToken((*token)->type, "RBRACK", type)) {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		else if ((*token)->type == "LBRACE") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
            if ((*token)->type != "RBRACE") {
                p = creatNode(root, "", "InitVal");
                grammarAnalysis(token, "InitVal", p, outfile);
				if (checkToken((*token)->type, "COLON", type)) {
					p = creatNode(root, (*token)->s, (*token)->type);
					if (nextToken(&(*token), outfile)) return;
				}
                p = creatNode(root, "", "InitVal");
                grammarAnalysis(token, "InitVal", p, outfile);
                while ((*token) != nullptr && (*token)->type == "COMMA") {
                    p = creatNode(root, (*token)->s, (*token)->type);
                    if (nextToken(&(*token), outfile)) return; 
                    p = creatNode(root, "", "InitVal");
                    grammarAnalysis(token, "InitVal", p, outfile);
					if (checkToken((*token)->type, "COLON", type)) {
						p = creatNode(root, (*token)->s, (*token)->type);
						if (nextToken(&(*token), outfile)) return;
					}
                    p = creatNode(root, "", "InitVal");
                    grammarAnalysis(token, "InitVal", p, outfile);
                }
            }
			if (checkToken((*token)->type, "RBRACE", type)) {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		else { 
			p = creatNode(root, "", "Exp");
			grammarAnalysis(token, "Exp", p, outfile);
		}
    }
    else if (type == "FuncFParams") {
		p = creatNode(root, "", "FuncFParam");
		grammarAnalysis(token, "FuncFParam", p, outfile);
		while ((*token) != nullptr && (*token)->type == "COMMA") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "FuncFParam");
			grammarAnalysis(token, "FuncFParam", p, outfile);
		}
    }
    else if (type == "FuncFParam") {
		if (checkToken((*token)->type, "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token)->type, "COLON", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "DataType");
			grammarAnalysis(token, "DataType", p, outfile);
		}
    }
    else if (type == "Stmt") {
		if ((*token)->type == "IDENFR" || (*token)->type == "SELFTK") {
            TOKEN* tmp = (*token)->next;
            string stmt_type = "Exp";
            while (tmp != nullptr) {
				if (tmp->type == "ASSIGN") {
					stmt_type = "assign";
					break;
				}
				if (tmp->type == "APPENDTK") {
					stmt_type = "append";
					break;
				}
				if (tmp->type == "LBRACK") {
					while (tmp->type != "RBRACK") tmp = tmp->next;
					tmp = tmp->next;
				}
				else if (tmp->type == "DOT") {
					tmp = tmp->next;
					if (tmp->type == "IDENFR") tmp = tmp->next;
				}
				else break;
			}
            if (stmt_type == "assign") {
                p = creatNode(root, "", "LVal");
                grammarAnalysis(token, "LVal", p, outfile);
                if (checkToken((*token)->type, "ASSIGN", type)) {
                    p = creatNode(root, (*token)->s, (*token)->type);
                    if (nextToken(&(*token), outfile)) return;
                } 
                p = creatNode(root, "", "Exp");
                grammarAnalysis(token, "Exp", p, outfile);
            }
			else if (stmt_type == "append") {
				// LVal '.' 'append' '(' Exp ')'
                p = creatNode(root, "", "LVal");
                grammarAnalysis(token, "LVal", p, outfile);
                if (checkToken((*token)->type, "DOT", type)) {
                    p = creatNode(root, (*token)->s, (*token)->type);
                    if (nextToken(&(*token), outfile)) return;
                } 
                if (checkToken((*token)->type, "APPENDTK", type)) {
                    p = creatNode(root, (*token)->s, (*token)->type);
                    if (nextToken(&(*token), outfile)) return;
                } 
                if (checkToken((*token)->type, "LPARENT", type)) {
                    p = creatNode(root, (*token)->s, (*token)->type);
                    if (nextToken(&(*token), outfile)) return;
                } 
                p = creatNode(root, "", "Exp");
                grammarAnalysis(token, "Exp", p, outfile);
                if (checkToken((*token)->type, "RPARENT", type)) {
                    p = creatNode(root, (*token)->s, (*token)->type);
                    if (nextToken(&(*token), outfile)) return;
                }
            } 
			else {
                p = creatNode(root, "", "Exp");
                grammarAnalysis(token, "Exp", p, outfile);
            }
		}
		else if ((*token)->type == "IFTK") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "Exp");
			grammarAnalysis(token, "Exp", p, outfile);
			p = creatNode(root, "", "Block");
			grammarAnalysis(token, "Block", p, outfile);
			if ((*token)->type == "ELSETK") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return; 
				p = creatNode(root, "", "Block");
				grammarAnalysis(token, "Block", p, outfile);
			}
		}
		else if ((*token)->type == "WHILETK") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "Exp");
			grammarAnalysis(token, "Exp", p, outfile);
			p = creatNode(root, "", "Block");
			grammarAnalysis(token, "Block", p, outfile);
		}
		else if ((*token)->type == "BREAKTK") {
		    p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		else if ((*token)->type == "CONTINUETK") {
		    p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		else if ((*token)->type == "RETURNTK") {
		    p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			if ((*token)->type != "DELTAB") {
				p = creatNode(root, "", "Exp");
				grammarAnalysis(token, "Exp", p, outfile);
			}
		}
        else if ((*token)->type == "PRINTTK") {
            p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			if (checkToken((*token)->type, "LPARENT", type)) {
                p = creatNode(root, (*token)->s, (*token)->type);
                if (nextToken(&(*token), outfile)) return;
			}
			if ((*token)->type != "RPARENT") {
                if ((*token)->type == "STRCON") {
                    p = creatNode(root, (*token)->s, (*token)->type);
                    if (nextToken(&(*token), outfile)) return;
                }
                else {
                    p = creatNode(root, "", "Exp");
                    grammarAnalysis(token, "Exp", p, outfile);
                }
                while ((*token)->type == "COMMA") {
                    p = creatNode(root, (*token)->s, (*token)->type);
                    if (nextToken(&(*token), outfile)) return;
                    if ((*token)->type == "STRCON") {
                        p = creatNode(root, (*token)->s, (*token)->type);
                        if (nextToken(&(*token), outfile)) return;
                    }
                    else {
                        p = creatNode(root, "", "Exp");
                        grammarAnalysis(token, "Exp", p, outfile);
                    }
                }
			}
			if (checkToken((*token)->type, "RPARENT", type)) {
                p = creatNode(root, (*token)->s, (*token)->type);
                if (nextToken(&(*token), outfile)) return;
			}
        }
		else {
			p = creatNode(root, "", "Exp");
			grammarAnalysis(token, "Exp", p, outfile);
		}
    }
    else if (type == "Exp") { 
		p = creatNode(root, "", "LOrExp");
		grammarAnalysis(token, "LOrExp", p, outfile);
	}
	else if (type == "AddExp") { 
		p = creatNode(root, "", "MulExp");
		grammarAnalysis(token, "MulExp", p, outfile);
		while ((*token) != nullptr && ((*token)->type == "PLUS" || (*token)->type == "MINU")) {
			if (outfile != nullptr) (*outfile) << "<AddExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "MulExp");
			grammarAnalysis(token, "MulExp", p, outfile);
			//(*outfile) << "<AddExp>" << endl;
		}
	}
    else if (type == "MulExp") { 
		p = creatNode(root, "", "UnaryExp");
		grammarAnalysis(token, "UnaryExp", p, outfile);
		while ((*token) != nullptr && ((*token)->type == "MULT" || (*token)->type == "DIV" || (*token)->type == "MOD")) {
			if (outfile != nullptr) (*outfile) << "<MulExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "UnaryExp");
			grammarAnalysis(token, "UnaryExp", p, outfile);
		}
	}
    else if (type == "UnaryExp") {
		if ((*token)->type == "PLUS" || (*token)->type == "MINU" || (*token)->type == "NOTTK") { 
			p = creatNode(root, "", "UnaryOp");
            if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "UnaryExp");
			grammarAnalysis(token, "UnaryExp", p, outfile);
		}
		else if ((*token)->type == "IDENFR" || (*token)->type == "SELFTK") {
			p = creatNode(root, "", "IdentExp");
			grammarAnalysis(token, "IdentExp", p, outfile);
		}
		else { 
			p = creatNode(root, "", "PrimaryExp");
			grammarAnalysis(token, "PrimaryExp", p, outfile);
		}
	}
	else if (type == "IdentExp") {
		// LVal [[GenericReal] '(' [FuncRParams] ')']
		p = creatNode(root, "", "LVal");
		grammarAnalysis(token, "LVal", p, outfile);
		if ((*token)->type == "LSS" && (*token)->next != nullptr && isDataType((*token)->next->type)) {
			p = creatNode(root, "", "GenericReal");
			grammarAnalysis(token, "GenericReal", p, outfile);
		}
		if ((*token)->type == "LPARENT") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			if ((*token)->type == "RPARENT") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			} else {
				p = creatNode(root, "", "FuncRParams");
				grammarAnalysis(token, "FuncRParams", p, outfile);
				if (checkToken((*token)->type, "RPARENT", type)) {
					p = creatNode(root, (*token)->s, (*token)->type);
					if (nextToken(&(*token), outfile)) return;
				}
			}
		} 
	}
	else if (type == "GenericReal") {
		// '<' DataType {',' DataType} '>'
		// 不支持再嵌套别的泛型
		if ((*token)->type == "LSS") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "DataType");
			grammarAnalysis(token, "DataType", p, outfile);
			while ((*token) != nullptr && (*token)->type == "COMMA") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return; 
				p = creatNode(root, "", "DataType");
				grammarAnalysis(token, "DataType", p, outfile);
			}
			if (checkToken((*token)->type, "GRE", type)) {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
	}
    else if (type == "PrimaryExp") {
		// '(' Exp ')' | IntConst | FloatConst | StrConst | 'True' | 'False'
		if ((*token)->type == "LPARENT") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "Exp");
			grammarAnalysis(token, "Exp", p, outfile);
			if (checkToken((*token)->type, "RPARENT", type)) {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		else {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(token, outfile)) return;
		}
	}
    else if (type == "FuncRParams") { 
		p = creatNode(root, "", "Exp");
		grammarAnalysis(token, "Exp", p, outfile);
		while ((*token) != nullptr && (*token)->type == "COMMA") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(token, outfile)) return; 
			p = creatNode(root, "", "Exp");
			grammarAnalysis(token, "Exp", p, outfile);
		}
	}
    else if (type == "LVal") {
		// ['self' '.' ] Ident {'[' Exp ']'} {'.' Ident {'[' Exp ']'}}
		if ((*token)->type == "SELFTK") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			if (checkToken((*token)->type, "DOT", type)) {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		if (checkToken((*token)->type, "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		while ((*token) != nullptr && (*token)->type == "LBRACK") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "Exp");
			grammarAnalysis(token, "Exp", p, outfile);
			if (checkToken((*token)->type, "RBRACK", type)) {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		while ((*token) != nullptr && (*token)->type == "DOT" && (*token)->next != nullptr && (*token)->next->type == "IDENFR") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			if (checkToken((*token)->type, "IDENFR", type)) {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
			while ((*token) != nullptr && (*token)->type == "LBRACK") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return; 
				p = creatNode(root, "", "Exp");
				grammarAnalysis(token, "Exp", p, outfile);
				if (checkToken((*token)->type, "RBRACK", type)) {
					p = creatNode(root, (*token)->s, (*token)->type);
					if (nextToken(&(*token), outfile)) return;
				}
			}
		}
	}
	else if (type == "LOrExp") { 
		p = creatNode(root, "", "LAndExp");
		grammarAnalysis(token, "LAndExp", p, outfile);
		while ((*token) != nullptr && (*token)->type == "ORTK") {
			if (outfile != nullptr) (*outfile) << "<LOrExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "LAndExp");
			grammarAnalysis(token, "LAndExp", p, outfile);
		}
	}
	else if (type == "LAndExp") { 
		p = creatNode(root, "", "EqExp");
		grammarAnalysis(token, "EqExp", p, outfile);
		while ((*token) != nullptr && (*token)->type == "ANDTK") {
			if (outfile != nullptr) (*outfile) << "<LAndExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "EqExp");
			grammarAnalysis(token, "EqExp", p, outfile);
		}
	}
	else if (type == "EqExp") { 
		p = creatNode(root, "", "RelExp");
		grammarAnalysis(token, "RelExp", p, outfile);
		while ((*token) != nullptr && ((*token)->type == "EQL" || (*token)->type == "NEQ")) {
			if (outfile != nullptr) (*outfile) << "<EqExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "RelExp");
			grammarAnalysis(token, "RelExp", p, outfile);
		}
	}
	else if (type == "RelExp") { 
		p = creatNode(root, "", "AddExp");
		grammarAnalysis(token, "AddExp", p, outfile);
		while ((*token) != nullptr && ((*token)->type == "GRE" || (*token)->type == "LSS" || (*token)->type == "GEQ" || (*token)->type == "LEQ")) {
			if (outfile != nullptr) (*outfile) << "<RelExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "AddExp");
			grammarAnalysis(token, "AddExp", p, outfile);
		}
	}
	if (type != "BlockItem" && outfile != nullptr) 
        (*outfile) << "<" << type << ">" << endl;
}