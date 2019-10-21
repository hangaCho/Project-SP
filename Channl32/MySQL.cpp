#include "MySQL.h"

extern Ini config;

MySQL::MySQL()
{
	config.SetSection("DB");
	auto ip = config.ReadString("ip", "121.200.112.133");
	u32 port = config.ReadInt("port", 3306);
	auto user = config.ReadString("user", "sp");
	auto pw = config.ReadString("pw", "jj102365");
	auto db = config.ReadString("db", "spgame");

	connection = mysql_init(0);
	if (mysql_real_connect(connection, ip.c_str(), user.c_str(), pw.c_str(), db.c_str(), port, 0, 0)) {
		my_bool reconnect = 1;
		mysql_options(connection, MYSQL_OPT_RECONNECT, &reconnect);
	}
	else {
		Log::Error("Unable to connect to MySQL server");
	}
}

MySQL::~MySQL()
{
	mysql_close(connection);
}

void MySQL::SetUserIP(char *name, char* ip)
{
	Query("UPDATE users SET usr_ip = '{}' WHERE usr_name = '{}'", ip, name);
}

void MySQL::GetUserIP(int id, char *ip)
{
	QuerySelect("SELECT usr_ip FROM users WHERE usr_id = {}", id);
	MYSQL_ROW result = mysql_fetch_row(res);
	if (!result)
	{
		Log::Warning("No Data\n");
		return;
	}
	strcpy(ip, result[0]);
	mysql_free_result(res);
}

void MySQL::GetUserInfo(char *name, MyCharInfo &info)
{
	QuerySelect("SELECT usr_id, usr_gender, usr_char, usr_points, usr_code,"
		" usr_level, usr_mission, usr_nslots, usr_water, usr_fire,"
		" usr_earth, usr_wind, usr_coins FROM users WHERE usr_name = '{}'", name);

	MYSQL_ROW result = mysql_fetch_row(res);
	if (!result)
	{
		Log::Warning("No Data\n");
		return;
	}

	info.usr_id = atoi(result[0]);
	info.gender = atoi(result[1]);
	info.DefaultCharacter = atoi(result[2]);
	info.Points = _atoi64(result[3]);
	info.Code = _atoi64(result[4]);
	info.Level = atoi(result[5]);
	info.Mission = atoi(result[6]);
	info.nSlots = atoi(result[7]);
	info.UserType = 30;
	info.Water = atoi(result[8]);
	info.Fire = atoi(result[9]);
	info.Earth = atoi(result[10]);
	info.Wind = atoi(result[11]);
	info.Coins = atoi(result[12]);
	strcpy(info.usr_name, name);
	mysql_free_result(res);
}

void MySQL::GetUserItems(int id, bool *bMyCard, int *IDMyCard, int *TypeMyCard, int *GFMyCard, int *LevelMyCard, int *SkillMyCard)
{
	QuerySelect("SELECT itm_id, itm_slot, itm_type, itm_gf, itm_level,"
		" itm_skill FROM items WHERE itm_usr_id = {}", id);

	MYSQL_ROW result;
	while (result = mysql_fetch_row(res))
	{
		int slot = atoi(result[1]);
		bMyCard[slot] = true;
		if (IDMyCard)IDMyCard[slot] = atoi(result[0]);
		TypeMyCard[slot] = atoi(result[2]);
		GFMyCard[slot] = atoi(result[3]);
		LevelMyCard[slot] = atoi(result[4]);
		SkillMyCard[slot] = atoi(result[5]);
	}
	mysql_free_result(res);
}

int MySQL::GetValidSlot(int id, int maxslots)
{
	QuerySelect("SELECT itm_slot FROM items WHERE itm_usr_id = {}", id);
	MYSQL_ROW result;

	char buffer[256];
	ASSERT(_countof(buffer) >= maxslots);

	for (int i = 0; i < maxslots; i++)
		buffer[i] = i;
	while (result = mysql_fetch_row(res))
	{
		int n = atoi(result[0]);
		buffer[n] = -1;
	}
	mysql_free_result(res);
	for (int i = 0; i < maxslots; i++)
		if (buffer[i] != -1)return buffer[i];
	return -1;
}

