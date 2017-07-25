#include "config.h"
#include "visiter.h"
#include "enumer.h"
#include "schema.h"
#include "service.h"

void GOVisiter::Accept(Enumer* enumer){
	std::stringstream ss;
	Printer printer(ss);
	const std::vector<std::string> & itemList = enumer->GetItemList();
	printer.PrintLine("package %1",package_);
	printer.PrintLine("// enum %1", enumer->GetName());
	printer.PrintLine("const( ");
	printer.Indent();
	for (size_t i = 0; i < itemList.size(); i++)
		printer.PrintLine("%1 = %2", itemList[i], i);
	printer.Outdent();
	printer.PrintLine(")");
	printer.PrintLine("const( ");
	printer.Indent();
	for (size_t i = 0; i < itemList.size(); i++)
		printer.PrintLine("K_%1 = \"%1\"", itemList[i]);
	printer.Outdent();
	printer.PrintLine(")");


	printer.PrintLine("func ToName_%1( id int )string{", enumer->GetName());
	printer.Indent();
	printer.PrintLine("switch(id){");
	printer.Indent();
	for (size_t i = 0; i < itemList.size(); i++){
		printer.PrintLine("case %1:", itemList[i]);
		printer.Indent();
		printer.PrintLine("return \"%1\"", itemList[i]);
		printer.Outdent();
	}
	printer.PrintLine("default:");
	printer.Indent();
	printer.PrintLine("return \"\"");
	printer.Outdent();
	printer.Outdent();
	printer.PrintLine("}");
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("func ToId_%1( name string ) int {", enumer->GetName());
	printer.Indent();
	printer.PrintLine("switch(name){");
	printer.Indent();
	for (size_t i = 0; i <itemList.size(); i++){
		printer.PrintLine("case \"%1\":", itemList[i]);
		printer.Indent();
		printer.PrintLine("return %1", itemList[i]);
		printer.Outdent();
	}
	printer.PrintLine("default:");
	printer.Indent();
	printer.PrintLine("return -1");
	printer.Outdent();
	printer.Outdent();
	printer.PrintLine("}");
	printer.Outdent();
	printer.PrintLine("}");

	CodeFile code(outpath_ + package_, enumer->GetName(), "go", ss.str());
}

void GOVisiter::PrintSchemaDef(Printer &printer, Schema* schema){
	const std::vector<Field> & fieldList = schema->GetFieldList();

	printer.PrintLine("type %1 struct{", schema->GetFullName());
	printer.Indent();
	if (schema->GetSuper())
		printer.PrintLine(schema->GetSuper()->GetFullName());
	for (size_t i = 0; i < fieldList.size(); ++i){
		printer.PrintLine("%1 %2  //%3", fieldList[i].GetName(), fieldList[i].GetGOType(),i);
	}
	printer.Outdent();
	printer.PrintLine("}");
}

