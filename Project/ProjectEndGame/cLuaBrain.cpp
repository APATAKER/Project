#include "cLuaBrain.h"

#include <iostream>

#include<glm/vec4.hpp>
iObject* pFindObjectByFriendlyNameLUA(std::string name);

extern std::vector<iObject*> g_vec_pGameObjects;
//extern int lookDef;




// This is the base interface for all commands and command groups
struct sPair
{
	sPair() {};
	sPair(std::string stringData, glm::vec4 numData)
	{
		this->stringData = stringData;
		this->numData = numData;
	}
	sPair(std::string stringData)
	{
		this->stringData = stringData;
	}
	sPair(glm::vec4 numData)
	{
		this->numData = numData;
	}
	sPair(glm::vec3 numData)
	{
		this->numData = glm::vec4(numData, 1.0f);
	}
	sPair(float num)
	{
		this->numData.x = num;
	}
	std::string stringData;
	glm::vec4 numData;
};

class iCommand
{
public:
	virtual ~iCommand() {};
	virtual void SetGameObject(iObject* pGO) = 0;
	virtual void Init(std::vector<sPair> vecDetails) = 0;
	virtual void Update(double deltaTime) = 0;
	virtual bool IsDone(void) = 0;

	virtual void setName(std::string name) = 0;
	virtual std::string getName(void) = 0;

	// This is if you want your commands to ALSO be command groups:
	virtual void AddCommandSerial(iCommand* pCommand) = 0;
	//	virtual void AddCommandParallel( iCommand* pCommand ) = 0;
	virtual void AddCommandsParallel(std::vector<iCommand*> vec_pCommands) = 0;
};
//------------------------------------------------------------------------------------------------------------------
class cCommandGroup : public iCommand
{
public:
	cCommandGroup();
	virtual void SetGameObject(iObject* pGO);
	virtual void Init(std::vector<sPair> vecDetails);
	virtual void Update(double deltaTime);
	virtual bool IsDone(void);
	virtual void setName(std::string name) { this->m_Name = name; }
	virtual std::string getName(void) { return this->m_Name; }


	// This is if you want your commands to ALSO be command groups:
	virtual void AddCommandSerial(iCommand* pCommand);
	virtual void AddCommandsParallel(std::vector<iCommand*> vec_pCommands);

private:
	std::string m_Name;

	std::vector< iCommand* > vecSerial;
	std::vector< iCommand* > vecParallel;

	bool m_SerialCommandIsEmpty;
};

cCommandGroup::cCommandGroup()
{
	this->m_SerialCommandIsEmpty = true;
	return;
}


void cCommandGroup::Init(std::vector<sPair> vecDetails)
{
	// The command group doesn't need to process any commands. 
	return;
}

void cCommandGroup::Update(double deltaTime)
{
	// For serial commands, run the "next" command until it's done.
	if (!this->vecSerial.empty())
	{
		iCommand* pCurrent = *this->vecSerial.begin();
		std::cout << pCurrent->getName() << std::endl;

		if (!pCurrent->IsDone())
		{
			pCurrent->Update(deltaTime);
		}
		else
		{
			delete pCurrent;
			// remove this command from the vector
			this->vecSerial.erase(this->vecSerial.begin());
		}

	}

	// parallel  : run Update on ALL the parallel commands. 
	//             ONLY IF THEY RETURN FALSE on IsDone();
	for (std::vector< iCommand* >::iterator itCommand = this->vecParallel.begin();
		itCommand != this->vecParallel.end(); itCommand++)
	{
		iCommand* pCurrent = *this->vecSerial.begin();

		if (!pCurrent->IsDone())
		{
			pCurrent->Update(deltaTime);
		}

	}
	return;
}