void MySQL::BuyElementCard(MyCharInfo *Info, CardType2 type, int amount)
{
	switch (type) {
	case fire:
		Info->Fire += amount;
		Query("UPDATE users SET usr_fire = (usr_fire + {}) WHERE usr_id = {}", amount, Info->usr_id);
		break;
	case wind:
		Info->Wind += amount;
		Query("UPDATE users SET usr_wind = (usr_wind + {}) WHERE usr_id = {}", amount, Info->usr_id);
		break;
	case earth:
		Info->Earth += amount;
		Query("UPDATE users SET usr_earth = (usr_earth + {}) WHERE usr_id = {}", amount, Info->usr_id);
		break;
	case water:
		Info->Water += amount;
		Query("UPDATE users SET usr_water = (usr_water + {}) WHERE usr_id = {}", amount, Info->usr_id);
		break;
	}
}

void MySQL::InsertNewItem(MyCharInfo *Info, int item, int gf, int level, int skill)
{
	int validslot = GetValidSlot(Info->usr_id, Info->nSlots);
	if (validslot == -1)return;
	Query("INSERT INTO items VALUES (0,{},{},{},{},{},{})", validslot, Info->usr_id, item, gf, level, skill);
}

void MySQL::UpdateItem(MyCharInfo *Info, int slot, int item_id, int gf)
{
	if (slot < -1 || slot > 96)return;
	Query("UPDATE items SET itm_gf = (itm_gf + {}) WHERE itm_usr_id = {} AND itm_slot = {}", gf, Info->usr_id, slot);
}

int MySQL::DeleteItem(int id, int slotn)
{
	QuerySelect("SELECT itm_type FROM items WHERE itm_usr_id = {} AND itm_slot = {}", id, slotn);
	MYSQL_ROW result = mysql_fetch_row(res);
	if (!result)
	{
		printf("No Data\n");
		return 0;
	}
	int itm_type = atoi(result[0]);
	mysql_free_result(res);
	Query("DELETE FROM items WHERE itm_usr_id = {} AND itm_slot = {}", id, slotn);
	return itm_type;
}

void MySQL::GetShopItemCost(int item, int gf, int quantity, int &code, int &cash)
{
	QuerySelect("SELECT itm_gf, itm_code_cost, itm_cash_cost, itm_coins_cost FROM shop WHERE itm_type = {} AND itm_quantity = {}", item, quantity);
	MYSQL_ROW result;
	while (result = mysql_fetch_row(res))
	{
		int x = atoi(result[0]);
		if (gf == x || (!gf && !x))
		{
			cash = atoi(result[2]);
			break;
		}
		else if (!gf && x)
		{
			code = atoi(result[1]);
			break;
		}
	}
	mysql_free_result(res);

}

void MySQL::InsertMsg(char *sender, char *reciever, char *msg)
{
	Query("INSERT INTO chats VALUES (0,1,'{}','{}','{}')", sender, reciever, msg);
}

void MySQL::ChangeEquips(int id, EquipChangeRequest *ECR)
{
	if (!ECR)return;
	Query("UPDATE equipments SET eqp_mag = {}, eqp_wpn = {}, eqp_arm = {}, eqp_pet = {},"
		" eqp_foot = {}, eqp_body = {}, eqp_hand1 = {}, eqp_hand2 = {}, eqp_face = {},"
		" eqp_hair = {}, eqp_head = {} WHERE usr_id = {}",
		ECR->mag, ECR->wpn, ECR->arm, ECR->pet,
		ECR->foot, ECR->body, ECR->hand1, ECR->hand2, ECR->face,
		ECR->hair, ECR->head, id);
}

