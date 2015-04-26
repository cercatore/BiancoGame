// ===============================================================================
//						  AGEIA PHYSX SDK TRAINING PROGRAMS
//							  LESSON 601: HEIGHTFIELDS
//
//						    Written by Bob Schade, 5-1-06
// ===============================================================================

#include <GL/glut.h>
#include <stdio.h>
#define NOMINMAX
#include "windows.h"
#include "NxPhysics.h"
#include "CommonCode2.h"
#include "Lesson601.h"
#include "Model_3DS.h"
#include "NXU_Streaming.h"
#include "iostream"
#include "socket.cpp"
//my vars
const float S_TRAC_LEN = 2.9f;

const float S_TRAC_WI = 1.4f;
const float S_SLOPE_L = 7.f;
const float S_SLOPE_H = 5;
const float S_SLOPED_H = -6.5f;
const float S_SLOPEUM_H = 0;
const float S_SLOPEUM_L = +9;
const float S_SLOPELM_L = 13;
const float S_SLOPELM_H = -6;
FILE * file = NULL;
#define CURVE_L 1
#define CURVE_R -1
#define DRITTO 0 
#define SLOPE_D 2
#define SLOPE_U 3
#define CROSS 4 
#define CROSS_I 5 
#define SLOPE_UM 6
#define SLOPE_DM 7
#define ELYX_AR 8
#define TURN_DL 9
using namespace std;
using namespace NXU;
// Physics SDK globals
extern NxPhysicsSDK*     gPhysicsSDK;
extern NxScene*          gScene;
extern NxVec3            gDefaultGravity;
NxVec3 zero = NxVec3(0,0,0);
// User report globals
extern DebugRenderer     gDebugRenderer;
extern UserAllocator*	 gAllocator;

// HUD globals
extern HUD hud;

// Camera globals
extern NxVec3 gCameraPos;
extern NxReal gCameraSpeed;
extern float gCameraAspectRatio;
int trackMode = 1;
bool disconnected = 0;
bool createGo;

// Force globals
extern NxVec3 gForceVec;
extern NxReal gForceStrength;
extern bool bForceMode;
NxActorGroup playerGroup;
NxActorGroup trackGroup;

// Simulation globals
extern bool bHardwareScene;
extern bool bPause;
extern bool bShadows;
extern bool bDebugWireframeMode;

// Actor globals
extern NxActor* groundPlane;
extern NxActor* box;
extern NxActor* sphere;
extern NxActor* capsule;
extern NxActor* pyramid;
extern NxActor* concaveObject;
extern NxActor* heightfield;
NxActor* terrainMesh;
float sphRag = 5.f;
float gScale = 10.f;
// Focus actor
extern NxActor* gSelectedActor;
FILE * fileLog = fopen("C:\\logRoll.txt", "w+" );
	

// Mesh globals
extern NxTriangleMeshDesc heightfieldMeshDesc;
#include "contactreport.h"
class MyContactReport : public NxUserContactReport
   {
      public:
         virtual void onContactNotify(NxContactPair& pair, NxU32 events)
         {
            ContactReport     contactReport;
//            NxAgeiaPhysicsHelper* ActorOne = NULL;
//            NxAgeiaPhysicsHelper* ActorTwo = NULL;
			//cout << "contact " << pair.actors[0];
			
		 }
};

void PrintControls()
{
	printf("\n Flight Controls:\n ----------------\n w = forward, s = back\n a = strafe left, d = strafe right\n q = up, z = down\n");
    printf("\n Force Controls:\n ---------------\n i = +z, k = -z\n j = +x, l = -x\n u = +y, m = -y\n");
	printf("\n Miscellaneous:\n --------------\n p = Pause\n r = Select Next Actor\n f = Toggle Force Mode\n b = Toggle Debug Wireframe Mode\n");

	// draw date
	// get date and format 
	SYSTEMTIME st; 
    GetSystemTime(&st);
	cout << "time: " << st.wDay << " " << st.wMonth << " " << st.wYear << "\n" ;

}

