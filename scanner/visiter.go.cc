#include "config.h"
#include "visiter.h"
#include "enumer.h"
#include "schema.h"
#include "service.h"

void GOVisiter::Begin(){
	std::stringstream ss;
	Printer printer(ss);
	
	printer.PrintLine("package %1", package_);
	printer.PrintLine("import(");
	printer.Indent();
	printer.PrintLine("\"bytes\"");
	printer.PrintLine("\"encoding/binary\"");
	printer.Outdent();
	printer.PrintLine(")");

	printer.PrintLine("const(");
	printer.Indent();
	printer.PrintLine("NoneDispatchMatchError = \"NoneDispatchMatchError\"");
	printer.PrintLine("NoneMethodError = \"NoneMethodError\"");
	printer.PrintLine("NoneBufferError = \"NoneBufferError\"");
	printer.PrintLine("NoneProxyError = \"NoneProxyError\"");

	printer.PrintLine("kFieldMaskOffset = 3");
	printer.PrintLine("kFieldMaskMagic  = 7");
	printer.PrintLine("kFieldMaskMasker = 128");

	printer.Outdent();
	printer.PrintLine(")");

	printer.PrintLine("type(");
	printer.Indent();

	printer.PrintLine("StubSender interface{");
	printer.Indent();
	printer.PrintLine("MethodBegin() *bytes.Buffer");
	printer.PrintLine("MethodEnd() error");
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("mask struct{");
	printer.Indent();
	printer.PrintLine("m []byte");
	printer.PrintLine("p uint");
	printer.Outdent();
	printer.PrintLine("}");

	printer.Outdent();
	printer.PrintLine(")");

	printer.PrintLine("func (this *mask) writeBit(b bool) {");
	printer.Indent();
	printer.PrintLine("if b{");
	printer.Indent();
	printer.PrintLine("this.m[this.p>>kFieldMaskOffset] |= (kFieldMaskMasker >> (this.p & kFieldMaskMagic))");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("this.p++");
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("func (this *mask) readBit() bool {");
	printer.Indent();
	printer.PrintLine("p := this.p");
	printer.PrintLine("this.p++");
	printer.PrintLine("return this.m[p>>kFieldMaskOffset]&(kFieldMaskMasker>>(p&kFieldMaskMagic)) != 0");
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("func (this *mask) bytes() *[]byte { return &this.m }");

	printer.PrintLine("func newMask0(buffer *bytes.Buffer, s int) (*mask, error) {");
	printer.Indent();
	printer.PrintLine("m := mask{make([]byte, s), 0}");
	printer.PrintLine("err := binary.Read(buffer, binary.LittleEndian, &(m.m))");
	printer.PrintLine("if err != nil {");
	printer.Indent();
	printer.PrintLine("return &m, err");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("return &m, nil");
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("func newMask1(s int) *mask { return &mask{make([]byte, s), 0} }");


	//writer
	printer.PrintLine("func writeSize(buffer *bytes.Buffer, s uint) error {");
	printer.Indent();
	printer.PrintLine("b := bytes.NewBuffer(nil)");
	printer.PrintLine("err := binary.Write(b, binary.LittleEndian, uint32(s))");
	printer.PrintLine("if err != nil {");
	printer.Indent();
	printer.PrintLine("return err");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("n := 0");
	printer.PrintLine("if s <= 0X3F {");
	printer.Indent();
	printer.PrintLine("n = 0");
	printer.Outdent();
	printer.PrintLine("} else if s <= 0X3FFF {");
	printer.Indent();
	printer.PrintLine("n = 1");
	printer.Outdent();
	printer.PrintLine("} else if s <= 0X3FFFFF {");
	printer.Indent();
	printer.PrintLine("n = 2");
	printer.Outdent();
	printer.PrintLine("} else if s <= 0X3FFFFFFF {");
	printer.Indent();
	printer.PrintLine("n = 3");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("b2 := b.Bytes()");
	printer.PrintLine("b2[n] = byte(int(b2[n]) | (n << 6))");
	printer.PrintLine("for i := n; i >= 0; i-- {");
	printer.Indent();
	printer.PrintLine("err := binary.Write(buffer, binary.LittleEndian, b2[i])");
	printer.PrintLine("if err != nil {");
	printer.Indent();
	printer.PrintLine("return err");
	printer.Outdent();
	printer.PrintLine("}");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("return nil");
	printer.Outdent();
	printer.PrintLine("}");
	
	printer.PrintLine("func writeString(buffer *bytes.Buffer, v string) error {");
	printer.Indent();
	printer.PrintLine("b := bytes.NewBufferString(v)");
	printer.PrintLine("err := writeSize(buffer, uint(b.Len()))");
	printer.PrintLine("if err != nil {");
	printer.Indent();
	printer.PrintLine("return err");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("return binary.Write(buffer, binary.LittleEndian, b.Bytes())");
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("func write(buffer *bytes.Buffer, i interface{}) error {");
	printer.Indent();
	printer.PrintLine("switch i.(type) {");
	printer.Indent();
	printer.PrintLine("case int:");
	printer.Indent();
	printer.PrintLine("return binary.Write(buffer, binary.LittleEndian, int8(i.(int)))");
	printer.Outdent();
	printer.PrintLine("case uint:");
	printer.Indent();
	printer.PrintLine("return writeSize(buffer, i.(uint))");
	printer.Outdent();
	printer.PrintLine("case string:");
	printer.Indent();
	printer.PrintLine("return writeString(buffer, i.(string))");
	printer.Outdent();
	printer.PrintLine("default:");
	printer.Indent();
	printer.PrintLine("return binary.Write(buffer, binary.LittleEndian, i)");
	printer.Outdent();
	printer.Outdent();
	printer.PrintLine("}");
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("func readSize(buffer *bytes.Buffer, s *uint) error {");
	printer.Indent();
	printer.PrintLine("*s = uint(0)");
	printer.PrintLine("b := uint8(0)");
	printer.PrintLine("err := binary.Read(buffer, binary.LittleEndian, &b)");
	printer.PrintLine("if err != nil {");
	printer.Indent();
	printer.PrintLine("return err");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("n := uint(b) & 0XC0 >> 6");
	printer.PrintLine("*s = uint(b) & 0X3F");
	printer.PrintLine("for i := uint(0); i < n; i++ {");
	printer.Indent();
	printer.PrintLine("err := binary.Read(buffer, binary.LittleEndian, &b)");
	printer.PrintLine("if err != nil {");
	printer.Indent();
	printer.PrintLine("return err");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("*s = (*s << 8) | uint(b)");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("return nil");
	printer.Outdent();
	printer.PrintLine("}");

	printer.PrintLine("func readString(buffer *bytes.Buffer, v *string) error {");
	printer.Indent();
	printer.PrintLine("s := uint(0)");
	printer.PrintLine("err := readSize(buffer, &s)");
	printer.PrintLine("if err != nil {");
	printer.Indent();
	printer.PrintLine("return err");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("if s == 0 {");
	printer.Indent();
	printer.PrintLine("return nil");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("b := make([]byte, s)");
	printer.PrintLine("err = binary.Read(buffer, binary.LittleEndian, &b)");
	printer.PrintLine("if err != nil {");
	printer.Indent();
	printer.PrintLine("return err");
	printer.Outdent();
	printer.PrintLine("}");
	printer.PrintLine("*v = string(b)");
	printer.PrintLine("return nil");
	printer.Outdent();
	printer.PrintLine("}");



	printer.PrintLine("func read(buffer *bytes.Buffer, i interface{}) error {");
	printer.Indent();
	printer.PrintLine("switch i.(type) {");
	printer.Indent();
	printer.PrintLine("case *int:");
	printer.Indent();
	printer.PrintLine("{");
	printer.Indent();
	printer.PrintLine("var value int8");
	printer.PrintLine("err := binary.Read(buffer, binary.LittleEndian, &value)");
	printer.PrintLine("*(i.(*int)) = int(value)");
	printer.PrintLine("return err");
	printer.Outdent();
	printer.PrintLine("}");
	printer.Outdent();
	printer.PrintLine("case *uint:");
	printer.Indent();
	printer.PrintLine("return readSize(buffer, i.(*uint))");
	printer.Outdent();
	printer.PrintLine("case *string:");
	printer.Indent();
	printer.PrintLine("return readString(buffer, i.(*string))");
	printer.Outdent();
	printer.PrintLine("default:");
	printer.Indent();
	printer.PrintLine("return binary.Read(buffer, binary.LittleEndian, i)");
	printer.Outdent();
	printer.Outdent();
	printer.PrintLine("}");
	printer.Outdent();
	printer.PrintLine("}");

	CodeFile code(outpath_ + package_, "conf", "go", ss.str());
	
}

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