void MySQL::GetUserData(UserInfoResponse* UIR)
{
	QuerySelect("SELECT usr_id, usr_gender, usr_char, usr_points, usr_code, usr_coins,"
		" usr_level, usr_mission, usr_wins, usr_losses, usr_ko, usr_down"
		" FROM users WHERE usr_name = '{}'", UIR->username);
	MYSQL_ROW result = mysql_fetch_row(res);
	if (!result)
	{
		printf("No Data\n");
		return;
	}
	int uid = atoi(result[0]);
	UIR->gender = atoi(result[1]);
	UIR->defaultcharacter = atoi(result[2]);
	UIR->Points = _atoi64(result[3]);
	UIR->Codes = _atoi64(result[4]);
	UIR->Coins = atoi(result[5]);
	UIR->level = atoi(result[6]);
	UIR->mission = atoi(result[7]);
	UIR->wins = atoi(result[8]);
	UIR->losses = atoi(result[9]);
	UIR->KO = atoi(result[10]);
	UIR->Down = atoi(result[11]);

	mysql_free_result(res);

	QuerySelect("SELECT itm_type, itm_gf, itm_level, itm_skill FROM equipments INNER JOIN items"
		" WHERE itm_usr_id = {} AND usr_id = {}"
		" AND (itm_slot = eqp_mag OR itm_slot = eqp_wpn OR itm_slot = eqp_arm"
		" OR itm_slot = eqp_pet OR itm_slot = eqp_foot OR itm_slot = eqp_body"
		" OR itm_slot = eqp_hand1 OR itm_slot = eqp_hand2 OR itm_slot = eqp_face"
		" OR itm_slot = eqp_hair OR itm_slot = eqp_head)", uid, uid);

	ItemId Item_id;
	while (result = mysql_fetch_row(res))
	{
		int itm_type = atoi(result[0]), itm_gf = atoi(result[1]), itm_level = atoi(result[2]), itm_skill = atoi(result[3]);
		int id1 = Item_id.Identify(itm_type);
		if (id1 == black || id1 == gold || id1 == pet)
		{
			switch (Item_id.IdentifyItem(itm_type))
			{
			case ct_mag:
				UIR->magic = itm_type;
				UIR->magicgf = itm_gf;
				UIR->magiclevel = itm_level;
				UIR->magicskill = itm_skill;
				break;
			case ct_wpn:
				UIR->weapon = itm_type;
				UIR->weapongf = itm_gf;
				UIR->weaponlevel = itm_level;
				UIR->weaponskill = itm_skill;
				break;
			case ct_arm:
				UIR->armour = itm_type;
				UIR->armourgf = itm_gf;
				UIR->armourlevel = itm_level;
				UIR->armourskill = itm_skill;
				break;
			case ct_pet:
				UIR->pet = itm_type;
				UIR->petgf = itm_gf;
				UIR->petlevel = itm_level;
				break;
			default:
				break;
			}
		}
		else if (id1 == avatar)
		{
			switch (Item_id.IdentifyItem(itm_type))
			{
			case ct_foot:
				UIR->foot = itm_type;
				break;
			case ct_body:
				UIR->body = itm_type;
				break;
			case ct_hand1:
				UIR->hand1 = itm_type;
				break;
			case ct_hand2:
				UIR->hand2 = itm_type;
				break;
			case ct_face:
				UIR->face = itm_type;
				break;
			case ct_hair:
				UIR->hair = itm_type;
				break;
			case ct_head:
				UIR->head = itm_type;
				break;
			default:
				break;
			}
		}
	}
	mysql_free_result(res);
}

void MySQL::GetEquipData(int id, RoomPlayerDataResponse *RPDR)
{
	QuerySelect("SELECT itm_type, itm_gf, itm_level, itm_skill FROM equipments INNER JOIN items"
		" WHERE itm_usr_id = {} AND usr_id = {} AND (itm_slot = eqp_mag"
		" OR itm_slot = eqp_wpn OR itm_slot = eqp_arm OR itm_slot = eqp_pet"
		" OR itm_slot = eqp_foot OR itm_slot = eqp_body OR itm_slot = eqp_hand1"
		" OR itm_slot = eqp_hand2 OR itm_slot = eqp_face OR itm_slot = eqp_hair OR itm_slot = eqp_head)",
		id, id);

	MYSQL_ROW result;
	ItemId Item_id;
	while (result = mysql_fetch_row(res))
	{
		int itm_type = atoi(result[0]), itm_gf = atoi(result[1]), itm_level = atoi(result[2]), itm_skill = atoi(result[3]);
		int id1 = Item_id.Identify(itm_type);
		if (id1 == black || id1 == gold || id1 == pet)
		{
			switch (Item_id.IdentifyItem(itm_type))
			{
			case ct_mag:
				RPDR->magictype = itm_type;
				RPDR->magicgf = itm_gf;
				RPDR->magiclevel = itm_level;
				RPDR->magicskill = itm_skill;
				break;
			case ct_wpn:
				RPDR->weapontype = itm_type;
				RPDR->weapongf = itm_gf;
				RPDR->weaponlevel = itm_level;
				RPDR->weaponskill = itm_skill;
				break;
			case ct_arm:
				RPDR->armortype = itm_type;
				RPDR->armorgf = itm_gf;
				RPDR->armorlevel = itm_level;
				RPDR->armorskill = itm_skill;
				break;
			case ct_pet:
				RPDR->pettype = itm_type;
				RPDR->petgf = itm_gf;
				RPDR->petlevel = itm_level;
				break;
			default:
				break;
			}
		}
		else if (id1 == avatar)
		{
			switch (Item_id.IdentifyItem(itm_type))
			{
			case ct_foot:
				RPDR->foot = itm_type;
				break;
			case ct_body:
				RPDR->body = itm_type;
				break;
			case ct_hand1:
				RPDR->hand1 = itm_type;
				break;
			case ct_hand2:
				RPDR->hand2 = itm_type;
				break;
			case ct_face:
				RPDR->face = itm_type;
				break;
			case ct_hair:
				RPDR->hair = itm_type;
				break;
			case ct_head:
				RPDR->head = itm_type;
				break;
			default:
				break;
			}
		}
	}

	mysql_free_result(res);
}

