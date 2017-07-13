#include "config.h"
#include "visiter.h"
#include <boost/filesystem.hpp>
Printer::Printer(std::ostream &stream)
: stream_(stream){

}
void Printer::Indent(){
	indent_ += "  ";
}
void Printer::Outdent(){
	if (indent_.empty()){
		return;
	}
	indent_.resize(indent_.size() - 2);
}
void Printer::PrintLine(const std::string &a){
	stream_ << indent_ << a << std::endl;
}

void Printer::Print(const std::string &a){
	stream_ << indent_ << a;
}

void Printer::Append(const std::string &a){
	stream_ << a;
}

CodeFile::CodeFile(const std::string &outPath, const std::string &basename, const std::string extname, const std::string &code){
	if (!boost::filesystem::exists(outPath)){
		boost::filesystem::create_directory(outPath);
	}
	boost::filesystem::path filepath(outPath);
	filepath = filepath / (basename + "." + extname) ;
	outFile_.open(filepath.string());
	outFile_ << code;
}
CodeFile::~CodeFile(){
	outFile_.flush();
	outFile_.close();
}