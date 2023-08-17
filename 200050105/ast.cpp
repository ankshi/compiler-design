#include "ast.hh"
#include <cstdarg>
#include<cstring>
#include<iostream>
#include<string>
#include<array>
// int count_lc=0;
int numpf=0;
int ljump=1;
int reg=0;
std::vector<std::pair<int, std::string>> printfl;
std::array<std::string,5> registers={"ecx","ebx","edi","edx","esi"};
// extern int getparamsize(SymbTab gst, string name);

//////////////////////////////

// statement_astnode::statement_astnode()
// {
// }

/////////////////////////////

empty_astnode::empty_astnode() : statement_astnode()
{
	astnode_type = EmptyNode;
}

void empty_astnode::print(int ntabs)
{
	cout << "\"empty\"" << endl;
}

void empty_astnode::gencode()
{
	// cout<<"empty\n";
}
//////////////////////////

seq_astnode::seq_astnode() : statement_astnode()
{

	astnode_type = SeqNode;
}

void seq_astnode::pushback(statement_astnode *child)
{
	children_nodes.push_back(child);
}

void seq_astnode::print(int ntabs)
{
	printblanks(ntabs);
	printAst("", "l", "seq", &children_nodes);
}

void seq_astnode::gencode()
{
	// for(int j=0;j<printfl.size();j++)
	// {
	// 	cout<<".LC"<<printfl[j].first<<":"<<endl;
	// 	cout << "\t.string " << printfl[j].second << endl;
	// }
	for (int i = 0; i < children_nodes.size(); i++)
	{
		children_nodes[i]->gencode();		
	}
	

	// cout<<"\tnop\n";
	// cout<<"\tleave\n";
	// cout<<"\tret\n";
	
}

///////////////////////////////////

assignS_astnode::assignS_astnode(exp_astnode *l, exp_astnode *r, string tc) : statement_astnode()
{
	typecast = tc;
	left = l;
	right = r;
	id = "Ass";
	astnode_type = AssNode;
}

void assignS_astnode::print(int ntabs)
{
	printAst("assignS", "aa", "left", left, "right", right);
}

void assignS_astnode::gencode()
{
	
	if (left->astnode_type==IdentifierNode && (right->astnode_type==MemberNode || right->astnode_type==IdentifierNode))
	{
		cout<<"\tmovl\t"<<right->offset<<"(%ebp), "<<"%eax"<<endl;
		cout<<"\tmovl\t"<<"%eax, "<<left->offset<<"(%ebp)"<<endl;
	}
	else if (left->astnode_type==IdentifierNode && (right->astnode_type!=MemberNode && right->astnode_type!=IdentifierNode))
	{
		right->gencode();

		// cout<<"\tmovl\t"<<right->offset<<"(%ebp), "<<left->offset<<"(%ebp)"<<endl;
		cout<<"\tmovl\t"<<"%eax, "<<left->offset<<"(%ebp)"<<endl;
	}
	else if(left->astnode_type==MemberNode && (right->astnode_type==MemberNode || right->astnode_type==IdentifierNode))
	{
		cout<<"\tmovl\t"<<right->offset<<"(%ebp), "<<"%eax"<<endl;
		cout<<"\tmovl\t"<<"%eax, "<<left->offset<<"(%ebp)"<<endl;
	}
	else if (left->astnode_type==MemberNode && (right->astnode_type!=MemberNode && right->astnode_type!=IdentifierNode))
	{
		right->gencode();
		// cout<<"\tmovl\t"<<right->offset<<"(%ebp), "<<left->offset<<"(%ebp)"<<endl;
		cout<<"\tmovl\t"<<"%eax, "<<left->offset<<"(%ebp)"<<endl;
	}
}

///////////////////////////////////

return_astnode::return_astnode(exp_astnode *c) : statement_astnode()
{
	child = c;
	id = "Return";
	astnode_type = ReturnNode;
}
void return_astnode::print(int ntabs)
{
	printAst("", "a", "return", child);
}

void return_astnode::gencode()
{
	int k=ljump;
	if(child->astnode_type==IntConstNode){
		
		cout<<"\tmovl\t$"<<child->int_val<<", %eax"<<endl;
		cout<<"\tjmp\t.L"<<k+1<<endl;	
	}
	else if(child->astnode_type==IdentifierNode || child->astnode_type==MemberNode){
		cout<<"\tmovl\t"<<child->offset<<"(%ebp), %eax"<<endl;
		cout<<"\tjmp\t.L"<<k+1<<endl;
	}
	else{
		child->gencode();
		cout<<"\tjmp\t.L"<<k+1<<endl;
	}

}

////////////////////////////////////

if_astnode::if_astnode(exp_astnode *l, statement_astnode *m, statement_astnode *r) : statement_astnode()
{
	left = l;
	middle = m;
	right = r;
	id = "If";
	astnode_type = IfNode;
}

void if_astnode::print(int ntabs)
{
	printAst("if", "aaa",
			 "cond", left,
			 "then", middle,
			 "else", right);
}

void if_astnode::gencode()
{
	int k=ljump;
	ljump=ljump+2;
	left->gencode();
	cout<<"\tcmpl\t$0, %eax"<<endl;
	cout<<"\tjne\t.L"<<k+1<<endl;
	right->gencode();
	cout<<"\tjmp\t.L"<<k+2<<endl;
	cout<<".L"<<k+1<<":"<<endl;
	middle->gencode();
	cout<<".L"<<k+2<<":"<<endl;
}
////////////////////////////////////

