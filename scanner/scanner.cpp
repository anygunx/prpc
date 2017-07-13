#include "config.h"
#include "scanner.h"
#include "node.h"
#include "visiter.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

int yyparse();
extern FILE *yyin;
extern std::string  CurrentFilename;
extern int			CurrentLineno;

Scanner& Scanner::Ref()
{
	static Scanner spec;
	return spec;
}

Scanner::Scanner(){
	AddNode(new Int8());
	AddNode(new Int16());
	AddNode(new Int32());
	AddNode(new Int64());
	AddNode(new UInt8());
	AddNode(new UInt16());
	AddNode(new UInt32());
	AddNode(new UInt64());
	AddNode(new Float32());
	AddNode(new Float64());
	AddNode(new String());
	AddNode(new Boolean());
}

Scanner::~Scanner()
{
	// 删除所有的definition实例.
	for(size_t i = 0; i < nodeList_.size(); i++)
		delete nodeList_[i];
}

Node* Scanner::GetNode(const std::string& name)
{
	for(size_t i = 0; i < nodeList_.size(); i++)
	{
		if(nodeList_[i]->GetName() == name)
			return nodeList_[i];
	}

	return NULL;
}

void Scanner::AddNode(Node * node){
	nodeList_.push_back(node);
}

void Scanner::AddImportFile(const std::string& filename){
	imports_.push_back(filename);
}


const std::vector<std::string> & Scanner::GetSourcePathes(){
	return sourcePathes_;
}

void Scanner::AddSourcePath(const std::string& pathname){
	sourcePathes_.push_back(pathname);
}

CodeVisiter* Scanner::CreateCodeVisiter(const std::string &name){
	if (name == "cpp"){
		return new CPPVisiter(package_, outputPath_);
	}
	else if (name == "cs"){
		return new CSVisiter(package_, outputPath_);
	}
	else if (name == "go"){
		return new GOVisiter(package_, outputPath_);
	}
	else {
		return NULL;
	}
}

int Scanner::Run(int argc, char *argv[])
{
	namespace po = boost::program_options;
	po::options_description desc("PRPC scanner.\nAllowed options");
	desc.add_options()
		("help,h", "produce help message.")
		("input,i", po::value<std::string>(), "input file name.")
		("output,o", po::value<std::string>(), "output directory.")
		("gen,g", po::value<std::string>(), "type of generated code.(cpp,cs,go)")
		("package", po::value<std::string>(), "defined namespace.")
		;
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cout << desc << "\n";
		return 1;
	}

	rootFilename_ = vm["input"].as<std::string>();
	
	if (vm.count("output")){
		outputPath_ = vm["output"].as<std::string>();
	}
	else {
		outputPath_ = "./";
	}

	CodeVisiter *visiter = CreateCodeVisiter(vm["gen"].as<std::string>());
	
	if (!visiter){
		return -1;
	}

	// 打开源文件.
	yyin = fopen(rootFilename_.c_str(), "r");
	if(yyin == NULL)
	{
		return 1;
	}

	boost::filesystem::path filepath(rootFilename_);
	AddSourcePath(filepath.parent_path().string() + "/");
	CurrentFilename = filepath.string();
	// 开始分析文件.
	int r;
	if(r = yyparse())
	{
		fclose(yyin);
		return r;
	}

	fclose(yyin);

	visiter->Begin();
	for (size_t i = 0; i < nodeList_.size(); ++i){
		nodeList_[i]->Enter(visiter);
	}
	visiter->End();
	delete visiter;

	return 0;
}
