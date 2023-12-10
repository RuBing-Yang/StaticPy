#include "definition.h"

void printAST(const string prefix, const ASTNODE* node, ofstream *outfile, bool isTail = true)
{
    if( node != nullptr )
    {
        (*outfile) << prefix;
        (*outfile) << "|--";
        (*outfile) << "<" << node->type << "> " << node->s << endl;
        ASTNODE* child = node->first_child;
        while (child != nullptr) {
            printAST(prefix + (isTail ? "     " : "|    "), child, outfile, child->next == nullptr);
            child = child->next;
        }
    }
}

int main()
{
    ifstream in_file;
	ofstream lex_file, grammar_file, ast_file, cpp_file;

    TOKEN *token=nullptr, *p;
    ASTNODE* root = new ASTNode("", "CompUnit");

    in_file.open("files/testfile.txt");
	lex_file.open("files/lex_file.txt");
    grammar_file.open("files/grammar_file.txt");
    ast_file.open("files/ast_file.txt");
    cpp_file.open("files/cpp_file.cpp");

    lexAnalysis(&in_file, &token, &lex_file);
    grammarAnalysis(&token, "CompUnit", root, &grammar_file);
    printAST("", root, &ast_file);

    genCppCode(root, "CompUnit", &cpp_file);

	return 0;
}
