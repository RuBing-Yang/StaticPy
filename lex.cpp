#include "definition.h"

void lexAnalysis(ifstream *infile, TOKEN **token, ofstream *outfile, int DEBUG)
{
    int printRes1 = 1;
    int i, line = 0;
    bool is_comment = false;
    int last_tab = 0;
    string s, t;
    TOKEN *start=nullptr, *p, *q;

    while (getline(*infile, s)) {
        line++;
        i = 0;
        if (s.length() == 0) continue;
        int blank = 0;
        while (i < s.length() && s[i]==' ') {
            i++;
            blank++;
        }
        if (i >= s.length()) continue;
        if (blank % 4 != 0) {
            cerr << "[line " << line << "] LexError: Indent is not an integer number of tabs!" << endl;
	        exit(1);
        }
        if (blank / 4 > last_tab) {
            if (blank / 4 > last_tab + 1) {
                cerr << "[line " << line << "] LexError: Exceed one tab one time!" << endl;
	            exit(1);
            }
            q = new Token("AddTab", "ADDTAB", line);
            p->next = q;
            p = q;
        } else if (blank / 4 < last_tab) {
            for (int j = 0; j < last_tab - blank / 4; j++) {
                q = new Token("DelTab", "DELTAB", line);
                p->next = q;
                p = q;
            }
        }
        last_tab = blank / 4;

        while (i < s.length()) {
            t = "";
            while (i < s.length() && (s[i]==' '||s[i]=='\n')) i++;
            if (i >= s.length()) {
                break;
            }
            if (is_comment) {
                if (i + 2 < s.length() && s[i]=='\"' && s[i+1]=='\"' && s[i+2]=='\"') {
                    is_comment = false;
                    i += 3;
                } else {
                    i++;
                }
                continue;
            }
            if (s[i]=='#') {
                break;
            }
            if (!is_comment && i + 2 < s.length() && s[i]=='\"' && s[i+1]=='\"' && s[i+2]=='\"') {
                is_comment = true;
                i += 3;
                continue;
            }

            //整数常量IntConst、浮点数常量FloatConst
            if (isdigit(s[i])) {
                bool isFloat = false;
                t += s[i++];
                while (i < s.length() && (isdigit(s[i]) || s[i]=='.')) {
                    if (s[i]=='.') isFloat = true;
                    t += s[i++];
                }
                if (isalpha(s[i])) { //数字后面紧跟着字母
                    cerr << "[line " << line << "] LexError: identifier starts immediately after numeric literal" << endl;
	                exit(1);
                }
                if (!isFloat) {
                    try {
                        long long int temp = stoll(t);
                        // int: 32bit: -2147483648 ~ 2147483647
                        if (temp > 0x7fffffff || temp < -0x7fffffff - 1) {
                            q = new Token(t, "LONGCON", line);
                        }
                        else {
                            q = new Token(t, "INTCON", line);
                        }
                    } catch (const std::out_of_range& oor) {
                        cerr << "[line " << line << "] LexError: long literal too large exceed 64bits" << endl;
                        exit(1);
                    }
                } else {
                    q = new Token(t, "FLOATCON", line);
                }
            }

            //以字母开头
            else if (isalpha(s[i]) || s[i]=='_') {
                while (i<s.length() && (isalpha(s[i]) || s[i]=='_' || isdigit(s[i]))) {
                    t += s[i];
                    i++;
                }
                q = new Token(t, "", line);
                // if(t=="main") q->type = "MAINTK";
                if(t=="def") q->type = "DEFTK";
                else if(t=="const") q->type = "CONSTTK";
                else if(t=="int") q->type = "INTTK";
                else if(t=="float") q->type = "FLOATTK";
                else if(t=="bool") q->type = "BOOLTK";
                else if(t=="str") q->type = "STRTK";
                else if(t=="long") q->type = "LONGTK";
                else if(t=="while") q->type = "WHILETK";
                else if(t=="break") q->type = "BREAKTK";
                else if(t=="continue") q->type = "CONTINUETK";
                else if(t=="if") q->type = "IFTK";
                else if(t=="else") q->type = "ELSETK";
                else if(t=="print") q->type = "PRINTTK";
                else if(t=="return") q->type = "RETURNTK";
                else if(t=="None") q->type = "NONETK";
                else if(t=="List") q->type = "LISTTK";
                else if(t=="Dict") q->type = "DICTTK";
                else if(t=="True") q->type = "TRUETK";
                else if(t=="False") q->type = "FALSETK";
                else if(t=="and") q->type = "ANDTK";
                else if(t=="or") q->type = "ORTK";
                else if(t=="TypeVar") q->type = "TYPEVARTK";
                else if(t=="class") q->type = "CLASSTK";
                else if(t=="self") q->type = "SELFTK";
                else if(t=="init") q->type = "INITTK";
                else if(t=="append") q->type = "APPENDTK";
                else q->type = "IDENFR";
            }

            //格式化字符串
            else if (s[i]=='"') {
                t += s[i];
                while (i<s.length() && s[++i]!='"') {
                    t += s[i];
                }
                t += '"';
                if (i >= s.length()) {
                    cerr << "[line " << line << "] LexError: Missing matching closing quote" << endl;
	                exit(1);
                }
                q = new Token(t, "STRCON", line);
                i++;
            }

            //逻辑符号
            else if (s[i]=='!') {
                if (s[++i]=='=') {
                    q = new Token("!=", "NEQ", line);
                    i++;
                } else {
                    q = new Token("!", "NOT", line);
                }
            }
            //运算符号
            else if (s[i]=='+') {
                q = new Token("+", "PLUS", line);
                i++;
            }
            else if (s[i]=='-') {
                if (s[++i]=='>') {
                    q = new Token("->","ARROW", line);
                    i++;
                } else {
                    q = new Token("-","MINU", line);
                }
            }
            else if (s[i]=='*') {
                q = new Token("*", "MULT", line);
                i++;
            }
            else if (s[i]=='/') {
                q = new Token("/", "DIV", line);
                i++;
            }
            else if (s[i]=='%') {
                q = new Token("%", "MOD", line);
                i++;
            }
            //比较符号
            else if (s[i]=='<') {
                if (s[++i]=='=') {
                    q = new Token("<=", "LEQ", line);
                    i++;
                } else {
                    q = new Token("<", "LSS", line);
                }
            }
            else if (s[i]=='>') {
                if (s[++i]=='=') {
                    q = new Token(">=", "GEQ", line);
                    i++;
                } else {
                    q = new Token(">", "GRE", line);
                }
            }
            else if (s[i]=='=') {
                if (s[++i]=='=') {
                    q = new Token("==", "EQL", line);
                    i++;
                } else {
                    q = new Token("=", "ASSIGN", line);
                }
            }
            //其他符号
            else if (s[i]==':') {
                q = new Token(":", "COLON", line);
                i++;
            }
            else if (s[i]==',') {
                q = new Token(",", "COMMA", line);
                i++;
            }
            else if (s[i]=='.') {
                q = new Token(".", "DOT", line);
                i++;
            }
            else if (s[i]=='(') {
                q = new Token("(", "LPARENT", line);
                i++;
            }
            else if (s[i]==')') {
                q = new Token(")", "RPARENT", line);
                i++;
            }
            else if (s[i]=='[') {
                q = new Token("[", "LBRACK", line);
                i++;
            }
            else if (s[i]==']') {
                q = new Token("]", "RBRACK", line);
                i++;
            }
            else if (s[i]=='{') {
                q = new Token("{", "LBRACE", line);
                i++;
            }
            else if (s[i]=='}') {
                q = new Token("}", "RBRACE", line);
                i++;
            }
            else {
                cerr << "[line " << line << "] LexError: Undefined characters:" << s[i] << endl;
                i++;
                continue;
            }
            
            if (start == nullptr) start = q;
            else p->next = q;
            p = q;
        }
    }
    if (last_tab > 0) {
        for (int j = 0; j < last_tab; j++) {
            q = new Token("DelTab", "DELTAB", line);
            p->next = q;
            p = q;
        }
    }

    *token = start;
    if (start != nullptr && outfile != nullptr) {
        p = start;
        while (p != nullptr) {
            (*outfile) << p->type << " " << p->s << endl;
            p = p->next;
        }
    }
    
    cout << "Lex Analysis OK!" << endl;
}
