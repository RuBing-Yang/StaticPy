#include "definition.h"

void printCST(const string prefix, const CSTNode* node, ofstream *outfile, bool isTail = true)
{
    if( node != nullptr )
    {
        (*outfile) << prefix;
        (*outfile) << "|--";
        (*outfile) << "<" << node->type << "> " << node->s << endl;
        CSTNode* child = node->first_child;
        while (child != nullptr) {
            printCST(prefix + (isTail ? "     " : "|    "), child, outfile, child->next == nullptr);
            child = child->next;
        }
    }
}

void printAST(const string prefix, const ASTNode* node, ofstream *outfile, bool isTail = true)
{
    if( node != nullptr )
    {
        (*outfile) << prefix;
        (*outfile) << "|--";
        (*outfile) << "<" << node->type << "> " << node->s;
        if (node->datatype.size() > 0)
            (*outfile) << " " << node->datatype.to_string();
        (*outfile) << endl;
        ASTNode* child = node->first_child;
        while (child != nullptr) {
            printAST(prefix + (isTail ? "     " : "|    "), child, outfile, child->next == nullptr);
            child = child->next;
        }
    }
}

int main(int argc, char* argv[])
{
    ifstream in_file;
	ofstream lex_file, grammar_file, cst_file, ast_file, cpp_file;
    string testfile_name = "files/testfile.txt";
    int DEBUG = 0;

    TOKEN *token=nullptr, *p;
    CSTNode* croot = new CSTNode("", "CompUnit", 0);
    ASTNode* aroot = new ASTNode("", "CompUnit");
    NestDataType expDataType = NestDataType();

    if (argc > 1) testfile_name = argv[1];
    if (argc > 2) DEBUG = stoi(argv[2]);
    in_file.open(testfile_name);
	lex_file.open("files/out/lex_file.txt");
    grammar_file.open("files/out/grammar_file.txt");
    ast_file.open("files/out/ast_file.txt");
    cst_file.open("files/out/cst_file.txt");
    cpp_file.open("files/out/cpp_file.cpp");

    lexAnalysis(&in_file, &token, &lex_file, DEBUG);
    grammarAnalysis(&token, "CompUnit", croot, &grammar_file, DEBUG);
    printCST("", croot, &cst_file);
    semanticAnalysis(croot, aroot, "CompUnit", expDataType, 0, DEBUG);
    printAST("", aroot, &ast_file);

    genCSTCppCode(croot, "CompUnit", &cpp_file, "", DEBUG);

	return 0;
}
