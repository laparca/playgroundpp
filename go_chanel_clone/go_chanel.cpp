#include <iostream>
#include <thread>
#include <vector>
#include <sstream>
#include <string>
#include <chanel.hpp>

int main(int argc, char** argv) {
	laparca::chanel<ssize_t> q;
	ssize_t totalElements = 1000;
	ssize_t numConsumers = 3;
	ssize_t numProducers = 2;

	using namespace std::string_literals;

	for (int i = 1; i < argc; i++) {
		if (argv[i] == "elements"s && i+1 < argc) {
			totalElements = std::stoi(argv[++i]);
		}
		else if (argv[i] == "consumers"s && i+1 < argc) {
			numConsumers = std::stoi(argv[++i]);
		}
		else if (argv[i] == "producers"s && i+1 < argc) {
			numProducers = std::stoi(argv[++i]);
		}
	}

	std::vector<std::unique_ptr<std::thread>> consumers(numConsumers);
	std::vector<std::unique_ptr<std::thread>> producers(numProducers);

	for (ssize_t consumer = 0; consumer < numConsumers; ++consumer)
		consumers[consumer] = std::make_unique<std::thread>([&q, consumer, totalElements, numConsumers]() {
			const ssize_t elementsToConsume = (totalElements / numConsumers) + (consumer + 1 == numConsumers ? (totalElements % numConsumers): 0);
			std::stringstream s;
			s << "[C:" << consumer<< "] Should consume " << elementsToConsume << " elements" << std::endl;
			std::clog << s.str();
			for (ssize_t i = 0; i < elementsToConsume; ++i) {
				auto v = q();
				if (v.has_value()) {
					std::stringstream s;
					s << "[C:" << consumer << "] Values is " << *v << std::endl;
					std::clog << s.str();
				}
			}
		});

	for (ssize_t producer = 0; producer < numProducers; ++producer)
		producers[producer] = std::make_unique<std::thread>([&q,producer, totalElements, numProducers](){
			const ssize_t elementsToProduce = (totalElements / numProducers) + (producer + 1 == numProducers ? (totalElements % numProducers) : 0);
			const ssize_t start = (totalElements / numProducers) * producer;

			std::clog << "[P:" << producer << "] Should produce " << elementsToProduce << " elements starting at " << start << std::endl;
			for (ssize_t i = 0; i < elementsToProduce; ++i) {
				std::clog << "[P:" << producer << "]: " << (i+start) << std::endl;
				q << (i+start);
			}
			
		});	

	for (auto &th : consumers)
		th->join();
	for (auto &th : producers)
		th->join();	
	return 0;
}
