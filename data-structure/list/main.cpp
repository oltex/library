#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "../my_class.h"
#include "list.h"

#include <iostream>
#include <list>
#include <intrin.h>

#include <any>
#include <tuple>

int main(void) noexcept {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	library::data_structure::list<int> list;
	library::data_structure::list<int>::iterator iter;
	list.emplace_back(30);
	list.emplace_front(20);
	list.emplace_back(40);
	list.emplace_front(10);
	list.clear();

	//list.pop_front();
	//list.pop_back();

	iter = list.begin();
	for (size_t i = 0; i < list.size(); ++i) {
		std::cout << (*iter) << std::endl;
		++iter;
	}

	//iter = list.begin();
	//++iter;
	//++iter;
	//--iter;
	//list.insert(iter++, 100);
	////list.erase(iter);

	//library::data_structure::list<int> list2;
	//list2.swap(list);

	//for (iter = list.begin(); iter != list.end(); ++iter)
	//	std::cout << *iter << std::endl;
	//for (iter = list2.begin(); iter != list2.end(); ++iter)
	//	std::cout << *iter << std::endl;

	////for (iter = list.begin(); iter != list.end();)
	////	iter = list.erase(iter);

	//library::data_structure::list<int> list3(list);
	//library::data_structure::list<int> list4(std::move(list));
	return 0;
}