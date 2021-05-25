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
set<string>SwitchCases;
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
	TYPE type;
	if(!IsDeclared(lexer->YYText())){
		cerr << "Erreur : Variable '"<<lexer->YYText()<<"' non déclarée"<<endl;
		exit(-1);
	}
	type=DeclaredVariables[lexer->YYText()];
	cout << "\tpush "<<lexer->YYText()<<endl;
	current=(TOKEN) lexer->yylex();
	return type;
}
//Number := chiffre | chiffre.chiffre{chiffre}
TYPE Number(void){
	if(current == FLOTANT){
	   double d = atof(lexer->YYText());
	   unsigned int *i ;
	   i = (unsigned int *) &d;
       //transférer des flottants à la pile générale//
	   cout <<"\tsubq $8,%rsp\t\t\t# allocate 8 bytes on stack's top"<<endl;
	   cout <<"\tmovl	$"<<*i<<", (%rsp)\t# Conversion of "<<d<<" (32 bit high part)"<<endl;
	   cout <<"\tmovl	$"<<*(i+1)<<", 4(%rsp)\t# Conversion of "<<d<<" (32 bit low part)"<<endl;
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
		switch(mulop){
			case AND:
			    if(val1 != BOOLEAN)//test si c'est un bool sinon erreur
				    Error("expected BOOLEAN");
				cout << "\tpop %rbx"<<endl;	// get first operand
				cout << "\tpop %rax"<<endl;	// get second operand
				cout << "\tandq	%rbx"<<endl;	// a * b -> %rdx:%rax
				cout << "\tpush %rax\t# AND"<<endl;	// store result
				break;
			case MUL:
			    if(val1 != DOUBLE || val2 != UNSIGNED_INT)//test si c'est un bool alors erreur
				    Error("expected type numerique");
				if(val1 == UNSIGNED_INT){
					cout << "\tpop %rbx"<<endl;	// get first operand
				    cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\tmulq	%rbx"<<endl;	// a * b -> %rdx:%rax
					cout << "\tpush %rax\t# MUL"<<endl;	// store result
				}else{
					cout<<"\tfldl	8(%rsp)\t"<<endl;
					cout<<"\tfldl	(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)"<<endl;
					cout<<"\tfmulp	%st(0),%st(1)\t# %st(0) <- op1 + op2 ; %st(1)=null"<<endl;
					cout<<"\tfstpl 8(%rsp)"<<endl;
					cout<<"\taddq	$8, %rsp\t# result on stack's top"<<endl; 
				}
				break;
			case DIV:
			    if(val1 != DOUBLE || val2 != UNSIGNED_INT)//test si c'est un bool alors erreur
				    Error("expected type numerique");
				if(val1 == UNSIGNED_INT){
					cout << "\tpop %rbx"<<endl;	// get first operand
				    cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\tmovq $0, %rdx"<<endl; 	// Higher part of numerator  
					cout << "\tdiv %rbx"<<endl;			// quotient goes to %rax
					cout << "\tpush %rax\t# DIV"<<endl;		// store result
				}else{
                    cout<<"\tfldl	(%rsp)\t"<<endl;
					cout<<"\tfldl	8(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)"<<endl;
					cout<<"\tfdivp	%st(0),%st(1)\t# %st(0) <- op1 + op2 ; %st(1)=null"<<endl;
					cout<<"\tfstpl 8(%rsp)"<<endl;
					cout<<"\taddq	$8, %rsp\t# result on stack's top"<<endl; 
				}	  
				break;
			case MOD:
			    if(val1 != UNSIGNED_INT)//test si c'est un bool sinon erreur
				    Error("expected type Integer");
				cout << "\tpop %rbx"<<endl;	// get first operand
				cout << "\tpop %rax"<<endl;	// get second operand
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
	TYPE type1;
	TYPE type2;
	type1 = Term();
	while(current==ADDOP){
		adop=AdditiveOperator();		// Save operator in local variable
		type2 = Term();
		if(type1 != type2)
		{
			Error("expected same type of term");
		}
		switch(adop){
			case OR:
				if(type2!=BOOLEAN)
					Error("opérande non booléenne pour l'opérateur OR");
				cout << "\tpop %rbx"<<endl;	// get first operand
				cout << "\tpop %rax"<<endl;	// get second operand
				cout << "\torq	%rbx, %rax\t# OR"<<endl;// operand1 OR operand2
				cout << "\tpush %rax"<<endl;			// store result
				break;				
			case ADD:
				if(type2!=UNSIGNED_INT && type2!=DOUBLE)
					Error("opérande non numérique pour l'addition");
				if(type2==UNSIGNED_INT){
					cout << "\tpop %rbx"<<endl;	// get first operand
					cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\taddq	%rbx, %rax\t# ADD"<<endl;	// add both operands
					cout << "\tpush %rax"<<endl;			// store result
				}
				else{
					cout<<"\tfldl	8(%rsp)\t"<<endl;
					cout<<"\tfldl	(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)"<<endl;
					cout<<"\tfaddp	%st(0),%st(1)\t# %st(0) <- op1 + op2 ; %st(1)=null"<<endl;
					cout<<"\tfstpl 8(%rsp)"<<endl;
					cout<<"\taddq	$8, %rsp\t# result on stack's top"<<endl; 
				}
				break;			
			case SUB:	
				if(type2!=UNSIGNED_INT&&type2!=DOUBLE)
					Error("opérande non numérique pour la soustraction");
				if(type2==UNSIGNED_INT){
					cout << "\tpop %rbx"<<endl;	// get first operand
					cout << "\tpop %rax"<<endl;	// get second operand
					cout << "\tsubq	%rbx, %rax\t# ADD"<<endl;	// add both operands
					cout << "\tpush %rax"<<endl;			// store result
				}
				else{
					cout<<"\tfldl	(%rsp)\t"<<endl;
					cout<<"\tfldl	8(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)"<<endl;
					cout<<"\tfsubp	%st(0),%st(1)\t# %st(0) <- op1 - op2 ; %st(1)=null"<<endl;
					cout<<"\tfstpl 8(%rsp)"<<endl;
					cout<<"\taddq	$8, %rsp\t# result on stack's top"<<endl; 
				}
				break;	
			default:
				Error("opérateur additif inconnu");
		}
		cout << "\tpush %rax"<<endl;			// store result
		
	}
return type1;
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
	TYPE type1;
	TYPE type2;
	type1 = SimpleExpression();
	if(current==RELOP){
		oprel=RelationalOperator();
		type2 = SimpleExpression();
		if(type1 != type2)
		{
			Error("expected same type");
		}
		if(type1!=DOUBLE){
			cout << "\tpop %rax"<<endl;
			cout << "\tpop %rbx"<<endl;
			cout << "\tcmpq %rax, %rbx"<<endl;
		}
		else{
			cout<<"\tfldl	(%rsp)\t"<<endl;
			cout<<"\tfldl	8(%rsp)\t# first operand -> %st(0) ; second operand -> %st(1)"<<endl;
			cout<<"\t addq $16, %rsp\t# 2x pop nothing"<<endl;
			cout<<"\tfcomip %st(1)\t\t# compare op1 and op2 -> %RFLAGS and pop"<<endl;
			cout<<"\tfaddp %st(1)\t# pop nothing"<<endl;
		}
		type1 = BOOLEAN;
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
	return type1;
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
void SwitchStatement(void);
void DoWhileStatement(void);
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
	                         if(strcmp(lexer->YYText(),"case")==0){ //si c'est un block "DISPLAY"
                                SwitchStatement();
	                         }else{
	                            if(strcmp(lexer->YYText(),"UNTIL")==0){ //si c'est un block "DISPLAY"
								   DoWhileStatement();
	                            }else{
	                               Error("previous 'Boucle' or 'Display' attendus");
	                            } 
	                         }
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
//DoWhileStatement := "REPEAT" Statement "UNTIL" Expression || "REPEAT" Statement "UNTIL NOT" Expression
void DoWhileStatement(void){
	int number = TagNumber + 1;
	cout <<"REPEAT"<<number<<":"<<endl;
	current=(TOKEN) lexer->yylex();
	Statement();
	if(strcmp(lexer->YYText(),"UNTIL")!=0 && strcmp(lexer->YYText(),"UNTIL NOT")!=0){
		Error("expected WHILE ");
	}
	if(strcmp(lexer->YYText(),"UNTIL")==0){ //test si c'est un UNTIL ou un UNTIL NOT
	   current=(TOKEN) lexer->yylex();
	   TYPE type = Expression();
	   if(type != BOOLEAN){
	       Error("expected boolean type expression after while");
	    }
	   cout << "\tpop %rax"<<endl;
	   cout << "\tcmpq $0,%rax"<<endl;
	   cout << "\tje REPEAT"<<number<<endl; //si c'est UNTIL on repete tant que elle est fausse 
	}else{
		current=(TOKEN) lexer->yylex();
		TYPE type = Expression();
		if(type != BOOLEAN){
			Error("expected boolean type expression after while");
		}
		cout << "\tpop %rax"<<endl;
		cout << "\tcmpq $0,%rax"<<endl;
		cout << "\tjne DO"<<number<<endl;//si c'est UNTIL NOT on repete tant que elle est Vrai 
	}

}
//ForStatement := "FOR" AssignementStatement "To" Expression "DO" Statement
void ForStatement(void){
    current=(TOKEN) lexer->yylex();
	TagNumber++;
	int number = TagNumber ;
	string var = lexer->YYText();
	AssignementStatement();
	if(DeclaredVariables[var]!=UNSIGNED_INT){
		Error("expected integer for the for statement");
	}
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
		TYPE type = Expression();
		if(type != UNSIGNED_INT){
			Error("expected INTEGER in for");
		} 
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
//<case label list> ::= <constant> {, <constant> } 
void CaseLabelList(string val,int number,int i){
TYPE type1 = DeclaredVariables[val];
TYPE type2;
switch (current){
	case NUMBER:
            type2 = Number();
			if(type1 != type2){//teste si les types sont compatibles
				Error("Expected same type for case statement");
			}
			cout << "\tpop %rax"<<endl;
			cout << "\tcmpq %rax,"<<val<<endl;
			cout <<"\tje Equal"<<i<<endl;
			break;
	case CHARCONST:
            type2 = CharConst();
			if(type1 != type2){//teste si les types sont compatibles
				Error("Expected same type for case statement");
			}
			cout << "\tpop %rax"<<endl;
			cout << "\tcmpq %rax,"<<val<<endl;
			cout <<"\tje Equal"<<i<<endl;
			break;
	case FLOTANT:
            type2 = Number();
			if(type1 != type2){//teste si les types sont compatibles
				Error("Expected same type for case statement");
			}
			cout<<"\tfldl	(%rsp)\t# case value -> %st(0)"<<endl;
			cout<<"\tfldl	"<<val<<"\t# variable value -> %st(1)"<<endl;
			cout<<"\t addq $8, %rsp\t#  pop nothing"<<endl;
			cout<<"\tfcomip %st(1)\t\t# compare case and variable -> %RFLAGS and pop"<<endl;
			cout<<"\tfaddp %st(1)\t# pop nothing"<<endl;
			cout <<"\tje Equal"<<i<<endl;
			break;
	default:
	       Error("Type non compatible with case statement");
	     
}
while(current == COMMA){
current=(TOKEN) lexer->yylex();
 switch (current){
	case NUMBER:
            type2 = Number();
			if(type1 != type2){//teste si les types sont compatibles
				Error("Expected same type for case statement");
			}
			cout << "\tpop %rax"<<endl;
			cout << "\tcmpq %rax,"<<val<<endl;
			cout <<"\tje Equal"<<i<<endl;
			break;
	case CHARCONST:
            type2 = CharConst();
			if(type1 != type2){//teste si les types sont compatibles
				Error("Expected same type for case statement");
			}
			cout << "\tpop %rax"<<endl;
			cout << "\tcmpq %rax,"<<val<<endl;
			cout <<"\tje Equal"<<i<<endl;
			break;
	case FLOTANT:
	        
            type2 = Number();
			if(type1 != type2){//teste si les types sont compatibles
				Error("Expected same type for case statement");
			}
			cout<<"\tfldl	(%rsp)\t# case value -> %st(0)"<<endl;
			cout<<"\tfldl	"<<val<<"\t# variable value -> %st(1)"<<endl;
			cout<<"\t addq $8, %rsp\t#  pop nothing"<<endl;
			cout<<"\tfcomip %st(1)\t\t# compare case and variable -> %RFLAGS and pop"<<endl;
			cout<<"\tfaddp %st(1)\t# pop nothing"<<endl;
			cout <<"\tje Equal"<<i<<endl;
			break;
	default:
	       Error("Type non compatible with case statement");
	     
}
}
   cout <<"\tjmp case"<<number<<i+1<<endl;
   cout <<"Equal"<<i<<":"<<endl;

}
//<case list element> ::= <case label list> : <statement> | <empty> //
void CaseListElement(string val,int number,int i){
   cout <<"case"<<number<<i<<":"<<endl;
   CaseLabelList(val,number,i);
   if(strcmp(lexer->YYText(),":")!=0)
        Error("expected :");
   current=(TOKEN) lexer->yylex();
   if(strcmp(lexer->YYText(),"end")!=0&&strcmp(lexer->YYText(),";")!=0)
        Statement();
   cout <<"\tjmp ENDSWITCH"<<number<<endl;
} 
//<case statement> ::= case <identifier> of <case list element> {; <case list element> } end 
void SwitchStatement(void){
	current=(TOKEN) lexer->yylex();
	TagNumber++;
	int number = TagNumber ;
	TYPE type1;
	TYPE type2;
	int i=0;
	string variable = lexer->YYText();
	Identifier();
	cout<<"SWITCH"<<number<<":"<<endl;
	if(strcmp(lexer->YYText(),"of")!=0){
		Error("Expected 'of'");
	}
	current=(TOKEN) lexer->yylex();
	CaseListElement(variable,number,i);
	i++;
	while(current == SEMICOLON){
		current=(TOKEN) lexer->yylex();
	    CaseListElement(variable,number,i);
		i++;
	}
	if(strcmp(lexer->YYText(),"end")!=0){
		Error("Expected 'end'");
	}
	current=(TOKEN) lexer->yylex();
    cout<<"case"<<number<<i<<":"<<endl;
    cout<<"ENDSWITCH"<<number<<":"<<endl;
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
  TagNumber++;
  int number = TagNumber ;
  TYPE type = Expression();
  if(type == UNSIGNED_INT){
		cout <<"\tpop %rdx # The value to be displayed"<<endl; //la valeur a affiché
		cout <<"\tmovq $FormatString1, %rsi "<<endl; //la methode d'affichage d'un integer
		cout <<"\tmovl $1, %edi"<<endl;
		cout <<"\tmovl $0, %eax"<<endl;
		cout <<"\tcall __printf_chk@PLT"<<endl; //l'appel du print 
  }else{
        if(type == DOUBLE){
		    cout << "\tmovsd	(%rsp), %xmm0\t\t# &stack top -> %xmm0"<<endl;
			cout << "\tsubq	$16, %rsp\t\t# allocation for 3 additional doubles"<<endl;
			cout << "\tmovsd %xmm0, 8(%rsp)"<<endl;
			cout << "\tmovq $FormatString2, %rdi\t# \"%lf\\n\""<<endl;
			cout << "\tmovq	$1, %rax"<<endl;
			cout << "\tcall	printf"<<endl;
			cout << "nop"<<endl;
			cout << "\taddq $24, %rsp\t\t\t# pop nothing"<<endl;	
		}else{
			if(type == BOOLEAN){
				cout << "\tpop %rdx\t# Zero : False, non-zero : true"<<endl;
				cout << "\tcmpq $0, %rdx"<<endl;
				cout << "\tje False"<<number<<endl;
				cout << "\tmovq $TrueString, %rdi\t# \"TRUE\\n\""<<endl;
				cout << "\tjmp Next"<<number<<endl;
				cout << "False"<<number<<":"<<endl;
				cout << "\tmovq $FalseString, %rdi\t# \"FALSE\\n\""<<endl;
				cout << "Next"<<number<<":"<<endl;
				cout << "\tcall	puts@PLT"<<endl;

			}else{
				if(type == CHAR){
					cout<<"\tpop %rsi\t\t\t# get character in the 8 lowest bits of %si"<<endl;
					cout << "\tmovq $FormatString3, %rdi\t# \"%c\\n\""<<endl;
					cout << "\tmovl	$0, %eax"<<endl;
					cout << "\tcall	printf@PLT"<<endl;
				}else{
                    Error("type non compatible with display");
				}
			}
		}
	}	   
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
	cout << "FormatString1:\t.string \"%llu\"\t# used by printf to display 64-bit unsigned integers"<<endl; 
	cout << "FormatString2:\t.string \"%lf\"\t# used by printf to display 64-bit floating point numbers"<<endl; 
	cout << "FormatString3:\t.string \"%c\"\t# used by printf to display a 8-bit single character"<<endl; 
	cout << "TrueString:\t.string \"TRUE\"\t# used by printf to display the boolean value TRUE"<<endl; 
	cout << "FalseString:\t.string \"FALSE\"\t# used by printf to display the boolean value FALSE"<<endl; 

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
		
			





