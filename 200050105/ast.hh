#ifndef AST_HH_INCLUDED
#define AST_HH_INCLUDED

#include <iostream>
#include <vector>
#include <utility>
#include "type.hh"

using namespace std;
class exp_astnode;
class identifier_astnode;

enum typeExp
{
	EmptyNode = 0,
	SeqNode = 1,
	AssNode,
	ReturnNode,
	IfNode,
	WhileNode,
	ForNode, // of statement kind
	OpBinaryNode,
	OpUnaryNode,
	AssignNode,
	FunCallNode,
	IntConstNode,
	FloatConstNode,
	StringConstNode,
	PointerNode,
	IdentifierNode,
	ArrayRefNode,
	DerefNode,
	MemberNode,
	ArrowNode
};
class funcall_astnode; //forward declaration
class abstract_astnode
{
public:
	virtual void print(int ntabs) = 0;
	virtual void gencode()=0; //gencode may require parameters
	enum typeExp astnode_type;
	// int size;
	std::string getID() const {
        return id;
    }

protected:
	std::string id;
	int offset;
};

class statement_astnode : public abstract_astnode
{
public:
	string typecast;
	virtual void print(int ntabs) = 0;
	virtual void gencode() = 0;
	// statement_astnode();
protected:
};

class exp_astnode : public abstract_astnode
{
public:
	bool lvalue; 
	int offset;  
	int size;
	string typecast;	// a string like "to_int etc
	datatype data_type; // the type of the expression
	bool is_calculable; // in case of constant folding
	int int_val;		// the folded integer
	virtual void print(int ntabs) = 0;
	virtual void gencode() = 0;
	string idname();
	// exp_astnode();
};

class ref_astnode : public exp_astnode
{
public:
	virtual void print(int ntabs) = 0;
	virtual void gencode() = 0;
	// int size;
	// ref_astnode();
};

/////////////////////////////////////////

class empty_astnode : public statement_astnode
{
public:
	empty_astnode();
	void print(int ntabs);
	void gencode();
};

class seq_astnode : public statement_astnode
{
public:
	seq_astnode();
	void pushback(statement_astnode *newchild);
	void print(int ntabs);
	void gencode();

protected:
	std::vector<statement_astnode *> children_nodes;
};

class assignS_astnode : public statement_astnode
{
public:
	assignS_astnode(exp_astnode *left, exp_astnode *right, string tc);
	void print(int ntabs);
	void gencode();

protected:
	exp_astnode *left;
	exp_astnode *right;
};

class return_astnode : public statement_astnode
{
public:
	return_astnode(exp_astnode *child);
	void print(int ntabs);
	void gencode();

protected:
	exp_astnode *child;
};

class if_astnode : public statement_astnode
{
public:
	if_astnode(exp_astnode *left, statement_astnode *middle, statement_astnode *right);
	void print(int ntabs);
	void gencode();

protected:
	exp_astnode *left;
	statement_astnode *middle;
	statement_astnode *right;
};

class while_astnode : public statement_astnode
{
public:
	while_astnode(exp_astnode *left, statement_astnode *right);
	void print(int ntabs);
	void gencode();

protected:
	exp_astnode *left;
	statement_astnode *right;
};

class for_astnode : public statement_astnode
{
public:
	for_astnode(exp_astnode *left, exp_astnode *middle1, exp_astnode *middle2, statement_astnode *right);
	void print(int ntabs);
	void gencode();

protected:
	exp_astnode *left;
	exp_astnode *middle1;
	exp_astnode *middle2;
	statement_astnode *right;
};

class proccall_astnode : public statement_astnode
{
public:
	proccall_astnode(funcall_astnode *fc);
	void print(int ntabs);
	void gencode();
	std::vector<exp_astnode *> children;

protected:
	identifier_astnode *procname;
};

///////////////////////////////////////////////////////////////

class op_binary_astnode : public exp_astnode
{
public:
	op_binary_astnode(std::string val, exp_astnode *left, exp_astnode *right);
	void print(int ntabs);
	void gencode();

protected:
	exp_astnode *left;
	exp_astnode *right;
};

class op_unary_astnode : public exp_astnode
{
public:
	op_unary_astnode(std::string val);
	op_unary_astnode(std::string val, exp_astnode *child);
	// op_unary_astnode(abstract_astnode* child1, abstract_astnode* child2);
	std::string getoperator();
	// void setchild(abstract_astnode* child);
	void print(int ntabs);
	void gencode();

protected:
	exp_astnode *child;
};

class assignE_astnode : public exp_astnode
{
public:
	assignE_astnode(exp_astnode *left, exp_astnode *right); // earlier was ref_astnode*, now no concept of lvalue in grammar
	void print(int ntabs);
	exp_astnode *left; // change to protected later
	exp_astnode *right;
	void gencode();
};

class funcall_astnode : public exp_astnode
{
public:
	funcall_astnode();
	funcall_astnode(identifier_astnode *child);
	void setname(std::string funcname);
	void pushback(exp_astnode *child);
	void print(int ntabs);
	std::vector<exp_astnode *> children;
	friend class proccall_astnode;
	void gencode();

protected:
	identifier_astnode *funcname;
};

class intconst_astnode : public exp_astnode
{
public:
	intconst_astnode(int value);
	void print(int ntabs);
	// protected:
	int value;
	void gencode();
};

class floatconst_astnode : public exp_astnode
{
public:
	floatconst_astnode(float value);
	void print(int ntabs);
	// protected:
	float value;
	void gencode();
};

class stringconst_astnode : public exp_astnode
{
public:
	stringconst_astnode(std::string value);
	void print(int ntabs);
	void gencode();
	// protected:
	std::string value;
};

////////////////////////////////////////////////

class identifier_astnode : public ref_astnode
{
public:
	identifier_astnode(std::string val,int offset,int size);
	void print(int ntabs);
	
	// string funccname(std::string id);
	void gencode();
};

// class pointer_astnode : public exp_astnode
// {
// public:
// 	pointer_astnode(ref_astnode *child);
// 	void print(int ntabs);

// protected:
// 	ref_astnode *child;
// };

class arrayref_astnode : public ref_astnode
{
public:
	arrayref_astnode(exp_astnode *left, exp_astnode *right);
	void print(int ntabs);
	void gencode();

protected:
	exp_astnode *left;
	exp_astnode *right;
};

class deref_astnode : public ref_astnode
{
public:
	deref_astnode(ref_astnode *child);
	void print(int ntabs);
	void gencode();

protected:
	ref_astnode *child;
};

class member_astnode : public ref_astnode
{
public:
	member_astnode(exp_astnode *left, identifier_astnode *right); // ref to exp
	void print(int ntabs);
	void gencode();
	// int offsetstruct;

protected:
	exp_astnode *left;
	identifier_astnode *right;
};

class arrow_astnode : public ref_astnode
{
public:
	arrow_astnode(exp_astnode *left, identifier_astnode *right); // ref to exp
	void print(int ntabs);
	void gencode();

protected:
	exp_astnode *left;
	identifier_astnode *right;
};
#endif
void printblanks(int ntabs);
void printAst(const char *astname, const char *fmt...);
char *stringTocharstar(string str);
