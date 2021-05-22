//  A compiler from a very simple Pascal-like structured language LL(k)
//  to 64-bit 80x86 Assembly langage
//  Copyright (C) 2019 Pierre Jourlin
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Build with "make compilateur"


#include <string>
#include <iostream>
#include <cstdlib>
#include <set>
#include <map>
#include <FlexLexer.h>
#include "tokeniser.h"
#include <cstring>

using namespace std;

enum OPREL {EQU, DIFF, INF, SUP, INFE, SUPE, WTFR};
enum OPADD {ADD, SUB, OR, WTFA};
enum OPMUL {MUL, DIV, MOD, AND ,WTFM};
enum TYPE {UNSIGNED_INT,BOOLEAN,DOUBLE,CHAR};

TOKEN current;				// Current token


FlexLexer* lexer = new yyFlexLexer; // This is the flex tokeniser
// tokens can be read using lexer->yylex()
// lexer->yylex() returns the type of the lexicon entry (see enum TOKEN in tokeniser.h)
// and lexer->YYText() returns the lexicon entry as a string

	
std::map<string, TYPE> DeclaredVariables;
unsigned long TagNumber=0;

bool IsDeclared(const char *id){
	return DeclaredVariables.find(id)!=DeclaredVariables.end();
}


void Error(string s){
	cerr << "Ligne n°"<<lexer->lineno()<<", lu : '"<<lexer->YYText()<<"'("<<current<<"), mais ";
	cerr<< s << endl;
	exit(-1);
}

// Program := [DeclarationPart] StatementPart
// DeclarationPart := "[" Letter {"," Letter} "]"
// StatementPart := Statement {";" Statement} "."
// Statement := AssignementStatement
// AssignementStatement := Letter "=" Expression

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
// SimpleExpression := Term {AdditiveOperator Term}
// Term := Factor {MultiplicativeOperator Factor}
// Factor := Number | Letter | "(" Expression ")"| "!" Factor
// Number := Digit{Digit}

// AdditiveOperator := "+" | "-" | "||"
// MultiplicativeOperator := "*" | "/" | "%" | "&&"
// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
// Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
// Letter := "a"|...|"z"
	
		
TYPE Identifier(void){
	TYPE type = DeclaredVariables[lexer->YYText()];
	switch (type){
	    case UNSIGNED_INT : 
		    cout << "\tpush "<<lexer->YYText()<<endl;
			break;
		case DOUBLE : 
		    cout << "\tfld "<<lexer->YYText()<<endl;
			break;
		case CHAR : 
		    cout << "\tmovq $0,%rcx"<<endl;
			cout << "\tmovb "<<lexer->YYText()<<",%cl"<<endl;//j'ai utiliser rcx au cas on a utiliser rax avant et on a besoin de resultat
			cout << "\tpush %rcx"<<endl;
			break;
	}
	current=(TOKEN) lexer->yylex();
	return UNSIGNED_INT;
}
//Number := chiffre | chiffre.chiffre{chiffre}
TYPE Number(void){
	if(current == FLOTANT){
       cout <<"\tfld $"<<lexer->YYText()<<endl;
	   current=(TOKEN) lexer->yylex();
	   return DOUBLE;
	}else{
	   cout <<"\tpush $"<<lexer->YYText()<<endl;
	   current=(TOKEN) lexer->yylex();
	   return UNSIGNED_INT;
	}
}
//CharConst := alpha| rien 
TYPE CharConst(void){
  cout << "\tmovq $0,%rcx"<<endl;
  cout << "\tmovb $"<<lexer->YYText()<<",%cl"<<endl;//j'ai utiliser rcx au cas on a utiliser rax avant et on a besoin de resultat
  cout << "\tpush %rcx"<<endl;
  current=(TOKEN) lexer->yylex();
  return CHAR;
}

TYPE Expression(void);			// Called by Term() and calls Term()

TYPE Factor(void){
	TYPE val;
	if(current==RPARENT){
		current=(TOKEN) lexer->yylex();
		val = Expression();
		
		if(current!=LPARENT)
			Error("')' était attendu");		// ")" expected
		else
			current=(TOKEN) lexer->yylex();
	}
	else 
		if (current==NUMBER || current == FLOTANT)
			val = Number();
	     	else
				if(current==ID)
					val = Identifier();
				else
					if(current==CHARCONST)
					    val = CharConst();
				    else
					   Error("'(' ou chiffre ou lettre attendue ");
	return val ;
}