void AddTerrainLight()
{
	float AmbientColor1[]    = { 0.0f, 0.1f, 0.2f, 0.0f };         glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientColor1);
    float DiffuseColor1[]    = { 0.9f, 0.9f, 0.9f, 0.0f };         glLightfv(GL_LIGHT1, GL_DIFFUSE, DiffuseColor1);
    float SpecularColor1[]   = { 0.9f, 0.9f, 0.9f, 0.0f };         glLightfv(GL_LIGHT1, GL_SPECULAR, SpecularColor1);
    float Position1[]        = { -25.0f, 55.0f, 15.0f, 1.0f };	   glLightfv(GL_LIGHT1, GL_POSITION, Position1);
}
// era AddShapeToActor , i dont know
void sacciCazz(){
	int nbActors=gScene->getNbActors();
	NxActor ** actors = gScene->getActors();
	for (NxU32 i = 0; i < nbActors; i++)
	{
		NxActor* actor = actors[i];

		// Release convex shapes scheduled for deletion
		NxU32 nbShapes = actor->getNbShapes();
		NxShape*const* shape = actor->getShapes();
		for (NxU32 j = 0; j < nbShapes; j++)
		{
			if (shape[j]->userData == (void*)-1)
			{
				float * norms = (float*) shape[j]->userData; // in cpp
				printf("%%%%%%%%%%%%%%%%%555 found shape with user data %d" , sizeof(norms) / sizeof(float));
				actor->releaseShape(*shape[j]);
				shape[j]->userData = 0;
			}
		}

		if (actor->isDynamic())  
			actor->updateMassFromShapes(0, actor->getMass());
	}
}

void RenderActors(bool shadows)
{
    // Render all the actors in the scene
    NxU32 nbActors = gScene->getNbActors();
    NxActor** actors = gScene->getActors();
    while (nbActors--)
    {
        NxActor* actor = *actors++;

        if (((ActorUserData*)(actor->userData))->flags & UD_NO_RENDER)  continue;

        DrawActor(actor, gSelectedActor, true);

        // Handle shadows
        if (shadows)
        {
			DrawActorShadow(actor, true);
        }
    }
}

void ProcessInputs()
{
    ProcessForceKeys();

    // Show debug wireframes
	if (bDebugWireframeMode)
	{
		if (gScene)  gDebugRenderer.renderData(*gScene->getDebugRenderable());
	}
}
float gameover_y = -250.0f;
void Controller_go(NxActor * pla){
	if (pla->getGlobalPosition().y <= gameover_y)
	{	
		printf("gameover");
		pla->setLinearVelocity( zero );
		StartPhysics();
		//GetPhysicsResults();
		Sleep(100);
		
		pla->setGlobalPosition(NxVec3(0,30.f,0));
		
		
	//ResetNx();
	}
}
void gluCLook(NxVec3 eye,NxVec3 look){
	gluLookAt(eye.x,eye.y,eye.z,look.x,look.y,look.z,0.f,1.f,0.f);
	
}
NxVec3 oblioSphere(NxActor *sphere){
	NxMat33 posa = sphere->getCMassGlobalPose().M;
	NxVec3 colum;
	posa.getColumn(1,colum);
	return sphere->getGlobalPosition() + sphRag * colum;
	}

void followCam(){
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, gCameraAspectRatio, 1.0f, 10000.0f);
	NxVec3 tochange = sphere->getCMassGlobalPose().t;
	NxVec3 speed = sphere->getLinearVelocity();
	NxVec3 spherePos = oblioSphere(sphere);
	tochange.y = spherePos.y + 79.f;
	tochange.z = spherePos.z - 65.f;
	if (!speed.magnitude() <0.2f) {
		tochange.x -= speed.x * 5 ; 
	}
	else tochange.x -= 10;
	
	gluCLook( tochange , spherePos );
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

