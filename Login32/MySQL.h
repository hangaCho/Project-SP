#ifndef __MYSQL_H__
#define __MYSQL_H__

#include "ServerEnum.h"
#include <mysql.h>

class MySQL {
private:
	MYSQL *connection = nullptr;
	MYSQL_RES *res = nullptr;

public:
	MySQL();
	~MySQL();
	int Login(char*, char*);
	void GetUserInfo(int, MyCharInfo&);
	void SetDefaultCharacter(int, Character);


	template <typename ...Args>
	void Query(Args &&... args) {
		auto str = fmt::format(std::forward<Args>(args)...);
		if(mysql_query(connection, str.c_str()))
			throw std::runtime_error(mysql_error(connection));
	}

	template <typename ...Args>
	void QuerySelect(Args &&... args) {
		auto str = fmt::format(std::forward<Args>(args)...);
		if(mysql_query(connection, str.c_str()))
			throw std::runtime_error(mysql_error(connection));

		res = mysql_use_result(connection);

		if(!res)
			throw std::runtime_error(mysql_error(connection));
	}

	#define mysql_query		 static_assert(0, "Use the method Query to execute queries!");
	#define mysql_use_result(x) nullptr; static_assert(0, "Use the method QuerySelect to execute select queries!");
};


#endif
