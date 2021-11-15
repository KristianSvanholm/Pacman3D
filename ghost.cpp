#include"ghost.h"
#include<vector>
#include<iostream>
#include "glm/glm/glm.hpp"

using namespace std;
vector<vector<int>> level;
extern bool gameOver;

Ghost::Ghost(std::vector<std::vector<int>> _level, int _x, int _y)
{
	prevGridPosition = gridPosition = glm::vec3(_x, -0.25, _y);
	dir = glm::vec2(0, 0);
	level = _level;

	//generate start direction
	currentDir = newDirection();
}

/// <summary>
/// Finds an initial direction to travel after spawning
/// </summary>
/// <returns>new direction</returns>
int Ghost::newDirection() {
	if (checkDir(1, 0)) { return 0; }
	else if (checkDir(-1, 0)) { return 1; }
	else if (checkDir(0, 1)) { return 2; }
	else if (checkDir(0, -1)) { return 3; }
}

bool Ghost::checkDir(int dirx, int diry) {
	//Checks if new position is valid or not
	int _x = gridPosition.x + dirx;
	int _y = gridPosition.z + diry;

	bool outOfX = (_x < 0 || _x > level.size() - 1);
	bool outOfY = (_y < 0 || _y > level[1].size() - 1);
	bool outOfBounds = (outOfX || outOfY);

	if (!outOfBounds && level[_x][_y] != 1) return true;
	return false;
}

/// <summary>
/// Ghost AI with three stages, Discovery, Choice & Action
/// discovers possible moves,
/// chooses one of all available
/// performs that choice by moving to that location
/// </summary>
/// <param name="time">Update the ghosts own time keeping</param>
void Ghost::move(float time) {
	transform = true;

	int options[4] = { 0,0,0,0 };
	int tempx, tempy;

	//DISCOVERY
	int j = 0;
	for (int i = 0; i < 4; i++) {
		switch (i)
		{
		case 0: tempx = 1; tempy = 0;  break;
		case 1:	tempx = -1; tempy = 0;  break;
		case 2: tempx = 0; tempy = 1;  break;
		case 3: tempx = 0; tempy = -1;  break;
		default: cerr << "Something went wrong with ghost ai"; return;
		}
		if (checkDir(tempx, tempy) && ((tempx != -dir.x && tempy == 0) || (tempy != -dir.y && tempx == 0))) {
			options[j] = i;
			j++;
		}
	}

	//CHOICE
	int choice = 0;
	switch (j) {
	case 0: turn = true; break;
	case 1: currentDir = options[j - 1];	break;
	case 2:
		choice = rand() % 2;
		currentDir = options[choice];	break;
	case 3:
		choice = rand() % 3;
		currentDir = options[choice];	break;
	}

	//ACTION
	switch (currentDir)
	{
	case 0: dir.x = 1; dir.y = 0; break; //cout << "East"  << endl; break;
	case 1:	dir.x = -1; dir.y = 0; break; //cout << "West"  << endl; break;
	case 2: dir.x = 0; dir.y = 1; break; //cout << "South" << endl; break;
	case 3: dir.x = 0; dir.y = -1; break; //cout << "North" << endl; break;
	default: cerr << "Something went wrong "; return;
	}

	//If no way other than backwards, flip the direction
	if (turn) {
		if (dir.x == 0) {
			dir.y *= -1;
		}
		else if (dir.y == 0) {
			dir.x *= -1;
		}
		turn = false;
	}
	prevGridPosition = gridPosition;

	gridPosition.x += dir.x;
	gridPosition.z += dir.y;

	//cout << gridPosition.x << " - " << gridPosition.z << endl;
	level[gridPosition.x][gridPosition.z] = 3;

	lastTime = time;
}

void Ghost::updateGhost(float time, float dt) {
	if (transform) {
		lerp(dt);
	}
	else {
		move(time);
	}
}

glm::vec3 Ghost::getPosition() {
	return exactPosition;
}

void Ghost::lerp(float dt) {
	linTime += dt*1;
	if (linTime <= 1) {
		float linx = ((float)prevGridPosition.x * (1.0 - linTime)) + ((float)gridPosition.x * linTime);
		float liny = -0.725+((float)prevGridPosition.z * (1.0 - linTime)) + ((float)gridPosition.z * linTime);
		exactPosition = glm::vec3(liny, gridPosition.y, linx);
	}
	else {
		//printMap();
		linTime = 0;
		transform = false;
	}
}

void Ghost::printMap() {
	system("CLS");
	for (int i = 0; i < level[1].size(); i++) {
		for (int j = 0; j < level.size(); j++) {
			cout << level[j][i] << " ";
		}
		cout << endl;
	}
}