#include "config.h"
#include "scanner.h"


/** arpcc���������. */
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

	// ����Դ�ļ�
	return 0;
}