while_astnode::while_astnode(exp_astnode *l, statement_astnode *r) : statement_astnode()
{
	left = l;
	right = r;
	id = "While";
	astnode_type = WhileNode;
}

void while_astnode::print(int ntabs)
{
	printAst("while", "aa",
			 "cond", left,
			 "stmt", right);
}
void while_astnode::gencode()
{
	int i;
	i=ljump;
	ljump=ljump+2;
	cout<<"\tjmp\t.L"<<i+1<<endl;
	cout<<".L"<<i+2<<":"<<endl;
	right->gencode();
	cout<<".L"<<i+1<<":"<<endl;
	left->gencode();
	cout<<"\tcmpl\t$0, %eax"<<endl;
	cout<<"\tjne\t.L"<<i+2<<endl;
}
/////////////////////////////////

for_astnode::for_astnode(exp_astnode *l, exp_astnode *m1, exp_astnode *m2, statement_astnode *r) : statement_astnode()
{
	left = l;
	middle1 = m1;
	middle2 = m2;
	right = r;
	id = "For";
	astnode_type = ForNode;
}

void for_astnode::print(int ntabs)
{
	printAst("for", "aaaa",
			 "init", left,
			 "guard", middle1,
			 "step", middle2,
			 "body", right);
}
void for_astnode::gencode()
{
	int j=ljump;
	ljump=ljump+2;
	left->gencode();
	cout<<"\tjmp\t.L"<<j+1<<endl;
	cout<<".L"<<j+2<<":"<<endl;
	right->gencode();
	// cout<<"\tmovl\t%eax, %ecx"<<endl;
	middle2->gencode();
	cout<<".L"<<j+1<<":"<<endl;
	middle1->gencode();
	cout<<"\tcmpl\t$0, %eax"<<endl;
	cout<<"\tjne\t.L"<<j+2<<endl;
}
//////////////////////////////////

// exp_astnode::exp_astnode() : abstract_astnode()
// {
// }

//////////////////////////////////
string exp_astnode::idname()
{
	return id;
};
op_binary_astnode::op_binary_astnode(string val, exp_astnode *l, exp_astnode *r) : exp_astnode()
{
	id = val;
	left = l;
	right = r;
	astnode_type = OpBinaryNode;
}

void op_binary_astnode::print(int ntabs)
{
	string str = "\"" + id + "\"";
	char *str1 = const_cast<char *>(str.c_str());
	printAst("op_binary", "saa", "op", str1, "left", left, "right", right);
}

