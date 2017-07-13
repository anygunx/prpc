#ifndef __Compiler_h__
#define __Compiler_h__
#include "config.h"
#include "strconv.h"
#include "strings.h"
#include "node.h"
#include "enumer.h"
#include "schema.h"
#include "service.h"

/** arpc���Ա�������.
	Compiler ͨ��flex��bison��Դ�ļ����з�������������arpc�﷨�淶�ĳ����﷨
	����Ȼ�����ɶ�Ӧ�Ĵ����ļ�.
*/
class Scanner
{
public:
	/** ���ȫ������. */
	static Scanner& Ref();

	Scanner();
	~Scanner();

	/** ����Դ�ļ�. */
	int Run(int argc, char *argv[]);

	/** �������Ʋ���һ������. 
	*/
	Node* GetNode( const std::string& name );

	void AddNode(Node * node);

	const std::vector<std::string> & GetSourcePathes();

	void AddSourcePath(const std::string& pathname);

	void AddImportFile(const std::string& filename);

	void SetPackage(const std::string &package){
		package_ = package;
	}

	class CodeVisiter* CreateCodeVisiter(const std::string &name);

	std::string						rootFilename_;
	std::string						outputPath_;
	std::vector<std::string>		sourcePathes_;
	std::string						package_;

	std::vector<std::string>		imports_;
	std::vector<Node*>	nodeList_;
};

#endif//__Compiler_h__