bool cCommandGroup::IsDone(void)
{
	bool bAllDone = true;

	// Serial commands
	for (std::vector< iCommand* >::iterator itCommand = this->vecSerial.begin();
		itCommand != this->vecSerial.end(); itCommand++)
	{
		iCommand* pCurrent = *itCommand;

		if (!pCurrent->IsDone())
		{
			bAllDone = false;
		}
	}

	// Parallel commands
	for (std::vector< iCommand* >::iterator itCommand = this->vecParallel.begin();
		itCommand != this->vecParallel.end(); itCommand++)
	{
		iCommand* pCurrent = *itCommand;

		if (!pCurrent->IsDone())
		{
			bAllDone = false;
		}
	}

	return bAllDone;
}

void cCommandGroup::AddCommandSerial(iCommand* pCommand)
{

	this->vecSerial.push_back(pCommand);

	return;
}

void cCommandGroup::AddCommandsParallel(std::vector<iCommand*> vec_pCommands)
{
	//TODO
	for(int i=0;i<vec_pCommands.size();i++)
	this->vecSerial.push_back(vec_pCommands[i]);
	return;
}

void cCommandGroup::SetGameObject(iObject* pGO)
{
	return;
}

//---------------------------------------------------------------------------------------------------------------
class cMoveTo_Start_End_Time : public iCommand
{
public:
	// Pass: 
	// - Start location (vec3)
	// - End location (vec3)
	// - Number of seconds to move (x)
	virtual void Init(std::vector<sPair> vecDetails);
	virtual void SetGameObject(iObject* pGO);
	virtual void Update(double deltaTime);
	virtual bool IsDone(void);

	virtual void setName(std::string name) { this->m_Name = name; }
	virtual std::string getName(void) { return this->m_Name; }

	// This is if you want your commands to ALSO be command groups:
	virtual void AddCommandSerial(iCommand* pCommand);
	virtual void AddCommandsParallel(std::vector<iCommand*> vec_pCommands);

private:
	std::string m_Name;

	std::vector< iCommand* > vecSerial;
	std::vector< iCommand* > vecParallel;

	iObject* m_pTheGO;

	glm::vec3 m_startPosition;
	glm::vec3 m_endPosition;
	float m_TimeToMove;

	glm::vec3 m_velocity;
	float m_totalDistance;	// when the object has moved this far, it's done

};

void cMoveTo_Start_End_Time::Init(std::vector<sPair> vecDetails)
{
	this->m_startPosition = glm::vec3(vecDetails[0].numData);
	this->m_endPosition = glm::vec3(vecDetails[1].numData);
	this->m_TimeToMove = glm::vec3(vecDetails[2].numData).x;

	// Calculate the velocity...
	glm::vec3 moveVector = this->m_endPosition - this->m_startPosition;

	this->m_totalDistance = glm::length(moveVector);

	float speed = this->m_totalDistance / this->m_TimeToMove;

	glm::vec3 direction = glm::normalize(moveVector);

	this->m_velocity = direction * speed;

	return;
}
void cMoveTo_Start_End_Time::SetGameObject(iObject* pGO)
{
	this->m_pTheGO = pGO;
	return;
}

void cMoveTo_Start_End_Time::Update(double deltaTime)
{
	glm::vec3 deltaStep = this->m_velocity * (float)deltaTime;

	// This could be done in the physics engine... 
	//this->m_pTheGO->getPositionXYZ() += deltaStep;
	this->m_pTheGO->setPositionXYZ(this->m_pTheGO->getPositionXYZ() + deltaStep);

	return;
}

bool cMoveTo_Start_End_Time::IsDone(void)
{
	float distance = glm::distance(this->m_pTheGO->getPositionXYZ(),
		this->m_startPosition);

	if (distance >= this->m_totalDistance)
	{
		return true;
	}

	return false;
}

void cMoveTo_Start_End_Time::AddCommandSerial(iCommand* pCommand)
{
	this->vecSerial.push_back(pCommand);
	return;
}

