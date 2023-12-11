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

int main(int argc, char* argv[])
{
    ifstream in_file;
	ofstream lex_file, grammar_file, ast_file, cpp_file;
    string testfile_name = "files/testfile.txt";

    TOKEN *token=nullptr, *p;
    ASTNODE* root = new ASTNode("", "CompUnit");

    if (argc > 1) testfile_name = argv[1];
    in_file.open(testfile_name);
	lex_file.open("files/out/lex_file.txt");
    grammar_file.open("files/out/grammar_file.txt");
    ast_file.open("files/out/ast_file.txt");
    cpp_file.open("files/out/cpp_file.cpp");

    lexAnalysis(&in_file, &token, &lex_file);
    grammarAnalysis(&token, "CompUnit", root, &grammar_file);
    printAST("", root, &ast_file);

    genCppCode(root, "CompUnit", &cpp_file);

	return 0;
}
