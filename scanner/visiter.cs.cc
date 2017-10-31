#include "config.h"
#include "visiter.h"
#include "enumer.h"
#include "schema.h"
#include "service.h"

void CSVisiter::PrintSchema(Printer &printer, Schema* schema, bool needPublic){
	const std::vector<Field> & fieldList = schema->GetFieldList();

	if (schema->GetSuper())
		printer.PrintLine("%3 class %1 : %2{", schema->GetFullName(), schema->GetSuper()->GetFullName(), (needPublic ? "public" : ""));
	else
		printer.PrintLine("%2 class %1{", schema->GetFullName(), (needPublic ? "public " : ""));

	printer.Indent();



	if (schema->GetSuper()){
		printer.PrintLine("public override void Serialize(IWriter w){");
		printer.Indent();
		printer.PrintLine("base.Serialize(w);");
	}
	else{
		printer.PrintLine("public virtual void Serialize(IWriter w){");
		printer.Indent();
	}

	if (!fieldList.empty()){
		printer.PrintLine("Mask mask = new Mask(%1);", schema->GetFieldMaskLength());
		for (size_t i = 0; i < fieldList.size(); ++i){
			if (fieldList[i].IsArray()){
				printer.PrintLine("mask.WriteBit(%1!=null&&%1.Length!=0);", fieldList[i].GetName());
			}
			else if (fieldList[i].IsString()){
				printer.PrintLine("mask.WriteBit(%1!=null&&%1.Length!=0&&%1!=\"\");", fieldList[i].GetName());
			}
			else if (fieldList[i].IsSchema()){
				printer.PrintLine("mask.WriteBit(true); // %1", fieldList[i].GetName());
			}
			else if (fieldList[i].IsBoolean()){
				printer.PrintLine("mask.WriteBit(%1);", fieldList[i].GetName());
			}
			else{
				printer.PrintLine("mask.WriteBit(%1!=%2);", fieldList[i].GetName(), fieldList[i].GetCSDefault());
			}
		}
		printer.PrintLine("w.Write(mask.Bytes);");

		//序列化字段
		for (size_t i = 0; i < fieldList.size(); ++i){
			printer.PrintLine("//S %1", fieldList[i].GetName());
			if (fieldList[i].IsArray()){
				printer.PrintLine("if(%1!=null&&%1.Length!=0){", fieldList[i].GetName());
				printer.Indent();
				printer.PrintLine("w.WriteSize(%1.Length);", fieldList[i].GetName());
				printer.PrintLine("for(int i=0; i<%1.Length; ++i){", fieldList[i].GetName());
				printer.Indent();
				if (fieldList[i].IsSchema()){
					printer.PrintLine("%1[i].Serialize(w);", fieldList[i].GetName());
				}
				else if (fieldList[i].IsEnumer()){
					printer.PrintLine("w.Write((byte)%1[i]);", fieldList[i].GetName());
				}
				else{
					printer.PrintLine("w.Write(%1[i]);", fieldList[i].GetName());
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
				printer.PrintLine("if(%1!=0){", fieldList[i].GetName());
				printer.Indent();
				printer.PrintLine("w.Write((byte)%1);", fieldList[i].GetName());
				printer.Outdent();
				printer.PrintLine("}");
			}
			else if (fieldList[i].IsString()){
				printer.PrintLine("if(%1!=null&&%1.Length!=0&&%1!=\"\"){", fieldList[i].GetName());
				printer.Indent();
				printer.PrintLine("w.Write(%1);", fieldList[i].GetName());
				printer.Outdent();
				printer.PrintLine("}");
			}
			else{
				printer.PrintLine("if(%1!=%2){", fieldList[i].GetName(), fieldList[i].GetCSDefault());
				printer.Indent();
				printer.PrintLine("w.Write(%1);", fieldList[i].GetName());
				printer.Outdent();
				printer.PrintLine("}");
			}
		}
	}

	printer.Outdent();
	printer.PrintLine("}");



	if (schema->GetSuper()){
		printer.PrintLine("public override bool Deserialize(IReader r){");
		printer.Indent();
		printer.PrintLine("if(!base.Deserialize(r)){");
		printer.Indent();
		printer.PrintLine("return false;");
		printer.Outdent();
		printer.PrintLine("}");
	}
	else {
		printer.PrintLine("public virtual bool Deserialize(IReader r){");
		printer.Indent();
	}

	//获得参数位
	if (!fieldList.empty()){
		printer.PrintLine("Mask mask = new Mask(%1);", schema->GetFieldMaskLength());
		printer.PrintLine("if(!r.Read(ref mask.Bytes)){");
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
				printer.PrintLine("int size = 0;");
				printer.PrintLine("if(!r.ReadSize(ref size) || size > %1){", fieldList[i].GetMaxLength());
				printer.Indent();
				printer.PrintLine("return false;");
				printer.Outdent();
				printer.PrintLine("}");
				printer.PrintLine("%1 = new %2[size];", fieldList[i].GetName(), fieldList[i].GetCSType(false));
				printer.PrintLine("for(int i=0; i<size; ++i){");
				printer.Indent();
				if (fieldList[i].IsSchema()){
					printer.PrintLine("%1[i] = new %2();", fieldList[i].GetName(), fieldList[i].GetCSType(false));
					printer.PrintLine("if(!%1[i].Deserialize(r)){", fieldList[i].GetName());
					printer.Indent();
					printer.PrintLine("return false;");
					printer.Outdent();
					printer.PrintLine("}");
				}
				else if (fieldList[i].IsEnumer()){
					printer.PrintLine("byte enumer = 0XFF;");
					printer.PrintLine("if(!r.Read(ref enumer) || enumer >= %1 ){", fieldList[i].GetType()->AsEnumer()->GetItemList().size());
					printer.Indent();
					printer.PrintLine("return false;");
					printer.Outdent();
					printer.PrintLine("}");
					printer.PrintLine("%1[i] = enumer;", fieldList[i].GetName());
				}
				else {
					printer.PrintLine("if(!r.Read(ref %1[i])){", fieldList[i].GetName());
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
				printer.PrintLine("if(mask.ReadBit()){", fieldList[i].GetName());
				printer.Indent();
				printer.PrintLine("if(%1==null){", fieldList[i].GetName());
				printer.Indent();
				printer.PrintLine("%1 = new %2();", fieldList[i].GetName(), fieldList[i].GetCSType());
				printer.Outdent();
				printer.PrintLine("}");
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
				printer.PrintLine("byte enumer = 0XFF;");
				printer.PrintLine("if(!r.Read(ref enumer) || enumer >= %1 ){", fieldList[i].GetType()->AsEnumer()->GetItemList().size());
				printer.Indent();
				printer.PrintLine("return false;");
				printer.Outdent();
				printer.PrintLine("}");
				printer.PrintLine("%1 = enumer;", fieldList[i].GetName());
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
				printer.PrintLine("if(!r.Read(ref %1)){", fieldList[i].GetName());
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

	printer.PrintLine("");

	if (schema->GetSuper())
		printer.PrintLine("public override void Release(){");
	else
		printer.PrintLine("public virtual void Release(){");
	printer.Indent();
	if (schema->GetSuper()){
		printer.PrintLine("base.Release();");
	}
	for (size_t i = 0; i < fieldList.size(); ++i){
		printer.PrintLine("this.%1 = %2;", fieldList[i].GetName(), fieldList[i].GetCSDefault());
	}
	printer.Outdent();
	printer.PrintLine("}");

	/*printer.PrintLine("public virtual void Copy(%1 from){", schema->GetFullName());
	printer.Indent();
	if (schema->GetSuper()){
	printer.PrintLine("base.Copy(from);");
	}
	for (size_t i = 0; i < fieldList.size(); ++i){
	printer.PrintLine("this.%1 = from.%1;", fieldList[i].GetName());
	}
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("public virtual void Clone(%1 to){", schema->GetFullName());
	printer.Indent();
	if (schema->GetSuper()){
	printer.PrintLine("base.Clone(to);");
	}
	for (size_t i = 0; i < fieldList.size(); ++i){
	printer.PrintLine("to.%1 = this.%1;", fieldList[i].GetName());
	}
	printer.Outdent();
	printer.PrintLine("}");*/

	for (size_t i = 0; i < fieldList.size(); ++i){
		printer.PrintLine("public %1 %2;", fieldList[i].GetCSType(), fieldList[i].GetName());
	}

	printer.Outdent();
	printer.PrintLine("}");
}

void CSVisiter::Accept(Enumer* enumer){

	std::stringstream ss;
	Printer printer(ss);

	printer.PrintLine("public class %1{", enumer->GetName());
	printer.Indent();

	const std::vector<std::string> & itemList = enumer->GetItemList();

	printer.PrintLine("static string[] _NAMES = new string[]{");
	printer.Indent();
	for (size_t i = 0; i < itemList.size(); ++i){
		printer.PrintLine("\"%1\"%2", itemList[i], (i + 1 != itemList.size() ? "," : ""));
	}
	printer.Outdent();
	printer.PrintLine("};");

	for (size_t i = 0; i < itemList.size(); ++i){
		printer.PrintLine("public const int %1 = %2;", itemList[i], i);
	}

	printer.PrintLine("public static int ToId(string name){");
	printer.Indent();
	printer.PrintLine("for(int i=0; i<NAMES.Length;++i){");
	printer.Indent();
	printer.PrintLine("if(_NAMES[i] == name){");
	printer.Indent();
	printer.PrintLine("return i;");
	printer.Outdent();
	printer.PrintLine("}");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("return -1;");
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("public static string ToName(int id){");
	printer.Indent();
	printer.PrintLine("if(id<0||id>=_NAMES.Length){");
	printer.Indent();
	printer.PrintLine("return \"\";");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("return _NAMES[id];");
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("public static string[] NAMES{ get {return _NAMES;} }");

	printer.Outdent();
	printer.PrintLine("}");

	CodeFile code(outpath_ + package_, enumer->GetName(), "cs", ss.str());
}

void CSVisiter::Accept(Schema* schema){
	std::stringstream ss;
	Printer printer(ss);

	PrintSchema(printer, schema);

	CodeFile code(outpath_ + package_, schema->GetName(), "cs", ss.str());
}

void CSVisiter::Accept(Service* service){
	std::stringstream ss;
	Printer printer(ss);

	//生成包定义
	const std::vector<Schema*> schemaList = service->GetSchemaList();
	for (size_t i = 0; i < schemaList.size(); ++i){
		if (schemaList[i]->GetFieldList().empty())
			continue;
		PrintSchema(printer, schemaList[i], false);
	}
	printer.PrintLine("public abstract class %1Stub {", service->GetName());
	printer.Indent();

	for (size_t i = 0; i < schemaList.size(); ++i){
		printer.Print("public void %1(", schemaList[i]->GetName());
		const std::vector<Field>&fieldList = schemaList[i]->GetFieldList();
		for (size_t j = 0; j < fieldList.size(); ++j){
			printer.Append("%1 %2%3 ", fieldList[j].GetCSType(), fieldList[j].GetName(), ((j + 1) != fieldList.size() ? "," : ""));
		}
		printer.Append("){\n", i);
		printer.Indent();
		printer.PrintLine("IWriter w = MethodBegin();");
		printer.PrintLine("if(w==null){");
		printer.Indent();
		printer.PrintLine("return;");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("w.Write((ushort)%1);", i);
		if (!fieldList.empty()){
			for (size_t j = 0; j < fieldList.size(); ++j){
				printer.PrintLine("_%1.%2 = %2;", i, fieldList[j].GetName());
			}
			printer.PrintLine("_%1.Serialize(w);", i);
		}
		printer.PrintLine("MethodEnd();");
		printer.Outdent();
		printer.PrintLine("}");

	}

	printer.PrintLine("public abstract IWriter MethodBegin();");
	printer.PrintLine("public abstract void MethodEnd();");
	printer.PrintLine("");
	for (size_t i = 0; i < schemaList.size(); ++i){
		if (!schemaList[i]->GetFieldList().empty()){
			printer.PrintLine("%1 _%2 = new %1();", schemaList[i]->GetFullName(), i);
		}
	}

	printer.Outdent();
	printer.PrintLine("};");

	printer.PrintLine("public interface I%1Proxy {", service->GetName());
	printer.Indent();
	for (size_t i = 0; i < schemaList.size(); ++i){
		printer.Print("bool %1(", schemaList[i]->GetName());
		const std::vector<Field>&fieldList = schemaList[i]->GetFieldList();
		for (size_t j = 0; j < fieldList.size(); ++j){
			printer.Append("%2 %1 %3%4 ", fieldList[j].GetCSType(), (fieldList[j].IsComplex() ? "ref" : ""), fieldList[j].GetName(), ((j + 1) != fieldList.size() ? "," : ""));
		}
		printer.Append("); // %1\n", i);
	}
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("public class %1Dispatcher {", service->GetName());
	printer.Indent();

	for (size_t i = 0; i < schemaList.size(); ++i){
		printer.PrintLine("public static bool %1(IReader r, I%2Proxy p){ // %3", schemaList[i]->GetName(), service->GetName() , i);
		const std::vector<Field>&fieldList = schemaList[i]->GetFieldList();
		printer.Indent();
		printer.PrintLine("if(r==null){");
		printer.Indent();
		printer.PrintLine("return false;");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("if(p==null){");
		printer.Indent();
		printer.PrintLine("return false;");
		printer.Outdent();
		printer.PrintLine("}");
		if (!fieldList.empty()){
			printer.PrintLine("_%1.Release();", i);
			printer.PrintLine("if(!_%1.Deserialize(r)){", i);
			printer.Indent();
			printer.PrintLine("return false;");
			printer.Outdent();
			printer.PrintLine("}");
			printer.Print("return p.%1(", schemaList[i]->GetName());
			for (size_t j = 0; j < fieldList.size(); ++j){
				printer.Append("%4 _%1.%2%3", i, fieldList[j].GetName(), ((j + 1) != fieldList.size() ? "," : ""), (fieldList[j].IsComplex() ? "ref":"" ));
			}
			printer.Append(");\n");
		}
		else{
			printer.PrintLine("return p.%1();", schemaList[i]->GetName());
		}
		printer.Outdent();
		
		printer.PrintLine("}");
	}

	printer.PrintLine("public static bool Execute(IReader r, I%1Proxy p){", service->GetName());
	if (!schemaList.empty()){
		printer.Indent();
		printer.PrintLine("if(r==null){");
		printer.Indent();
		printer.PrintLine("return false;");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("if(p==null){");
		printer.Indent();
		printer.PrintLine("return false;");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("ushort pid = 0XFFFF;");
		printer.PrintLine("if(!r.Read(ref pid)){");
		printer.Indent();
		printer.PrintLine("return false;");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("switch(pid){");
		printer.Indent();
		for (size_t i = 0; i < schemaList.size(); ++i){
			printer.PrintLine("case %1 :", i);
			printer.Indent();
			printer.PrintLine("return %2(r,p);", service->GetName(), schemaList[i]->GetName());
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

	printer.PrintLine("");
	for (size_t i = 0; i < schemaList.size(); ++i){
		if (!schemaList[i]->GetFieldList().empty()){
			printer.PrintLine("static %1 _%2 = new %1();", schemaList[i]->GetFullName(), i);
		}
	}
	printer.Outdent();
	printer.PrintLine("}");


	CodeFile code(outpath_ + package_, service->GetName(), "cs", ss.str());
}