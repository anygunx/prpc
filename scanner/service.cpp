#include "config.h"
#include "service.h"
#include "visiter.h"

Service::~Service(){
	for (size_t i = 0; i < schemaList_.size(); ++i){
		delete schemaList_[i];
	}	
}

Schema* Service::GetSchema(const std::string& name){
	for (size_t i = 0; i < schemaList_.size(); ++i){
		if (schemaList_[i]->GetName() == name){
			return schemaList_[i];
		}
	}
	return NULL;
}

void Service::Enter(CodeVisiter* visiter){
	visiter->Accept(this);
}