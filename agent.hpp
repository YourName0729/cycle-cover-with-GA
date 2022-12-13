#pragma once

#include <sstream>
#include <iostream>
#include <string>
#include <map>

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss("name=unknown role=unknown " + args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			if (!value.empty() && value.front() == '\'') {
				std::string appd;
				while (value.back() != '\'') {
					ss >> appd;
					value += " " + appd;
				}
				if (value.size()) value.pop_back();
				if (value.size()) value.erase(0, 1);	
			}
			meta[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}

public:
	virtual std::string property(const std::string& key) const {
		return meta.find(key) != meta.end()? meta.at(key) : std::string();
	}
	virtual void notify(const std::string& msg) { meta[msg.substr(0, msg.find('='))] = { msg.substr(msg.find('=') + 1) }; }
	virtual std::string name() const { return property("name"); }
	virtual std::string role() const { return property("role"); }

protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> meta;
};