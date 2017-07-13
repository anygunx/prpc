#ifndef __ENUMER_H__
#define __ENUMER_H__

#include "config.h"
#include "node.h"

/** 枚举定义类.
	Enum 表示一个和c++ enum类似的枚举定义。
*/
class Enumer : public Node
{
public:
	Enumer()
		:Node(NT_Enumer)
	{}

	void Reset(){
		name_.clear();
		itemList_.clear();
	}
	
	bool AddItem(const std::string& item );
	
	Enumer* AsEnumer() { return this; }

	Node* Clone(){
		return new Enumer(*this);
	}

	void Enter(class CodeVisiter* visiter);

	const std::vector< std::string > & GetItemList(){
		return itemList_;
	}

private:
	std::vector< std::string >	itemList_;	///< 枚举项列表.
};


#endif//__Enum_h__