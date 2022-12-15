#pragma once

#include "constructor.hpp"

class ConstructorFactory {
public:
    static std::shared_ptr<constructor> produce(std::string arg = "") {
        ArgContainer ac(arg);
        std::string name = "dummy";
        if (ac().find("name") != ac().end()) {
            name = static_cast<std::string>(ac()["name"]);
            // ac().erase("name");
        }

        if (name == "dummy")           return std::make_shared<DummyConstructor>(static_cast<std::string>(ac));
        else if (name == "es")         return std::make_shared<EvolutionStrategy>(static_cast<std::string>(ac));
        else if (name == "min-deploy" ) return std::make_shared<InstanceMinDeploy>(static_cast<std::string>(ac));
        else                           return std::make_shared<DummyConstructor>(static_cast<std::string>(ac));
    }
};