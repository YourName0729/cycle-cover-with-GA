#pragma once

#include <string>
#include <sstream>
#include <map>

class ArgContainer {
public:
    ArgContainer(const std::string& args = "") {
        std::stringstream ss(args);
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

    operator std::string() const {
        std::string re;
        for (const auto& [k, v] : meta) {
			if (v.value.find(' ') != std::string::npos) re += k + "=\'" + v.value + "\' ";
			else re += k + '=' + v.value + ' ';
		}
        if (meta.size()) re.pop_back();
        return re;
    }

public:
    typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};

public:
    std::map<key, value>& operator()() { return meta; }
    const std::map<key, value>& operator()() const { return meta; }

	void insert(const std::string& k, const std::string& v) { meta[k].value = v; }

private:
	std::map<key, value> meta;
};