void cMoveTo_Start_End_Time::AddCommandsParallel(std::vector<iCommand*> vec_pCommands)
{
	for (int i = 0; i < vec_pCommands.size(); i++)
		this->vecParallel.push_back(vec_pCommands[i]);
	return;
}
//------------------------------------------------------------------------------------------------------------
class cRotateRelativeOverTime : public iCommand
{
public:
	cRotateRelativeOverTime()
	{
		this->m_UpdateHasBeeCalled = false;
	}
	// Rotate object RELATIVE to where command starts
	// Pass: 
	// - End rotation (Euler degrees) (vec3)
	// - Number of seconds to move (x)
	virtual void Init(std::vector<sPair> vecDetails);
	virtual void SetGameObject(iObject* pGO);
	virtual void Update(double deltaTime);
	virtual bool IsDone(void);

	virtual void setName(std::string name) { this->m_Name = name; }
	virtual std::string getName(void) { return this->m_Name; }

	// This is if you want your commands to ALSO be command groups:
	virtual void AddCommandSerial(iCommand* pCommand);
	virtual void AddCommandsParallel(std::vector<iCommand*> vec_pCommands);

private:

	// Flag to see if update has been called once
	bool m_UpdateHasBeeCalled;

	std::string m_Name;

	std::vector< iCommand* > vecSerial;
	std::vector< iCommand* > vecParallel;

	iObject* m_pTheGO;

	glm::quat m_startOrentation;		// Starting location
	glm::quat m_endOrientation;			// Ending location
	glm::quat m_rotationalSpeed;		// Like velocity
	float m_TimeToRotate;

	double m_TimeElapsed;
};
void cRotateRelativeOverTime::Init(std::vector<sPair> vecDetails)
{
	// - End rotation (Euler degrees) (vec3)
	// - Number of seconds to move (x)

	glm::vec3 EulerAngle;
	EulerAngle.x = glm::radians(vecDetails[0].numData.x);
	EulerAngle.y = glm::radians(vecDetails[0].numData.y);
	EulerAngle.z = glm::radians(vecDetails[0].numData.z);

	this->m_endOrientation = glm::quat(EulerAngle);

	this->m_TimeToRotate = vecDetails[1].numData.x;

	return;
}
void cRotateRelativeOverTime::Update(double deltaTime)
{
	if (!this->m_UpdateHasBeeCalled)
	{
		this->m_startOrentation = this->m_pTheGO->getRotationXYZ();

		// https://stackoverflow.com/questions/22157435/difference-between-the-two-quaternions
		glm::quat invStart = glm::inverse(this->m_startOrentation);
		glm::quat qRoationChange = invStart * this->m_endOrientation;

		// How "fast" do we rotation this? 
//		this->m_rotationalSpeed = qRoationChange / this->m_TimeToRotate;

		this->m_UpdateHasBeeCalled = true;
	}

	// Just like updating the position, we scale (SLERP) based on deltaTime

	// get a number between 0.0 and 1.0f
	float AmountOfRotationCompleted = (float)this->m_TimeElapsed / this->m_TimeToRotate;

	glm::quat qCurrentRotation
		= glm::slerp(this->m_startOrentation, this->m_endOrientation,
			AmountOfRotationCompleted);

	this->m_pTheGO->setRotationXYZ(qCurrentRotation);



	this->m_TimeElapsed += deltaTime;


	return;
}
bool cRotateRelativeOverTime::IsDone(void)
{
	if (this->m_TimeElapsed >= this->m_TimeToRotate)
	{
		return true;
	}
	return false;
}
void cRotateRelativeOverTime::SetGameObject(iObject* pGO)
{
	this->m_pTheGO = pGO;
	return;
}


// This is if you want your commands to ALSO be command groups:
void cRotateRelativeOverTime::AddCommandSerial(iCommand* pCommand)
{
	this->vecSerial.push_back(pCommand);
	return;
}
void cRotateRelativeOverTime::AddCommandsParallel(std::vector<iCommand*> vec_pCommands)
{
	for (int i = 0; i < vec_pCommands.size(); i++)
		this->vecParallel.push_back(vec_pCommands[i]);
	return;
}


