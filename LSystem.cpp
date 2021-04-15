#include "LSystem.h"
#include <fstream>
#include <random>

LSystem::LSystem() {
}

LSystem::~LSystem() {
}

LSystem::LSystem(std::string fileName) {
	std::ifstream file_stream;
	file_stream.open(fileName);

	if (file_stream.is_open() && file_stream.good())
	{
		// read the initial currentBuildInstructions
		std::string line;
		std::getline(file_stream, line);
		currentBuildInstructions = line;

		// add the rules of the axiom
		while (std::getline(file_stream, line)) {
			std::pair<std::string, float> production;

			// index of production
			const unsigned int index = stoi(line.substr(0, line.find(':')));

			// symbol of production
			line.erase(0, line.find(':') + 1);
			const char symbol = (line.substr(0, line.find(':')))[0];

			// output of given production
			line.erase(0, line.find(':') + 1);
			production.first = (line.substr(0, line.find(':')));

			// probability of prdouction
			line.erase(0, line.find(':') + 1);
			production.second = std::stof(line.substr(0, line.find(':')));

			// if the given index/production already exists append to list of productions for the current symbol
			if (index < axiom.size()) {
				axiom[index].second.push_back(production);
			}
			// else add the new production to the axiom
			else {
				std::pair<char, std::vector<std::pair<std::string, float>>> newProduction;
				newProduction.first = symbol;
				newProduction.second.push_back(production);
				axiom.push_back(newProduction);
			}
		}
	}
}

void LSystem::simulateGeneration() {
	std::mt19937 rng;
	rng.seed(std::random_device()());
	const std::uniform_real_distribution<float> rndPercentageGenerator(0, 1);

	std::string newBuildInstructions;
	for (char characterToCheck : currentBuildInstructions) {
		// iterate over given currentBuildInstructions and replace chars with the production output
		for (auto productionList : axiom) {
			if (characterToCheck == productionList.first) {
				float productionProbability = 0.0f;
				float rndPercentage = rndPercentageGenerator(rng);
				//float rndPercentage = rand() / SHRT_MAX;

				for (auto production : productionList.second) {
					productionProbability += production.second;
					if (rndPercentage <= productionProbability) {
						newBuildInstructions += production.first;
						break;
					}
				}
			}
			// if no production was found/triggerd the character stays the same in the next generation
			else if (axiom[axiom.size() - 1] == productionList)
			{
				newBuildInstructions += characterToCheck;
			}
		}
	}
	currentBuildInstructions = newBuildInstructions;
}