#include "CraftBorg.cpp"
GameSingleRecord *gsr;
extern int packetSen;
void RenderCallback()
{
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ProcessCameraKeys();
	if (!trackMode) SetupCamera();
	else followCam();

    if (gScene && !bPause)
	{
		GetPhysicsResults();
        ProcessInputs();
		StartPhysics();
	}

	Controller_go(sphere);
	// NETWORKING OUTPUT, non riesco a trovare una migliore sistemazione, cosi
	//
	// for ( players )
	//    game = gamerecords[i]
	//    if game.client != 0 && status = ready
	//    spheres[game.id].getGlobalPos
	//    client.sendLine(formatLine(xxx))
	if ( gsr != NULL ){
		NxVec3 pos = oblioSphere(sphere);
		std::string out;
		char from [50] = "";
		sprintf(from,"%.3f,%.3f,%.3f", pos.x,pos.y,pos.z);
		out.append(from);
		gsr->l->SendLine(out);
		packetSen++;
	}
	
	
    // Display scene
 	RenderActors(bShadows);

	if (bForceMode)
		DrawForce(gSelectedActor, gForceVec, 20 * gScale * NxVec3(1,1,0));
	else
		DrawForce(gSelectedActor, gForceVec, 20 * gScale * NxVec3(0,1,1));
	gForceVec = NxVec3(0,0,0);

	// Render HUD
	hud.Render();

    glFlush();
    glutSwapBuffers();
}


void SpecialKeys(unsigned char key, int x, int y)
{
}
NxActor * setUpTriMesh();
void InitNx()
{
	// Create a memory allocator
    gAllocator = new UserAllocator;

    // Create the physics SDK
    gPhysicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, gAllocator);
    if (!gPhysicsSDK)  return;

	// Set the physics parameters
	gPhysicsSDK->setParameter(NX_SKIN_WIDTH, 0.01);

	// Set the debug visualization parameters
	gPhysicsSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	gPhysicsSDK->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, 1);

    // Create the scene
    NxSceneDesc sceneDesc;
    sceneDesc.gravity               = gDefaultGravity;
	sceneDesc.simType				= NX_SIMULATION_HW;
    gScene = gPhysicsSDK->createScene(sceneDesc);

	// If hardware is unavailable, fall back on software
	if (!gScene)
	{
        sceneDesc.simType				= NX_SIMULATION_SW;
        gScene = gPhysicsSDK->createScene(sceneDesc);
	}

	// Create the default material
	NxMaterial* defaultMaterial = gScene->getMaterialFromIndex(0); 
	defaultMaterial->setRestitution(0.2);
	defaultMaterial->setStaticFriction(0.3);
	defaultMaterial->setDynamicFriction(2.0);

	// Create the objects in the scene
	//groundPlane = CreateGroundPlane();
	//groundPlane->setGlobalPosition(NxVec3(0,-250,0));
	NxVec3 A = NxVec3(1,0,0);
	NxVec3 B = NxVec3(0,0,1);
	NxVec3 C;

	C = A.cross(B);
	
	//heightfield = CreateHeightfield(NxVec3(0,0,0), 0, 200);

	gCameraPos = NxVec3(-55,55,615);
	NxVec3 iniLook = NxVec3(-25,0,615);
	gluCLook(gCameraPos,iniLook);
	gCameraSpeed = 150;

	box = CreateBox(gCameraPos + NxVec3(5,0,10), NxVec3(0.5,1,0.5), 20);
	sphere = CreateSphere(gCameraPos + NxVec3(0,0,15), sphRag, 0.2);
	capsule = CreateCapsule(gCameraPos + NxVec3(-5,0,10), 2, 0.5, 10);
	pyramid = CreatePyramid(gCameraPos + NxVec3(0,0,10), NxVec3(1,0.5,1.5), 10);
	concaveObject = CreateConcaveObject(gCameraPos + NxVec3(-4,0,15), NxVec3(1,1,1), 5);
	sphere->setGroup(playerGroup);
	//AddUserDataToActors(gScene);

	// We want to attach the raw mesh to the heightfield shape
	// user data rather than the cooked one.  Cooking the mesh
	// changes the face indices and we want the original mesh
	// attached with the original ordering intact to get the 
	// correct triangles from the raycast queries.
	/*NxShape** shapes = (NxShape**)(heightfield->getShapes());
    ShapeUserData* sud = (ShapeUserData*)(shapes[0]->userData);
	if (sud && sud->mesh)
	{
		delete sud->mesh;
		sud->mesh = &heightfieldMeshDesc;
	}

	// Set the heightfield to use light #1 for rendering
	((ActorUserData*)(heightfield->userData))->flags |= UD_RENDER_USING_LIGHT1;
*/
	// Page in the hardware meshes
	PageInHardwareMeshes(gScene);

	gSelectedActor = sphere;
	gForceStrength = 100000;

	// Initialize HUD
	InitializeHUD();

	// Get the current time
	UpdateTime();

	
	// Start the first frame of the simulation
	
}
float currAngle = 0;
NxVec3 global_pos = NxVec3(0,0,0);
NxVec3 dest;
int packetRec,packetSen,pSen,pRec,diffRec,diffSen,wait=0;
float torqueTop,torqueLeft;
bool jump=false,gameok=false,isJumping = false;
void HudRefresh(char * line){

	hud.Clear();
	    hud.AddDisplayString(line, 0.f, 0.84f);
}