iCommand* pScene = new cCommandGroup();
std::vector<iCommand*> vParallelComd;
//void CreateCommand(std::string commandType)
int CreateCommand(lua_State* L)
{
	std::string commandType = lua_tostring(L, 1);
	

	if (commandType == "Script")
	{
		
		//lookDef = 1;
		iObject* pEagleTocommand = pFindObjectByFriendlyNameLUA("eagle");
		{
			iCommand* rotate = new cRotateRelativeOverTime();
			rotate->SetGameObject(pEagleTocommand);
			rotate->setName("Rotate to");
			sPair EndOrientation;		EndOrientation.numData.x = 0.0f;
			EndOrientation.numData.y = -90.0f;
			EndOrientation.numData.z = 0.0f;
			sPair Time;			Time.numData.x = 3.0f;
			std::vector<sPair> vecParams;
			vecParams.push_back(EndOrientation);
			vecParams.push_back(Time);
			rotate->Init(vecParams);
		//	pScene->AddCommandSerial(rotate);
			vParallelComd.push_back(rotate);
			//pScene->AddCommandsParallel(vParallelComd);
		}
		{
			iCommand* pMoveTo = new cMoveTo_Start_End_Time();
			pMoveTo->SetGameObject(pEagleTocommand);
			pMoveTo->setName("Move to");
			sPair From;		From.numData = glm::vec4(pEagleTocommand->getPositionXYZ(), 1.0f);
			sPair To;		To.numData = glm::vec4(-244.4, 194.3, 3.7, 1.0f);
			sPair Speed;	Speed.numData.x = 10.0f;		// 1 unit per second
			std::vector<sPair> vecParams;
			vecParams.push_back(From);
			vecParams.push_back(To);
			vecParams.push_back(Speed);
			pMoveTo->Init(vecParams);
			//pScene->AddCommandSerial(pMoveTo);
			vParallelComd.push_back(pMoveTo);
			//pScene->AddCommandsParallel(vParallelComd);
		}
		{
			iCommand* rotate = new cRotateRelativeOverTime();
			rotate->SetGameObject(pEagleTocommand);
			rotate->setName("Rotate to");
			sPair EndOrientation;		EndOrientation.numData.x = 0.0f;
										EndOrientation.numData.y = 0.0f;
										EndOrientation.numData.z = 0.0f;
			sPair Time;			Time.numData.x = 3.0f;
			std::vector<sPair> vecParams;
			vecParams.push_back(EndOrientation);
			vecParams.push_back(Time);
			rotate->Init(vecParams);
		//	pScene->AddCommandSerial(rotate);
			vParallelComd.push_back(rotate);
			//pScene->AddCommandsParallel(vParallelComd);
		}

		{
			iCommand* pMoveTo = new cMoveTo_Start_End_Time();
			pMoveTo->SetGameObject(pEagleTocommand);
			pMoveTo->setName("Move to");
			sPair From;		From.numData = glm::vec4(-244.4, 194.3, 3.7, 1.0f);
			sPair To;		To.numData = glm::vec4(-73.4, 238.1, 527.0, 1.0f);
			sPair Speed;	Speed.numData.x = 10.0f;		// 1 unit per second
			std::vector<sPair> vecParams;
			vecParams.push_back(From);
			vecParams.push_back(To);
			vecParams.push_back(Speed);
			pMoveTo->Init(vecParams);
		//	pScene->AddCommandSerial(pMoveTo);
			vParallelComd.push_back(pMoveTo);
			//pScene->AddCommandsParallel(vParallelComd);
		}
		{
			iCommand* rotate = new cRotateRelativeOverTime();
			rotate->SetGameObject(pEagleTocommand);
			rotate->setName("Rotate to");
			sPair EndOrientation;		EndOrientation.numData.x = 0.0f;
			EndOrientation.numData.y = 90.0f;
			EndOrientation.numData.z = 0.0f;
			sPair Time;			Time.numData.x = 3.0f;
			std::vector<sPair> vecParams;
			vecParams.push_back(EndOrientation);
			vecParams.push_back(Time);
			rotate->Init(vecParams);
		//	pScene->AddCommandSerial(rotate);
			vParallelComd.push_back(rotate);
			//pScene->AddCommandsParallel(vParallelComd);
		}
		{
			iCommand* pMoveTo = new cMoveTo_Start_End_Time();
			pMoveTo->SetGameObject(pEagleTocommand);
			pMoveTo->setName("Move to");
			sPair From;		From.numData = glm::vec4(-73.4, 238.1, 527.0, 1.0f);
			sPair To;		To.numData = glm::vec4(294.6, 205.8, 378.8, 1.0f);
			sPair Speed;	Speed.numData.x = 10.0f;		// 1 unit per second
			std::vector<sPair> vecParams;
			vecParams.push_back(From);
			vecParams.push_back(To);
			vecParams.push_back(Speed);
			pMoveTo->Init(vecParams);
	//		pScene->AddCommandSerial(pMoveTo);
			vParallelComd.push_back(pMoveTo);
			//pScene->AddCommandsParallel(vParallelComd);
		}
		{
			iCommand* rotate = new cRotateRelativeOverTime();
			rotate->SetGameObject(pEagleTocommand);
			rotate->setName("Rotate to");
			sPair EndOrientation;		EndOrientation.numData.x = 180.0f;
			EndOrientation.numData.y = 0.0f;
			EndOrientation.numData.z = 180.0f;
			sPair Time;			Time.numData.x = 3.0f;
			std::vector<sPair> vecParams;
			vecParams.push_back(EndOrientation);
			vecParams.push_back(Time);
			rotate->Init(vecParams);
	//		pScene->AddCommandSerial(rotate);
			vParallelComd.push_back(rotate);
			//pScene->AddCommandsParallel(vParallelComd);
		}
		{
			iCommand* pMoveTo = new cMoveTo_Start_End_Time();
			pMoveTo->SetGameObject(pEagleTocommand);
			pMoveTo->setName("Move to");
			sPair From;		From.numData = glm::vec4(294.6, 205.8, 378.8, 1.0f);
			sPair To;		To.numData = glm::vec4(404.2, 330.0, -73.0, 1.0f);
			sPair Speed;	Speed.numData.x = 10.0f;		// 1 unit per second
			std::vector<sPair> vecParams;
			vecParams.push_back(From);
			vecParams.push_back(To);
			vecParams.push_back(Speed);
			pMoveTo->Init(vecParams);
		//	pScene->AddCommandSerial(pMoveTo);
			vParallelComd.push_back(pMoveTo);
			//pScene->AddCommandsParallel(vParallelComd);
		}
		{
			iCommand* rotate = new cRotateRelativeOverTime();
			rotate->SetGameObject(pEagleTocommand);
			rotate->setName("Rotate to");
			sPair EndOrientation;		EndOrientation.numData.x = 180.0f;
			EndOrientation.numData.y = 270.0f;
			EndOrientation.numData.z = 180.0f;
			sPair Time;			Time.numData.x = 3.0f;
			std::vector<sPair> vecParams;
			vecParams.push_back(EndOrientation);
			vecParams.push_back(Time);
			rotate->Init(vecParams);
		//	pScene->AddCommandSerial(rotate);
			vParallelComd.push_back(rotate);
			//pScene->AddCommandsParallel(vParallelComd);
		}
		{
			iCommand* pMoveTo = new cMoveTo_Start_End_Time();
			pMoveTo->SetGameObject(pEagleTocommand);
			pMoveTo->setName("Move to");
			sPair From;		From.numData = glm::vec4(404.2, 330.0, -73.0, 1.0f);
			sPair To;		To.numData = glm::vec4(12.5, 34.7, -19.6, 1.0f);
			sPair Speed;	Speed.numData.x = 10.0f;		// 1 unit per second
			std::vector<sPair> vecParams;
			vecParams.push_back(From);
			vecParams.push_back(To);
			vecParams.push_back(Speed);
			pMoveTo->Init(vecParams);
		//	pScene->AddCommandSerial(pMoveTo);
			vParallelComd.push_back(pMoveTo);
			//pScene->AddCommandsParallel(vParallelComd);
		}
		{
			iCommand* rotate = new cRotateRelativeOverTime();
			rotate->SetGameObject(pEagleTocommand);
			rotate->setName("Rotate to");
			sPair EndOrientation;		EndOrientation.numData.x = 180.0f;
			EndOrientation.numData.y = 0.0f;
			EndOrientation.numData.z = 180.0f;
			sPair Time;			Time.numData.x = 3.0f;
			std::vector<sPair> vecParams;
			vecParams.push_back(EndOrientation);
			vecParams.push_back(Time);
			rotate->Init(vecParams);
//			pScene->AddCommandSerial(rotate);
			vParallelComd.push_back(rotate);
			pScene->AddCommandsParallel(vParallelComd);

		}

	}

	//delete pCommand;

	//lua_pushboolean(L, true);
	return 0;
}

