#ifndef __Service_h__
#define __Service_h__

#include "Node.h"
#include "Schema.h"
#include <iostream>
class Service : public Node
{
public:

	Service()
		:Node(NT_Service)
	{}

	~Service();

	Service* AsService()	{ return this; }

	Node* Clone(){
		return new Service(*this);
	}

	/** 查找一个方法是否存在. */
	Schema* GetSchema(const std::string& name);

	const std::vector<Schema*>& GetSchemaList(){
		return schemaList_;
	}

	void AddSchema(Schema *schema){
		schema->SetPrefixName(name_ + "_");
		schemaList_.push_back(schema);
	}

	void Reset(){
		name_.clear();
		schemaList_.clear();
	}

	void Enter(class CodeVisiter* visiter);
	
private:
	std::vector<Schema*>	schemaList_;
};


#endif//__Service_h__