void MySQL::GetMoneyAmmount(int id, int *cash, unsigned __int64 *code, char sign, int cashcost, unsigned __int64 codecost)
{
	if (sign)
	{
		if (cashcost)
			Query("UPDATE users SET usr_cash = (usr_cash{}{}) WHERE usr_id = {}", sign, cashcost, id);

		if (codecost)
			Query("UPDATE users SET usr_code = (usr_code{}{}) WHERE usr_id = {}", sign, codecost, id);
	}

	QuerySelect("SELECT usr_cash, usr_code FROM users WHERE usr_id = {}", id);
	MYSQL_ROW result = mysql_fetch_row(res);
	if (!result) return;

	if (cash)*cash = atoi(result[0]);
	if (code)*code = _atoi64(result[1]);
	mysql_free_result(res);
}

void MySQL::UpgradeCard(MyCharInfo *Info, CardUpgradeResponse *CUR)
{
	QuerySelect("SELECT itm_type, itm_gf, itm_level, itm_skill FROM items WHERE itm_usr_id = {} AND itm_slot = {}",
		Info->usr_id, CUR->Slot);
	MYSQL_ROW result = mysql_fetch_row(res);
	if (!result)
		return;
	ItemId Item;
	CUR->Type = atoi(result[0]);
	CUR->GF = atoi(result[1]);
	CUR->Level = atoi(result[2]);
	int old_skill = atoi(result[3]);
	mysql_free_result(res);
	String query;
	CUR->UpgradeResult = 5; // 1 level, 5 skill

	// try to upgrade and get a result
	bool levelUpFailed = false;
	if (CUR->UpgradeType == 1 || CUR->UpgradeType == 3)
	{
		if (Item.UpgradeItem(CUR->GF, CUR->Level))
		{
			CUR->Level += 1;
			CUR->UpgradeResult = 1;  //1
		}
		else
		{
			levelUpFailed = true;
		}
	}
	int CompensateNum = 0;
	// caculating costs of upgrade
	switch (CUR->UpgradeType)
	{
	case 1://Element Level
	case 2://Element Skill
	{
		int ItemSpirite = (CUR->Type % 100) / 10;   //정령소모값 10/10 설정할 시 소모안됨
		int EleCost = Item.GetUpgradeCost(CUR->Type, CUR->Level, CUR->UpgradeType);
		if(CUR->Level > 4) CompensateNum = COMPENSATE_RATE * EleCost + COMPENSATE_RATE * (rand() % EleCost);
		if (ItemSpirite == 1)
		{
			Info->Water -= EleCost;
			if (levelUpFailed) Info->Water += CompensateNum;
			query = format("UPDATE users SET usr_water = {} WHERE usr_id = {}",
				Info->Water, Info->usr_id);
		}
		else if (ItemSpirite == 2)
		{
			Info->Fire -= EleCost;
			if (levelUpFailed) Info->Fire += CompensateNum;
			query = format("UPDATE users SET usr_fire = {} WHERE usr_id = {}",
				Info->Fire, Info->usr_id);
		}
		else if (ItemSpirite == 3)
		{
			Info->Earth -= EleCost;
			if (levelUpFailed) Info->Earth += CompensateNum;
			query = format("UPDATE users SET usr_earth = {} WHERE usr_id = {}",
				Info->Earth, Info->usr_id);
		}
		else if (ItemSpirite == 4)
		{
			Info->Wind -= EleCost;
			if (levelUpFailed) Info->Wind += CompensateNum;
			query = format("UPDATE users SET usr_wind = {} WHERE usr_id = {}",
				Info->Wind, Info->usr_id);
		}
		if (!query.empty())
			Query(query);

	}
	break;
	case 3://Level Fusion
	case 4://Skill Fusion
	case 5://Skill 1 Fusion
	case 6://Skill 2 Fusion
	{
		QuerySelect("SELECT itm_level,itm_slot FROM `items` WHERE "
			" itm_usr_id = {} AND itm_type = {} ORDER BY itm_level ASC LIMIT 1",
			Info->usr_id, CUR->UpgradeType + 2010);
		MYSQL_ROW result = mysql_fetch_row(res);
		if (!result)
			return;
		int fusionlevel = atoi(result[0]);
		int fusionslot = atoi(result[1]);
		mysql_free_result(res);
		if (fusionlevel == 1)
			DeleteItem(Info->usr_id, fusionslot);
		else
			query = format("UPDATE items SET itm_level = {} WHERE itm_usr_id = {} AND itm_slot = {}",
				--fusionlevel, Info->usr_id, fusionslot);
		if (!query.empty())
			Query(query);
	}
	break;
	case 7://Skill 1 - 1 Fusion
	case 8://Skill 2 - 1 Fusion
	case 9://Skill 2 - 2 Fusion
	{
		QuerySelect("SELECT itm_level,itm_slot FROM `items` WHERE "
			" itm_usr_id = {} AND itm_type = {} ORDER BY itm_level ASC LIMIT 1",
			Info->usr_id, (CUR->UpgradeType + 2011));
		MYSQL_ROW result = mysql_fetch_row(res);
		if (!result)
			return;
		int fusionlevel = atoi(result[0]);
		int fusionslot = atoi(result[1]);
		mysql_free_result(res);
		if (fusionlevel == 1)
			DeleteItem(Info->usr_id, fusionslot);
		else
			query = format("UPDATE items SET itm_level = {} WHERE itm_usr_id = {} AND itm_slot = {}",
				--fusionlevel, Info->usr_id, fusionslot);
		if (!query.empty())
			Query(query);
	}
	break;
	}

	CUR->Skill = Item.GenerateSkill(CUR->Level, CUR->Type, CUR->UpgradeType, old_skill);
	CUR->CompensateNum = CompensateNum;
	CUR->WaterElements = Info->Water;
	CUR->FireElements = Info->Fire;
	CUR->EarthElements = Info->Earth;
	CUR->WindElements = Info->Wind;
	Query("UPDATE items SET itm_level = {}, itm_skill = {} WHERE itm_usr_id = {} AND itm_slot = {}",
		CUR->Level, CUR->Skill, Info->usr_id, CUR->Slot);
}


