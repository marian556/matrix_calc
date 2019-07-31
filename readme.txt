# matrix_calc
expression calculator for matrices (multiplication , addition, determinant) with matrix variables

Kegasoft Limited is the copy right holder of this project , all of the source code.


Project is developed in C++17 on ubuntu using g++ compiler version 8.3.0

g++ (Ubuntu 8.3.0-6ubuntu1) 8.3.0
Copyright (C) 2018 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

How to build:

marian@ubuntu:~/eclipse-workspace/Matrix_calc/$./build.sh

How to execute:

marian@ubuntu:~/eclipse-workspace/Matrix_calc/$./matrix_calc
A=[[1,2],[4,5]]; B=[[2,3],[4,5]] ; C=A*B; det(C);D=[[4,7,5,2,1],[3,2,1,4,3],[2,1,3,2,5],[1,2,3,4,5],[3,2,1,5,-2]];det(D);
[[6]]
[[832]]


you can run tests too:
marian@ubuntu:~/eclipse-workspace/Matrix_calc/$./test


Simple language:

Statements are separated either by ';' or by new line.
Statement is either assignment , in which case it is silent.
Or expression, in which case it is printed.

determinant is printed into new matrix of the size [1,1]
function transpose is also supported.

You can see doxygen generated html:
firefox matrix_calc/html/index.html
I suggest you lookup document relating to high level
StatementProcessor class

