#include "definition.h"

void printAST(const string prefix, const CSTNode* node, ofstream *outfile, bool isTail = true)
{
    if( node != nullptr )
    {
        (*outfile) << prefix;
        (*outfile) << "|--";
        (*outfile) << "<" << node->type << "> " << node->s << endl;
        CSTNode* child = node->first_child;
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
    CSTNode* croot = new CSTNode("", "CompUnit", 0);
    ASTNode* aroot = new ASTNode("", "CompUnit");
    NestDataType expDataType = NestDataType();

    if (argc > 1) testfile_name = argv[1];
    in_file.open(testfile_name);
	lex_file.open("files/out/lex_file.txt");
    grammar_file.open("files/out/grammar_file.txt");
    ast_file.open("files/out/ast_file.txt");
    cpp_file.open("files/out/cpp_file.cpp");

    lexAnalysis(&in_file, &token, &lex_file);
    grammarAnalysis(&token, "CompUnit", croot, &grammar_file);
    printAST("", croot, &ast_file);
    semanticAnalysis(croot, aroot, "CompUnit", expDataType, 0);

    cout << "Generate C++ code..." << endl;

    genCppCode(croot, "CompUnit", &cpp_file);

	return 0;
}