void GOVisiter::PrintSchemaFun(Printer &printer, Schema* schema){
	const std::vector<Field> & fieldList = schema->GetFieldList();

	printer.PrintLine("func (this *%1)Serialize(buffer *bytes.Buffer) error {", schema->GetFullName());
	printer.Indent();
	if (schema->GetSuper()){
		printer.PrintLine("{");
		printer.Indent();
		
		printer.PrintLine("err := this.%1.Serialize(buffer);", schema->GetSuper()->GetFullName());
		printer.PrintLine("if err != nil {");
		printer.Indent();
		printer.PrintLine("return err");
		printer.Outdent();
		printer.PrintLine("}");

		printer.Outdent();
		printer.PrintLine("}");
	}

	printer.PrintLine("//field mask");
	printer.PrintLine("mask := prpc.NewMask1(%1)", schema->GetFieldMaskLength());
	for (size_t i = 0; i < fieldList.size(); ++i){
		if (fieldList[i].IsArray() || fieldList[i].IsString())
			printer.PrintLine("mask.WriteBit(len(this.%1) != 0)", fieldList[i].GetName());
		else{
			if (fieldList[i].IsSchema())
				printer.PrintLine("mask.WriteBit(true) //%1", fieldList[i].GetName());
			else if (fieldList[i].IsBoolean())
				printer.PrintLine("mask.WriteBit(this.%1)", fieldList[i].GetName());
			else
				printer.PrintLine("mask.WriteBit(this.%1!=%2)", fieldList[i].GetName(), fieldList[i].GetGODefault());
		}
	}
	
	printer.PrintLine("{");
	printer.Indent();
	
	printer.PrintLine("err := prpc.Write(buffer,mask.Bytes())");
	printer.PrintLine("if err != nil {");
	printer.Indent();
	printer.PrintLine("return err");
	printer.Outdent();
	printer.PrintLine("}");
	printer.Outdent();
	printer.PrintLine("}");

	for (size_t i = 0; i < fieldList.size(); ++i){
		printer.PrintLine("// serialize %1", fieldList[i].GetName());

		if (fieldList[i].IsArray())
		{
			// 数组属性，先序列化数组大小
			printer.PrintLine("if len(this.%1) != 0{", fieldList[i].GetName());
			printer.Indent();

			printer.PrintLine("{");
			printer.Indent();

			printer.PrintLine("err := prpc.Write(buffer,uint(len(this.%1)))", fieldList[i].GetName());
			printer.PrintLine("if err != nil {");
			printer.Indent();
			printer.PrintLine("return err");
			printer.Outdent();
			printer.PrintLine("}");

			printer.Outdent();
			printer.PrintLine("}");

			printer.PrintLine("for _, value := range this.%1 {", fieldList[i].GetName());
			printer.Indent();
			if (fieldList[i].IsSchema()){
				printer.PrintLine("err := value.Serialize(buffer)");
				printer.PrintLine("if err != nil {");
				printer.Indent();
				printer.PrintLine("return err");
				printer.Outdent();
				printer.PrintLine("}");

			}
			else{
				printer.PrintLine("err := prpc.Write(buffer,value)");
				printer.PrintLine("if err != nil {");
				printer.Indent();
				printer.PrintLine("return err");
				printer.Outdent();
				printer.PrintLine("}");
			}
			printer.Outdent();
			printer.PrintLine("}");
			printer.Outdent();
			printer.PrintLine("}");
		}
		else if (fieldList[i].IsString())
		{
			// 数组属性，先序列化数组大小
			printer.PrintLine("if len(this.%1) != 0{", fieldList[i].GetName());
			printer.Indent();
			printer.PrintLine("err := prpc.Write(buffer,this.%1)", fieldList[i].GetName());
			printer.PrintLine("if err != nil {");
			printer.Indent();
			printer.PrintLine("return err");
			printer.Outdent();
			printer.PrintLine("}");
			printer.Outdent();
			printer.PrintLine("}");
		}
		else
		{
			printer.PrintLine("{");
			printer.Indent();
			if (fieldList[i].IsSchema())
			{
				printer.PrintLine("err := this.%1.Serialize(buffer)", fieldList[i].GetName());
				printer.PrintLine("if err != nil{");
				printer.Indent();
				printer.PrintLine("return err");
				printer.Outdent();
				printer.PrintLine("}");
			}
			else if (fieldList[i].IsBoolean()){

			}
			else{
				printer.PrintLine("if(this.%1!=%2){", fieldList[i].GetName(), fieldList[i].GetGODefault());
				printer.Indent();
				printer.PrintLine("err := prpc.Write(buffer,this.%1)", fieldList[i].GetName());
				printer.PrintLine("if err != nil{");
				printer.Indent();
				printer.PrintLine("return err");
				printer.Outdent();
				printer.PrintLine("}");
				printer.Outdent();
				printer.PrintLine("}");
			}
			printer.Outdent();
			printer.PrintLine("}");
		}
	}

	printer.PrintLine("return nil");
	printer.Outdent();
	printer.PrintLine("}");


	printer.PrintLine("func (this *%1)Deserialize(buffer *bytes.Buffer) error{", schema->GetFullName());
	printer.Indent();
	if (schema->GetSuper()){
		printer.PrintLine("{");
		printer.Indent();
		printer.PrintLine("this.%1.Deserialize(buffer);", schema->GetSuper()->GetFullName());
		printer.Outdent();
		printer.PrintLine("}");
	}

	printer.PrintLine("//field mask");
	printer.PrintLine("mask, err:= prpc.NewMask0(buffer,%1);", schema->GetFieldMaskLength());
	printer.PrintLine("if err != nil{");
	printer.Indent();
	printer.PrintLine("return err");
	printer.Outdent();
	printer.PrintLine("}");

	for (size_t i = 0; i < fieldList.size(); ++i){
		printer.PrintLine("// deserialize %1", fieldList[i].GetName());
		if (fieldList[i].IsArray())
		{
			// 数组属性，先反序列化数组大小
			printer.PrintLine("if mask.ReadBit() {");
			printer.Indent();
			printer.PrintLine("var size uint");
			printer.PrintLine("err := prpc.Read(buffer,&size)");
			printer.PrintLine("if err != nil{");
			printer.Indent();
			printer.PrintLine("return err");
			printer.Outdent();
			printer.PrintLine("}");
			printer.PrintLine("this.%1 = make(%2,size)", fieldList[i].GetName(), fieldList[i].GetGOType());
			// 遍历反序列化每个元素.
			printer.PrintLine("for i,_ := range this.%1{", fieldList[i].GetName());
			printer.Indent();
			if (fieldList[i].IsSchema())
			{
				printer.PrintLine("err := this.%1[i].Deserialize(buffer)", fieldList[i].GetName());
				printer.PrintLine("if err != nil{");
				printer.Indent();
				printer.PrintLine("return err");
				printer.Outdent();
				printer.PrintLine("}");
			}
			else
			{
				printer.PrintLine("err := prpc.Read(buffer,&this.%1[i])", fieldList[i].GetName());
				printer.PrintLine("if err != nil{");
				printer.Indent();
				printer.PrintLine("return err");
				printer.Outdent();
				printer.PrintLine("}");
			}

			printer.Outdent();
			printer.PrintLine("}");
			printer.Outdent();
			printer.PrintLine("}");
		}
		else
		{
			if (fieldList[i].IsSchema())
			{
				printer.PrintLine("if mask.ReadBit() {");
				printer.Indent();
				printer.PrintLine("err := this.%1.Deserialize(buffer)", fieldList[i].GetName());
				printer.PrintLine("if err != nil{");
				printer.Indent();
				printer.PrintLine("return err");
				printer.Outdent();
				printer.PrintLine("}");
				printer.Outdent();
				printer.PrintLine("}");
			}
			else if (fieldList[i].IsBoolean())
			{
				printer.PrintLine("this.%1 = mask.ReadBit();", fieldList[i].GetName());
			}
			else
			{
				printer.PrintLine("if mask.ReadBit() {");
				printer.Indent();
				printer.PrintLine("err := prpc.Read(buffer,&this.%1)", fieldList[i].GetName());
				printer.PrintLine("if err != nil{");
				printer.Indent();
				printer.PrintLine("return err");
				printer.Outdent();
				printer.PrintLine("}");
				printer.Outdent();
				printer.PrintLine("}");
			}
		}
	}

	printer.PrintLine("return nil");
	printer.Outdent();
	printer.PrintLine("}");

}