void MySQL::GetScrolls(MyCharInfo *Info)
{
	QuerySelect("SELECT usr_scroll1,usr_scroll2,usr_scroll3 FROM users WHERE usr_id = {}",
		Info->usr_id);
	MYSQL_ROW result = mysql_fetch_row(res);
	if (!result)
		return;

	for (int i = 0; i < 3; i++)
		Info->scrolls[i] = atoi(result[i]);
	mysql_free_result(res);
}

void MySQL::UpdateScrolls(int id, int slot, int scroll)
{
	Query("UPDATE users SET usr_scroll{} = {} WHERE usr_id = {}",
		slot + 1, scroll, id);
}

void MySQL::IncreaseMission(int id, int n)
{
	Query("UPDATE users SET usr_mission = usr_mission + {} WHERE usr_id = {}",
		n, id);
}

void MySQL::SearchShop(CardSearchResponse *CSR, SearchType type)
{
	QuerySelect("SELECT * FROM cardshop WHERE shop_type = {}", type);

	CSR->total = res->row_count;
	MYSQL_ROW result = 0;
	int i = 0;
	while ((result = mysql_fetch_row(res)) != 0 && i < 5) {
		CSR->rooms[i] = atoi(result[2]);
		strcpy(CSR->name[i], result[3]);
		CSR->levels[i] = atoi(result[4]);
		CSR->gender[i] = atoi(result[5]);
		CSR->price[i] = _atoi64(result[6]);
		CSR->card[i] = atoi(result[7]);
		CSR->gf[i] = atoi(result[8]);
		CSR->level[i] = atoi(result[9]);
		CSR->skill[i] = atoi(result[10]);
		i++;
	}

	while (i < 5) {
		CSR->rooms[i] = -1;
		CSR->name[i][0] = 0;
		CSR->levels[i] = -1;
		CSR->gender[i] = 0;
		CSR->price[i] = -1;
		CSR->card[i] = -1;
		CSR->gf[i] = -1;
		CSR->level[i] = -1;
		CSR->skill[i] = -1;
		i++;
	}

	mysql_free_result(res);
}

