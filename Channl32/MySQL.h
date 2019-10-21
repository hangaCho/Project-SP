#ifndef __MYSQL_H__
#define __MYSQL_H__

#include "ServerEnum.h"
#include "ClientPackets.h"
#include "ServerPackets.h"
#include <mysql.h>

class MySQL
{
private:
	MYSQL *connection = nullptr;
	MYSQL_RES *res = nullptr;

public:
	MySQL();
	~MySQL();
	void SetUserIP(char*, char*);
	void GetUserIP(int, char*);
	void GetUserInfo(char*, MyCharInfo&);
	void GetUserItems(int id, bool*, int*, int*, int*, int*, int*);
	int GetValidSlot(int, int);
	void InsertNewItem(MyCharInfo*, int, int, int, int);
	void GetShopItemCost(int, int, int, int&, int&);
	int DeleteItem(int, int);
	void InsertMsg(char *, char *, char *);
	void ChangeEquips(int, EquipChangeRequest*);
	void GetUserData(UserInfoResponse*);
	void GetEquipData(int, RoomPlayerDataResponse*);
	void GetMoneyAmmount(int, int*, unsigned __int64*, char = 0, int = 0, unsigned __int64 = 0);
	void UpgradeCard(MyCharInfo *, CardUpgradeResponse*);
	void GetScrolls(MyCharInfo *);
	void UpdateScrolls(int id, int slot, int scroll);
	void IncreaseMission(int id, int n);
	void SearchShop(CardSearchResponse*, SearchType);
	void GetExp(int usr_id, int usr_exp, const char *Elements = 0, int amount = 0);
	void BuyElementCard(MyCharInfo *Info, CardType2 type, int amount);
	void UpdateItem(MyCharInfo*, int, int, int);
	void AddCardSlot(int, int);
	bool IsNewDayLogin(int);
	void VisitBonus(int, int, int, int, int);
	void GoldForceCardUse(int, int, int, int);

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

struct LobbyUser
{
	string name;
	bool gender;
	int level, mission;
	LobbyUser *ptr;
};

class LobbyList
{
private:
	int count;
	LobbyUser *root;

public:
	LobbyList();
	void Insert(LobbyUser);
	void Delete(string);
	int GetList(unsigned char*);
};

#endif