// MultiplicativeOperator := "*" | "/" | "%" | "&&"
OPMUL MultiplicativeOperator(void){
	OPMUL opmul;
	if(strcmp(lexer->YYText(),"*")==0)
		opmul=MUL;
	else if(strcmp(lexer->YYText(),"/")==0)
		opmul=DIV;
	else if(strcmp(lexer->YYText(),"%")==0)
		opmul=MOD;
	else if(strcmp(lexer->YYText(),"&&")==0)
		opmul=AND;
	else opmul=WTFM;
	current=(TOKEN) lexer->yylex();
	return opmul;
}

// Term := Factor {MultiplicativeOperator Factor}
TYPE Term(void){
	OPMUL mulop;
	//stockage des types de factor
	TYPE val1;
	TYPE val2;
	val1 = Factor();
	while(current==MULOP){
		mulop=MultiplicativeOperator();		// Save operator in local variable
		val2 = Factor();
		if(val1 != val2) //test de corespendence
		{
			Error("expected same type of factor");
		}
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		switch(mulop){
			case AND:
			    if(val1 != BOOLEAN)//test si c'est un bool sinon erreur
				    Error("expected BOOLEAN");
				cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
				cout << "\tpush %rax\t# AND"<<endl;	// store result
				break;
			case MUL:
			    if(val1 != DOUBLE || val2 != UNSIGNED_INT)//test si c'est un bool sinon erreur
				    Error("expected type numerique");
				cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
				cout << "\tpush %rax\t# MUL"<<endl;	// store result
				break;
			case DIV:
			    if(val1 != DOUBLE || val2 != UNSIGNED_INT)//test si c'est un bool sinon erreur
				    Error("expected type numerique");
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// quotient goes to %rax
				cout << "\tpush %rax\t# DIV"<<endl;		// store result
				break;
			case MOD:
			    if(val1 != DOUBLE || val2 != UNSIGNED_INT)//test si c'est un bool sinon erreur
				    Error("expected type numerique");
				cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
				cout << "\tdiv %rbx"<<endl;			// remainder goes to %rdx
				cout << "\tpush %rdx\t# MOD"<<endl;		// store result
				break;
			default:
				Error("opérateur multiplicatif attendu");
		}
	}
	return val1;
}

// AdditiveOperator := "+" | "-" | "||"
OPADD AdditiveOperator(void){
	OPADD opadd;
	if(strcmp(lexer->YYText(),"+")==0)
		opadd=ADD;
	else if(strcmp(lexer->YYText(),"-")==0)
		opadd=SUB;
	else if(strcmp(lexer->YYText(),"||")==0)
		opadd=OR;
	else opadd=WTFA;
	current=(TOKEN) lexer->yylex();
	return opadd;
}

// SimpleExpression := Term {AdditiveOperator Term}
TYPE SimpleExpression(void){
	OPADD adop;
	TYPE val1;
	TYPE val2;
	val1 = Term();
	while(current==ADDOP){
		adop=AdditiveOperator();		// Save operator in local variable
		val2 = Term();
		if(val1 != val2)
		{
			Error("expected same type of term");
		}
		cout << "\tpop %rbx"<<endl;	// get first operand
		cout << "\tpop %rax"<<endl;	// get second operand
		switch(adop){
			case OR:
				cout << "\taddq	%rbx, %rax\t# OR"<<endl;// operand1 OR operand2
				break;			
			case ADD:
				cout << "\taddq	%rbx, %rax\t# ADD"<<endl;	// add both operands
				break;			
			case SUB:	
				cout << "\tsubq	%rbx, %rax\t# SUB"<<endl;	// substract both operands
				break;
			default:
				Error("opérateur additif inconnu");
		}
		cout << "\tpush %rax"<<endl;			// store result
		
	}
return val1;
}

// VarDeclaration := Ident {"," Ident} ":" Type

