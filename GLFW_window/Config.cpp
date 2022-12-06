#include "Config.h"
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<unordered_map>

bool LoadConfig(Configuration &config)
{
	//make a hashmap of string tags to our configuration enums
	//Hashmaps map keys to data in pairs
	std::unordered_map<std::string, ConfigTags> tagMap;

	//assign key/data pairs into the hashmap
	tagMap["DEADZONE"] = ConfigTags::DEADZONE;
	tagMap["MOVE"] = ConfigTags::MOVE;
	tagMap["STRAFE"] = ConfigTags::STRAFE;
	tagMap["YAW"] = ConfigTags::YAW;
	tagMap["PITCH"] = ConfigTags::PITCH;

	std::ifstream inFile("config.txt");

	if (!inFile.is_open()) return false; //ERROR reading file

										 //process the config file
	while (!inFile.eof())
	{
		std::string line;
		std::getline(inFile, line);

		//skip blank lines
		if (line.size() == 0) continue;
		//skip comments
		if (line[0] == '#') continue;

		std::string tag;
		float value;

		std::stringstream lstream;
		lstream << line;

		lstream >> tag;
		//check for lone tags with no data!
		if (lstream.str().empty()) continue;
		lstream >> value;

		//look up this tag in my hashmap
		auto search = tagMap.find(tag);
		if (search == tagMap.end()) continue; //no key in the map matches this tag

		switch (search->second)
		{
		case ConfigTags::DEADZONE:
			config.deadzone = value;
			break;
		case ConfigTags::MOVE:
			config.axis_move = value;
			break;
		case ConfigTags::PITCH:
			config.axis_pitch = value;
			break;
		case ConfigTags::STRAFE:
			config.axis_strafe = value;
			break;
		case ConfigTags::YAW:
			config.axis_yaw = value;
			break;
		default:
			break;
		}//end switch		
	}
	return true;
}

void DisplayConfig(Configuration &config)
{
	std::cout << "Deadzone: " << config.deadzone << "\n";
	std::cout << "Strafe: " << config.axis_strafe << "\n";
	std::cout << "Movement: " << config.axis_move << "\n";
	std::cout << "Yaw: " << config.axis_yaw << "\n";
	std::cout << "Pitch: " << config.axis_pitch << "\n";
}