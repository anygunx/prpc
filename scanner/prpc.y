

/* Declarations */

%{
#define YYSTYPE std::string

#include "config.h"
#include "scanner.h"

void yyerror (const char *);
int yylex (void);

Enumer EnumerContext;
Schema SchemaContext;
Service ServiceContext;

extern std::string CurrentFilename;
extern int CurrentLineno;
%}

%error-verbose


/*
 * Token types: These are returned by the lexer
 */
%token				TOKEN_IDENTIFIER
%token				TOKEN_ENUM
%token				TOKEN_STRUCT
%token				TOKEN_SERVICE
%token				TOKEN_INT64
%token				TOKEN_UINT64
%token				TOKEN_DOUBLE
%token				TOKEN_FLOAT
%token				TOKEN_INT32
%token				TOKEN_UINT32
%token				TOKEN_INT16
%token				TOKEN_UINT16
%token				TOKEN_INT8
%token				TOKEN_UINT8
%token				TOKEN_BOOL
%token				TOKEN_STRING
%token				TOKEN_ARRAY
%token				TOKEN_UINTEGER_LITERAL


%%

/*
 * Production starts here.
 */
start: DEFINITIONS;

/* definitions */
DEFINITIONS: 
	DEFINITIONS DEFINITION
	| 
	/* empty */
	;

/*definition*/
DEFINITION:
	ENUMER ';'
	|
	SCHEMA ';'
	|
	SERVICE ';'
    ;

/*enumeration*/
ENUMER:
    TOKEN_ENUM 
    TOKEN_IDENTIFIER
    {
    	// Check enum name.
		if( Scanner::Ref().GetNode( $2 ) ){
			yyerror("enumer type redef ");
			YYERROR; 
		};
		
		EnumerContext.SetName($2);
    }
    '{' ENUMER_ITEM_LIST '}'
	{
		Scanner::Ref().AddNode(EnumerContext.Clone());
		EnumerContext.Reset();
	}
	;



/*enum_items*/
ENUMER_ITEM_LIST:
	ENUMER_ITEM_LIST ENUMER_ITEM
	|
	/*empty*/
	;

/*enum_item*/	
ENUMER_ITEM:
	TOKEN_IDENTIFIER ','
	{
		// Check enum item name.
		if( Scanner::Ref().GetNode( $2 ) ){
			yyerror("enumer type redef ");
			YYERROR; 
		};
		if (!EnumerContext.AddItem($1)){
			yyerror("enumer field redef ");
			YYERROR;
		}
	}
	;

////////////////////////////////////////////////////////////////////////////////////////////////
SCHEMA:
	TOKEN_STRUCT
	TOKEN_IDENTIFIER
	{
		// Check struct name.
		if( Scanner::Ref().GetNode( $2 )){
			yyerror("struct type redef ");
			YYERROR; 
		};
		
		SchemaContext.SetName($2);
	}
	OPT_SUPER_SCHEMA
	'{' STRUCT_FIELD_LIST '}'
	{
		// Add this struct definition.
		Scanner::Ref().AddNode(SchemaContext.Clone());
		SchemaContext.Reset();
	}
	;

OPT_SUPER_SCHEMA:
	SUPER_SCHEMA
	|
	/*empty*/
	;

/*super_structure*/
SUPER_SCHEMA:
	':' TOKEN_IDENTIFIER
	{
		Node* super = Scanner::Ref().GetNode($2);
		if( !super || !super->AsSchema() || SchemaContext.GetName() == $2 ){
			yyerror("super type not found  ");
			YYERROR;
		}
		SchemaContext.SetSuper(super);
	}
	;

/*struct_fields*/
STRUCT_FIELD_LIST:
	STRUCT_FIELD_LIST STRUCT_FIELD
	|
	/* empty */
	;
	
/*struct_field*/
STRUCT_FIELD:
	FIELD_TYPE TOKEN_IDENTIFIER ';'
	{
		// Check field name.
		if( Scanner::Ref().GetNode( $2 ) ){
			yyerror("member name same to class name  ");
			YYERROR;
		}
		if( SchemaContext.GetField( $2 ) ){
			yyerror("member name redef ");
			YYERROR;
		}
		Node *type = Scanner::Ref().GetNode( $1 );
		if (!type){
			yyerror("type not found  ");
			YYERROR;
		}

		SchemaContext.AddField(type,$2);
	}
	|
	FIELD_TYPE '[' ']' TOKEN_IDENTIFIER ';'
	{
		// Check field name.
		if( Scanner::Ref().GetNode( $4 ) ){
			yyerror("member name same to class name  ");
			YYERROR;
		}
		if( SchemaContext.GetField( $4) ){
			yyerror("member name redef ");
			YYERROR;
		}
		Node *type = Scanner::Ref().GetNode( $1 );
		if (!type){
			yyerror("type not found  ");
			YYERROR;
		}

		SchemaContext.AddField(type,$4,0XFF);
	}
	|
	FIELD_TYPE '[' TOKEN_UINTEGER_LITERAL ']' TOKEN_IDENTIFIER ';'
	{
		// Check field name.
		if( Scanner::Ref().GetNode( $5 ) ){
			yyerror("member name same to class name  ");
			YYERROR;
		}
		if( SchemaContext.GetField( $5 ) ){
			yyerror("member name redef ");
			YYERROR;
		}
		Node *type = Scanner::Ref().GetNode( $1 );
		if (!type){
			yyerror("type not found  ");
			YYERROR;
		}

		SchemaContext.AddField(type,$5,strconv::Atoi($3));
	}
	;

