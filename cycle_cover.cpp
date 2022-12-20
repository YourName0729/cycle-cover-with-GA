#include "constructor_factory.hpp"

int main(int argc, char* argv[]) {
	std::string con_args;
	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		auto match_arg = [&](std::string flag) -> bool {
			auto it = arg.find_first_not_of('-');
			return arg.find(flag, it) == it;
		};
		auto next_opt = [&]() -> std::string {
			auto it = arg.find('=') + 1;
			return it ? arg.substr(it) : argv[++i];
		};
        if (match_arg("constructor")) {
            con_args = next_opt();
        }
	}


	auto con = ConstructorFactory::produce(con_args);
    auto pr = con->construct();

    // std::cout << "problem: \n" << *pr.first << '\n';
    // std::cout << "solution: \n" << pr.second << '\n';
	// std::cout << "objective: " << pr.first->objective(pr.second) << '\n';

    return 0;
}