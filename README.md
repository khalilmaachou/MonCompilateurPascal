# CERIcompiler
# Nom : MAACHOU Prenom : Khalil Groupe :04

# Explication de projet 
Le principe de ce projet et de faire une compilation en utilisant LL(k) d'un script qui contient des instruction pascal on demaront de la version 0.0 du monsieur Jourlin j'ai produisé un code qui compile les element suivant :

##  Une declaration des variables :  
    VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
    VarDeclaration := Ident {"," Ident} ":" Type
##  Des StatementsPart :
    Le point de demare de l'ensemble de la compilation des instruction pascal et structures differentes  
###     Des statments :
            gére les structure (boucle ....ect) et aussi les instructions simples en utilisant :
####        IfStatement :  
                IfStatement := "IF" Expression "THEN" Statement [ "ELSE" Statement ]
####        WhileStatement :
                WhileStatement := "WHILE" Expression DO Statement
####        ForStatement :
                ForStatement := "FOR" AssignementStatement "To" Expression "DO" Statement
####        BlockStatement :
                BlockStatement := "BEGIN" Statement { ";" Statement }
####        DoUntil :
                DoWhileStatement := "REPEAT" Statement "UNTIL" Expression | "REPEAT" Statement "UNTIL NOT" Expression
####        SwitchCase :
                case statement := "case" Expression "of" case list element {; case list element } "end" 
####        AssignementStatement :
                AssignementStatement := Identifier ":=" Expression
                gére l'ensemble des affectations et permuation entre les variable(instruction simple)
##  Les types :
CHAR , DOUBLE , INTEGER, BOOLEAN
tous les chiffres numerique et les caractere sont compiler en utilisant la fonction Factor() qui fait l'appel aux fonctions suivantes : CharConst() : gére les caracteres
                      Number() : gére les nombres flotants et entiers non signé 
                      Identifier() : gére les variables
##  Les expression :
Arithmetique & Logique , Relationnelle :
###     Arithmetique & Logique : 
en utilisant la fonction Simple Expression et term :SimpleExpression := Term {AdditiveOperator Term}
                                                    Term := Factor {MultiplicativeOperator Factor}
####        Type : 
                AdditiveOperator := "+(addition)" | "-(soustraction)" | "||(ou logique)"
                MultiplicativeOperator := "*(multiplicatif)" | "/(division)" | "%(modulo)" | "&&(et logique)"
                RelationalOperator := "==(égal)" | "!=(Non égal)" | "<(inferieur)" | ">(superieur)" | "<=(inferieur ou égal)" | ">=(superieur ou égal)" 

###     Relationnelle :
            en utilisant la fonction Expression() qui gére l'ensemble de toutes les Expressions arithmetiques et relationnelles possibles 

# Étapes d'utilisation :
1) Y a deux methodes pour récupérer mon tp soit on utilisant :
           - git clone git@framagit.org:khalilmaachou1/moncompilateur.git (dans mon cas on utilise la deuxieme mon tp est  privé)
           - unzip khalilMaachouCompiler.zip -d dossier_distination
2) Éxecuter cette commande pour accéder au dossier : cd dossier_distination
3) On commence Les testes :
##  Les testes : 
je presente dans mon projet 7 fichiers pour tester l'ensemble des fonctionalités de mon compilateur :
###     If simple : 
            Ce fichier une conditions if else il affiche TRUE s'il accede au IF et FALSE sinon
            Commande de compilation : make test
###     Model de mr pierre jourlin :
            Ce model contient une boucle while et à l'interieur un ensemble d'instruction sur les char ,flotants et les entiers 
            Commande de compilation : make ModelJourlin
###     Boucle For Embriqué :
            Ce model contient 3 boucle For embriqué entre elle et le ressultat de cette éxecution le nombre d'iteration de boucle pour verifier la notion "embiqué" si ça marche 
            Commande de compilation : make BoucleForEmbr
###     Switch Case : 
            Pour cette partie j'ai créé 3 model different pour chaque des 3 type (double,char,integer) et le resultat affiche quelle case le programme a accedé en fonction d'entrée:
####            Flotant:
                    Commande de compilation : make SwitchCaseFlo
####            Char:
                    Commande de compilation : make SwitchCaseChar
####            Integer:
                    Commande de compilation : make SwitchCaseInt
###     Do Until : 
            Pour ce model il contient une boucle do while la condition est vrai  
            Commande de compilation : make BoucleUntil
###     Do Until Not : 
            Pour ce model il contient une boucle do while la condition est fausse  
            Commande de compilation : make BoucleUntilNot
Aprés l'utilisation de l'un des methode "make" pour afficher le resultat éxecutez la commande : ./test et pour éxecutez le model étape par étape utilisez le programme ddd avec la commande suivante : ddd ./test

# Grammaire utilisé :
 Program := StatementPart
 VarDeclarationPart := "VAR" VarDeclaration {";" VarDeclaration} "."
 VarDeclaration := Ident {"," Ident} ":" Type
 StatementPart := Statement {";" Statement} "."
 Statement := AssignementStatement | IfStatement | WhileStatement | ForStatement | BlockStatement | DoWhileStatement | case statement 
 AssignementStatement := Identifier "=" Expression

 Expression := SimpleExpression [RelationalOperator SimpleExpression]
 SimpleExpression := Term {AdditiveOperator Term}
 Term := Factor {MultiplicativeOperator Factor}
 Factor := Number |Identifier | "(" Expression ")"| "!" Factor
 Number := Digit{Digit} | Digit"."{Digit}

 AdditiveOperator := "+" | "-" | "||"
 MultiplicativeOperator := "*" | "/" | "%" | "&&"
 RelationalOperator := "==" | "!=" | "<" | ">" | "<=" | ">="  
 Digit := "0"|"1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"
 Identifier := {alpha}({alpha}|{Digit})