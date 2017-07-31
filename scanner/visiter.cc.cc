#include "config.h"
#include "visiter.h"
#include "enumer.h"
#include "schema.h"
#include "service.h"

void CPPVisiter::PrintSchemaINC(Printer &printer, Schema* schema){
	const std::vector<Field> & fieldList = schema->GetFieldList();

	printer.PrintLine("enum {");
	printer.Indent();
	int fieldCountBegin = 0;
	if (schema->GetSuper())
		fieldCountBegin = schema->GetSuper()->GetFieldCount();
	for (size_t i = 0; i < fieldList.size(); ++i){
		printer.PrintLine("k%1_%2 = %3,", schema->GetFullName(), fieldList[i].GetName(), fieldCountBegin + i);
	}
	printer.PrintLine("k%1_Max = %2,", schema->GetFullName(), schema->GetFieldCount());
	printer.Outdent();
	printer.PrintLine("};");

	if (schema->GetSuper())
		printer.PrintLine("struct %1 : public %2{", schema->GetFullName(), schema->GetSuper()->GetFullName());
	else
		printer.PrintLine("struct %1{", schema->GetFullName());

	printer.Indent();

	printer.PrintLine("%1();", schema->GetFullName());
	printer.PrintLine("template<class WRITER>void Serialize(WRITER *w) const ;");
	printer.PrintLine("template<class READER>bool Deserialize(READER *r);");

	
	for (size_t i = 0; i < fieldList.size(); ++i){
		printer.PrintLine("%1 %2;", fieldList[i].GetCPPType(), fieldList[i].GetName());
	}
	printer.Outdent();
	printer.PrintLine("};");
}