unsigned long CALLBACK tickCounter( void * lpt){
    char str[50] = "";
	while (1){
		diffRec = packetRec - pRec;
		diffSen = packetSen - pSen;
		sprintf(str, "stats: BW #IN %d  - #OU %d--- top:%f, left%f, jump %d" , diffRec, diffSen, torqueTop,torqueLeft, isJumping);
		HudRefresh(str);
        pRec = packetRec;
		pSen = packetSen;
		if (wait-- >0 ) {
			//ApplyForceToActor(sphere,NxVec3(0,1.f,0),gForceStrength * 8 , 1); 
			NxVec3 veloc = sphere->getLinearVelocity();
			sphere->setLinearVelocity(NxVec3(veloc.x,20.f,veloc.z));
			}
		Sleep(100);
		if (jump) { wait = 1;jump = 0;isJumping=true;}
		if (wait==0){ isJumping =false;}


	}
}
void Jump(){
	NxMat33 roll2;
	printf( "JUMP alfa:   %f , global x %f global.z %f\n"  , currAngle,global_pos.x,global_pos.z);
	roll2.rotY(currAngle);
	global_pos += roll2.getColumn(2) * 3.f * 20.f * S_TRAC_LEN;
}	
bool tit = false;
NxActor * setUpTriMesh(char * ss, int tipo_trac){
	float gScale = 20.0f;
	// createBox , centered in the center of pipe , dimension w5 h 3 z4
	//boxa = CreateConvexObjectComputeHull(NxVec3(-10,0,0),NxVec3(4,-2,4.10), 1.f);
	//boxa->raiseBodyFlag(NX_BF_FROZEN_POS);
	//AttachModelToActor(boxa,"pipeh.3ds",0,1.f);
	NxTriangleMeshShapeDesc terrainShapeDesc;
	Model_3DS* model = new Model_3DS;
	//printf( "flag value%s\n" ,1);
	assert(ss != NULL );//(assert(!fopen(ss,"r"));
	model->Load(ss);
	// carico le norm
	if (model->Objects == NULL ) 
		MessageBoxA(0, "warning, multi obj", "", MB_OK);
	float * normals = model->Objects[0].Normals;
	//for (int i = 0; i< model->numObjects ; i++);
	//	cout << "Mesh3ds origin: (" << model->pos.y << "sizes buff " << sizeof((float*)model->Objects[0].Vertexes) << ",ex " << model->Objects[0].numFaces << " numVert " << model->Objects[0].numVerts <<"\n";
// EDIT MESH PROPERTIES
		NxVec3 * vertexBuf = new NxVec3[model->totalVerts];
		float invertz = 1.f;
		if (tipo_trac != 0 ) invertz = -1;
		float Z_THRESHOLD = .5f;
		float localMax = -1000.f;
		for (int i = 0; i < model->totalVerts; i++){
			float tmp = model->Objects[0].Vertexes[3*i +1];
			vertexBuf[i] = gScale*  NxVec3(model->Objects[0].Vertexes[3*i ], tmp ,model->Objects[0].Vertexes[3*i +2] * invertz);
			if ( tmp > localMax ) localMax = tmp;
			if (tipo_trac == SLOPE_D && abs(vertexBuf[i].z) < Z_THRESHOLD )
				printf( "found vertex in range z=0: %d, x:(%2.3f, %2.3f, %2.3f)\n" , i, vertexBuf[i].x,vertexBuf[i].y,vertexBuf[i].z); 
			if (tipo_trac == CROSS || tipo_trac == CROSS_I)
				vertexBuf[i].x -= 0.1f * gScale;
		}
		NxU32 *indicesBuf = new NxU32[model->totalFaces * 3];
	for (int i = 0; i < model->totalFaces * 3 ; i++)
			indicesBuf[i] = model->Objects[0].Faces[i];

	NxActorDesc triActorDesc;
	NxTriangleMeshDesc triDesc;
	triDesc.numVertices         = model->totalVerts ;
	triDesc.numTriangles        = model->totalFaces ;
	triDesc.pointStrideBytes      = sizeof(NxVec3);
	triDesc.triangleStrideBytes     = 3 * sizeof(NxU32);
	triDesc.points            = vertexBuf;
	triDesc.triangles         = indicesBuf;
	
	if (tipo_trac == 0 ) triDesc.flags = NX_MF_HARDWARE_MESH;
	else triDesc.flags = NX_MF_FLIPNORMALS;

	
	bool hwAvailable = (gPhysicsSDK->getHWVersion() != NX_HW_VERSION_NONE);
	if(!hwAvailable)
	{
		// set the flag to make it as height field
		triDesc.heightFieldVerticalAxis		= NX_Y;//TODO DELETE
	}
	triDesc.heightFieldVerticalExtent	= localMax * gScale;
	terrainShapeDesc.density= 1.f;
	
	//terrainShapeDesc.materialIndex = 0;
	terrainShapeDesc.localPose.t = NxVec3(0,10,0);
	// Cooking from memory
	/*NXU::MemoryWriteBuffer buf;
	buf.data = new NxU8[1024];
  
	NxInitCooking();
	bool status = NxCookTriangleMesh(triDesc, buf);
	MemoryReadBuffer readBuffer(buf.data);
	terrainShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(readBuffer);*/
	NxInitCooking();
	bool status = NxCookTriangleMesh(triDesc, NXU::UserStream("c:\\tmp.bin", false));
	terrainShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(NXU::UserStream("c:\\tmp.bin", true));
	if (!tit) terrainShapeDesc.userData= (void*)0;   //TODO DELETE
	else  terrainShapeDesc.userData = (void*) normals;
	//terrainShapeDesc.userData = (void *) NULL;
	//cout << " t Desc isValid " << triDesc.isValid() << " valid, " << terrainShapeDesc.isValid() << " flags " << terrainShapeDesc.meshFlags << "\n";
	
	//NxTriangleMesh* zumm = terrainShapeDesc.meshData;
	

	NxActorDesc ActorDesc;

    ActorDesc.shapes.pushBack(&terrainShapeDesc);
	NxActor * gTerrain = gScene->createActor(ActorDesc);
	gTerrain->raiseBodyFlag(NX_BF_FROZEN_POS);
	NxMat34 trasl = NxMat34();
	NxMat33 roll2;
	printf( "curr alfa:   %2.2f , global.x %2.2f global.z %2.2f::: max %3.3f\n"  , currAngle,global_pos.x,global_pos.z, localMax);
	fprintf(fileLog,  "curr alfa:   %-2.2f, global.x %-2.2f, global.z %-2.2f\n"  , currAngle,global_pos.x,global_pos.z, localMax);
	
	roll2.rotY(currAngle);
	trasl.t = global_pos ;
	trasl.M = roll2;
	gTerrain->setGlobalPose(trasl);
	
	NxVec3 disp = NxVec3(0,0,0);
	switch( tipo_trac ) {
		case ELYX_AR:
			disp = NxVec3( -(4) * gScale , (5 + 0.3 ) * gScale, ( 4 + S_TRAC_WI)*gScale);
			roll2.multiply( disp, dest);
			global_pos += dest;
			currAngle -= NxPi*0.5f;break;
		case TURN_DL:
			disp = NxVec3( +(3) * gScale , (-6 ) * gScale, ( 4 )*gScale);
			roll2.multiply( disp, dest);
			global_pos += dest;
			currAngle += NxPi*0.5f;break;
		
		case SLOPE_DM:
			disp.z = ( S_SLOPELM_L * gScale );
			disp.y = ( S_SLOPELM_H * gScale );
			roll2.multiply(disp, dest);
			global_pos += dest;
			break;
		case SLOPE_UM:
			disp.z = ( S_SLOPEUM_L * gScale );  // caso particolare sempre prima di un LOWER
			disp.y = ( S_SLOPEUM_H * gScale );
			roll2.multiply(disp, dest);
			global_pos += dest;
			break;
		case SLOPE_U:
			disp.z = ( S_SLOPE_L * gScale );
			disp.y = ( S_SLOPE_H * gScale );
			roll2.multiply(disp, dest);
			global_pos += dest;
			break;
		case SLOPE_D:
			disp.z = ( S_SLOPE_L * gScale );
			disp.y = ( S_SLOPED_H * gScale );
			roll2.multiply(disp, dest);
			global_pos += dest;
			break;
		case CROSS:
			disp = NxVec3( -(3.55 + S_TRAC_LEN ) * gScale , -2.f , 2.5*gScale);
			roll2.multiply( disp, dest);
			global_pos += dest;
			currAngle -= NxPi*0.5f;break;
		case CROSS_I:
			disp = NxVec3( (3.45 + S_TRAC_LEN ) * gScale , -2.f , 2.5f*gScale);
			roll2.multiply( disp, dest);
			global_pos += dest;
			currAngle += NxPi*0.5f;break;
		case CURVE_L:
			disp = NxVec3( -(3 + S_TRAC_LEN ) * gScale , 0 , 3.1f*gScale);
			roll2.multiply( disp, dest);
			global_pos += dest;
			currAngle -= NxPi*0.5f;break;
		case CURVE_R:
			disp = NxVec3( (3 + S_TRAC_LEN ) * gScale , 0 ,   3.f*gScale );
			roll2.multiply( disp, dest);
			global_pos += dest;currAngle += NxPi*0.5f;
			break;
		case DRITTO:
			disp.z = ( 1.0f*  S_TRAC_LEN * gScale );
			roll2.multiply(disp, dest);
			global_pos += dest;
			break;
	}
	
	sacciCazz();
	NxCloseCooking();
	AddUserDataToActors(gScene);
	gTerrain->setGroup(trackGroup);
	delete [] vertexBuf;
	delete  [] indicesBuf;
	return gTerrain;
}
char curve2RPath[26] = ".\\models\\curve2L.3ds";
char curve2LPath[26] = ".\\models\\curve2.3ds";
char slopeUpper[26] = ".\\models\\slopeUpper.3ds";
char slopeLower[26] = ".\\models\\slopeLower.3ds";
char slopeUpper2[36] = ".\\models\\slopeUpper2More.3ds";
char slopeLower2[36] = ".\\models\\slopeLowerMore.3ds";
char elyx_ar[36] = ".\\models\\elixASC_L.3ds";
char turn_dl[36] = ".\\models\\turnDISC_L.3ds";
	void claudioRialzati(){
		setUpTriMesh(".\\models\\pippo.3ds", DRITTO );
		setUpTriMesh(".\\models\\pippo.3ds", DRITTO );
		setUpTriMesh(curve2LPath, CURVE_R );
		setUpTriMesh(".\\models\\pippo.3ds", DRITTO );
		
		setUpTriMesh(".\\models\\curve2L.3ds",CURVE_L);
		setUpTriMesh(".\\models\\pippo.3ds", DRITTO );
		setUpTriMesh(".\\models\\curve2L.3ds",CURVE_L);
		setUpTriMesh(".\\models\\pippo.3ds", DRITTO );
		setUpTriMesh(".\\models\\curve2.3ds", CURVE_R);
		for (int i =0 ; i< 3; i++) setUpTriMesh(".\\models\\pippo.3ds", DRITTO );
		// prepare jump
		global_pos.z += 20.f * S_TRAC_LEN;	
		global_pos.y -= 20.f * 0.30;
		NxVec3 savedPos = global_pos;
		setUpTriMesh(".\\models\\tcrosslEAN.3ds",  CROSS);
		sphere->setGlobalPosition(global_pos);
		global_pos.y += 20.f * .24;
		setUpTriMesh(".\\models\\pippo.3ds" , DRITTO);
		Jump();
		setUpTriMesh(".\\models\\pippo.3ds" , DRITTO);
		setUpTriMesh( elyx_ar, ELYX_AR) ;
		setUpTriMesh(".\\models\\pippo.3ds" , DRITTO);
		setUpTriMesh(slopeUpper, SLOPE_U);
		setUpTriMesh(slopeLower, SLOPE_D);
		setUpTriMesh(".\\models\\pippo.3ds" , DRITTO);
		//setUpTriMesh(turn_dl, TURN_DL);
		setUpTriMesh(slopeUpper2, SLOPE_UM);
		setUpTriMesh(slopeLower2, SLOPE_DM);
		for (int i = 0 ; i < 5 ; i++ ) setUpTriMesh(".\\models\\pippo.3ds" , DRITTO);
		Jump();
		setUpTriMesh(".\\models\\pippo.3ds" , DRITTO);
		global_pos.z += 20.f * S_TRAC_LEN;	// errore se poroc prima sono a destra/sx
		global_pos.y -= 20.f * 0.30;
		NxVec3 savedPos2 = global_pos;
		setUpTriMesh(".\\models\\tcrosslEAN.3ds",  CROSS_I);
		global_pos.y += 20.f * .24;
		tit = false;
		setUpTriMesh(".\\models\\pippo.3ds" , DRITTO);
		
		
		
		
		
		gScene->setActorGroupPairFlags(playerGroup, trackGroup, NX_NOTIFY_ALL);
	}