SERVICE:
	TOKEN_SERVICE
	TOKEN_IDENTIFIER
	{
		if( Scanner::Ref().GetNode( $2 )){ 
			yyerror("service name redef ");
			YYERROR; 
		};
		
		ServiceContext.SetName($2);
	
	}
	'{' SERVICE_METHOD_LIST '}'
	{
		// Add this service.
		Scanner::Ref().AddNode(ServiceContext.Clone());
		ServiceContext.Reset();
	}
	;

SERVICE_METHOD_LIST:
	SERVICE_METHOD_LIST SERVICE_METHOD
	|
	/*empty*/
	;

SERVICE_METHOD:
	TOKEN_IDENTIFIER '(' SERVICE_METHOD_PARAM_LIST ')' ';'
	{
		if( ServiceContext.GetSchema( $1 ) ){
			yyerror("service function name redef ");
			YYERROR;
		}
		SchemaContext.SetName($1);
		ServiceContext.AddSchema(SchemaContext.Clone()->AsSchema());
		SchemaContext.Reset();
	}
	;

SERVICE_METHOD_PARAM_LIST:
	SERVICE_METHOD_PARAM_LIST ',' SERVICE_METHOD_PARAM
	|
	SERVICE_METHOD_PARAM
	|
	/*empty*/
	;

SERVICE_METHOD_PARAM:
	FIELD_TYPE TOKEN_IDENTIFIER
	{
		if( SchemaContext.GetField( $2 ) ){
			yyerror("service function parameter name redef ");
			YYERROR;
		}

		Node *type = Scanner::Ref().GetNode( $1 );
		if (!type){
			yyerror("service function parameter type not found ");
			YYERROR;
		}
		SchemaContext.AddField(type,$2);
	}
	|
	FIELD_TYPE '[' ']' TOKEN_IDENTIFIER
	{
		if( SchemaContext.GetField( $4 ) ){
			yyerror("service function parameter name redef ");
			YYERROR;
		}
		Node *type = Scanner::Ref().GetNode( $1 );
		if (!type){
			yyerror("service function parameter type not found ");
			YYERROR;
		}

		SchemaContext.AddField(type,$4,0XFF);
	}
	|
	FIELD_TYPE '[' TOKEN_UINTEGER_LITERAL ']' TOKEN_IDENTIFIER
	{
		if( SchemaContext.GetField( $5 ) ){
			yyerror("service function parameter name redef ");
			YYERROR;
		}
		Node *type = Scanner::Ref().GetNode( $1 );
		if (!type){
			yyerror("service function parameter type not found ");
			YYERROR;
		}

		SchemaContext.AddField(type,$5,strconv::Atoi($3));
	}
	;

FIELD_TYPE:
	TOKEN_INT64	{ $$="Int64"; }
	|
	TOKEN_UINT64	{ $$="UInt64"; }
	|
	TOKEN_DOUBLE	{ $$="Float64"; }
	|
	TOKEN_FLOAT	{ $$="Float32"; }
	|
	TOKEN_INT32	{ $$="Int32"; }
	|
	TOKEN_UINT32	{ $$="UInt32"; }
	|
	TOKEN_INT16	{ $$="Int16"; }
	|
	TOKEN_UINT16	{ $$="UInt16"; }
	|
	TOKEN_INT8	{ $$="Int8"; }
	|
	TOKEN_UINT8	{ $$="UInt8"; }
	|
	TOKEN_BOOL	{ $$="Boolean"; }
	|
	TOKEN_STRING { $$="String"; }
	|
	TOKEN_IDENTIFIER	
	{ 
		$$=$1;
	}
	;

%%


int yywrap (void)
{
  return 1;
}

void yyerror (const char *msg)
{
	std::cerr << "YACC ERROR | FILE[" << CurrentFilename << "] LINE(" << CurrentLineno << ") | " << msg;
}
