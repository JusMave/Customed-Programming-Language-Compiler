# The Micro Language

###  Informal description of Micro
A program of the Micro language has only integer data. The possible actionsare the following:
+ Input values of a list of variables (read).
+ Output/print the values of a list of expressions (write).
+ Assign (the value of) an expression to a variable.
+ Each expression is a sequence of the common arithmetic operations of
integers: addition, minus, multiplication, integer division (result is an
integer).

The grammar of a Micro programming language has the following features:
+ Comment starts with two dashes (–) and ends at the end of the line.
+ Each program starts with the word “begin” and ends with the word “end”.
+ Each statement ends with a “;”.
+ Each statement is either a read, or a write, or an assignment statement.


###  A sample program

&nbsp;&nbsp;&nbsp;-- a simple micro program <br />
&nbsp;&nbsp;&nbsp;-- good luck. It is a comment <br /> <br />
&nbsp;&nbsp;&nbsp;begin <br />
&nbsp;&nbsp;&nbsp;read(x, y, z); -- input three integers <br />
&nbsp;&nbsp;&nbsp;a := x+y; <br />
&nbsp;&nbsp;&nbsp;b := 314; <br />
&nbsp;&nbsp;&nbsp;c := 1 + a * (b - 1) / 2; <br />
&nbsp;&nbsp;&nbsp;write(a, b, c, (a+b+c)/(x+y) ); <br />
&nbsp;&nbsp;&nbsp;end <br />

###  A grammar of Micro
<nobr aria-hidden="true"><</nobr> program> --> begin <nobr aria-hidden="true"><</nobr>statement-list> end   <br/>
<nobr aria-hidden="true"><</nobr>statement-list> --> <nobr aria-hidden="true"><</nobr>statement> {<nobr aria-hidden="true"><</nobr>statement>} <br/>
<nobr aria-hidden="true"><</nobr>statement> --> <nobr aria-hidden="true"><</nobr>read-stmt> | <nobr aria-hidden="true"><</nobr>write-stmt> | <nobr aria-hidden="true"><</nobr>assign-stmt> <br/>
<nobr aria-hidden="true"><</nobr>assign-stmt> --> id := <nobr aria-hidden="true"><</nobr>expression>; <br/>
<nobr aria-hidden="true"><</nobr>read-stmt> --> read ( <nobr aria-hidden="true"><</nobr>id-list> ); <br/>
<nobr aria-hidden="true"><</nobr>write-stmt> --> write ( <nobr aria-hidden="true"><</nobr>expression-list> ); <br/>
<nobr aria-hidden="true"><</nobr>id-list> --> id | {, id } <br/>
<nobr aria-hidden="true"><</nobr>expression-list> --> <nobr aria-hidden="true"><</nobr>expression> {, <expression> } <br/>
<nobr aria-hidden="true"><</nobr>expression> --> ??? <br/>

