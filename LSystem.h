#pragma once
#include <string>
#include <vector>

class LSystem {
public:
	LSystem();
	~LSystem();
	
	LSystem(std::string fileName);
	void simulateGeneration();

	std::string currentBuildInstructions;
private:
	std::vector<std::pair<char, std::vector<std::pair<std::string, float>>>> axiom;
};

