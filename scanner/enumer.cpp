#include "config.h"
#include "enumer.h"
#include "visiter.h"

bool Enumer::AddItem(const std::string& item){
	for (size_t i = 0; i < itemList_.size(); ++i){
		if (itemList_[i] == item){
			return false;
		}
	}

	itemList_.push_back(item);
	return true;
}

void Enumer::Enter(CodeVisiter* visiter){
	visiter->Accept(this);
}