void CPPVisiter::PrintSchemaSRC(Printer &printer, Schema* schema){
	//构造
	const std::vector<Field> & fieldList = schema->GetFieldList();
	printer.PrintLine("%1::%1()", schema->GetFullName());
	bool commCheck = false;
	for (size_t i = 0; i < fieldList.size(); ++i){
		if (fieldList[i].IsComplex())
			continue;
		printer.PrintLine("%1 %2(%3)", (commCheck ? "," : ":"), fieldList[i].GetName(), fieldList[i].GetCPPDefault());
		commCheck = true;
	}
	printer.PrintLine("{}");

	//序列化
	printer.PrintLine("template<class WRITER>void %1::Serialize(WRITER *w) const{", schema->GetFullName());
	printer.Indent();

	if (schema->GetSuper()){
		printer.PrintLine("%1::Serialize<WRITER>(w);", schema->GetSuper()->GetFullName());
	}

	//标记参数位
	if (!fieldList.empty()){
		printer.PrintLine("typedef PRPCMask<%1> MaskType;", schema->GetFieldMaskLength());
		printer.PrintLine("MaskType mask;");
		for (size_t i = 0; i < fieldList.size(); ++i){
			if (fieldList[i].IsArray()){
				printer.PrintLine("mask.WriteBit(!%1.empty());", fieldList[i].GetName());
			}
			else if (fieldList[i].IsString()){
				printer.PrintLine("mask.WriteBit(!%1.empty());", fieldList[i].GetName());
			}
			else if (fieldList[i].IsSchema()){
				printer.PrintLine("mask.WriteBit(true); // %1", fieldList[i].GetName());
			}
			else if (fieldList[i].IsBoolean()){
				printer.PrintLine("mask.WriteBit(!%1);", fieldList[i].GetName());
			}
			else{
				printer.PrintLine("mask.WriteBit(%1!=%2);", fieldList[i].GetName(), fieldList[i].GetCPPDefault());
			}
		}
		printer.PrintLine("w->WriteBytes(mask.Bytes(),MaskType::LENGTH);");

		//序列化字段
		for (size_t i = 0; i < fieldList.size(); ++i){
			printer.PrintLine("//S %1", fieldList[i].GetName());
			if (fieldList[i].IsArray()){
				printer.PrintLine("if(!%1.empty()){", fieldList[i].GetName());
				printer.Indent();
				printer.PrintLine("w->WriteSize(%1.size());", fieldList[i].GetName());
				printer.PrintLine("for(size_t i=0; i<%1.size(); ++i){", fieldList[i].GetName());
				printer.Indent();
				if (fieldList[i].IsSchema()){
					printer.PrintLine("%1[i].Serialize(w);", fieldList[i].GetName());
				}
				else if (fieldList[i].IsEnumer()){
					printer.PrintLine("w->Write((uint8_t)%1[i]);", fieldList[i].GetName());
				}
				else{
					printer.PrintLine("w->Write(%1[i]);", fieldList[i].GetName());
				}
				printer.Outdent();
				printer.PrintLine("}");
				printer.Outdent();
				printer.PrintLine("}");
			}
			else if (fieldList[i].IsSchema()){
				printer.PrintLine("{");
				printer.Indent();
				printer.PrintLine("%1.Serialize(w);", fieldList[i].GetName());
				printer.Outdent();
				printer.PrintLine("}");
			}
			else if (fieldList[i].IsBoolean()){
				printer.PrintLine("{");
				printer.PrintLine("}");
			}
			else if (fieldList[i].IsEnumer()){
				printer.PrintLine("if(!!%1){", fieldList[i].GetName());
				printer.Indent();
				printer.PrintLine("w->Write((uint8_t)%1);", fieldList[i].GetName());
				printer.Outdent();
				printer.PrintLine("}");
			}
			else if (fieldList[i].IsString()){
				printer.PrintLine("if(!%1.empty()){", fieldList[i].GetName(), fieldList[i].GetCPPDefault());
				printer.Indent();
				printer.PrintLine("w->Write(%1);", fieldList[i].GetName());
				printer.Outdent();
				printer.PrintLine("}");
			}
			else{
				printer.PrintLine("if(%1!=%2){", fieldList[i].GetName(), fieldList[i].GetCPPDefault());
				printer.Indent();
				printer.PrintLine("w->Write(%1);", fieldList[i].GetName());
				printer.Outdent();
				printer.PrintLine("}");
			}
		}
	}

	printer.Outdent();
	printer.PrintLine("}");

	//反序列化
	printer.PrintLine("template<class READER>bool %1::Deserialize(READER *r) {", schema->GetFullName());
	printer.Indent();
	if (schema->GetSuper()){
		printer.PrintLine("if(!%1::Deserialize<READER>(r)){", schema->GetSuper()->GetFullName());
		printer.Indent();
		printer.PrintLine("return false;");
		printer.Outdent();
		printer.PrintLine("}");
	}
	//获得参数位
	if (!fieldList.empty()){
		printer.PrintLine("typedef PRPCMask<%1> MaskType;", schema->GetFieldMaskLength());
		printer.PrintLine("MaskType mask;");
		printer.PrintLine("if(!r->ReadBytes(mask.Bytes(),MaskType::LENGTH)){");
		printer.Indent();
		printer.PrintLine("return false;");
		printer.Outdent();
		printer.PrintLine("}");

		//反序列化字段
		for (size_t i = 0; i < fieldList.size(); ++i){
			printer.PrintLine("//D %1", fieldList[i].GetName());
			if (fieldList[i].IsArray()){
				printer.PrintLine("if(mask.ReadBit()){");
				printer.Indent();
				printer.PrintLine("size_t size = 0;");
				printer.PrintLine("if(!r->ReadSize(size) || size > %1){", fieldList[i].GetMaxLength());
				printer.Indent();
				printer.PrintLine("return false;");
				printer.Outdent();
				printer.PrintLine("}");
				printer.PrintLine("%1.resize(size);", fieldList[i].GetName());
				printer.PrintLine("for(size_t i=0; i<size; ++i){");
				printer.Indent();
				if (fieldList[i].IsSchema()){
					printer.PrintLine("if(!%1[i].Deserialize(r)){", fieldList[i].GetName());
					printer.Indent();
					printer.PrintLine("return false;");
					printer.Outdent();
					printer.PrintLine("}");
				}
				else if (fieldList[i].IsEnumer()){
					printer.PrintLine("uint8_t enumer = 0;");
					printer.PrintLine("if(!r->Read(enumer) || enumer >= %1 ){", fieldList[i].GetType()->AsEnumer()->GetItemList().size());
					printer.Indent();
					printer.PrintLine("return false;");
					printer.Outdent();
					printer.PrintLine("}");
					printer.PrintLine("%1[i] = (%2)enumer;", fieldList[i].GetName(), fieldList[i].GetType()->GetName());
				}
				else {
					printer.PrintLine("if(!r->Read(%1[i])){", fieldList[i].GetName());
					printer.Indent();
					printer.PrintLine("return false;");
					printer.Outdent();
					printer.PrintLine("}");
				}
				printer.Outdent();
				printer.PrintLine("}");
				printer.Outdent();
				printer.PrintLine("}");
			}
			else if (fieldList[i].IsSchema()){
				printer.PrintLine("if(mask.ReadBit()){");
				printer.Indent();
				printer.PrintLine("if(!%1.Deserialize(r)){", fieldList[i].GetName());
				printer.Indent();
				printer.PrintLine("return false;");
				printer.Outdent();
				printer.PrintLine("}");
				printer.Outdent();
				printer.PrintLine("}");
			}
			else if (fieldList[i].IsEnumer()){
				printer.PrintLine("if(mask.ReadBit()){");
				printer.Indent();
				printer.PrintLine("uint8_t enumer = 0;");
				printer.PrintLine("if(!r->Read(enumer) || enumer >= %1 ){", fieldList[i].GetType()->AsEnumer()->GetItemList().size());
				printer.Indent();
				printer.PrintLine("return false;");
				printer.Outdent();
				printer.PrintLine("}");
				printer.PrintLine("%1 = (%2)enumer;", fieldList[i].GetName(), fieldList[i].GetType()->GetName());
				printer.Outdent();
				printer.PrintLine("}");
			}
			else if (fieldList[i].IsBoolean()){
				printer.PrintLine("{");
				printer.Indent();
				printer.PrintLine("%1 = mask.ReadBit();", fieldList[i].GetName());
				printer.Outdent();
				printer.PrintLine("}");
			}
			else {
				printer.PrintLine("if(mask.ReadBit()){");
				printer.Indent();
				printer.PrintLine("if(!r->Read(%1)){", fieldList[i].GetName());
				printer.Indent();
				printer.PrintLine("return false;");
				printer.Outdent();
				printer.PrintLine("}");
				printer.Outdent();
				printer.PrintLine("}");
			}
		}
	}
	printer.PrintLine("return true;");
	printer.Outdent();
	printer.PrintLine("}");
}

