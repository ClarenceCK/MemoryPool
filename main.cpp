#include <iostream>
#include "OperatorMem.h" 
/* run this program using the console pauser or add your own getch, system("pause") or input loop */
int main(int argc, char** argv) {
	char* p = new char[128];
 	delete[] p;
	return 0;
}
