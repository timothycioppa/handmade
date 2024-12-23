#ifndef _P_MAIN_HPP
#define _P_MAIN_HPP

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <reactphysics3d/reactphysics3d.h>
#include "Transform.hpp"

extern std::atomic<bool> updatePhysics;

struct PhysicsProcContext 
{ 
//	std::vector<GameEntity> * Entities;
	reactphysics3d::PhysicsWorld* world;
};

void P_Main(void* arg);
#endif