unsigned long CALLBACK grogWorker(void * lpt);
unsigned long CALLBACK grogger(void * lpt)
{
try {
		SocketServer s(5555,0);
		//s.SendLine("GET / HTTP/1.0");
		//s.SendLine("Host: www.google.com");
		//s.SendLine("");
		int a;
		while ( 1 ) {
		Socket* l = s.Accept();
		char ip[60] ="";
		sockaddr saddr;
		int  ftemp = sizeof(struct sockaddr);
		getpeername(l->s_, &saddr ,  &ftemp);
		
		std::string strInput;
		if (!l) exit(-4);
		in_addr fucker;
		sprintf(ip, "%d %d -- %d.%d.%d.%d" ,(unsigned char)saddr.sa_data[0],(unsigned char)saddr.sa_data[1],(unsigned char)saddr.sa_data[2],(unsigned char)saddr.sa_data[3],(unsigned char)saddr.sa_data[4],(unsigned char)saddr.sa_data[5],(unsigned char)saddr.sa_data[6]);
		printf("%s" , ip);
		cout << " goooooooo" << endl;
			char returnvalue[10] ="";
			while (l) {
			strInput.append(l->ReceiveLine());
			if (strInput.empty()) break;

			//if (strInput.find("terminate")!=std::string::npos)	// terminate proc
			//	KILL_PROC_BY_NAME(szName);
			if (strInput.find("bind")!=std::string::npos)  // TEST >BIND PORT
				;//BindPort(80,s);
			if (strInput.find("go")!=std::string::npos)	;// EXIT
			if (strInput.find("create")!=std::string::npos)	{// CREATE OBJ ACTOR
					gsr = new GameSingleRecord(1, "192.168.137.117" , 0 , l);
					l->SendLine("ok");createGo = true;
					CreateThread(0,0, &grogWorker, (LPVOID) l,0,0);
					break;
				}
			strInput.clear();
			}
		}
		} 
  catch (char * s) {
    cout << s << endl;
  } 
  catch (std::string s) {
    cout << s << endl;
  } 
  catch (...) {
    cout << "unhandled exception\n";
  }
  system("pause");
  exit(-1);
}