int updateCommands(lua_State* L)
{
	float deltatime = lua_tointeger(L, 1);
	pScene->Update(0.03);

	return 0;
}

int KillAllHumans(lua_State *L)
{
	// There are 3 things on the stack: "Justin", 47, 3.14159 
	const char* name = lua_tostring(L, 1);	// get "Justin"		
	int age = lua_tonumber(L, 2);			// "Cat"
	float pi = lua_tonumber(L, 3);			// 3.14159

	std::cout << "KillAllHumans(): "
		<< name << ", " << age << ", " << pi << std::endl;
	return 0;
}

cLuaBrain::cLuaBrain()
{
	this->m_p_vecGOs = nullptr;

	// Create new Lua state.
	// NOTE: this is common to ALL script in this case
	this->m_pLuaState = luaL_newstate();

	luaL_openlibs(this->m_pLuaState);					/* Lua 5.3.3 */

//	lua_pushcfunction( this->m_pLuaState, l_KillAllHumans );
//	lua_setglobal( this->m_pLuaState, "WhatBenderTheRobotSays" );

	lua_pushcfunction( this->m_pLuaState, KillAllHumans);
	lua_setglobal( this->m_pLuaState, "JustinTrudeauIsOurPM" );

	// 	static int l_UpdateObject( lua_State *L );		// c function
	lua_pushcfunction( this->m_pLuaState, cLuaBrain::l_UpdateObject );
	lua_setglobal( this->m_pLuaState, "setObjectState" );

	lua_pushcfunction( this->m_pLuaState, cLuaBrain::l_GetObjectState );
	lua_setglobal( this->m_pLuaState, "getObjectState" );

	lua_pushcfunction(this->m_pLuaState, CreateCommand);
	lua_setglobal( this->m_pLuaState, "CreateCommand" );

	lua_pushcfunction(this->m_pLuaState, updateCommands);
	lua_setglobal(this->m_pLuaState, "updateCommands");

	return;
}

