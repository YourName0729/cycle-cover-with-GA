#pragma once

#include "constructor.hpp"
#include "grid_search.hpp"

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
        else if (name == "grid-search") return std::make_shared<GridSearch>(static_cast<std::string>(ac));
        else                           return std::make_shared<DummyConstructor>(static_cast<std::string>(ac));
    }
};