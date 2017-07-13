#ifndef __Definition_h__
#define __Definition_h__

#include <stdio.h>
#include <string>

enum NodeType
{
	NT_Min,
	NT_Boolean ,
	NT_Int8,
	NT_Int16,
	NT_Int32,
	NT_Int64,
	NT_UInt8,
	NT_UInt16,
	NT_UInt32,
	NT_UInt64,
	NT_Float32,
	NT_Float64,
	NT_Enumer,
	NT_String,
	NT_Schema,
	NT_Service,
	NT_Max,
};

class Node
{
public:
	Node()
	{}
	Node(NodeType nodeType)
		: type_(nodeType)
	{}

	Node(NodeType type, const std::string& name)
		: type_(type)
		, name_(name)
	{}
	virtual ~Node() {};

	void SetName(const std::string& name){ name_ = name; }
	const std::string& GetName(){ return name_; }
	NodeType GetType(){ return type_; }
	virtual void Enter(class CodeVisiter* visiter){}
	virtual Node* Clone(){return NULL;}

#define __AS_FUNC(NAME) virtual class NAME* As##NAME(){ return NULL;}
	__AS_FUNC(Enumer)
	__AS_FUNC(Schema)
	__AS_FUNC(Service)
	__AS_FUNC(Boolean)
	__AS_FUNC(Int8)
	__AS_FUNC(Int16)
	__AS_FUNC(Int32)
	__AS_FUNC(Int64)
	__AS_FUNC(UInt8)
	__AS_FUNC(UInt16)
	__AS_FUNC(UInt32)
	__AS_FUNC(UInt64)
	__AS_FUNC(Float32)
	__AS_FUNC(Float64)
	__AS_FUNC(String)
#undef  __AS_FUNC
	

protected:
	NodeType				type_;
	std::string				name_;	///< ¶¨ÒåÃû³Æ.
};

#define __EMBEND_TYPE(NAME) \
class NAME : public Node{\
public : \
NAME():Node(NT_##NAME,#NAME){} \
NAME* As##NAME(){return this;}\
};
__EMBEND_TYPE(Boolean)
__EMBEND_TYPE(Int8)
__EMBEND_TYPE(Int16)
__EMBEND_TYPE(Int32)
__EMBEND_TYPE(Int64)
__EMBEND_TYPE(UInt8)
__EMBEND_TYPE(UInt16)
__EMBEND_TYPE(UInt32)
__EMBEND_TYPE(UInt64)
__EMBEND_TYPE(Float32)
__EMBEND_TYPE(Float64)
__EMBEND_TYPE(String)

#undef __EMBEND_TYPE 
#endif//__Definition_h__