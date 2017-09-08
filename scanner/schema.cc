#include "config.h"
#include "schema.h"
#include "visiter.h"

std::string Field::GetCCType()const {
	std::string typeName;
	if (maxLength_)
		typeName = "std::vector<";
	switch (type_->GetType())
	{
	case NT_Int64:
		typeName += "int64_t";
		break;
	case NT_UInt64:
		typeName += "uint64_t";
		break;
	case NT_Float64:
		typeName += "double";
		break;
	case NT_Float32:
		typeName += "float";
		break;
	case NT_Int32:
		typeName += "int32_t";
		break;
	case NT_UInt32:
		typeName += "uint32_t";
		break;
	case NT_Int16:
		typeName += "int16_t";
		break;
	case NT_UInt16:
		typeName += "uint16_t";
		break;
	case NT_Int8:
		typeName += "int8_t";
		break;
	case NT_UInt8:
		typeName += "uint8_t";
		break;
	case NT_Boolean:
		typeName += "bool";
		break;
	case NT_String:
		typeName += "std::string";
		break;
	case NT_Schema:
		typeName += type_->GetName();
		break;
	case NT_Enumer:
		typeName += type_->GetName();
		typeName += "::Type";
		break;
	default:
		throw "Invalid field type.";
	}

	if (maxLength_)
		typeName += ">";
	return typeName;
}

std::string Field::GetCSType(bool needArrayFlag)const {
	std::string typeName;
	
	switch (type_->GetType())
	{
	case NT_Int64:
		typeName += "long";
		break;
	case NT_UInt64:
		typeName += "ulong";
		break;
	case NT_Float64:
		typeName += "double";
		break;
	case NT_Float32:
		typeName += "float";
		break;
	case NT_Int32:
		typeName += "int";
		break;
	case NT_UInt32:
		typeName += "uint";
		break;
	case NT_Int16:
		typeName += "short";
		break;
	case NT_UInt16:
		typeName += "ushort";
		break;
	case NT_Int8:
		typeName += "sbyte";
		break;
	case NT_UInt8:
		typeName += "byte";
		break;
	case NT_Boolean:
		typeName += "bool";
		break;
	case NT_String:
		typeName += "string";
		break;
	
	case NT_Enumer:
		typeName += "int";
		break;
	case NT_Schema:
		typeName += type_->GetName();
		break;
	default:
		throw "Invalid field type.";
	}

	if (maxLength_ && needArrayFlag)
		typeName += "[]";
	return typeName;
}

std::string Field::GetGOType()const {
	std::string typeName;
	if (maxLength_)
		typeName = "[]";
	switch (type_->GetType())
	{
	case NT_Int64:
		typeName += "int64";
		break;
	case NT_UInt64:
		typeName += "uint64";
		break;
	case NT_Float64:
		typeName += "float64";
		break;
	case NT_Float32:
		typeName += "float32";
		break;
	case NT_Int32:
		typeName += "int32";
		break;
	case NT_UInt32:
		typeName += "uint32";
		break;
	case NT_Int16:
		typeName += "int16";
		break;
	case NT_UInt16:
		typeName += "uint16";
		break;
	case NT_Int8:
		typeName += "int8";
		break;
	case NT_UInt8:
		typeName += "uint8";
		break;
	case NT_Boolean:
		typeName += "bool";
		break;
	case NT_Enumer:
		typeName += "int";
		break;
	case NT_String:
		typeName += "string";
		break;
	case NT_Schema:
	
		typeName += type_->GetName();
		break;
	default:
		throw "Invalid field type.";
	}

	return typeName;
}

std::string Field::GetCCDefault()const{

	if (IsComplex())
		return "";
	std::string ret;

	switch (type_->GetType())
	{
	case NT_Int64:
	case NT_UInt64:
	case NT_Float64:
	case NT_Float32:
	case NT_Int32:
	case NT_UInt32:
	case NT_Int16:
	case NT_UInt16:
	case NT_Int8:
	case NT_UInt8:
		ret = "0";
		break;
	case NT_Boolean:
		ret = "false";
		break;
	case NT_Enumer:
		ret = "(" + type_->GetName() + "::Type)(-1)";
		break;
	default:
		throw "Invalid field type.";
	}

	return ret;
}

std::string Field::GetCSDefault()const{

	if (IsComplex())
		return "null";
	std::string ret;

	switch (type_->GetType())
	{
	case NT_Int64:
	case NT_UInt64:
	case NT_Float64:
	case NT_Float32:
	case NT_Int32:
	case NT_UInt32:
	case NT_Int16:
	case NT_UInt16:
	case NT_Int8:
	case NT_UInt8:
	case NT_Enumer:
		ret = "0";
		break;
	case NT_Boolean:
		ret = "false";
		break;
	default:
		throw "Invalid field type.";
	}

	return ret;
}

std::string Field::GetGODefault()const{
	if (IsComplex())
		return "";
	std::string ret;

	switch (type_->GetType())
	{
	case NT_Int64:
	case NT_UInt64:
	case NT_Float64:
	case NT_Float32:
	case NT_Int32:
	case NT_UInt32:
	case NT_Int16:
	case NT_UInt16:
	case NT_Int8:
	case NT_UInt8:
	case NT_Enumer:
		ret = "0";
		break;
	case NT_Boolean:
		ret = "false";
		break;
	default:
		throw "Invalid field type.";
	}

	return ret;
}

bool Schema::AddField(Node* type, const std::string& name, int maxLength){
	if (NULL != GetField(name)){
		return false;
	}
	Field f(maxLength, type, name);
	fieldList_.push_back(f);
	return true;
}

Field* Schema::GetField( const std::string& name )
{	
	for (size_t i = 0; i < fieldList_.size(); ++i){
		if (fieldList_[i].GetName() == name){
			return &fieldList_[i];
		}
	}

	if (super_){
		return super_->GetField(name);
	}
	
	return NULL;
}


void Schema::Enter(CodeVisiter* visiter){
	visiter->Accept(this);
}