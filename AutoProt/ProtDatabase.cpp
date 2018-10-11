#define _CRT_SECURE_NO_WARNINGS

#include "SQLiteCpp/Database.h"
#include "ProtDatabase.h"
#include "AutoProt.h"
#include "API/ARK/Ark.h"
#include <iostream>


	bool ProtDatabase::PlayerExists(uint64 steamId)
	{
		auto& db = getDB();

		try {
			SQLite::Statement query(db, "SELECT count(1) FROM protection WHERE SteamId = ? LIMIT 1;");
			query.bind(1, static_cast<int64>(steamId));
			query.executeStep();
			return query.getColumn(0).getInt() != 0;
		}
		catch (SQLite::Exception e) {
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, e.what());
		}
	}


	void ProtDatabase::AddPlayer(std::string name, uint64 steamId, uint64 tribeId, uint64 endtime)
	{
		auto& db = getDB();

		std::string prefix = AutoProt::config["DBPrefix"];
		std::string insert = "INSERT INTO protection (Name, SteamId, " + prefix + "_TribeId, EndTime) VALUES (?,?,?,?);";
		try {
			SQLite::Statement query(db, insert);
			query.bind(1, name);
			query.bind(2, static_cast<int64>(steamId));
			query.bind(3, static_cast<int64>(tribeId));
			query.bind(4, static_cast<int64>(endtime));
			query.exec();
		}
		catch (SQLite::Exception e) {
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, e.what());
		}
	}

	void ProtDatabase::SetPlayerTribe(uint64 steamId, uint64 tribeId) {
		auto& db = getDB();

		try {
			std::string prefix = AutoProt::config["DBPrefix"];
			std::string update = "UPDATE protection SET " + prefix + "_TribeId = ? WHERE SteamId = ?;";
			SQLite::Statement query(db, update);
			query.bind(1, static_cast<int64>(tribeId));
			query.bind(2, static_cast<int64>(steamId));
			query.exec();
		}
		catch (SQLite::Exception e) {
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, e.what());
		}
	}

	void ProtDatabase::AddPlayerIfNotPresent(std::string name, uint64 steamId, uint64 tribeId, uint64 endTime) {
		if (!ProtDatabase::PlayerExists(steamId)) {
			ProtDatabase::AddPlayer(name, steamId, tribeId, endTime);
		}
	}

	uint64 ProtDatabase::getTribeId(uint64 steamId) {
		auto& db = getDB();

		std::string prefix = AutoProt::config["DBPrefix"];
		std::string column = prefix + "_TribeId";

		try {
			SQLite::Statement query(db, "SELECT * FROM protection WHERE SteamId = ? LIMIT 1;");
			query.bind(1, static_cast<int64>(steamId));
			if (!query.executeStep()) return 0;
			return query.getColumn(column.c_str()).getInt64();
		}
		catch (SQLite::Exception e) {
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, e.what());
		}
	}

	void ProtDatabase::SetTime(uint64 steamId, bool wholeTribe, uint64 additionalTime) {
		auto& db = getDB();

		try {
			if (wholeTribe) {
				uint64 tribeId = getTribeId(steamId);
				std::string prefix = AutoProt::config["DBPrefix"];
				std::string update = "UPDATE protection SET EndTime = ? WHERE " + prefix + "_TribeId = ?;";
				SQLite::Statement query(db, update);
				query.bind(1, static_cast<int64>((uint64)(additionalTime * 3600) + std::time(0)));
				query.bind(2, static_cast<int64>(tribeId));
				query.exec();
				return;
			}
			SQLite::Statement query(db, "UPDATE protection SET EndTime = ? WHERE SteamId = ?;");
			query.bind(1, static_cast<int64>((uint64)(additionalTime * 3600) + std::time(0)));
			query.bind(2, static_cast<int64>(steamId));
			query.exec();
		}
		catch (SQLite::Exception e) {
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, e.what());
		}
	}

	SQLite::Database& ProtDatabase::getDB() {

		const std::string path = AutoProt::config["DBFilePath"];
		static SQLite::Database db((path + "/protection.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
		return db;
	}

	void ProtDatabase::InitDatabase()
	{  
		std::string prefix = AutoProt::config["DBPrefix"];
		try {
			SQLite::Statement create(getDB(), "CREATE TABLE IF NOT EXISTS protection(Name text, SteamId integer not null, EndTime long);");
			create.exec();	
		}
		catch (SQLite::Exception e) {
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, e.what());
		}
		try {
			std::string add = "ALTER TABLE protection ADD " + prefix + "_TribeId integer";
			SQLite::Statement alter(getDB(), add);
			alter.exec();
		}
		catch (SQLite::Exception ignored) {
			// Its all good since a Check if the Column already exists would just be unnecessary
		}
	}

	uint64 ProtDatabase::getEndTime(uint64 steamId) {
		if (steamId == 0) return 0;

		try {
			auto& db = getDB();

			SQLite::Statement query(db, "SELECT * FROM protection WHERE SteamId = ? LIMIT 1;");
			query.bind(1, static_cast<int64>(steamId));
			if (!query.executeStep()) return 0;
			return query.getColumn("EndTime").getInt64();
		}
		catch (SQLite::Exception e) {
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, e.what());
		}
	}

	uint64 ProtDatabase::getTribeEndTime(uint64 tribeId) {

		auto& db = getDB();
		
		uint64 lowestTime = _UI64_MAX;
		std::string prefix = AutoProt::config["DBPrefix"];
		std::string select = "SELECT * FROM protection WHERE " + prefix + "_TribeId = ? LIMIT 1;";

		SQLite::Statement query(db, select);
		query.bind(1, static_cast<int64>(tribeId));

		try {
			while (query.executeStep()) {
				uint64 current = query.getColumn("EndTime").getInt64();
				if (current < lowestTime) lowestTime = current;
			}
		}
		catch (SQLite::Exception e) {
			Log::GetLog()->error("({} {}) Unexpected DB error {}", __FILE__, __FUNCTION__, e.what());
			return 0;
		}
		return lowestTime;
	}