void MySQL::GetExp(int usr_id, int usr_exp, const char *Elements, int Ammount)
{
	if (Elements) {
		Query("UPDATE users SET usr_points = (usr_points + {}), usr_code = (usr_code + {}), usr_{} = (usr_{} + {}) WHERE usr_id = {}",
			usr_exp, usr_exp, Elements, Elements, Ammount, usr_id);
	}
	else {
		Query("UPDATE users SET usr_points = (usr_points + {}), usr_code = (usr_code + {})  WHERE usr_id = {}",
			usr_exp, usr_exp, usr_id);
	}
}

void MySQL::AddCardSlot(int usr_id, int slotn)
{
	int addslot = 0;
	int slot_type = 0;
	int nslot = 0;
	QuerySelect("SELECT itm_type FROM items WHERE itm_usr_id = {} AND itm_slot = {}",
		usr_id, slotn);

	MYSQL_ROW result = mysql_fetch_row(res);
	if (!result) return;
	if (result[0])
	{
		slot_type = atoi(result[0]);
		mysql_free_result(res);
	}
	else return;
	QuerySelect("SELECT usr_nslots FROM users WHERE usr_id = {}",
		usr_id);
	MYSQL_ROW result2 = mysql_fetch_row(res);
	if (!result2) return;
	if (result2[0])
	{
		nslot = atoi(result[0]);
		mysql_free_result(res);
	}
	else return;
	if (slot_type == 2005) addslot = 12;
	else if (slot_type == 2004) addslot = 6;
	else addslot = 0;
	if ((nslot + addslot) >= 96)
		addslot = 0;
	DeleteItem(usr_id, slotn);
	Query("UPDATE users SET usr_nslots = (usr_nslots + {}) WHERE usr_id = {}",
		addslot, usr_id);
}

bool MySQL::IsNewDayLogin(int usr_id) {
	time_t tick;
	tm now_time;
	tm last_login_time;

	QuerySelect("SELECT usr_last_login FROM users WHERE usr_id = {}", usr_id);

	MYSQL_ROW result = mysql_fetch_row(res);
	if (!result) return false;
	if (result[0])
	{
		last_login_time.tm_year = atoi(result[0]) / 10000;
		last_login_time.tm_mon = (atoi(result[0]) / 100) % 100;
		last_login_time.tm_mday = atoi(result[0]) % 100;
	}
	else
	{
		last_login_time.tm_year = 0;
		last_login_time.tm_mon = 0;
		last_login_time.tm_yday = 0;
	}
	mysql_free_result(res);
	tick = time(NULL);
	now_time = *localtime(&tick);
	now_time.tm_year += 1900;
	now_time.tm_mon++;
	int sql_last_login_time = now_time.tm_year * 10000 + now_time.tm_mon * 100 + now_time.tm_mday;

	Query("UPDATE users SET usr_last_login = {} WHERE usr_id = {}",
		sql_last_login_time, usr_id);

	if (last_login_time.tm_year <= now_time.tm_year)
		if (last_login_time.tm_mon <= now_time.tm_mon)
			if (last_login_time.tm_mday < now_time.tm_mday)
				return true;
	return false;
}

void MySQL::VisitBonus(int code, int type, int base, int multiple, int usr_id)  //코드를 캐쉬로바꿈
{
	int Ele_Cal = base * multiple;
	Query("UPDATE users SET usr_cash = (usr_cash + {}),usr_{} = (usr_{} + {}) WHERE usr_id = {}",
		code, ElementTypes[type], ElementTypes[type], Ele_Cal, usr_id);
}

void MySQL::GoldForceCardUse(int usr_id, int slot, int type, int gfslot)
{
	int gfday = 0;
	QuerySelect("SELECT itm_gf FROM items WHERE itm_slot = {} AND itm_usr_id = {}", gfslot, usr_id);
	MYSQL_ROW result = mysql_fetch_row(res);
	if (!result) return;
	if (!result[0]) gfday = atoi(result[0]);
	if (DeleteItem(usr_id, gfslot))
		Query("UPDATE items SET itm_gf = (itm_gf + {}) WHERE itm_slot = {} AND itm_type = {} AND itm_usr_id = {}",
			gfday, slot, type, usr_id);
}