void GOVisiter::PrintSchemaDef(Printer &printer, Schema* schema, bool needSync){
	const std::vector<Field> & fieldList = schema->GetFieldList();

	printer.PrintLine("type %1 struct{", schema->GetFullName());
	printer.Indent();
	if (schema->GetSuper())
		printer.PrintLine(schema->GetSuper()->GetFullName());
	else if (needSync) {
		printer.PrintLine("sync.Mutex");
	}
	for (size_t i = 0; i < fieldList.size(); ++i){
		printer.PrintLine("%1 %2  //%3", fieldList[i].GetName(), fieldList[i].GetGOType(),i);
	}
	printer.Outdent();
	printer.PrintLine("}");
}

void GOVisiter::PrintSchemaFun(Printer &printer, Schema* schema, bool needSync){
	const std::vector<Field> & fieldList = schema->GetFieldList();
	if (needSync){
		for (size_t i = 0; i < fieldList.size(); ++i){
			printer.PrintLine("func (this *%1)Set%2(value %3) {", schema->GetFullName(), fieldList[i].GetName(), fieldList[i].GetGOType());
			printer.Indent();

			printer.PrintLine("this.Lock()");
			printer.PrintLine("defer this.Unlock()");

			printer.PrintLine("this.%1 = value", fieldList[i].GetName());
			printer.Outdent();
			printer.PrintLine("}");

			printer.PrintLine("func (this *%1)Get%2() %3 {", schema->GetFullName(), fieldList[i].GetName(), fieldList[i].GetGOType());
			printer.Indent();

			printer.PrintLine("this.Lock()");
			printer.PrintLine("defer this.Unlock()");

			printer.PrintLine("return this.%1", fieldList[i].GetName());
			printer.Outdent();
			printer.PrintLine("}");
		}
	}
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

	if (needSync){
		printer.PrintLine("this.Lock()");
		printer.PrintLine("defer this.Unlock()");
	}

	printer.PrintLine("//field mask");
	printer.PrintLine("mask := newMask1(%1)", schema->GetFieldMaskLength());
	for (size_t i = 0; i < fieldList.size(); ++i){
		if (fieldList[i].IsArray() || fieldList[i].IsString())
			printer.PrintLine("mask.writeBit(len(this.%1) != 0)", fieldList[i].GetName());
		else{
			if (fieldList[i].IsSchema())
				printer.PrintLine("mask.writeBit(true) //%1", fieldList[i].GetName());
			else if (fieldList[i].IsBoolean())
				printer.PrintLine("mask.writeBit(this.%1)", fieldList[i].GetName());
			else
				printer.PrintLine("mask.writeBit(this.%1!=%2)", fieldList[i].GetName(), fieldList[i].GetGODefault());
		}
	}

	printer.PrintLine("{");
	printer.Indent();

	printer.PrintLine("err := write(buffer,mask.bytes())");
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

			printer.PrintLine("err := write(buffer,uint(len(this.%1)))", fieldList[i].GetName());
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
				printer.PrintLine("err := write(buffer,value)");
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
			printer.PrintLine("err := write(buffer,this.%1)", fieldList[i].GetName());
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
				printer.PrintLine("err := write(buffer,this.%1)", fieldList[i].GetName());
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

	if (needSync){
		printer.PrintLine("this.Lock()");
		printer.PrintLine("defer this.Unlock()");
	}

	printer.PrintLine("//field mask");
	printer.PrintLine("mask, err:= newMask0(buffer,%1);", schema->GetFieldMaskLength());
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
			printer.PrintLine("if mask.readBit() {");
			printer.Indent();
			printer.PrintLine("var size uint");
			printer.PrintLine("err := read(buffer,&size)");
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
				printer.PrintLine("err := read(buffer,&this.%1[i])", fieldList[i].GetName());
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
				printer.PrintLine("if mask.readBit() {");
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
				printer.PrintLine("this.%1 = mask.readBit();", fieldList[i].GetName());
			}
			else
			{
				printer.PrintLine("if mask.readBit() {");
				printer.Indent();
				printer.PrintLine("err := read(buffer,&this.%1)", fieldList[i].GetName());
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


	//string
	if (needSync){
		printer.PrintLine("func (this *%1)String() string{", schema->GetFullName());
		printer.Indent();
		printer.PrintLine("b, e := json.Marshal(this)");
		printer.PrintLine("if e != nil{");
		printer.Indent();
		printer.PrintLine("return e.Error()");
		printer.Outdent();
		printer.PrintLine("}else{");
		printer.Indent();
		printer.PrintLine("return string(b)");
		printer.Outdent();
		printer.PrintLine("}");
		printer.Outdent();
		printer.PrintLine("}");
	}
}

