

/* IMPORT START CONDITION */
%x IMPORT

%{

#include <stdio.h>

/* bison value type. */
#include <string>
#define YYSTYPE std::string
#include "prpc.tab.hpp"
#include "scanner.h"

struct FileStackNode
{
	std::string		filename_;
	int				lineno_;
	YY_BUFFER_STATE	bs_;
};
std::vector<FileStackNode>	FileStack;
std::set<std::string> ImportFileSet;
std::string CurrentFilename;
int CurrentLineno = 0;

%}

/* SO we don't choke on files that use \r\n */
NL [\r?\n]

%array
%option never-interactive
%option noyywrap


%%

enum            return TOKEN_ENUM;
struct			return TOKEN_STRUCT;
service			return TOKEN_SERVICE;
int64			return TOKEN_INT64;
uint64			return TOKEN_UINT64;
double			return TOKEN_DOUBLE;
float			return TOKEN_FLOAT;
int32			return TOKEN_INT32;
uint32			return TOKEN_UINT32;
int16			return TOKEN_INT16;
uint16			return TOKEN_UINT16;
int8			return TOKEN_INT8;
uint8			return TOKEN_UINT8;
bool			return TOKEN_BOOL;
string			return TOKEN_STRING;
array			return TOKEN_ARRAY;


#import			{	
					/* Begin 'IMPORT' start condition if we met "import" keyword */
					BEGIN(IMPORT);
				}
<IMPORT>[ \t]*		/* eat the white space */
<IMPORT>[^ \t\n]+	{ 
					
					/* got the import file name */
					std::string importFilename = strings::Trim(std::string( yytext, yyleng),"\r\n\t");
					
					if( ImportFileSet.find( importFilename ) != ImportFileSet.end() )
					{
						BEGIN( INITIAL );
					}
					else
					{
						// save this filename to imported files.
						ImportFileSet.insert( importFilename );

						// Save current file state.
						FileStackNode n;
						n.filename_ = CurrentFilename;
						n.lineno_	= CurrentLineno + 1;	
						n.bs_		= YY_CURRENT_BUFFER;
						FileStack.push_back( n );
						
						CurrentFilename	= importFilename;
						CurrentLineno	= 1;
						if( FileStack.size() == 1 )
						{
							/* root import */
							Scanner::Ref().AddImportFile( importFilename );
						}
						
						// Open new file to parse.
						yyin = fopen( importFilename.c_str(), "r" );
						if( !yyin )
						{
							// Iterate all import path to find file.
							const std::vector<std::string> pathes = Scanner::Ref().GetSourcePathes();
							for( size_t i = 0; i < pathes.size(); ++i )
							{
								std::string tempfilename = pathes[i] + importFilename;
								yyin = fopen( tempfilename.c_str(), "r" );
								if( yyin )
									break;
							}
							if( !yyin )
							{
								// Can not found imported file.
								fprintf( stderr, "failed to import file \"%s\".\n", importFilename.c_str() );
								exit(1);
							}
						}
						yy_switch_to_buffer( yy_create_buffer( yyin, YY_BUF_SIZE ) );
						BEGIN( INITIAL );
					}
				}
<<EOF>>			{
					/* End of file */
					if( FileStack.size() == 0 )
					{
						// Root file, teminated.
						yyterminate();
					}
					else
					{
						// Recover file state.
						FileStackNode n = FileStack.back();
						FileStack.pop_back();
						CurrentFilename = n.filename_;
						CurrentLineno	= n.lineno_;
						yy_delete_buffer( YY_CURRENT_BUFFER );
						yy_switch_to_buffer( n.bs_ ); 
					}
				}

[a-ij-rs-zA-IJ-RS-Z_][a-ij-rs-zA-IJ-RS-Z0-9_]* {
					/* Identifier */
					yylval = std::string( yytext, yyleng );
					return TOKEN_IDENTIFIER;
				}
				
[1-9][0-9]*		{
					/* unsigned int */
					yylval = std::string( yytext, yyleng );
					return TOKEN_UINTEGER_LITERAL;
				}
				
\/\/.*{NL}      {
					/* C++ Comment */
					CurrentLineno++;
                }


"/*"			{
					/* C Comment. */
					for(;;) {
						char const c = yyinput();
						if (c == '*') {
							char const next = yyinput();
							if (next == '/')
								break;
							else
								unput(next);
						}
						else if (c == '\n') {
							CurrentLineno++;
						}
					}
				}

[ \t]*          { /*White space.*/ }

{NL}            { /*New Line.*/ CurrentLineno++;}

.               return yytext[0];

%%

/* subroutines */