void GOVisiter::Accept(Schema* schema){
	std::stringstream ss;
	Printer printer(ss);
	printer.PrintLine("package %1", package_);
	
	printer.PrintLine("import(");
	printer.Indent();
	printer.PrintLine("\"bytes\"");
	printer.PrintLine("\"suzuki/prpc\"");
	printer.Outdent();
	printer.PrintLine(")");

	PrintSchemaDef(printer, schema);
	PrintSchemaFun(printer, schema);

	CodeFile code(outpath_ + package_, schema->GetName(), "go", ss.str());
}

void GOVisiter::Accept(Service* service){
	const std::vector<Schema*> schemaList = service->GetSchemaList();
	std::stringstream ss;
	Printer printer(ss);
	printer.PrintLine("package %1", package_);
	
	printer.PrintLine("import(");
	printer.Indent();
	printer.PrintLine("\"bytes\"");
	printer.PrintLine("\"errors\"");
	printer.PrintLine("\"suzuki/prpc\"");
	printer.Outdent();
	printer.PrintLine(")");
	//声明包
	for (size_t i = 0; i < schemaList.size(); ++i){
		if (schemaList[i]->GetFieldList().empty())
			continue;
		PrintSchemaDef(printer, schemaList[i]);
	}

	printer.PrintLine("type %1Stub struct{", service->GetName());
	printer.Indent();
	printer.PrintLine("Sender prpc.StubSender");
	printer.Outdent();
	printer.PrintLine("}");
	//声明接收端接口
	printer.PrintLine("type %1Proxy interface{", service->GetName());
	printer.Indent();
	for (size_t i = 0; i < schemaList.size(); ++i){
		printer.Print("%1(", schemaList[i]->GetName());
		const std::vector<Field>&fieldList = schemaList[i]->GetFieldList(); 
		for (size_t j = 0; j < fieldList.size(); ++j){
			printer.Append("%1 %2%3 ", fieldList[j].GetName(), fieldList[j].GetGOType(), ((j + 1) != fieldList.size() ? "," : ""));
		}
		printer.Append(") error // %1\n", i);
	}
	printer.Outdent();
	printer.PrintLine("}");
	
	
	//定义包
	for (size_t i = 0; i < schemaList.size(); ++i){
		if (schemaList[i]->GetFieldList().empty())
			continue;
		PrintSchemaFun(printer, schemaList[i]);
	}
	
	//定义发送端
	for (size_t i = 0; i < schemaList.size(); ++i){
		printer.Print("func(this* %1Stub)%2(", service->GetName(), schemaList[i]->GetName());
		const std::vector<Field>&fieldList = schemaList[i]->GetFieldList();
		for (size_t j = 0; j < fieldList.size(); ++j){
			printer.Append("%1 %2%3 ", fieldList[j].GetName(), fieldList[j].GetGOType(), ((j + 1) != fieldList.size() ? "," : ""));
		}
		printer.Append(") error {\n", i);
		printer.Indent();
		printer.PrintLine("buffer := this.Sender.MethodBegin()");
		printer.PrintLine("if buffer == nil{");
		printer.Indent();
		printer.PrintLine("return errors.New(prpc.NoneBufferError)");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("err := prpc.Write(buffer,uint16(%1))", i);
		printer.PrintLine("if err != nil{");
		printer.Indent();
		printer.PrintLine("return err");
		printer.Outdent();
		printer.PrintLine("}");
		if (!fieldList.empty()){
			printer.PrintLine("_%1 := %2{}", i, schemaList[i]->GetFullName());
			for (size_t j = 0; j < fieldList.size(); ++j){
				printer.PrintLine("_%1.%2 = %2;", i, fieldList[j].GetName());
			}
			printer.PrintLine("err = _%1.Serialize(buffer)", i);
			printer.PrintLine("if err != nil{");
			printer.Indent();
			printer.PrintLine("return err");
			printer.Outdent();
			printer.PrintLine("}");
		}
		printer.PrintLine("return this.Sender.MethodEnd()");
		printer.Outdent();
		printer.PrintLine("}");
	}


	for (size_t i = 0; i < schemaList.size(); ++i){
		printer.PrintLine("func Bridging_%1_%2(buffer *bytes.Buffer, p %1Proxy) error {", service->GetName(), schemaList[i]->GetName());
		const std::vector<Field>&fieldList = schemaList[i]->GetFieldList();
		printer.Indent();
		printer.PrintLine("if buffer == nil{");
		printer.Indent();
		printer.PrintLine("return errors.New(prpc.NoneBufferError)");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("if p == nil {");
		printer.Indent();
		printer.PrintLine("return errors.New(prpc.NoneProxyError)");
		printer.Outdent();
		printer.PrintLine("}");
		if (!fieldList.empty()){
			printer.PrintLine("_%1 := %2{}", i, schemaList[i]->GetFullName());
			printer.PrintLine("err := _%1.Deserialize(buffer)", i);
			printer.PrintLine("if err != nil{");
			printer.Indent();
			printer.PrintLine("return err");
			printer.Outdent();
			printer.PrintLine("}");
			printer.Print("return p.%1(", schemaList[i]->GetName());
			for (size_t j = 0; j < fieldList.size(); ++j){
				printer.Append("_%1.%2%3", i, fieldList[j].GetName(), ((j + 1) != fieldList.size() ? "," : ""));
			}
			printer.Append(")\n");
		}
		else{
			printer.PrintLine("return p.%1()", schemaList[i]->GetName());
		}
		printer.Outdent();
		printer.PrintLine("}");

	}
	
	printer.PrintLine("func %1Dispatch(buffer *bytes.Buffer, p %1Proxy) error {", service->GetName());
	printer.Indent();
	if (!schemaList.empty()){
		printer.PrintLine("if buffer == nil {");
		printer.Indent();
		printer.PrintLine("return errors.New(prpc.NoneBufferError)");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("if p == nil {");
		printer.Indent();
		printer.PrintLine("return errors.New(prpc.NoneProxyError)");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("pid := uint16(0XFFFF)");
		printer.PrintLine("err := prpc.Read(buffer,&pid)");
		printer.PrintLine("if err != nil{");
		printer.Indent();
		printer.PrintLine("return err");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("switch(pid){");
		printer.Indent();
		for (size_t i = 0; i < schemaList.size(); ++i){
			printer.PrintLine("case %1 :", i);
			printer.Indent();
			printer.PrintLine("return Bridging_%1_%2(buffer,p);", service->GetName(), schemaList[i]->GetName());
			printer.Outdent();
		}
		printer.PrintLine("default:");
		printer.Indent();
		printer.PrintLine("return errors.New(prpc.NoneDispatchMatchError)");
		printer.Outdent();
		printer.Outdent();
		printer.PrintLine("}");
	}
	else{
		printer.PrintLine("return errors.New(prpc.NoneMethodError)");
	}
	printer.Outdent();
	printer.PrintLine("}");

	CodeFile code(outpath_ + package_, service->GetName(), "go", ss.str());
}