cLuaBrain::~cLuaBrain()
{
	lua_close(this->m_pLuaState);
	return;
}


// Saves (and overwrites) any script
// scriptName is just so we can delete them later
void cLuaBrain::LoadScript( std::string scriptName, 
					        std::string scriptSource )
{
	this->m_mapScripts[scriptName] = scriptSource;
	return;
}


void cLuaBrain::DeleteScript( std::string scriptName )
{
	// TODO: delete this scritp
	return;
}

// Passes a pointer to the game object vector
void cLuaBrain::SetObjectVector( std::vector< iObject* >* p_vecGOs )
{
	this->m_p_vecGOs = p_vecGOs;
	return;
}

void cLuaBrain::RunThis(std::string theLuaScript)
{
	int error = luaL_loadstring( this->m_pLuaState,
								theLuaScript.c_str() );

	if (error != 0 /*no error*/)
	{
		std::cout << "-------------------------------------------------------" << std::endl;
		std::cout << "Error running Lua script: ";
		std::cout << this->m_decodeLuaErrorToString(error) << std::endl;
		std::cout << "-------------------------------------------------------" << std::endl;
	}

	// execute funtion in "protected mode", where problems are 
	//  caught and placed on the stack for investigation
	error = lua_pcall(this->m_pLuaState,	/* lua state */
					  0,	/* nargs: number of arguments pushed onto the lua stack */
					  0,	/* nresults: number of results that should be on stack at end*/
					  0);	/* errfunc: location, in stack, of error function.
							if 0, results are on top of stack. */
	if (error != 0 /*no error*/)
	{
		std::cout << "Lua: There was an error..." << std::endl;
		std::cout << this->m_decodeLuaErrorToString(error) << std::endl;

		std::string luaError;
		// Get error information from top of stack (-1 is top)
		luaError.append(lua_tostring(this->m_pLuaState, -1));

		// Make error message a little more clear
		std::cout << "-------------------------------------------------------" << std::endl;
		std::cout << "Error running Lua script: ";
		std::cout << luaError << std::endl;
		std::cout << "-------------------------------------------------------" << std::endl;
		// We passed zero (0) as errfunc, so error is on stack)
		lua_pop(this->m_pLuaState, 1);  /* pop error message from the stack */

	}

	return;
}