void VarDeclaration(void){
	set<string> id; 
	TYPE var_type;
	if(current!=ID)
		Error("Un identificater était attendu");
	id.insert(lexer->YYText());
	current=(TOKEN) lexer->yylex();
	while(current==COMMA){
		current=(TOKEN) lexer->yylex();
		if(current!=ID) //s'il existe un variable
			Error("Un identificateur était attendu");
		id.insert(lexer->YYText());
		current=(TOKEN) lexer->yylex();
	}
	if(current!=TWOPOINTS)
		Error("caractère ':' attendu");
	current=(TOKEN) lexer->yylex();
	// teste des types //
	
	if(strcmp(lexer->YYText(),"BOOLEAN")==0)
		var_type = BOOLEAN;
	else if(strcmp(lexer->YYText(),"DOUBLE")==0)
		var_type = DOUBLE;
	else if(strcmp(lexer->YYText(),"INTEGER")==0)
		var_type = UNSIGNED_INT;
	else if(strcmp(lexer->YYText(),"CHAR")==0)
		var_type = CHAR;
	else Error("UNDEFINED TYPE"); 

	current=(TOKEN) lexer->yylex();
	//sauvegarde des variable //
	set<string>::iterator it = id.begin();
	for(it;it != id.end();++it){
		if(var_type == DOUBLE)
			cout << *it << ":\t.double 0.0"<<endl;
		else if(var_type == UNSIGNED_INT)
			cout << *it << ":\t.quad 0"<<endl;
		else if(var_type == CHAR)
			cout << *it << ":\t.byte 0"<<endl;
       DeclaredVariables[*it] = var_type;
	}
}
// VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
void VarDeclarationPart(void){
	if(current!=KEYWORDS)
		Error("key word 'VAR' attendue ");
	cout << "\t.data"<<endl;
	cout << "\t.align 8"<<endl;
	current=(TOKEN) lexer->yylex();
	VarDeclaration();
	while(current == SEMICOLON){
       current=(TOKEN) lexer->yylex();
	   VarDeclaration();
	}
	if(current != DOT){
		Error("'.' expected to end declaration");
	}
	current=(TOKEN) lexer->yylex();
}

// RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
OPREL RelationalOperator(void){
	OPREL oprel;
	if(strcmp(lexer->YYText(),"==")==0)
		oprel=EQU;
	else if(strcmp(lexer->YYText(),"!=")==0)
		oprel=DIFF;
	else if(strcmp(lexer->YYText(),"<")==0)
		oprel=INF;
	else if(strcmp(lexer->YYText(),">")==0)
		oprel=SUP;
	else if(strcmp(lexer->YYText(),"<=")==0)
		oprel=INFE;
	else if(strcmp(lexer->YYText(),">=")==0)
		oprel=SUPE;
	else oprel=WTFR;
	current=(TOKEN) lexer->yylex();
	return oprel;
}

// Expression := SimpleExpression [RelationalOperator SimpleExpression]
TYPE Expression(void){
	OPREL oprel;
	TYPE val1;
	TYPE val2;
	val1 = SimpleExpression();
	if(current==RELOP){
		oprel=RelationalOperator();
		val2 = SimpleExpression();
		if(val1 != val2)
		{
			cout<<val1<<endl;
			Error("expected same type");
		}
		val1 = BOOLEAN;
		cout << "\tpop %rax"<<endl;
		cout << "\tpop %rbx"<<endl;
		cout << "\tcmpq %rax, %rbx"<<endl;
		switch(oprel){
			case EQU:
				cout << "\tje Vrai"<<++TagNumber<<"\t# If equal"<<endl;
				break;
			case DIFF:
				cout << "\tjne Vrai"<<++TagNumber<<"\t# If different"<<endl;
				break;
			case SUPE:
				cout << "\tjae Vrai"<<++TagNumber<<"\t# If above or equal"<<endl;
				break;
			case INFE:
				cout << "\tjbe Vrai"<<++TagNumber<<"\t# If below or equal"<<endl;
				break;
			case INF:
				cout << "\tjb Vrai"<<++TagNumber<<"\t# If below"<<endl;
				break;
			case SUP:
				cout << "\tja Vrai"<<++TagNumber<<"\t# If above"<<endl;
				break;
			default:
				Error("Opérateur de comparaison inconnu");
		}
		cout << "\tpush $0\t\t# False"<<endl;
		cout << "\tjmp Suite"<<TagNumber<<endl;
		cout << "Vrai"<<TagNumber<<":\tpush $0xFFFFFFFFFFFFFFFF\t\t# True"<<endl;	
		cout << "Suite"<<TagNumber<<":"<<endl;
	}
	return val1;
}