void CPPVisiter::Accept(Enumer* enumer){
	
	{
		std::stringstream ss;
		Printer printer(ss);
		printer.PrintLine("enum %1 {", enumer->GetName());
		printer.Indent();
		const std::vector<std::string> & itemList = enumer->GetItemList();
		for (size_t i = 0; i < itemList.size(); ++i){
			printer.PrintLine("%1 = %2,", itemList[i], i);
		}
		printer.Outdent();
		printer.PrintLine("};");

		printer.PrintLine("std::string ToName_%1(%1 id);", enumer->GetName());
		printer.PrintLine("%1 ToId_%1(const std::string & name);", enumer->GetName());
		
		
		CodeFile code(outpath_ + package_, enumer->GetName(), "inc", ss.str());
	}

	{
		std::stringstream ss;
		Printer printer(ss);
		const std::vector<std::string> & itemList = enumer->GetItemList();

		printer.PrintLine("static const char* NAME_%1[] = {", enumer->GetName());
		printer.Indent();
		for (size_t i = 0; i < itemList.size(); ++i){
			printer.PrintLine("\"%1\",", itemList[i]);
		}
		printer.Outdent();
		printer.PrintLine("};");

		printer.PrintLine("std::string ToName_%1(%1 id){", enumer->GetName());
		printer.Indent();
		printer.PrintLine("if(id < 0 || id >= %1){", itemList.size());
		printer.Indent();
		printer.PrintLine("return \"\";");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("return NAME_%1[id];", enumer->GetName());
		printer.Outdent();
		printer.PrintLine("}");

		printer.PrintLine("%1 ToId_%1(const std::string & name){", enumer->GetName());
		printer.Indent();
		printer.PrintLine("for(size_t i=0; i<%1; ++i){", itemList.size());
		printer.Indent();
		printer.PrintLine("if(name==NAME_%1[i]){", enumer->GetName());
		printer.Indent();
		printer.PrintLine("return (%1)i;", enumer->GetName());
		printer.Outdent();
		printer.PrintLine("}");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("return (%1)-1;", enumer->GetName());
		printer.Outdent();
		printer.PrintLine("}");

		CodeFile code(outpath_ + package_, enumer->GetName(), "src", ss.str());
	}
	{
		Printer printer(incCode_);
		printer.PrintLine("#include \"%1.inc\"",enumer->GetName());
		printer.PrintLine("#include \"%1.src\"", enumer->GetName());
	}

}