unsigned long CALLBACK grogWorker(void * lpt) {
	float Top=0,Left=0;
	try{
		Socket *client = (Socket*) lpt;
		std::string inputLine;
		while (1 && client){
			assert (client != NULL);
			inputLine.append(client->ReceiveLine());
			if ( inputLine.empty() ) break;
			if ( inputLine.find("j")!= string::npos && !isJumping )
				jump = true;
			if (inputLine.find("reset")!= string::npos)
				sphere->setGlobalPosition(NxVec3(0,0,0));
			cout << inputLine  <<endl;
			// CONTROLS MAIN LOOP
						// parse
						if ( sscanf( inputLine.c_str(), "%f %d", &Left,&Top ) != 1)
							printf("%s", "received null");// throw error
						
			if ( Top < 0 )						
						ApplyForceToActor(sphere, NxVec3(1,0,0),gForceStrength, bForceMode );
			if ( Top > 0 )						
						ApplyForceToActor(sphere, NxVec3(-1,0,0),gForceStrength, bForceMode );
			if (Left < 0 )						
						ApplyForceToActor(sphere, NxVec3(0,0,1),gForceStrength, bForceMode );
			if (Left > 0 )						
						ApplyForceToActor(sphere, NxVec3(0,0,-1),gForceStrength, bForceMode );
			inputLine.clear();
			
		}
	}
    catch (std::string s) {
		cout << s << endl;
	} 
  catch (...) {
    cout << "unhandled exception\n";
  }
  cout << "thread exit" << endl;
  return 0;
 }

int main(int argc, char** argv)
{
	PrintControls();
	WSAData wsadata;
	WSAStartup(0x0202,&wsadata);
	
	InitGlut(argc, argv, "Lesson 601: Claudio dove sei matteo mi hai lasciato");
	AddTerrainLight();
	CreateThread(0,0, &grogger, (LPVOID) 0 , 0,0);
	CreateThread(0,0, &tickCounter, (LPVOID) 0 , 0,0);
	
	InitNx();
	claudioRialzati();
	fclose(fileLog);
    MyContactReport * mcr = new MyContactReport();
	gScene->setUserContactReport(mcr);
	if (gScene)  StartPhysics();

	glutMainLoop();

	ReleaseNx();
	return 0;
}