// AssignementStatement := Identifier ":=" Expression
void AssignementStatement(void){
	string variable;
	if(current!=ID)
		Error("Identificateur attendu");
	if(!IsDeclared(lexer->YYText())){
		cerr << "Erreur : Variable '"<<lexer->YYText()<<"' non déclarée"<<endl;
		exit(-1);
	}
	TYPE type1 = DeclaredVariables[lexer->YYText()];
	variable=lexer->YYText();
	current=(TOKEN) lexer->yylex();
	if(current!=ASSIGN)
		Error("caractères ':=' attendus");
	current=(TOKEN) lexer->yylex();
	TYPE type2 = Expression();
	if(type1 != type2){//teste si expression a le meme type de variable
		Error("expected same type");
	}
	cout << "\tpop "<<variable<<endl;
}

void IfStatement(void);
void WhileStatement(void);
void ForStatement(void);
void BlockStatement(void);
void Display(void);
//Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement
void Statement(void){
	if(current==KEYWORDS){
	   if(strcmp(lexer->YYText(),"IF")==0){ //si c'est un block "IF"
              IfStatement();
	   }else{
	        if(strcmp(lexer->YYText(),"FOR")==0){ //si c'est un block "FOR"
                 ForStatement();
	        }else{
	             if(strcmp(lexer->YYText(),"WHILE")==0){ //si c'est un block "WHILE"
                        WhileStatement();
	              }else{
	                  if(strcmp(lexer->YYText(),"BEGIN")==0){ //si c'est un block "BLOCK"
                          BlockStatement();
	                  }else{
	                      if(strcmp(lexer->YYText(),"DISPLAY")==0){ //si c'est un block "DISPLAY"
                             Display();
	                      }else{
	                         Error("previous 'Boucle' or 'Display' attendus");
	                      }
	                  }
	              }
	        }
	   }

	}else{AssignementStatement();}
}

//IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
void IfStatement(void){
	current=(TOKEN) lexer->yylex();
	int number = TagNumber+1;
	TYPE val1;
	string nexttag;
	cout << "IF"<<number<<":"<<endl;
    nexttag = "FINSI";
    val1 = Expression();
	if(val1 != BOOLEAN){ //test si un boolean ou pas pour verifier la condition 
        Error("Expected Boolean type expression");
	}
	if(strcmp(lexer->YYText(),"THEN")!=0){ //si mot clef 'THEN' n'existe pas
       Error("'THEN' attendus");
	}else{
		current=(TOKEN) lexer->yylex();
		cout << "\tpop %rax"<<endl;
		cout << "\tcmpq %rax,$0"<<endl;
        cout << "\tje ELSE"<<number<<endl;
		Statement();
        cout << "ELSE"<<number<<":"<<endl;
		if(strcmp(lexer->YYText(),"ELSE")==0){ //Si existe un block 'ELSE'
			current=(TOKEN) lexer->yylex();
			Statement();
		}
		cout <<"FINSI"<<number<<":"<<endl;
	}
}
//WhileStatement := "WHILE" Expression DO Statement
void WhileStatement(void){
	current=(TOKEN) lexer->yylex();
	int number = TagNumber + 1;
	cout << "WHILE"<<number<<":"<<endl;
	TYPE val1;
	val1 = Expression();
	if(val1 != BOOLEAN){ //test si un boolean ou pas pour verifier la condition 
        Error("Expected Boolean type expression");
	}
	cout << "\tpop %rax"<<endl;
	cout << "\tcmpq $0,%rax"<<endl;
	cout << "\tje ENDWHILE"<<number<<endl;
	if(current==KEYWORDS){
	   if(strcmp(lexer->YYText(),"DO")!=0){ //si mot clef 'DO' n'existe pas
          Error("'DO' attendus");
	   }else{
		   current=(TOKEN) lexer->yylex();
		   Statement();
		   cout << "\tjmp WHILE"<<number<<endl;
		   cout <<"ENDWHILE"<<number<<":"<<endl;
	   }
	}
}
//ForStatement := "FOR" AssignementStatement "To" Expression "DO" Statement
void ForStatement(void){
    current=(TOKEN) lexer->yylex();
	TagNumber++;
	int number = TagNumber ;
	string var = lexer->YYText();
	AssignementStatement();
	int typeDeBoucleFor = 0; //savoir si cest un "DOWN TO" ou c'est un "TO"
	if(strcmp(lexer->YYText(),"TO")!=0 && strcmp(lexer->YYText(),"DOWN TO")!=0){ //si mot clef 'TO' et le mot clef 'DOWN TO' n'existe pas
       Error("'TO' attendus");
	}else{
		if(strcmp(lexer->YYText(),"DOWN TO") == 0){ //si c'est un "DOWN TO" ON CHANGE LA VALEUR DE LA VARIABLE
		   typeDeBoucleFor = 1;
		}
		current=(TOKEN) lexer->yylex();
		string borne = lexer->YYText();
		TOKEN c = current;
		Expression();
		cout << "\tpop %rax"<<endl; //stocké l'expression dans rax
		cout << "\tincq %rsi"<<endl; //incrementer l'indice pour stocké la limite  de la boucle courante
		cout << "\tmovq %rax,(%rsi)"<<endl;
	    cout <<"FOR"<<number<<":"<<endl; //stocké la limite de la boucle courante dans la case memoire qui a l'indice stocké dans rsi
	    cout <<"\tmovq (%rsi),%rax"<<endl;
	    cout << "\tcmpq %rax,"<<var<<endl;
        cout << "\tje ENDFOR"<<number<<endl;
		if(strcmp(lexer->YYText(),"DO")!=0){ //si mot clef 'DO' n'existe pas
            Error("'DO' attendus");
	    }else{
		    current=(TOKEN) lexer->yylex();
		    Statement();
		    if(typeDeBoucleFor == 0){
			   cout << "\tincq "<<var<<"\t#ADD"<<endl;
			}else{
				cout << "\tsubq $1,"<<var<<"\t#ADD"<<endl;
			}
			cout<<"\tjmp FOR"<<number<<endl;
			cout <<"ENDFOR"<<number<<":"<<endl;
			cout <<"\tsubq $1,%rsi"<<endl;
    	}
	}
}
//BlockStatement := "BEGIN" Statement { ";" Statement } "END"
void BlockStatement(void){
    current=(TOKEN) lexer->yylex();
	int number = TagNumber + 1;
	cout <<"Begin"<<number<<":"<<endl;
	Statement();
	while(current==SEMICOLON){
       current=(TOKEN) lexer->yylex();
	   Statement();
	}
	if(strcmp(lexer->YYText(),"END")!=0){
		Error("'END' attendus");
	}else{
		current=(TOKEN) lexer->yylex();
	}
    cout <<"END"<<number<<":"<<endl;
}

