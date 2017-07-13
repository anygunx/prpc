
#ifndef __SCHEMA_H__
#define __SCHEMA_H__

#include "config.h"
#include "node.h"

class Field{
public:
	Field(int maxLength, Node* type, const std::string & name)
		: maxLength_(maxLength)
		, type_(type)
		, name_(name){

	}
public:
	bool IsArray()const{
		return maxLength_ != 0;
	}
	bool IsString()const{
		return !!type_->AsString();
	}
	bool IsSchema()const{
		return !!type_->AsSchema();
	}
	bool IsEnumer()const{
		return !!type_->AsEnumer();
	}
	bool IsBoolean()const{
		return !!type_->AsBoolean();
	}
	bool IsComplex()const{
		return type_->GetType() > NT_Enumer || IsArray();
	}
	NodeType GetNodeType()const{
		return type_->GetType();
	}
	Node* GetType()const{
		return type_;
	}
	std::string GetName(){
		return name_;
	}
	std::string GetName() const {
		return name_;
	}
	int GetMaxLength()const{
		return maxLength_;
	}
	std::string GetCPPType() const ;
	std::string GetCPPDefault()const ;
	std::string GetCSType(bool needArrayFlag = true) const;
	std::string GetCSDefault()const;
	std::string GetGOType() const;
	std::string GetGODefault()const;
private:
	int   maxLength_;
	Node* type_;
	std::string name_;
};

class Schema : public Node
{
public:
	
	Schema()
		:Node(NT_Schema)
		,super_(NULL)
	{}

	void Reset(){
		super_ = NULL;
		name_.clear();
		fieldList_.clear();
	}

	
	void SetSuper(Node* node){
		super_ = node->AsSchema();
	}

	Schema* GetSuper(){
		return super_;
	}

	size_t GetFieldCount(){
		if (super_){
			return super_->GetFieldCount() + fieldList_.size();
		}
		return fieldList_.size();
	}

	size_t GetFieldMaskLength(){
		return (fieldList_.size() - 1) / 8 + 1;
	}
	
	bool AddField(Node* type, const std::string& name, int maxLength = 0);

	Field* GetField(const std::string& name);
	
	Schema*	AsSchema(){
		return this;
	}

	void SetPrefixName(const std::string& prefixName){
		prefixName_ = prefixName;
	}

	std::string GetFullName(){
		return prefixName_ + name_;
	}
	
	Node* Clone(){
		return new Schema(*this);
	}

	const std::vector<Field>& GetFieldList() const{
		return fieldList_;
	}

	void Enter(class CodeVisiter* visiter);
private:
	Schema*						super_;		///< ผฬณะ
	std::string					prefixName_;
	std::vector<Field>			fieldList_; ///
};


#endif//__Struct_h__