// Call all the active scripts that are loaded
void cLuaBrain::Update(float deltaTime)
{
//	std::cout << "cLuaBrain::Update() called" << std::endl;
	for( std::map< std::string /*name*/, std::string /*source*/>::iterator itScript = 
		 this->m_mapScripts.begin(); itScript != this->m_mapScripts.end(); itScript++ )
	{

		// Pass the script into Lua and update
//		int error = luaL_loadbuffer(L, buffer, strlen(buffer), "line");

		std::string curLuaScript = itScript->second;

		int error = luaL_loadstring( this->m_pLuaState, 
									 curLuaScript.c_str() );

		if ( error != 0 /*no error*/)	
		{
			std::cout << "-------------------------------------------------------" << std::endl;
			std::cout << "Error running Lua script: ";
			std::cout << itScript->first << std::endl;
			std::cout << this->m_decodeLuaErrorToString(error) << std::endl;
			std::cout << "-------------------------------------------------------" << std::endl;
			continue;
		}

		// execute funtion in "protected mode", where problems are 
		//  caught and placed on the stack for investigation
		error = lua_pcall(this->m_pLuaState,	/* lua state */
						  0,	/* nargs: number of arguments pushed onto the lua stack */
						  0,	/* nresults: number of results that should be on stack at end*/
						  0);	/* errfunc: location, in stack, of error function. 
								   if 0, results are on top of stack. */
		if ( error != 0 /*no error*/)	
		{
			std::cout << "Lua: There was an error..." << std::endl;
			std::cout << this->m_decodeLuaErrorToString(error) << std::endl;

			std::string luaError;
			// Get error information from top of stack (-1 is top)
			luaError.append( lua_tostring(this->m_pLuaState, -1) );

			// Make error message a little more clear
			std::cout << "-------------------------------------------------------" << std::endl;
			std::cout << "Error running Lua script: ";
			std::cout << itScript->first << std::endl;
			std::cout << luaError << std::endl;
			std::cout << "-------------------------------------------------------" << std::endl;
			// We passed zero (0) as errfunc, so error is on stack)
			lua_pop(this->m_pLuaState, 1);  /* pop error message from the stack */

			continue;
		}

	}

	// TODO: Lots of Lua stuff here...
	return;
}

