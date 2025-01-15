#include "p_main.hpp"


reactphysics3d::PhysicsCommon physicsCommon;
reactphysics3d::PhysicsWorld* world;
std::atomic<bool> updatePhysics { true };

void P_Init() { 
   //world = physicsCommon.createPhysicsWorld();
   //	world->setIsGravityEnabled(true);

}

void P_Destroy() { 
//	physicsCommon.destroyPhysicsWorld(world);
}

void P_TerminatePhysicsThread() 
{ 
    // gUpdatePhysics = false;
	// physicsThread.join();
	// physicsCommon.destroyPhysicsWorld(world);
}

void P_Main(void* arg) 
{ 
	// PhysicsProcContext* context = (PhysicsProcContext* ) arg;
	// double currentTime = 0;
	// double lastPhysicsUpdateTime = 0;
	// double physicsUpdateRate = 1.0 / 60.0;

	// while (gUpdatePhysics) 
	// { 	
	// 	currentTime = glfwGetTime();

	// 	if (currentTime - lastPhysicsUpdateTime > physicsUpdateRate) 
	// 	{ 	
	// 		context->world->update(physicsUpdateRate);

	// 		entityAccessLock.lock();	
	// 		{
	// 			unsigned int entityCount = context->Entities->size();
				
	// 			for (unsigned int i = 0 ; i < entityCount; i++) 
	// 			{ 
	// 				GameEntity & entity = context->Entities->at(i);
	// 				reactphysics3d::RigidBody * rb = entity.rigidBody;		
	// 				SyncTransform((entity.sceneObject)->transform, rb);				
	// 			}
	// 		}
	// 		entityAccessLock.unlock();

	// 		lastPhysicsUpdateTime = currentTime;
	// 	}			
	// }
}

reactphysics3d::RigidBody * CreateRigidBody(Transform & transform) 
{ 
        glm::vec3 pos =     transform.position;
		reactphysics3d::Vector3 position(pos.x, pos.y, pos.z);
		reactphysics3d::Quaternion orientation = reactphysics3d::Quaternion::identity();
		reactphysics3d::Transform rb_transform(position, orientation);
		return world->createRigidBody(rb_transform);
}

void SyncTransform(Transform & transform, reactphysics3d::RigidBody * rb) { 
	    
		reactphysics3d::Vector3 newPosition = rb->getTransform().getPosition();
		transform.position = { newPosition.x, newPosition.y, newPosition.z };
		update_model_matrix(transform);
}