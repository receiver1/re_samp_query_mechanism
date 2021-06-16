#pragma once

/*
*	For fast read from sstream
*/
class c_fast_reader 
{
	std::stringstream *_ss;
public:
	void set_stream(std::stringstream &ss)
	{
		_ss = &ss;
	}

	template<typename T>
	T read_num(std::streampos pos = -1)
	{
		T result;
		if (pos != -1) {
			_ss->seekg(pos);
		}
		_ss->read(reinterpret_cast<char *>(&result), sizeof(T));
		return result;
	}

	template<typename T = uint32_t>
	std::string read_str(std::streampos pos = -1)
	{
		std::string result;
		T result_length = read_num<T>();
		result.resize(result_length);
		_ss->read(&result[0], result_length);
		return result;
	}
};

struct s_server_info : private c_fast_reader
{
	uint8_t is_locked;
	uint16_t players;
	uint16_t max_players;
	std::string host_name;
	std::string game_mode;
	std::string language;

	s_server_info(std::stringstream &ss)
	{
		ss.seekg(11); // set read cursor to 11 byte
		// why 11 bytes?: https://sampwiki.blast.hk/wiki/Query#Recieving_the_packets
		
		set_stream(ss);
		is_locked = read_num<uint8_t>();
		players = read_num<uint16_t>();
		max_players = read_num<uint16_t>();
		host_name = read_str();
		game_mode = read_str();
		language = read_str();
	}
};

struct s_server_ping : private c_fast_reader
{
	uint8_t first_ping, second_ping, third_ping, fourth_ping;

	s_server_ping(std::stringstream &ss)
	{
		ss.seekg(11);
		set_stream(ss);
		first_ping = read_num<uint8_t>();
		second_ping = read_num<uint8_t>();
		third_ping = read_num<uint8_t>();
		fourth_ping = read_num<uint8_t>();
	}
};

struct s_server_rules : private c_fast_reader
{
	std::map<std::string, std::string> rules;

	s_server_rules(std::stringstream &ss)
	{
		ss.seekg(11);
		set_stream(ss);
		uint16_t rule_count = read_num<uint16_t>();
		for (uint16_t i = 0; i < rule_count; ++i) {
			std::string rule, value;
			rule = read_str<uint8_t>();
			value = read_str<uint8_t>();
			rules.emplace(rule, value);
		}
	}
};

struct s_player_list : private c_fast_reader
{
	std::map<std::string, int> players;

	s_player_list(std::stringstream &ss)
	{
		ss.seekg(11);
		set_stream(ss);
		uint16_t players_count = read_num<uint16_t>();
		for (uint16_t i = 0; i < players_count; ++i) {
			std::string name = read_str<uint8_t>();
			int score = read_num<int>();
			players.emplace(name, score);
		}
	}
};
