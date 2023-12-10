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

void grammarAnalysis(TOKEN **token, string type, ASTNODE *root, ofstream *outfile)
{
    ASTNODE *p;
	TOKEN *t;
	int flag;
	if ((*token) == nullptr) return;
    if (type == "CompUnit") {
		while ((*token) != nullptr && (*token)->type == "DEFTK") {
			p = creatNode(root, "", "FuncDef");
			grammarAnalysis(token, "FuncDef", p, outfile);
		}
    }
    else if (type == "InitVal") {
		if ((*token)->type == "LBRACK") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "InitVal");
			grammarAnalysis(token, "InitVal", p, outfile);
			while ((*token) != nullptr && (*token)->type == "COMMA") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return; 
				p = creatNode(root, "", "InitVal");
				grammarAnalysis(token, "InitVal", p, outfile);
			}
			if ((*token)->type == "RBRACK") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		else if ((*token)->type == "LBRACE") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "InitVal");
			grammarAnalysis(token, "InitVal", p, outfile);
			while ((*token) != nullptr && (*token)->type == "COMMA") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return; 
				p = creatNode(root, "", "InitVal");
				grammarAnalysis(token, "InitVal", p, outfile);
			}
			if ((*token)->type == "RBRACE") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
		else { 
			p = creatNode(root, "", "Exp");
			grammarAnalysis(token, "Exp", p, outfile);
		}
    }
    else if (type == "FuncDef") {
        if ((*token)->type == "DEFTK") {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
		if ((*token)->type == "IDENFR") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "LPARENT") {
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
			if ((*token)->type == "RPARENT") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
        if ((*token)->type == "ARROW") {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
        if ((*token)->type == "NONETK" || (*token)->type == "INTTK" || (*token)->type == "LISTTK" || (*token)->type == "DICTTK") {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
		p = creatNode(root, "", "Block");
		grammarAnalysis(token, "Block", p, outfile);
    }
    else if (type == "Block") {
		if ((*token)->type == "COLON") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "ADDTAB") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		while ((*token) != nullptr && (*token)->type != "DELTAB") {
			p = creatNode(root, "", "BlockItem");
			grammarAnalysis(token, "BlockItem", p, outfile);
		}
		if ((*token)->type == "DELTAB") {
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
		if ((*token)->type == "IDENFR") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "COLON") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
        if ((*token)->type == "INTTK" || (*token)->type == "LISTTK" || (*token)->type == "DICTTK") {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
		if ((*token)->type == "ASSIGN") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
            p = creatNode(root, "", "InitVal");
            grammarAnalysis(token, "InitVal", p, outfile);
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
		if ((*token)->type == "IDENFR") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		if ((*token)->type == "COLON") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
        if ((*token)->type == "INTTK" || (*token)->type == "LISTTK" || (*token)->type == "DICTTK") {
			p = creatNode(root, (*token)->s, (*token)->type);
            if (nextToken(&(*token), outfile)) return;
        }
    }
    else if (type == "Stmt") {
		if ((*token)->type == "IDENFR") {
            // TODO: 数组/字典中元素赋值
            p = creatNode(root, "", "LVal");
            grammarAnalysis(token, "LVal", p, outfile);
            if ((*token)->type == "ASSIGN") {
                p = creatNode(root, (*token)->s, (*token)->type);
                if (nextToken(&(*token), outfile)) return;
            } 
            p = creatNode(root, "", "Exp");
            grammarAnalysis(token, "Exp", p, outfile);
		}
		else if ((*token)->type == "IFTK") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, "", "Cond");
			grammarAnalysis(token, "Cond", p, outfile);
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
			p = creatNode(root, "", "Cond");
			grammarAnalysis(token, "Cond", p, outfile);
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
			if ((*token)->type != "DelTab") {
				p = creatNode(root, "", "Exp");
				grammarAnalysis(token, "Exp", p, outfile);
			}
		}
        else if ((*token)->type == "PRINTTK") {
            p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			if ((*token)->type == "LPARENT") {
                p = creatNode(root, (*token)->s, (*token)->type);
                if (nextToken(&(*token), outfile)) return;
			}
			if ((*token)->type == "STRCON") {
                p = creatNode(root, (*token)->s, (*token)->type);
                if (nextToken(&(*token), outfile)) return;
			}
			if ((*token)->type == "RPARENT") {
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
		p = creatNode(root, "", "AddExp");
		grammarAnalysis(token, "AddExp", p, outfile);
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
		while ((*token) != nullptr && ((*token)->type == "MULT" || 
			(*token)->type == "DIV" || (*token)->type == "MOD")) {
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
			grammarAnalysis(token, "UnaryOp", p, outfile); 
			p = creatNode(root, "", "UnaryExp");
			grammarAnalysis(token, "UnaryExp", p, outfile);
		}
		else if ((*token)->type == "IDENFR" && (*token)->next != nullptr && (*token)->next->type == "LPARENT") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
			if ((*token)->type == "RPARENT") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			} else { 
				p = creatNode(root, "", "FuncRParams");
				grammarAnalysis(token, "FuncRParams", p, outfile);
				if ((*token)->type == "RPARENT") {
					p = creatNode(root, (*token)->s, (*token)->type);
					if (nextToken(&(*token), outfile)) return;
				}
			}
		}
		else { 
			p = creatNode(root, "", "PrimaryExp");
			grammarAnalysis(token, "PrimaryExp", p, outfile);
		}
	}
    else if (type == "PrimaryExp") {
		if ((*token)->type == "LPARENT") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "Exp");
			grammarAnalysis(token, "Exp", p, outfile);
			if ((*token)->type == "RPARENT") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		} else if ((*token)->type == "INTCON") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(token, outfile)) return;
		} else { 
			p = creatNode(root, "", "LVal");
			grammarAnalysis(token, "LVal", p, outfile);
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
		if ((*token)->type == "IDENFR") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return;
		}
		while ((*token) != nullptr && (*token)->type == "LBRACK") {
			p = creatNode(root, (*token)->s, (*token)->type);
			if (nextToken(&(*token), outfile)) return; 
			p = creatNode(root, "", "Exp");
			grammarAnalysis(token, "Exp", p, outfile);
			if ((*token)->type == "RBRACK") {
				p = creatNode(root, (*token)->s, (*token)->type);
				if (nextToken(&(*token), outfile)) return;
			}
		}
	}
    else if (type == "Cond") { 
		p = creatNode(root, "", "LOrExp");
		grammarAnalysis(token, "LOrExp", p, outfile);
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
		while ((*token) != nullptr && ((*token)->type == "GRE" || (*token)->type == "LSS" 
			|| (*token)->type == "GEQ" || (*token)->type == "LEQ")) {
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