// Called by Lua
// Passes object ID, new velocity, etc.
// Returns valid (true or false)
// Passes: 
// - position (xyz)
// - velocity (xyz)
// called "setObjectState" in lua
/*static*/ int cLuaBrain::l_UpdateObject( lua_State *L )
{
	int objectID = lua_tonumber(L, 1);	/* get argument */
	
	// Exist? 
	iObject* pGO = cLuaBrain::m_findObjectByID(objectID);

	if ( pGO == nullptr )
	{	// No, it's invalid
		lua_pushboolean( L,  false );
		// I pushed 1 thing on stack, so return 1;
		return 1;	
	}

	// Object ID is valid
	// Get the values that lua pushed and update object
	//pGO->x = lua_tonumber(L, 2);	/* get argument */
	//pGO->y = lua_tonumber(L, 3);	/* get argument */
	//pGO->z = lua_tonumber(L, 4);	/* get argument */
	//pGO->Vx = lua_tonumber(L, 5);	/* get argument */
	//pGO->Vy = lua_tonumber(L, 6);	/* get argument */
	//pGO->Vz = lua_tonumber(L, 7);	/* get argument */

	lua_pushboolean( L, true );	// index is OK
	
	return 1;		// I'm returning ONE thing

}

// Passes object ID
// Returns valid (true or false)
// - position (xyz)
// - velocity (xyz)
// called "getObjectState" in lua
/*static*/ int cLuaBrain::l_GetObjectState( lua_State *L )
{
	int objectID = lua_tonumber(L, 1);	/* get argument */
	
	// Exist? 
	iObject* pGO = cLuaBrain::m_findObjectByID(objectID);

	if ( pGO == nullptr )
	{	// No, it's invalid
		lua_pushboolean( L,  false );
		// I pushed 1 thing on stack, so return 1;
		return 1;	
	}

	// Object ID is valid
	//lua_pushboolean( L, true );	// index is OK
	//lua_pushnumber( L, pGO->x );		
	//lua_pushnumber( L, pGO->y );		
	//lua_pushnumber( L, pGO->z );		
	//lua_pushnumber( L, pGO->Vx );		
	//lua_pushnumber( L, pGO->Vy );		
	//lua_pushnumber( L, pGO->Vz );		
	
	return 7;		// There were 7 things on the stack
}

/*static*/ 
std::vector< iObject* >* cLuaBrain::m_p_vecGOs;


// returns nullptr if not found
/*static*/ iObject* cLuaBrain::m_findObjectByID( int ID )
{
	for ( std::vector<iObject*>::iterator itGO = cLuaBrain::m_p_vecGOs->begin();
		  itGO != cLuaBrain::m_p_vecGOs->end(); itGO++ )
	{
		if ( (*itGO)->getUniqueID() == ID )
		{	// Found it!
			return (*itGO);
		}
	}//for ( std::vector<iObject*>::iterator itGO...
	// Didn't find it
	return nullptr;
}


std::string cLuaBrain::m_decodeLuaErrorToString( int error )
{
	switch ( error )
	{
	case 0:
		return "Lua: no error";
		break;
	case LUA_ERRSYNTAX:
		return "Lua: syntax error"; 
		break;
	case LUA_ERRMEM:
		return "Lua: memory allocation error";
		break;
	case LUA_ERRRUN:
		return "Lua: Runtime error";
		break;
	case LUA_ERRERR:
		return "Lua: Error while running the error handler function";
		break;
	}//switch ( error )

	// Who knows what this error is?
	return "Lua: UNKNOWN error";
}

iObject* pFindObjectByFriendlyNameLUA(std::string name)
{
	// Do a linear search 
	for (unsigned int index = 0;
		index != ::g_vec_pGameObjects.size(); index++)
	{
		if (::g_vec_pGameObjects[index]->getFriendlyName() == name)
		{
			// Found it!!
			return ::g_vec_pGameObjects[index];
		}
	}
	// Didn't find it
	return NULL;
}
