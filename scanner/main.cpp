#include "config.h"
#include "scanner.h"

int main( int argc, char *argv[] ){
	try {
		Scanner::Ref().Run(argc,argv);	
	}
	catch(...) {
		return 1;
	}
	return 0;
}
