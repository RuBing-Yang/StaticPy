#include "definition.h"

CSTNode* creatNode(CSTNode* root, string s, string type, int line) {
	CSTNode *p = new CSTNode(s, type, line);
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

bool checkToken(TOKEN *token, string target_type, string grammar_type) {
	if (token->type == target_type) return true;
	cerr << "[line " << token->line << "] SyntaxError: <" << grammar_type << "> Expect " << target_type << " but get " << token->type << endl;
	exit(2);
	return false;
}

void grammarAnalysis(TOKEN **token, string type, CSTNode *root, ofstream *outfile, int DEBUG)
{
    CSTNode *p;
	TOKEN *t;
	if ((*token) == nullptr) return;
    if (type == "CompUnit") {
		while ((*token) != nullptr) {
			if ((*token)->type == "IDENFR") {
				p = creatNode(root, "", "GenericDefs", (*token)->line);
				grammarAnalysis(token, "GenericDefs", p, outfile, DEBUG);
			}
			else if ((*token)->type == "DEFTK") {
				p = creatNode(root, "", "FuncDef", (*token)->line);
				grammarAnalysis(token, "FuncDef", p, outfile, DEBUG);
			}
			else if ((*token)->type == "CLASSTK") {
				p = creatNode(root, "", "ClassDef", (*token)->line);
				grammarAnalysis(token, "ClassDef", p, outfile, DEBUG);
			}
		}
        cout << "Grammar Analysis OK!" << endl;
    }
	else if (type == "GenericDefs") {
		while ((*token) != nullptr && (*token)->type == "IDENFR") {
			p = creatNode(root, "", "GenericDef", (*token)->line);
			grammarAnalysis(token, "GenericDef", p, outfile, DEBUG);
		}
	}
	else if (type == "GenericDef") {
		if (checkToken((*token), "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "ASSIGN", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "TYPEVARTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "LPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "STRCON", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "RPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
	}
	else if (type == "ClassDef") {
		// 'class' Ident ':' 'AddTab' {ClassAttrDef} {ClassInitDef} {ClassFuncDef} 'DelTab'
		if (checkToken((*token), "CLASSTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "COLON", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "ADDTAB", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
		}
		while ((*token)->type == "IDENFR") {
			p = creatNode(root, "", "ClassAttrDef", (*token)->line);
			grammarAnalysis(token, "ClassAttrDef", p, outfile, DEBUG);
		}
		while ((*token)->type == "DEFTK") {
			if ((*token)->next->type == "INITTK") {
				p = creatNode(root, "", "ClassInitDef", (*token)->line);
				grammarAnalysis(token, "ClassInitDef", p, outfile, DEBUG);
			} else {
				p = creatNode(root, "", "ClassFuncDef", (*token)->line);
				grammarAnalysis(token, "ClassFuncDef", p, outfile, DEBUG);
			}
		}
		if (checkToken((*token), "DELTAB", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
		}
	}
	else if (type == "ClassAttrDef") {
		if (checkToken((*token), "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "COLON") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "DataType", (*token)->line);
			grammarAnalysis(token, "DataType", p, outfile, DEBUG);
		}
	}
	else if (type == "ClassInitDef") {
		// 'def' 'init' '(' [FuncFParams] ')' Block
		if (checkToken((*token), "DEFTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
        }
		if (checkToken((*token), "INITTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
        }
		if (checkToken((*token), "LPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "RPARENT") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		else {
			p = creatNode(root, "", "FuncFParams", (*token)->line);
			grammarAnalysis(token, "FuncFParams", p, outfile, DEBUG);
			if (checkToken((*token), "RPARENT", type)) {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		p = creatNode(root, "", "Block", (*token)->line);
		grammarAnalysis(token, "Block", p, outfile, DEBUG);
	}
	else if (type == "ClassFuncDef") {
		// 'def' Ident '(' 'self' [',' FuncFParams] ')' '->' FuncType Block
        if (checkToken((*token), "DEFTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
        }
		if (checkToken((*token), "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "LPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "SELFTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "COMMA") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "FuncFParams", (*token)->line);
			grammarAnalysis(token, "FuncFParams", p, outfile, DEBUG);
		}
		if (checkToken((*token), "RPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
        if (checkToken((*token), "ARROW", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
        }
        if ((*token)->type == "NONETK" || isDataType((*token)->type) || (*token)->type == "IDENFR") {
			p = creatNode(root, "", "FuncType", (*token)->line);
			grammarAnalysis(token, "FuncType", p, outfile, DEBUG);
        }
		p = creatNode(root, "", "Block", (*token)->line);
		grammarAnalysis(token, "Block", p, outfile, DEBUG);
	}
    else if (type == "FuncDef") {
        if (checkToken((*token), "DEFTK", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
        }
		if (checkToken((*token), "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "LPARENT", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "RPARENT") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		else {
			p = creatNode(root, "", "FuncFParams", (*token)->line);
			grammarAnalysis(token, "FuncFParams", p, outfile, DEBUG);
			if (checkToken((*token), "RPARENT", type)) {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return;
			}
		}
        if (checkToken((*token), "ARROW", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
        }
        if ((*token)->type == "NONETK" || isDataType((*token)->type) || (*token)->type == "IDENFR") {
			p = creatNode(root, "", "FuncType", (*token)->line);
			grammarAnalysis(token, "FuncType", p, outfile, DEBUG);
        }
		p = creatNode(root, "", "Block", (*token)->line);
		grammarAnalysis(token, "Block", p, outfile, DEBUG);
    }
    else if (type == "FuncType") {
        if ((*token)->type == "NONETK") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
		} else {
			p = creatNode(root, "", "DataType", (*token)->line);
			grammarAnalysis(token, "DataType", p, outfile, DEBUG);
		}
    }
    else if (type == "DataType") {
		if ((*token)->type == "IDENFR") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
			if ((*token)->type == "LSS") {
				p = creatNode(root, "", "GenericReal", (*token)->line);
				grammarAnalysis(token, "GenericReal", p, outfile, DEBUG);
			}
		}
        else if ((*token)->type == "LISTTK") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
			if ((*token)->type == "LBRACK") {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            	if (nextToken(&(*token), outfile)) return;
			}
			p = creatNode(root, "", "DataType", (*token)->line);
			grammarAnalysis(token, "DataType", p, outfile, DEBUG);
			if (checkToken((*token), "RBRACK", type)) {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            	if (nextToken(&(*token), outfile)) return;
			}
		}
		else if ((*token)->type == "DICTTK") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            if (nextToken(&(*token), outfile)) return;
			if ((*token)->type == "LBRACK") {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            	if (nextToken(&(*token), outfile)) return;
			}
			p = creatNode(root, "", "DataType", (*token)->line);
			grammarAnalysis(token, "DataType", p, outfile, DEBUG);
			if ((*token)->type == "COMMA") {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            	if (nextToken(&(*token), outfile)) return;
				p = creatNode(root, "", "DataType", (*token)->line);
				grammarAnalysis(token, "DataType", p, outfile, DEBUG);
			}
			if ((*token)->type == "RBRACK") {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
            	if (nextToken(&(*token), outfile)) return;
			}
		}
		else {
			// INTTK | FLOATTK
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
    }
    else if (type == "Block") {
		if (checkToken((*token), "COLON", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "ADDTAB", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		while ((*token) != nullptr && (*token)->type != "DELTAB") {
			p = creatNode(root, "", "BlockItem", (*token)->line);
			grammarAnalysis(token, "BlockItem", p, outfile, DEBUG);
		}
		if (checkToken((*token), "DELTAB", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			nextToken(&(*token), outfile);
		}
    }
    else if (type == "BlockItem") {
		if ((*token)->type == "IDENFR" && (*token)->next != nullptr && (*token)->next->type == "COLON") {
			p = creatNode(root, "", "Decl", (*token)->line);
			grammarAnalysis(token, "Decl", p, outfile, DEBUG);
		}
		else {
			p = creatNode(root, "", "Stmt", (*token)->line);
			grammarAnalysis(token, "Stmt", p, outfile, DEBUG);
		}
    }
    else if (type == "Decl") {
		if (checkToken((*token), "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "COLON", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "DataType", (*token)->line);
			grammarAnalysis(token, "DataType", p, outfile, DEBUG);
		}
		if ((*token)->type == "ASSIGN") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
            p = creatNode(root, "", "InitVal", (*token)->line);
            grammarAnalysis(token, "InitVal", p, outfile, DEBUG);
		}
    }
    else if (type == "InitVal") {
		if ((*token)->type == "LBRACK") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
            if ((*token)->type != "RBRACK") {
                p = creatNode(root, "", "InitVal", (*token)->line);
                grammarAnalysis(token, "InitVal", p, outfile, DEBUG);
                while ((*token) != nullptr && (*token)->type == "COMMA") {
                    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
                    if (nextToken(&(*token), outfile)) return; 
                    p = creatNode(root, "", "InitVal", (*token)->line);
                    grammarAnalysis(token, "InitVal", p, outfile, DEBUG);
                }
            }
			if (checkToken((*token), "RBRACK", type)) {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		else if ((*token)->type == "LBRACE") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
            if ((*token)->type != "RBRACE") {
                p = creatNode(root, "", "Exp", (*token)->line);
                grammarAnalysis(token, "Exp", p, outfile, DEBUG);
				if (checkToken((*token), "COLON", type)) {
					p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
					if (nextToken(&(*token), outfile)) return;
				}
                p = creatNode(root, "", "InitVal", (*token)->line);
                grammarAnalysis(token, "InitVal", p, outfile, DEBUG);
                while ((*token) != nullptr && (*token)->type == "COMMA") {
                    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
                    if (nextToken(&(*token), outfile)) return; 
                    p = creatNode(root, "", "Exp", (*token)->line);
                    grammarAnalysis(token, "Exp", p, outfile, DEBUG);
					if (checkToken((*token), "COLON", type)) {
						p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
						if (nextToken(&(*token), outfile)) return;
					}
                    p = creatNode(root, "", "InitVal", (*token)->line);
                    grammarAnalysis(token, "InitVal", p, outfile, DEBUG);
                }
            }
			if (checkToken((*token), "RBRACE", type)) {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		else { 
			p = creatNode(root, "", "Exp", (*token)->line);
			grammarAnalysis(token, "Exp", p, outfile, DEBUG);
		}
    }
    else if (type == "FuncFParams") {
		p = creatNode(root, "", "FuncFParam", (*token)->line);
		grammarAnalysis(token, "FuncFParam", p, outfile, DEBUG);
		while ((*token) != nullptr && (*token)->type == "COMMA") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "FuncFParam", (*token)->line);
			grammarAnalysis(token, "FuncFParam", p, outfile, DEBUG);
		}
    }
    else if (type == "FuncFParam") {
		if (checkToken((*token), "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		if (checkToken((*token), "COLON", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "DataType", (*token)->line);
			grammarAnalysis(token, "DataType", p, outfile, DEBUG);
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
                p = creatNode(root, "", "LVal", (*token)->line);
                grammarAnalysis(token, "LVal", p, outfile, DEBUG);
                if (checkToken((*token), "ASSIGN", type)) {
                    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
                    if (nextToken(&(*token), outfile)) return;
                } 
                p = creatNode(root, "", "Exp", (*token)->line);
                grammarAnalysis(token, "Exp", p, outfile, DEBUG);
            }
			else if (stmt_type == "append") {
				// LVal '.' 'append' '(' Exp ')'
                p = creatNode(root, "", "LVal", (*token)->line);
                grammarAnalysis(token, "LVal", p, outfile, DEBUG);
                if (checkToken((*token), "DOT", type)) {
                    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
                    if (nextToken(&(*token), outfile)) return;
                } 
                if (checkToken((*token), "APPENDTK", type)) {
                    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
                    if (nextToken(&(*token), outfile)) return;
                } 
                if (checkToken((*token), "LPARENT", type)) {
                    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
                    if (nextToken(&(*token), outfile)) return;
                } 
                p = creatNode(root, "", "Exp", (*token)->line);
                grammarAnalysis(token, "Exp", p, outfile, DEBUG);
                if (checkToken((*token), "RPARENT", type)) {
                    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
                    if (nextToken(&(*token), outfile)) return;
                }
            } 
			else {
                p = creatNode(root, "", "Exp", (*token)->line);
                grammarAnalysis(token, "Exp", p, outfile, DEBUG);
            }
		}
		else if ((*token)->type == "IFTK") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "Exp", (*token)->line);
			grammarAnalysis(token, "Exp", p, outfile, DEBUG);
			p = creatNode(root, "", "Block", (*token)->line);
			grammarAnalysis(token, "Block", p, outfile, DEBUG);
			if ((*token)->type == "ELSETK") {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return; 
				p = creatNode(root, "", "Block", (*token)->line);
				grammarAnalysis(token, "Block", p, outfile, DEBUG);
			}
		}
		else if ((*token)->type == "WHILETK") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "Exp", (*token)->line);
			grammarAnalysis(token, "Exp", p, outfile, DEBUG);
			p = creatNode(root, "", "Block", (*token)->line);
			grammarAnalysis(token, "Block", p, outfile, DEBUG);
		}
		else if ((*token)->type == "BREAKTK") {
		    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		else if ((*token)->type == "CONTINUETK") {
		    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		else if ((*token)->type == "RETURNTK") {
		    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
			if ((*token)->type != "DELTAB") {
				p = creatNode(root, "", "Exp", (*token)->line);
				grammarAnalysis(token, "Exp", p, outfile, DEBUG);
			}
		}
        else if ((*token)->type == "PRINTTK") {
            p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
			if (checkToken((*token), "LPARENT", type)) {
                p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
                if (nextToken(&(*token), outfile)) return;
			}
			if ((*token)->type != "RPARENT") {
                p = creatNode(root, "", "Exp", (*token)->line);
                grammarAnalysis(token, "Exp", p, outfile, DEBUG);
                while ((*token)->type == "COMMA") {
                    p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
                    if (nextToken(&(*token), outfile)) return;
                    p = creatNode(root, "", "Exp", (*token)->line);
                    grammarAnalysis(token, "Exp", p, outfile, DEBUG);
                }
			}
			if (checkToken((*token), "RPARENT", type)) {
                p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
                if (nextToken(&(*token), outfile)) return;
			}
        }
		else {
			p = creatNode(root, "", "Exp", (*token)->line);
			grammarAnalysis(token, "Exp", p, outfile, DEBUG);
		}
    }
    else if (type == "Exp") { 
		p = creatNode(root, "", "LOrExp", (*token)->line);
		grammarAnalysis(token, "LOrExp", p, outfile, DEBUG);
	}
	else if (type == "AddExp") { 
		p = creatNode(root, "", "MulExp", (*token)->line);
		grammarAnalysis(token, "MulExp", p, outfile, DEBUG);
		while ((*token) != nullptr && ((*token)->type == "PLUS" || (*token)->type == "MINU")) {
			if (outfile != nullptr) (*outfile) << "<AddExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "MulExp", (*token)->line);
			grammarAnalysis(token, "MulExp", p, outfile, DEBUG);
			//(*outfile) << "<AddExp>" << endl;
		}
	}
    else if (type == "MulExp") { 
		p = creatNode(root, "", "UnaryExp", (*token)->line);
		grammarAnalysis(token, "UnaryExp", p, outfile, DEBUG);
		while ((*token) != nullptr && ((*token)->type == "MULT" || (*token)->type == "DIV" || (*token)->type == "MOD")) {
			if (outfile != nullptr) (*outfile) << "<MulExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "UnaryExp", (*token)->line);
			grammarAnalysis(token, "UnaryExp", p, outfile, DEBUG);
		}
	}
    else if (type == "UnaryExp") {
		if ((*token)->type == "PLUS" || (*token)->type == "MINU" || (*token)->type == "NOTTK") { 
			p = creatNode(root, "", "UnaryOp", (*token)->line);
            if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "UnaryExp", (*token)->line);
			grammarAnalysis(token, "UnaryExp", p, outfile, DEBUG);
		}
		else if ((*token)->type == "IDENFR" || (*token)->type == "SELFTK") {
			p = creatNode(root, "", "IdentExp", (*token)->line);
			grammarAnalysis(token, "IdentExp", p, outfile, DEBUG);
		}
		else { 
			p = creatNode(root, "", "PrimaryExp", (*token)->line);
			grammarAnalysis(token, "PrimaryExp", p, outfile, DEBUG);
		}
	}
	else if (type == "IdentExp") {
		// LVal [[GenericReal] '(' [FuncRParams] ')']
		p = creatNode(root, "", "LVal", (*token)->line);
		grammarAnalysis(token, "LVal", p, outfile, DEBUG);
		if ((*token)->type == "LSS" && (*token)->next != nullptr && isDataType((*token)->next->type)) {
			p = creatNode(root, "", "GenericReal", (*token)->line);
			grammarAnalysis(token, "GenericReal", p, outfile, DEBUG);
		}
		if ((*token)->type == "LPARENT") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
			if ((*token)->type == "RPARENT") {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return;
			} else {
				p = creatNode(root, "", "FuncRParams", (*token)->line);
				grammarAnalysis(token, "FuncRParams", p, outfile, DEBUG);
				if (checkToken((*token), "RPARENT", type)) {
					p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
					if (nextToken(&(*token), outfile)) return;
				}
			}
		} 
	}
	else if (type == "GenericReal") {
		// '<' DataType {',' DataType} '>'
		// 不支持再嵌套别的泛型
		if ((*token)->type == "LSS") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "DataType", (*token)->line);
			grammarAnalysis(token, "DataType", p, outfile, DEBUG);
			while ((*token) != nullptr && (*token)->type == "COMMA") {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return; 
				p = creatNode(root, "", "DataType", (*token)->line);
				grammarAnalysis(token, "DataType", p, outfile, DEBUG);
			}
			if (checkToken((*token), "GRE", type)) {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return;
			}
		}
	}
    else if (type == "PrimaryExp") {
		// '(' Exp ')' | IntConst | FloatConst | LONGCON | STRCON | 'True' | 'False'
		if ((*token)->type == "LPARENT") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "Exp", (*token)->line);
			grammarAnalysis(token, "Exp", p, outfile, DEBUG);
			if (checkToken((*token), "RPARENT", type)) {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		else {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(token, outfile)) return;
		}
	}
    else if (type == "FuncRParams") { 
		p = creatNode(root, "", "Exp", (*token)->line);
		grammarAnalysis(token, "Exp", p, outfile, DEBUG);
		while ((*token) != nullptr && (*token)->type == "COMMA") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(token, outfile)) return; 
			p = creatNode(root, "", "Exp", (*token)->line);
			grammarAnalysis(token, "Exp", p, outfile, DEBUG);
		}
	}
    else if (type == "LVal") {
		// ['self' '.' ] Ident {'[' Exp ']'} {'.' Ident {'[' Exp ']'}}
		if ((*token)->type == "SELFTK") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
			if (checkToken((*token), "DOT", type)) {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		if (checkToken((*token), "IDENFR", type)) {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return;
		}
		while ((*token) != nullptr && (*token)->type == "LBRACK") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "Exp", (*token)->line);
			grammarAnalysis(token, "Exp", p, outfile, DEBUG);
			if (checkToken((*token), "RBRACK", type)) {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		while ((*token) != nullptr && (*token)->type == "DOT" && (*token)->next != nullptr && (*token)->next->type == "IDENFR") {
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return; 
			if (checkToken((*token), "IDENFR", type)) {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return;
			}
			while ((*token) != nullptr && (*token)->type == "LBRACK") {
				p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
				if (nextToken(&(*token), outfile)) return; 
				p = creatNode(root, "", "Exp", (*token)->line);
				grammarAnalysis(token, "Exp", p, outfile, DEBUG);
				if (checkToken((*token), "RBRACK", type)) {
					p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
					if (nextToken(&(*token), outfile)) return;
				}
			}
		}
	}
	else if (type == "LOrExp") { 
		p = creatNode(root, "", "LAndExp", (*token)->line);
		grammarAnalysis(token, "LAndExp", p, outfile, DEBUG);
		while ((*token) != nullptr && (*token)->type == "ORTK") {
			if (outfile != nullptr) (*outfile) << "<LOrExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "LAndExp", (*token)->line);
			grammarAnalysis(token, "LAndExp", p, outfile, DEBUG);
		}
	}
	else if (type == "LAndExp") { 
		p = creatNode(root, "", "EqExp", (*token)->line);
		grammarAnalysis(token, "EqExp", p, outfile, DEBUG);
		while ((*token) != nullptr && (*token)->type == "ANDTK") {
			if (outfile != nullptr) (*outfile) << "<LAndExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "EqExp", (*token)->line);
			grammarAnalysis(token, "EqExp", p, outfile, DEBUG);
		}
	}
	else if (type == "EqExp") { 
		p = creatNode(root, "", "RelExp", (*token)->line);
		grammarAnalysis(token, "RelExp", p, outfile, DEBUG);
		while ((*token) != nullptr && ((*token)->type == "EQL" || (*token)->type == "NEQ")) {
			if (outfile != nullptr) (*outfile) << "<EqExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "RelExp", (*token)->line);
			grammarAnalysis(token, "RelExp", p, outfile, DEBUG);
		}
	}
	else if (type == "RelExp") { 
		p = creatNode(root, "", "AddExp", (*token)->line);
		grammarAnalysis(token, "AddExp", p, outfile, DEBUG);
		while ((*token) != nullptr && ((*token)->type == "GRE" || (*token)->type == "LSS" || (*token)->type == "GEQ" || (*token)->type == "LEQ")) {
			if (outfile != nullptr) (*outfile) << "<RelExp>" << endl;
			p = creatNode(root, (*token)->s, (*token)->type, (*token)->line);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "AddExp", (*token)->line);
			grammarAnalysis(token, "AddExp", p, outfile, DEBUG);
		}
	}
	if (type != "BlockItem" && outfile != nullptr) 
        (*outfile) << "<" << type << ">" << endl;
}