void op_binary_astnode::gencode()
{
	// cout<<id<<endl;
	if(id=="PLUS_INT"){
		if ((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode))
		{
			cout<<"\tmovl\t"<<right->offset<<"(%ebp), %eax"<<endl;
			cout<<"\taddl\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}
		else if((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && right->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\taddl\t"<<"$"<<right->int_val<<", %eax"<<endl;
			
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t"<<right->offset<<"(%ebp), %eax"<<endl;
			cout<<"\taddl\t"<<"$"<<left->int_val<<", %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t$"<<(left->int_val+right->int_val)<<",%eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\taddl\t$"<<right->int_val<<", %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\taddl\t$"<<left->int_val<<", %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			left->gencode();
			// cout<<"ll\n";
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			// cout<<"\tmovl\t%eax, %ecx"<<endl;
			cout<<"\taddl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
			// cout<<"\tmovl\t%ecx, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\taddl\t"<<right->offset<<"(%ebp), %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\taddl\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			// cout<<"\tmovl\t%eax, %ecx"<<endl;
			cout<<"\taddl\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}
		else if(left->astnode_type==OpUnaryNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			// cout<<"\tmovl\t%eax, %ecx"<<endl;
			cout<<"\taddl\t"<<right->offset<<"(%ebp), %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\taddl\t$"<<left->int_val<<", %eax"<<endl;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\taddl\t$"<<right->int_val<<", %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\taddl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			

			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\taddl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			

			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\taddl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\taddl\t$"<<right->int_val<<", %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\taddl\t"<<right->offset<<"(%ebp), %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\taddl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\taddl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\taddl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\taddl\t$"<<left->int_val<<", %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			cout<<"\taddl\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\taddl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;

		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\taddl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
	}
	else if(id=="MINUS_INT"){
		if ((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode))
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tsubl\t"<<right->offset<<"(%ebp), %eax"<<endl;
		}
		else if((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && right->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tsubl\t"<<"$"<<right->int_val<<", %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t$"<<left->int_val<<"(%ebp), %eax"<<endl;
			cout<<"\tsubl\t"<<right->offset<<", %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t$"<<(left->int_val-right->int_val)<<",%eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tsubl\t$"<<right->int_val<<", %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tsubl\t$"<<left->int_val<<", %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			// cout<<"\tmovl\t%eax, %ecx"<<endl;
			cout<<"\tsubl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tmovl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tsubl\t"<<right->offset<<"(%ebp), %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tsubl\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}

		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			// cout<<"\tmovl\t%eax, %ecx"<<endl;
			cout<<"\tsubl\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}
		else if(left->astnode_type==OpUnaryNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			// cout<<"\tmovl\t%eax, %ecx"<<endl;
			cout<<"\tsubl\t"<<right->offset<<"(%ebp), %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\tsubl\t$"<<left->int_val<<", %eax"<<endl;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\tsubl\t$"<<right->int_val<<", %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			

			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tsubl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tsubl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tsubl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}


		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\tsubl\t$"<<right->int_val<<", %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\tsubl\t"<<right->offset<<"(%ebp), %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tsubl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tsubl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tsubl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\tsubl\t$"<<left->int_val<<", %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			cout<<"\tsubl\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tsubl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;

		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tsubl\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}



	}
	else if(id=="MULT_INT"){
		if ((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && (right->astnode_type==IdentifierNode|| right->astnode_type==MemberNode))
		{
			cout<<"\tmovl\t"<<right->offset<<"(%ebp), %eax"<<endl;
			cout<<"\timull\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}
		else if((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && right->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\timull\t"<<"$"<<right->int_val<<", %eax, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t"<<right->offset<<"(%ebp), %eax"<<endl;
			cout<<"\timull\t"<<"$"<<left->int_val<<", %eax, %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t$"<<(left->int_val*right->int_val)<<",%eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\timull\t$"<<right->int_val<<", %eax, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\timull\t$"<<left->int_val<<", %eax, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			

			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			left->gencode();
			// cout<<"\tmovl\t%eax, %ecx"<<endl;
			cout<<"\timull\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
			// cout<<"\tmovl\t%ecx, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\timull\t"<<right->offset<<"(%ebp), %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\timull\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}


		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			cout<<"\timull\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}
		else if(left->astnode_type==OpUnaryNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\timull\t"<<right->offset<<"(%ebp), %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\timull\t$"<<left->int_val<<", %eax, %eax"<<endl;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\timull\t$"<<right->int_val<<", %eax, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;	
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\timull\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;	
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\timull\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\timull\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}

		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\timull\t$"<<right->int_val<<", %eax, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\timull\t"<<right->offset<<"(%ebp), %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\timull\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\timull\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\timull\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\timull\t$"<<left->int_val<<", %eax, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			cout<<"\timull\t"<<left->offset<<"(%ebp), %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\timull\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;

		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\timull\t%"<<registers[cnt]<<", %eax"<<endl;
			reg--;
		}

		
	}
	else if(id=="DIV_INT"){
		if ((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode))
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t"<<right->offset<<"(%ebp)"<<endl;
		}
		else if((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && right->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcdq\n";
			cout<<"\tmovl\t$"<<right->int_val<<", %esi"<<endl;
			cout<<"\tidiv\t%esi"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t$"<<left->int_val<<", %eax"<<endl;
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t"<<right->offset<<"(%ebp)"<<endl;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode)
		{
			cout<<"\tmovl\t$"<<(left->int_val/right->int_val)<<", %eax"<<endl;
		}
		
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==IntConstNode)
		{
			int cnt=reg;

			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tmovl\t$"<<left->int_val<<", %eax"<<endl;
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcdq\n";
			cout<<"\tmovl\t$"<<right->int_val<<", %esi"<<endl;
			cout<<"\tidiv\t%esi"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t"<<right->offset<<"(%ebp)"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			

			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}


		else if(right->astnode_type==OpUnaryNode && left->astnode_type==IntConstNode)
		{
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tmovl\t$"<<left->int_val<<", %eax"<<endl;
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcdq\n";
			cout<<"\tmovl\t$"<<right->int_val<<", %esi"<<endl;
			cout<<"\tidiv\t%esi"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t"<<right->offset<<"(%ebp)"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
	
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\tcdq\n";
			cout<<"\tmovl\t$"<<right->int_val<<", %esi"<<endl;
			cout<<"\tidiv\t%esi"<<endl;
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t"<<right->offset<<"(%ebp)"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tmovl\t$"<<left->int_val<<", %eax"<<endl;
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;

		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			right->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			left->gencode();
			cout<<"\tcltd"<<endl;
			cout<<"\tidivl\t%"<<registers[cnt]<<endl;
			reg--;
		}

		
	}
	else if(id=="GT_OP_INT"){
		if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t"<<right->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<right->int_val<<", "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode)
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode)
		{
			if((left->int_val)>(right->int_val)){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;

		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}


		else if(right->astnode_type==OpUnaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;

		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}

	}
	else if(id=="LT_OP_INT"){
		if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t"<<right->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(right->int_val)-1<<", "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode)
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(left->int_val)<<", "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode)
		{
			if((left->int_val)<(right->int_val)){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;

		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;

			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}



		else if(right->astnode_type==OpUnaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;

		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}

		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetl\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}


	}

	else if(id=="LE_OP_INT"){
		if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t"<<right->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(right->int_val)<<", "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode)
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode)
		{
			if((left->int_val)<=(right->int_val)){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;

		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}



		else if(right->astnode_type==OpUnaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;

		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			

			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}

		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}

	}

	else if(id=="GE_OP_INT"){
		if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t"<<right->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(right->int_val)-1<<", "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode)
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(left->int_val)<<", "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode)
		{
			if((left->int_val)>=(right->int_val)){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}



		else if(right->astnode_type==OpUnaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;		
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetg\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;		
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;		
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			int cnt=reg;		
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)-1<<", %eax"<<endl;
			cout<<"\tsetle\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			int cnt=reg;		
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;		
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetge\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}

	}
	else if(id=="NE_OP_INT"){
		if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t"<<right->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(right->int_val)<<", "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode)
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(left->int_val)<<", "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode)
		{
			if((left->int_val)!=(right->int_val)){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
		}

		else if(right->astnode_type==OpBinaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)<<", %eax"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;

		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}


		else if(right->astnode_type==OpUnaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)<<", %eax"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;

		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}

		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)<<", %eax"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsetne\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %"<<registers[cnt]<<endl;
			reg--;
		}




	}
	else if(id=="EQ_OP_INT"){
		if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t"<<right->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(right->int_val)<<", "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode)
		{
			// cout<<"\tmovl\t"<<left->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tcmpl\t$"<<(left->int_val)<<", "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode)
		{
			if((left->int_val)==(right->int_val)){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
		}

		else if(right->astnode_type==OpBinaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)<<", %eax"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;

		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}


		else if(right->astnode_type==OpUnaryNode && left->astnode_type==IntConstNode)
		{
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)<<", %eax"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;

		}

		else if(right->astnode_type==IntConstNode && left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpUnaryNode)
		{
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode))
		{
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode)
		{
			int cnt=reg;
			
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			
			
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==OpBinaryNode && left->astnode_type==OpUnaryNode)
		{
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}

		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			left->gencode();
			cout<<"\tcmpl\t$"<<(right->int_val)<<", %eax"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\tcmpl\t%eax, "<<(right->offset)<<"(%ebp)"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			right->gencode();
			cout<<"\tcmpl\t$"<<(left->int_val)<<", %eax"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			right->gencode();
			cout<<"\tcmpl\t%eax, "<<(left->offset)<<"(%ebp)"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			int cnt=reg;
			left->gencode();
			reg++;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			right->gencode();
			cout<<"\tcmpl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
			reg--;
		}
	}
	else if(id=="OR_OP"){
		int t=ljump;
		ljump=ljump+3;
		if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode){
			if(left->int_val==0 && right->int_val==0){
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
			else{
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode){
			if(left->int_val!=0){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				cout<<"\tcmpl\t$0, "<<right->offset<<"(%ebp)"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(right->astnode_type==IntConstNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			if(right->int_val!=0){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				cout<<"\tcmpl\t$0, "<<left->offset<<"(%ebp)"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}

		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode){
			if(right->int_val!=0){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				left->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}

		}
		else if(left->astnode_type==IntConstNode && right->astnode_type==OpBinaryNode){
			if(left->int_val!=0){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				right->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}

		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpUnaryNode){
			if(right->int_val!=0){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				left->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}

		}
		else if(left->astnode_type==IntConstNode && right->astnode_type==OpUnaryNode){
			if(left->int_val!=0){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				right->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}

		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==OpUnaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(left->astnode_type==OpUnaryNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			cout<<"\tcmpl\t$0, "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			cout<<"\tcmpl\t$0, "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			cout<<"\tcmpl\t$0, "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			cout<<"\tcmpl\t$0, "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && right->astnode_type==OpBinaryNode){
			cout<<"\tcmpl\t$0, "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			cout<<"\tcmpl\t$0, "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(left->astnode_type==OpBinaryNode && right->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			if(right->int_val!=0){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				left->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			cout<<"\tcmpl\t$0, "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			if(left->int_val!=0){
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
			else{
				right->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			cout<<"\tcmpl\t$0, "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tjne\t.L"<<t+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<t+2<<endl;
			cout<<".L"<<t+1<<":"<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<t+3<<endl;	
			cout<<".L"<<t+2<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<t+3<<":"<<endl;
		}

	}
	else if(id=="AND_OP"){
		int k=ljump;
		ljump=ljump+2;
		if(right->astnode_type==IntConstNode && left->astnode_type==IntConstNode){
			if(left->int_val==0 || right->int_val==0){
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
			else{
				cout<<"\tmovl\t$1, %eax"<<endl;
			}
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==IntConstNode){
			if(left->int_val==0){
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
			else{
				cout<<"\tcmpl\t$0, "<<right->offset<<"(%ebp)"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(right->astnode_type==IntConstNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			if(right->int_val==0){
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
			else{
				cout<<"\tcmpl\t$0, "<<left->offset<<"(%ebp)"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpBinaryNode){
			if(right->int_val==0){
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
			else{
				left->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(left->astnode_type==IntConstNode && right->astnode_type==OpBinaryNode){
			if(left->int_val==0){
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
			else{
				right->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(right->astnode_type==IntConstNode && left->astnode_type==OpUnaryNode){
			if(right->int_val==0){
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
			else{
				left->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(left->astnode_type==IntConstNode && right->astnode_type==OpUnaryNode){
			if(left->int_val==0){
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
			else{
				right->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==OpUnaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(left->astnode_type==OpUnaryNode && right->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && left->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(left->astnode_type==OpUnaryNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tcmpl\t$0, "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(right->astnode_type==OpUnaryNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			cout<<"\tcmpl\t$0, "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			cout<<"\tcmpl\t$0, "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tcmpl\t$0, "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if((left->astnode_type==IdentifierNode || left->astnode_type==MemberNode) && right->astnode_type==OpBinaryNode){
			cout<<"\tcmpl\t$0, "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if((right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)&& left->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tcmpl\t$0, "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(left->astnode_type==OpBinaryNode && right->astnode_type==OpBinaryNode){					
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}

		else if(left->astnode_type==FunCallNode && right->astnode_type==IntConstNode){
			if(right->int_val==0){
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
			else{
				left->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(left->astnode_type==FunCallNode && (right->astnode_type==IdentifierNode || right->astnode_type==MemberNode)){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tcmpl\t$0, "<<right->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpUnaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(left->astnode_type==FunCallNode && right->astnode_type==FunCallNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==IntConstNode){
			if(left->int_val==0){
				cout<<"\tmovl\t$0, %eax"<<endl;
			}
			else{
				right->gencode();
				cout<<"\ttestl\t%eax, %eax"<<endl;
				cout<<"\tsetne\t%al"<<endl;
				cout<<"\tmovzbl\t%al, %eax"<<endl;
			}
		}
		else if(right->astnode_type==FunCallNode && (left->astnode_type==IdentifierNode || left->astnode_type==MemberNode)){
			cout<<"\tcmpl\t$0, "<<left->offset<<"(%ebp)"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpUnaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}
		else if(right->astnode_type==FunCallNode && left->astnode_type==OpBinaryNode){
			left->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			right->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tje\t.L"<<k+1<<endl;
			cout<<"\tmovl\t$1, %eax"<<endl;
			cout<<"\tjmp\t.L"<<k+2<<endl;
			cout<<".L"<<k+1<<":"<<endl;
			cout<<"\tmovl\t$0, %eax"<<endl;
			cout<<".L"<<k+2<<":"<<endl;
		}

	}
	
}
///////////////////////////////////

op_unary_astnode::op_unary_astnode(string val) : exp_astnode()
{
	id = val;
	astnode_type = OpUnaryNode;
}

void op_unary_astnode::print(int ntabs)

{
	string str = "\"" + id + "\"";
	char *str1 = const_cast<char *>(str.c_str());
	printAst("op_unary", "sa", "op", str1, "child", child);
}

op_unary_astnode::op_unary_astnode(string val, exp_astnode *l) : exp_astnode()
{
	id = val;
	child = l;
	astnode_type = OpUnaryNode;
}

string op_unary_astnode::getoperator()
{
	return id;
}
void op_unary_astnode::gencode()
{
	if(getoperator()=="NOT"){
		if(child->astnode_type==IntConstNode){
			cout<<"\tcmpl\t$0, "<<child->int_val<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(child->astnode_type==IdentifierNode || child->astnode_type==MemberNode){
			cout<<"\tcmpl\t$0, "<<child->offset<<"(%ebp)"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(child->astnode_type==OpBinaryNode){
			child->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
		else if(child->astnode_type==FunCallNode){
			child->gencode();
			cout<<"\ttestl\t%eax, %eax"<<endl;
			cout<<"\tsete\t%al"<<endl;
			cout<<"\tmovzbl\t%al, %eax"<<endl;
		}
	}
	else if(getoperator()=="UMINUS"){
		// if(child->astnode_type==IntConstNode){
		// 	if(child->int_val==0){
				
		// 	}
		// }
		if(child->astnode_type==IdentifierNode || child->astnode_type==MemberNode){
			cout<<"\tmovl\t"<<child->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tnegl\t%eax"<<endl;
		}
		else if(child->astnode_type==OpBinaryNode){
			child->gencode();
			cout<<"\tnegl\t%eax"<<endl;
		}
		else if(child->astnode_type==FunCallNode){
			child->gencode();
			cout<<"\tnegl\t%eax"<<endl;
		}
	}
	// cout<<getoperator()<<endl;
	else if(getoperator()=="PP"){
		if(child->astnode_type==IdentifierNode || child->astnode_type==MemberNode){
			int cnt=reg;
			reg++;
			cout<<"\tmovl\t"<<child->offset<<"(%ebp), %eax"<<endl;
			cout<<"\tmovl\t%eax, %"<<registers[cnt]<<endl;
			cout<<"\taddl\t$1, %"<<registers[cnt]<<endl;
			cout<<"\tmovl\t%"<<registers[cnt]<<", "<<child->offset<<"(%ebp)"<<endl;
			reg--;
		}
		// else if(child->astnode_type==OpBinaryNode){
		// 	child->gencode();
		// 	cout<<"\taddl\t$1, %eax"<<endl;
		// }
		else if(child->astnode_type==FunCallNode){
			child->gencode();
			cout<<"\taddl\t$1, %eax"<<endl;
		}

	}

}
///////////////////////////////////

assignE_astnode::assignE_astnode(exp_astnode *l, exp_astnode *r) : exp_astnode()
{
	left = l;
	right = r;
	astnode_type = AssignNode;
}

void assignE_astnode::print(int ntabs)
{
	printAst("assignE", "aa", "left", left, "right", right);
}

void assignE_astnode::gencode()
{
	if (left->astnode_type==IdentifierNode && (right->astnode_type==MemberNode || right->astnode_type==IdentifierNode))
	{
		cout<<"\tmovl\t"<<right->offset<<"(%ebp), "<<"%eax"<<endl;
		cout<<"\tmovl\t"<<"%eax, "<<left->offset<<"(%ebp)"<<endl;
	}
	else if (left->astnode_type==IdentifierNode && (right->astnode_type!=MemberNode && right->astnode_type!=IdentifierNode))
	{
		right->gencode();

		// cout<<"\tmovl\t"<<right->offset<<"(%ebp), "<<left->offset<<"(%ebp)"<<endl;
		cout<<"\tmovl\t"<<"%eax, "<<left->offset<<"(%ebp)"<<endl;
	}
	else if(left->astnode_type==MemberNode && (right->astnode_type==MemberNode || right->astnode_type==IdentifierNode))
	{
		cout<<"\tmovl\t"<<right->offset<<"(%ebp), "<<"%eax"<<endl;
		cout<<"\tmovl\t"<<"%eax, "<<left->offset<<"(%ebp)"<<endl;
	}
	else if (left->astnode_type==MemberNode && (right->astnode_type!=MemberNode && right->astnode_type!=IdentifierNode))
	{
		right->gencode();
		// cout<<"\tmovl\t"<<right->offset<<"(%ebp), "<<left->offset<<"(%ebp)"<<endl;
		cout<<"\tmovl\t"<<"%eax, "<<left->offset<<"(%ebp)"<<endl;
	}
}
///////////////////////////////////

funcall_astnode::funcall_astnode() : exp_astnode()
{
	astnode_type = FunCallNode;
}

funcall_astnode::funcall_astnode(identifier_astnode *child)
{
	funcname = child;
	astnode_type = FunCallNode;
}

void funcall_astnode::setname(string name)
{
	funcname = new identifier_astnode(name,offset,size);
}

void funcall_astnode::pushback(exp_astnode *subtree)
{
	children.push_back(subtree);
}

void funcall_astnode::print(int ntabs)
{
	printAst("funcall", "al", "fname", funcname, "params", &children);
}
void funcall_astnode::gencode()
{
	int pf=reg;
	
		// cout<<reg<<"   1"<<endl;
		for(int i=0;i<children.size();i++){
			
			if(children[i]->astnode_type==IdentifierNode || children[i]->astnode_type==MemberNode){
			}
			else if(children[i]->astnode_type==StringConstNode){
			}
			else if(children[i]->astnode_type==OpBinaryNode){
				int k=reg;
				reg++;
				children[i]->gencode();
				
				cout<<"\tmovl\t%eax, %"<<registers[k]<<endl;
				// cout<<"\tpushl\t%eax"<<endl;
			}
			else if(children[i]->astnode_type==OpUnaryNode){
				int sa=reg;
				reg++;
				children[i]->gencode();
				cout<<"\tmovl\t%eax, %"<<registers[sa]<<endl;
			}
			else if(children[i]->astnode_type==FunCallNode){
				int sa=reg;
				// children[i]->gencode();
				reg++;
				children[i]->gencode();
				cout<<"\tmovl\t%eax, %"<<registers[sa]<<endl;
			}	
		}
		// reg--;
		// cout<<reg<<"   2"<<endl;
		reg=pf;
		for(int i=0;i<children.size();i++){
			
			// if(children[i]->astnode_type==IdentifierNode) {
			// 	cout<<"\tpushl\t"<<children[i]->offset<<"(%ebp)"<<endl;
			// 	// cout<<"kkk "<<children[i]->size<<endl;
			// }
			if(children[i]->astnode_type==IdentifierNode || children[i]->astnode_type==MemberNode){
				int off;
				if(children[i]->size > 4){
					off=children[i]->offset;
					// cout<<off<<endl;
					for(int k=0;k<(children[i]->size)/4;k++){
						cout<<"\tpushl\t"<<off+(children[i]->size)-4<<"(%ebp)\n";
						off=off-4;
					}
				}
				else{
					cout<<"\tpushl\t"<<children[i]->offset<<"(%ebp)"<<endl;
				}
				
			}
			else if(children[i]->astnode_type==StringConstNode){
				// cout<<"\tpushl\t"<<"$.LC"<<numpf<<endl;
				children[i]->gencode();	
			}
			else if(children[i]->astnode_type==IntConstNode){
				cout<<"\tpushl\t$"<<children[i]->int_val<<endl;
			}
			else if(children[i]->astnode_type==OpBinaryNode){
				int ja=reg;
				reg++;
				cout<<"\tpushl\t%"<<registers[ja]<<endl;
			}
			else if(children[i]->astnode_type==OpUnaryNode){
				int ka=reg;
				reg++;
				cout<<"\tpushl\t%"<<registers[ka]<<endl;
				
			}	
			else if(children[i]->astnode_type==FunCallNode){
				int ka=reg;
				reg++;
				cout<<"\tpushl\t%"<<registers[ka]<<endl;
			}	
		}
	cout<<"\tcall\t"<<funcname->getID()<<endl;
	reg=pf;
	cout<<"\taddl\t$"<<4*children.size()<<", %esp"<<endl;
}

proccall_astnode::proccall_astnode (funcall_astnode *fc)
{
	procname = fc->funcname;
	children = fc->children;
}
void proccall_astnode::print(int ntabs)
{
    printAst("proccall", "al", "fname", procname, "params", &children);
}
void proccall_astnode::gencode()
{
	if (procname->idname()=="printf")
	{
		int pf=reg;
		// cout<<reg<<"   1"<<endl;
		for(int i=children.size()-1;i>=0;i--){
			
			if(children[i]->astnode_type==IdentifierNode || children[i]->astnode_type==MemberNode){
			}
			else if(children[i]->astnode_type==StringConstNode){
			}
			else if(children[i]->astnode_type==OpBinaryNode){
				int k=reg;
				reg++;
				children[i]->gencode();
				
				cout<<"\tmovl\t%eax, %"<<registers[k]<<endl;
				// cout<<"\tpushl\t%eax"<<endl;
			}
			else if(children[i]->astnode_type==OpUnaryNode){
				int sa=reg;
				reg++;
				children[i]->gencode();
				cout<<"\tmovl\t%eax, %"<<registers[sa]<<endl;
			}
			else if(children[i]->astnode_type==FunCallNode){
				int sa=reg;
				// children[i]->gencode();
				reg++;
				children[i]->gencode();
				cout<<"\tmovl\t%eax, %"<<registers[sa]<<endl;
			}	
		}
		// reg--;
		// cout<<reg<<"   2"<<endl;
		reg=pf;
		for(int i=children.size()-1;i>=0;i--){
			
			if(children[i]->astnode_type==IdentifierNode || children[i]->astnode_type==MemberNode){
				cout<<"\tpushl\t"<<children[i]->offset<<"(%ebp)"<<endl;
			}
			else if(children[i]->astnode_type==StringConstNode){
				// cout<<"\tpushl\t"<<"$.LC"<<numpf<<endl;
				children[i]->gencode();	
			}
			else if(children[i]->astnode_type==IntConstNode){
				cout<<"\tpushl\t$"<<children[i]->int_val<<endl;
			}
			else if(children[i]->astnode_type==OpBinaryNode){
				int ja=reg;
				reg++;
				cout<<"\tpushl\t%"<<registers[ja]<<endl;
				
			}
			else if(children[i]->astnode_type==OpUnaryNode){
				int ka=reg;
				reg++;
				cout<<"\tpushl\t%"<<registers[ka]<<endl;
				
			}	
			else if(children[i]->astnode_type==FunCallNode){
				int ka=reg;
				reg++;
				cout<<"\tpushl\t%"<<registers[ka]<<endl;
			}	
		}
		
		cout<<"\tpushl\t"<<"$.LC"<<numpf<<endl;
		cout<<"\tcall\t"<<"printf"<<endl;
		cout<<"\taddl\t$"<<4*children.size()<<", %esp"<<endl;
		numpf++;
	}
	else{
		int pf=reg;
		// cout<<reg<<"   1"<<endl;
		for(int i=children.size()-1;i>=0;i--){
			
			if(children[i]->astnode_type==IdentifierNode || children[i]->astnode_type==MemberNode){
			}
			else if(children[i]->astnode_type==StringConstNode){
			}
			else if(children[i]->astnode_type==OpBinaryNode){
				int k=reg;
				reg++;
				children[i]->gencode();
				
				cout<<"\tmovl\t%eax, %"<<registers[k]<<endl;
				// cout<<"\tpushl\t%eax"<<endl;
			}
			else if(children[i]->astnode_type==OpUnaryNode){
				int sa=reg;
				reg++;
				children[i]->gencode();
				cout<<"\tmovl\t%eax, %"<<registers[sa]<<endl;
			}
			else if(children[i]->astnode_type==FunCallNode){
				int sa=reg;
				// children[i]->gencode();
				reg++;
				children[i]->gencode();
				cout<<"\tmovl\t%eax, %"<<registers[sa]<<endl;
			}	
		}
		// reg--;
		// cout<<reg<<"   2"<<endl;
		reg=pf;
		for(int i=children.size()-1;i>=0;i--){
			
			if(children[i]->astnode_type==IdentifierNode || children[i]->astnode_type==MemberNode){
				cout<<"\tpushl\t"<<children[i]->offset<<"(%ebp)"<<endl;
			}
			else if(children[i]->astnode_type==StringConstNode){
				// cout<<"\tpushl\t"<<"$.LC"<<numpf<<endl;
				children[i]->gencode();	
			}
			else if(children[i]->astnode_type==IntConstNode){
				cout<<"\tpushl\t$"<<children[i]->int_val<<endl;
			}
			else if(children[i]->astnode_type==OpBinaryNode){
				int ja=reg;
				reg++;
				cout<<"\tpushl\t%"<<registers[ja]<<endl;
				
			}
			else if(children[i]->astnode_type==OpUnaryNode){
				int ka=reg;
				reg++;
				cout<<"\tpushl\t%"<<registers[ka]<<endl;
				
			}	
			else if(children[i]->astnode_type==FunCallNode){
				int ka=reg;
				reg++;
				cout<<"\tpushl\t%"<<registers[ka]<<endl;
			}	
		}
	cout<<"\tcall\t"<<procname->idname()<<endl;
	cout<<"\taddl\t$"<<4*children.size()<<", %esp"<<endl;
	}	
}
/////////////////////////////////////

intconst_astnode::intconst_astnode(int val) : exp_astnode()
{
	value = val;
	astnode_type = IntConstNode;
}

void intconst_astnode::print(int ntabs)
{

	printAst("", "i", "intconst", value);
}
void intconst_astnode::gencode()
{
	cout<<"\tmovl\t$"<<value<<", %eax"<<endl;
}

/////////////////////////////////////
floatconst_astnode::floatconst_astnode(float val) : exp_astnode()
{
	value = val;
	astnode_type = FloatConstNode;
}

void floatconst_astnode::print(int ntabs)
{
	printAst("", "f", "floatconst", value);
}

void floatconst_astnode::gencode()
{

}
///////////////////////////////////
stringconst_astnode::stringconst_astnode(string val) : exp_astnode()
{
	value = val;
	astnode_type = StringConstNode;
}

void stringconst_astnode::print(int ntabs)
{
	printAst("", "s", "stringconst", stringTocharstar(value));
}

void stringconst_astnode::gencode()
{
	
	// string str="\"%d /n\"";
	// char *b = const_cast<char *>(str.c_str());
	// char *b=stringTocharstar("\"%d \\n\"");
	
	printfl.push_back(std::make_pair(numpf, value));
	
}
// ref_astnode::ref_astnode() : exp_astnode()
// {
// 	lvalue = true;
// }

/////////////////////////////////

identifier_astnode::identifier_astnode(string val,int loffset,int ssize) : ref_astnode()
{
	id = val;
	offset=loffset;
	size=ssize;
	astnode_type = IdentifierNode;
}

void identifier_astnode::print(int ntabs)
{
	string str = "\"" + id + "\"";
	char *str1 = const_cast<char *>(str.c_str());
	printAst("", "s", "identifier", str1);
}
// string identifier_astnode::funccname(string id){
// 	return id;
// }
void identifier_astnode::gencode()
{
	
}
////////////////////////////////

arrayref_astnode::arrayref_astnode(exp_astnode *l, exp_astnode *r) : ref_astnode() // again, changed from ref to exp
{
	left = l;
	right = r;
	id = "ArrayRef";
	astnode_type = ArrayRefNode;
}

void arrayref_astnode::print(int ntabs)
{
	printAst("arrayref", "aa", "array", left, "index", right);
}
void arrayref_astnode::gencode()
{

}
///////////////////////////////

// pointer_astnode::pointer_astnode(ref_astnode *c) : ref_astnode()
// {
// 	child = c;
// 	id = "Pointer";
// 	astnode_type = PointerNode;
// }

// void pointer_astnode::print(int ntabs)
// {
// 	printAst("", "a", "pointer", child);
// }

////////////////////////////////

deref_astnode::deref_astnode(ref_astnode *c) : ref_astnode()
{
	child = c;
	id = "Deref";
	astnode_type = DerefNode;
}

void deref_astnode::print(int ntabs)
{
	printAst("", "a", "deref", child);
}
void deref_astnode::gencode()
{

}
/////////////////////////////////

member_astnode::member_astnode(exp_astnode *l, identifier_astnode *r) // change from ref to exp(1st arg)
{
	left = l;
	right = r;
	offset=l->offset+r->offset;
	astnode_type = MemberNode;
}

void member_astnode::print(int ntabs)
{

	printAst("member", "aa", "struct", left, "field", right);
}

void member_astnode::gencode()
{
	// int off=left->offset+right->offset;
	// cout<<size<<endl;
}
/////////////////////////////////

arrow_astnode::arrow_astnode(exp_astnode *l, identifier_astnode *r)
{
	left = l;
	right = r;
	astnode_type = ArrowNode;
}

void arrow_astnode::print(int ntabs)
{

	printAst("arrow", "aa", "pointer", left, "field", right);
}
void printblanks(int blanks)
{
	for (int i = 0; i < blanks; i++)
		cout << " ";
}
void arrow_astnode::gencode()
{

}
/////////////////////////////////

void printAst(const char *astname, const char *fmt...) // fmt is a format string that tells about the type of the arguments.
{
	typedef vector<abstract_astnode *> *pv;
	va_list args;
	va_start(args, fmt);
	if ((astname != NULL) && (astname[0] != '\0'))
	{
		cout << "{ ";
		cout << "\"" << astname << "\""
			 << ": ";
	}
	cout << "{" << endl;
	while (*fmt != '\0')
	{
		if (*fmt == 'a')
		{
			char *field = va_arg(args, char *);
			abstract_astnode *a = va_arg(args, abstract_astnode *);
			cout << "\"" << field << "\": " << endl;

			a->print(0);
		}
		else if (*fmt == 's')
		{
			char *field = va_arg(args, char *);
			char *str = va_arg(args, char *);
			cout << "\"" << field << "\": ";

			cout << str << endl;
		}
		else if (*fmt == 'i')
		{
			char *field = va_arg(args, char *);
			int i = va_arg(args, int);
			cout << "\"" << field << "\": ";

			cout << i;
		}
		else if (*fmt == 'f')
		{
			char *field = va_arg(args, char *);
			double f = va_arg(args, double);
			cout << "\"" << field << "\": ";
			cout << f;
		}
		else if (*fmt == 'l')
		{
			char *field = va_arg(args, char *);
			pv f = va_arg(args, pv);
			cout << "\"" << field << "\": ";
			cout << "[" << endl;
			for (int i = 0; i < (int)f->size(); ++i)
			{
				(*f)[i]->print(0);
				if (i < (int)f->size() - 1)
					cout << "," << endl;
				else
					cout << endl;
			}
			cout << endl;
			cout << "]" << endl;
		}
		++fmt;
		if (*fmt != '\0')
			cout << "," << endl;
	}
	cout << "}" << endl;
	if ((astname != NULL) && (astname[0] != '\0'))
		cout << "}" << endl;
	va_end(args);
}

char *stringTocharstar(string str)
{
	char *charstar = const_cast<char *>(str.c_str());
	return charstar;
}
