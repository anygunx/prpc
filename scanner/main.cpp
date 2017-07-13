#include "config.h"
#include "scanner.h"


/** arpcc编译器入口. */
int main( int argc, char *argv[] )
{

	try 
	{
		Scanner::Ref().Run(argc,argv);	
	}
	catch(...) 
	{
		return 1;
	}

	// 编译源文件
	return 0;
}
