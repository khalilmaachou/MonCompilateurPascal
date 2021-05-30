VAR 
e: DOUBLE;
a,b,c,d:INTEGER;
h :CHAR.
c:=0;
h := 'b';
case h of 
'a', 'b': c:=1;
'h' : c:=2;
'k' : end;
DISPLAY c.