void CPPVisiter::Accept(Schema* schema){
	{
		std::stringstream ss;
		Printer printer(ss);
		
		PrintSchemaINC(printer,schema);

		CodeFile code(outpath_ + package_, schema->GetName(), "inc", ss.str());
	}

	{
		std::stringstream ss;
		Printer printer(ss);
		
		PrintSchemaSRC(printer, schema);

		CodeFile code(outpath_ + package_, schema->GetName(), "src", ss.str());
	}

	{
		Printer printer(incCode_);
		printer.PrintLine("#include \"%1.inc\"", schema->GetName());
		printer.PrintLine("#include \"%1.src\"", schema->GetName());
	}
}

void CPPVisiter::Accept(Service* service){
	{
		std::stringstream ss;
		Printer printer(ss);

		//生成包定义
		const std::vector<Schema*> schemaList = service->GetSchemaList();
		for (size_t i = 0; i < schemaList.size(); ++i){
			if (schemaList[i]->GetFieldList().empty())
				continue;
			PrintSchemaINC(printer, schemaList[i]);
		}

		//生成发送端 S

		printer.PrintLine("template<class WRITER>class %1Stub {", service->GetName());
		printer.PrintLine("public:");
		printer.Indent();
		
		for (size_t i = 0; i < schemaList.size(); ++i){
			printer.Print("void %1(",schemaList[i]->GetName());
			const std::vector<Field>&fieldList = schemaList[i]->GetFieldList();
			for (size_t j = 0; j < fieldList.size(); ++j){
				printer.Append("%1 %2%3%4 ", fieldList[j].GetCPPType(), (fieldList[j].IsComplex() ? "&" : ""), fieldList[j].GetName(), ((j + 1) != fieldList.size() ? "," : ""));
			}
			printer.Append("); // %1\n",i);
		}

		printer.PrintLine("virtual WRITER* MethodBegin() = 0;");
		printer.PrintLine("virtual void MethodEnd() = 0;");

		printer.Outdent();
		printer.PrintLine("};");

	

		//生成解包处理
		printer.PrintLine("template<class READER,class PROXY>class %1Dispatcher {", service->GetName());
		printer.PrintLine("public:");
		printer.Indent();

		for (size_t i = 0; i < schemaList.size(); ++i){
			printer.PrintLine("static bool %1(READER *r, PROXY *p); // %2", schemaList[i]->GetName(), i);
		}

		printer.PrintLine("static bool Execute(READER *r, PROXY *p);");

		printer.Outdent();
		printer.PrintLine("};");

		CodeFile code(outpath_ + package_, service->GetName(), "inc", ss.str());
		

	}
	{
		std::stringstream ss;
		Printer printer(ss);
		const std::vector<Schema*> schemaList = service->GetSchemaList();
		//生成接受端
		for (size_t i = 0; i < schemaList.size(); ++i){
			printer.Print("bool %1(", schemaList[i]->GetName());
			const std::vector<Field>&fieldList = schemaList[i]->GetFieldList();
			for (size_t j = 0; j < fieldList.size(); ++j){
				printer.Append("%1 %2%3%4 ", fieldList[j].GetCPPType(), (fieldList[j].IsComplex() ? "&" : ""), fieldList[j].GetName(), ((j + 1) != fieldList.size() ? "," : ""));
			}
			printer.Append("); // %1\n", i);
		}

		CodeFile code(outpath_ + package_, service->GetName(), "fun", ss.str());
	}

	{
		std::stringstream ss;
		Printer printer(ss);

		const std::vector<Schema*> schemaList = service->GetSchemaList();
		for (size_t i = 0; i < schemaList.size(); ++i){
			if (schemaList[i]->GetFieldList().empty())
				continue;
			PrintSchemaSRC(printer, schemaList[i]);
		}

		//发送端函数实现
		for (size_t i = 0; i < schemaList.size(); ++i){
			printer.Print("template<class WRITER>void %1Stub<WRITER>::%2(", service->GetName(), schemaList[i]->GetName());
			const std::vector<Field>&fieldList = schemaList[i]->GetFieldList();
			for (size_t j = 0; j < fieldList.size(); ++j){
				printer.Append("%1 %2%3%4 ", fieldList[j].GetCPPType(), (fieldList[j].IsComplex() ? "&" : ""), fieldList[j].GetName(), ((j + 1) != fieldList.size() ? "," : ""));
			}
			printer.Append("){\n", i);
			printer.Indent();
			printer.PrintLine("WRITER *w = MethodBegin();");
			printer.PrintLine("if(!w){");
			printer.Indent();
			printer.PrintLine("return;");
			printer.Outdent();
			printer.PrintLine("}");
			printer.PrintLine("w->Write<uint16_t>(%1);", i);
			if (!fieldList.empty()){
				printer.PrintLine("%1 _%2;", schemaList[i]->GetFullName(), i);
				for (size_t j = 0; j < fieldList.size(); ++j){
					printer.PrintLine("_%1.%2 = %2;", i, fieldList[j].GetName());
				}
				printer.PrintLine("_%1.Serialize<WRITER>(w);", i);
			}
			printer.PrintLine("MethodEnd();");
			printer.Outdent();
			printer.PrintLine("}");

		}

		//解析端函数实现
		for (size_t i = 0; i < schemaList.size(); ++i){
			printer.PrintLine("template<class READER,class PROXY>bool %1Dispatcher<READER,PROXY>::%2(READER *r, PROXY *p){", service->GetName(), schemaList[i]->GetName());
			const std::vector<Field>&fieldList = schemaList[i]->GetFieldList();
			printer.Indent();
			printer.PrintLine("if(!r){");
			printer.Indent();
			printer.PrintLine("return false;");
			printer.Outdent();
			printer.PrintLine("}");
			printer.PrintLine("if(!p){");
			printer.Indent();
			printer.PrintLine("return false;");
			printer.Outdent();
			printer.PrintLine("}");
			if (!fieldList.empty()){
				printer.PrintLine("%1 _%2;", schemaList[i]->GetFullName(), i);
				printer.PrintLine("if(!_%1.Deserialize<READER>(r)){", i);
				printer.Indent();
				printer.PrintLine("return false;");
				printer.Outdent();
				printer.PrintLine("}");
				printer.Print("return p->%1(", schemaList[i]->GetName());
				for (size_t j = 0; j < fieldList.size(); ++j){
					printer.Append("_%1.%2%3", i, fieldList[j].GetName(), ((j + 1) != fieldList.size() ? "," : ""));
				}
				printer.Append(");\n");
			}
			else{
				printer.PrintLine("return p->%1();", schemaList[i]->GetName());
			}
			printer.Outdent();
			printer.PrintLine("}");

		}

		printer.PrintLine("template<class READER,class PROXY>bool %1Dispatcher<READER,PROXY>::Execute(READER *r, PROXY *p){", service->GetName());
		if (!schemaList.empty()){
			printer.Indent();
			printer.PrintLine("if(!r){");
			printer.Indent();
			printer.PrintLine("return false;");
			printer.Outdent();
			printer.PrintLine("}");
			printer.PrintLine("if(!p){");
			printer.Indent();
			printer.PrintLine("return false;");
			printer.Outdent();
			printer.PrintLine("}");
			printer.PrintLine("uint16_t pid = -1;");
			printer.PrintLine("if(!r->Read(pid)){");
			printer.Indent();
			printer.PrintLine("return false;");
			printer.Outdent();
			printer.PrintLine("}");
			printer.PrintLine("switch(pid){");
			printer.Indent();
			for (size_t i = 0; i < schemaList.size(); ++i){
				printer.PrintLine("case %1 :", i);
				printer.Indent();
				printer.PrintLine("return %1Dispatcher<READER,PROXY>::%2(r,p);", service->GetName(), schemaList[i]->GetName());
				printer.Outdent();
			}
			printer.PrintLine("default:");
			printer.Indent();
			printer.PrintLine("return false;");
			printer.Outdent();
			printer.Outdent();
			printer.PrintLine("}");
		}
		else{
			printer.Indent();
			printer.PrintLine("return true;");
		}
		printer.Outdent();
		printer.PrintLine("}");

		CodeFile code(outpath_ + package_, service->GetName(), "src", ss.str());

	}

	{
		Printer printer(incCode_);
		printer.PrintLine("#include \"%1.inc\"", service->GetName());
		printer.PrintLine("#include \"%1.src\"", service->GetName());
	}
}

void CPPVisiter::Begin(){
	{
		Printer printer(incCode_);
		printer.PrintLine("#ifndef ____PRPC_GEN_H___");
		printer.PrintLine("#define ____PRPC_GEN_H___");
		printer.PrintLine("#include \"prpc.h\"");
		if (!package_.empty())
			printer.PrintLine("namespace %1{", package_);
	}
}

void CPPVisiter::End(){
	{
		Printer printer(incCode_);
		if (!package_.empty())
			printer.PrintLine("}");
		printer.PrintLine("#endif //____PRPC_GEN_H___");

		CodeFile code(outpath_, "prpc.gen", "hpp", incCode_.str());
	}
}
