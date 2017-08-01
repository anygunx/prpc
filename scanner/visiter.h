#ifndef __VISITER_H__
#define __VISITER_H__
#include "config.h"
#include "strings.h"

class Printer{
public:
	Printer(std::ostream &stream);
	void Indent();
	void Outdent();

	template<class T>
	void PrintLine(const std::string &a, const T& v1){
		stream_ << indent_ << strings::Sprintf(a, v1) << std::endl;
	}
	template<class T1, class T2>
	void PrintLine(const std::string &a, const T1& v1, const T2& v2){
		stream_ << indent_ << strings::Sprintf(a, v1, v2) << std::endl;
	}
	template<class T1, class T2, class T3>
	void PrintLine(const std::string &a, const T1& v1, const T2& v2, const T3& v3){
		stream_ << indent_ << strings::Sprintf(a, v1, v2, v3) << std::endl;
	}
	template<class T1, class T2, class T3, class T4>
	void PrintLine(const std::string &a, const T1& v1, const T2& v2, const T3& v3, const T4& v4){
		stream_ << indent_ << strings::Sprintf(a, v1, v2, v3, v4) << std::endl;
	}
	template<class T1, class T2, class T3, class T4, class T5>
	void PrintLine(const std::string &a, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5){
		stream_ << indent_ << strings::Sprintf(a, v1, v2, v3, v4, v5) << std::endl;
	}
	template<class T1, class T2, class T3, class T4, class T5, class T6>
	void PrintLine(const std::string &a, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6){
		stream_ << indent_ << strings::Sprintf(a, v1, v2, v3, v4, v5, v6) << std::endl;
	}

	template<class T>
	void Print(const std::string &a, const T& v1){
		stream_ << indent_ << strings::Sprintf(a, v1);
	}
	template<class T1, class T2>
	void Print(const std::string &a, const T1& v1, const T2& v2){
		stream_ << indent_ << strings::Sprintf(a, v1, v2);
	}
	template<class T1, class T2, class T3>
	void Print(const std::string &a, const T1& v1, const T2& v2, const T3& v3){
		stream_ << indent_ << strings::Sprintf(a, v1, v2, v3);
	}
	template<class T1, class T2, class T3, class T4>
	void Print(const std::string &a, const T1& v1, const T2& v2, const T3& v3, const T4& v4){
		stream_ << indent_ << strings::Sprintf(a, v1, v2, v3, v4);
	}
	template<class T1, class T2, class T3, class T4, class T5>
	void Print(const std::string &a, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5){
		stream_ << indent_ << strings::Sprintf(a, v1, v2, v3, v4, v5);
	}
	template<class T1, class T2, class T3, class T4, class T5, class T6>
	void Print(const std::string &a, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6){
		stream_ << indent_ << strings::Sprintf(a, v1, v2, v3, v4, v5, v6);
	}

	template<class T>
	void Append(const std::string &a, const T& v1){
		stream_ << strings::Sprintf(a, v1);
	}
	template<class T1, class T2>
	void Append(const std::string &a, const T1& v1, const T2& v2){
		stream_ << strings::Sprintf(a, v1, v2);
	}
	template<class T1, class T2, class T3>
	void Append(const std::string &a, const T1& v1, const T2& v2, const T3& v3){
		stream_ << strings::Sprintf(a, v1, v2, v3);
	}
	template<class T1, class T2, class T3, class T4>
	void Append(const std::string &a, const T1& v1, const T2& v2, const T3& v3, const T4& v4){
		stream_ << strings::Sprintf(a, v1, v2, v3, v4);
	}
	template<class T1, class T2, class T3, class T4, class T5>
	void Append(const std::string &a, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5){
		stream_ << strings::Sprintf(a, v1, v2, v3, v4, v5);
	}
	template<class T1, class T2, class T3, class T4, class T5, class T6>
	void Append(const std::string &a, const T1& v1, const T2& v2, const T3& v3, const T4& v4, const T5& v5, const T6& v6){
		stream_  << strings::Sprintf(a, v1, v2, v3, v4, v5, v6);
	}


	void PrintLine(const std::string &a);
	void Print(const std::string &a);
	void Append(const std::string &a);
	
private:
	std::ostream &stream_;
	std::string indent_;
};

class CodeFile{
public:
	CodeFile(const std::string &outPath, const std::string &basename, const std::string extname , const std::string &code);
	~CodeFile();
	std::ofstream outFile_;
};

class CodeVisiter {
public:
	virtual void Begin(){}
	virtual void End(){}
	virtual void Accept(class Enumer* enumer) = 0;
	virtual void Accept(class Schema* schema) = 0;
	virtual void Accept(class Service* service) = 0;

protected:
	std::string package_;
	std::string outpath_;
};


//////////////////////////////////////////////////////////////////////////

class CCVisiter : public CodeVisiter{
public:
	CCVisiter(const std::string & package, const std::string& outpath){
		package_ = package;
		outpath_ = outpath;
	}

	void Begin();
	void End();

	void Accept(Enumer* enumer);
	void Accept(Schema* schema);
	void Accept(Service* service);
	
private:
	void PrintSchemaINC(Printer &printer, Schema* schema);
	void PrintSchemaSRC(Printer &printer, Schema* schema);

private:
	std::stringstream incCode_;
	std::stringstream srcCode_;
};

//////////////////////////////////////////////////////////////////////////

class CSVisiter : public CodeVisiter{
public:

	CSVisiter(const std::string & package, const std::string& outpath){
		package_ = package;
		outpath_ = outpath;
	}

	void Accept(Enumer* enumer);
	void Accept(Schema* schema);
	void Accept(Service* service);
private:
	void PrintSchema(Printer &printer, Schema* schema, bool needPublic = true);
};


class GOVisiter : public CodeVisiter{
public:
	GOVisiter(const std::string & package, const std::string& outpath){
		package_ = package;
		outpath_ = outpath;
		if (package_.empty())
			package_ = "prpc";
	}
	
	void Accept(Enumer* enumer);
	void Accept(Schema* schema);
	void Accept(Service* service);
private:
	void PrintSchemaDef(Printer &printer, Schema* schema);
	void PrintSchemaFun(Printer &printer, Schema* schema);
};

#endif// __CodeFile_h__