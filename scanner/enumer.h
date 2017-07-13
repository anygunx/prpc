#ifndef __ENUMER_H__
#define __ENUMER_H__

#include "config.h"
#include "node.h"

/** ö�ٶ�����.
	Enum ��ʾһ����c++ enum���Ƶ�ö�ٶ��塣
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
	std::vector< std::string >	itemList_;	///< ö�����б�.
};


#endif//__Enum_h__