void Display(void){
  current=(TOKEN) lexer->yylex();
  Expression();
  cout <<"\tpop %rdx # The value to be displayed"<<endl; //la valeur a affiché
  cout <<"\tmovq $FormatString1, %rsi "<<endl; //la methode d'affichage d'un integer
  cout <<"\tmovl $1, %edi"<<endl;
  cout <<"\tmovl $0, %eax"<<endl;
  cout <<"\tcall __printf_chk@PLT"<<endl; //l'appel du print 
}


// StatementPart := Statement {";" Statement} "."
void StatementPart(void){
	cout << "\t.text\t\t# The following lines contain the program"<<endl;
	cout << "\t.globl main\t# The main function must be visible from outside"<<endl;
	cout << "main:\t\t\t# The main function body :"<<endl;
	cout << "\tmovq %rsp, %rbp\t# Save the position of the stack's top"<<endl;
	Statement();
	while(current==SEMICOLON){
		current=(TOKEN) lexer->yylex();
		Statement();
	}
	if(current!=DOT)
		Error("caractère '.' attendu");
	current=(TOKEN) lexer->yylex();
}

// Program := [VarDeclarationPart] StatementPart
void Program(void){
	if(current==KEYWORDS)
		VarDeclarationPart();
	StatementPart();	
}

int main(void){	// First version : Source code on standard input and assembly code on standard output
	// Header for gcc assembler / linker
	cout << "\t\t\t# This code was produced by the CERI Compiler"<<endl;
	// Let's proceed to the analysis and code production
	current=(TOKEN) lexer->yylex();
	Program();
	// Trailer for the gcc assembler / linker
	cout << "\tmovq %rbp, %rsp\t\t# Restore the position of the stack's top"<<endl;
	cout << "\tret\t\t\t# Return from main function"<<endl;
	if(current!=FEOF){
		cerr <<"Caractères en trop à la fin du programme : ["<<current<<"]";
		Error("."); // unexpected characters at the end of program
	}

}
		
			