void GOVisiter::Accept(Schema* schema){
	std::stringstream ss;
	Printer printer(ss);
	printer.PrintLine("package %1", package_);
	
	printer.PrintLine("import(");
	printer.Indent();
	printer.PrintLine("\"bytes\"");
	if (schema->GetSuper() == NULL)
		printer.PrintLine("\"sync\"");
	printer.PrintLine("\"encoding/json\"");
	
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
	printer.Outdent();
	printer.PrintLine(")");
	//声明包
	for (size_t i = 0; i < schemaList.size(); ++i){
		if (schemaList[i]->GetFieldList().empty())
			continue;
		PrintSchemaDef(printer, schemaList[i], false);
	}

	printer.PrintLine("type %1Stub struct{", service->GetName());
	printer.Indent();
	printer.PrintLine("Sender StubSender");
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
		PrintSchemaFun(printer, schemaList[i], false);
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
		printer.PrintLine("return errors.New(NoneBufferError)");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("err := write(buffer,uint16(%1))", i);
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
		printer.PrintLine("return errors.New(NoneBufferError)");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("if p == nil {");
		printer.Indent();
		printer.PrintLine("return errors.New(NoneProxyError)");
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
		printer.PrintLine("return errors.New(NoneBufferError)");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("if p == nil {");
		printer.Indent();
		printer.PrintLine("return errors.New(NoneProxyError)");
		printer.Outdent();
		printer.PrintLine("}");
		printer.PrintLine("pid := uint16(0XFFFF)");
		printer.PrintLine("err := read(buffer,&pid)");
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
		printer.PrintLine("return errors.New(NoneDispatchMatchError)");
		printer.Outdent();
		printer.Outdent();
		printer.PrintLine("}");
	}
	else{
		printer.PrintLine("return errors.New(NoneMethodError)");
	}
	printer.Outdent();
	printer.PrintLine("}");

	CodeFile code(outpath_ + package_, service->GetName(), "go", ss.str());
}