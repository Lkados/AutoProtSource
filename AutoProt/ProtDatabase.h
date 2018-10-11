#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <SQLiteCpp/Database.h>

#include <API/ARK/Ark.h>
#include <API/UE/Math/ColorList.h>


namespace ProtDatabase
{

	SQLite::Database& getDB();

	/**
	 * checks if the steamId(player) exists in the database
	 */
	bool PlayerExists(uint64 steamId);

	/*
	 * adds the steamId to the database and set a timespamp with the additional time
	 */
	void AddPlayer(std::string name, uint64 steamId, uint64 tribeId, uint64 timestamp);

	void SetPlayerTribe(uint64 steamId, uint64 tribeId);

	
	/**
	 * adds a player to the database if he is not existing
	 */
	void AddPlayerIfNotPresent(std::string name, uint64 steamId, uint64 tribeId, uint64 timestamp);

	void SetTime(uint64 steamId, bool wholeTribe, uint64 additionalTime);

	/**
	* prepares the database for further queries
	*/
	void InitDatabase();

	uint64 getEndTime(uint64 steamId);
	uint64 getTribeEndTime(uint64 tribeId);

	uint64 getTribeId(uint64 steamId);
}