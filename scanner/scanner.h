#ifndef __Compiler_h__
#define __Compiler_h__
#include "config.h"
#include "strconv.h"
#include "strings.h"
#include "node.h"
#include "enumer.h"
#include "schema.h"
#include "service.h"

/** arpc语言编译器类.
	Compiler 通过flex和bison对源文件进行分析，构建符合arpc语法规范的抽象语法
	树，然后生成对应的代码文件.
*/
class Scanner
{
public:
	/** 获得全局引用. */
	static Scanner& Ref();

	Scanner();
	~Scanner();

	/** 编译源文件. */
	int Run(int argc, char *argv[]);

	/** 根据名称查找一个定义. 
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