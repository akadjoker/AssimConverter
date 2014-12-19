// assimteste.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

#include <windows.h>

#include <stdio.h>

#include <fstream>
#include <iostream>
#include <vector>

//to map image filenames to textureIds
#include <string.h>
#include <map>



#include "assimp\Importer.hpp"	//OO version Header!
#include "assimp\PostProcess.h"
#include "assimp\Scene.h"
#include "assimp\cimport.h"
#include "assimp\DefaultLogger.hpp"
#include "assimp\LogStream.hpp"
#include "assimp\vector3.h"
#include "assimp\matrix3x3.h"
#include "assimp\matrix4x4.h"


using namespace std;


Assimp::Importer importer;


  void  Write_Int(FILE* pFile,int i){

	fwrite(&i,1,4,pFile);

	return;

}
    void  Write_Float(FILE* pFile,float f){

	fwrite(&f,1,4,pFile);

	return;

}
	
  void  WriteString(FILE* pFile,const char* cs)
{

	fputs(cs,pFile);

}
  void  Write_String(FILE* pFile,const char* cs)
{
	string s=string(cs);
	Write_Int(pFile,s.length());
	WriteString(pFile,s.c_str());

}

   void  Close_File(FILE* f)
{
	fclose(f);
}
  
 FILE*  Write_File(const char* filename){



	FILE* pFile=fopen(filename, "wb" );

	if(pFile==NULL){
		
		printf("Error: Can't Write File %s \n",filename);
		return NULL;
	}


	return pFile;

}
 


void createAILogger()
{
	//Assimp::Logger::LogSeverity severity = Assimp::Logger::NORMAL;
	Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;

	// Create a logger instance for Console Output
	Assimp::DefaultLogger::create("",severity, aiDefaultLogStream_STDOUT);

	// Create a logger instance for File Output (found in project folder or near .exe)
	Assimp::DefaultLogger::create("assimp.txt",severity, aiDefaultLogStream_FILE);

	// Now I am ready for logging my stuff
	Assimp::DefaultLogger::get()->info("Assim 4 delphi BY Luis Santos AKA DJOKER - 01/09/2014");
}

void destroyAILogger()
{
	// Kill it after the work is done
	Assimp::DefaultLogger::kill();
}

void logInfo(std::string logString)
{
	//Will add message to File with "info" Tag
	Assimp::DefaultLogger::get()->info(logString.c_str());
}

void logDebug(const char* logString)
{
	//Will add message to File with "debug" Tag
	Assimp::DefaultLogger::get()->debug(logString);
}


	


std::string removeExtension(const std::string filename) 
{
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot); 
}

 map<string,int> m_BoneMapping; // maps a bone name to its index
 int m_NumBones;
 const aiScene* scene = NULL;

 struct Weight
 {
	int vert_bone_no; 
	float vert_weight;
};

 
 struct VertexWeight
 {
	vector<Weight> weights;
	aiVector3D vertex;
	int numbones;
 };


    struct Surface
    {  
      vector<VertexWeight> vertexWeights;
    };
        
struct AnimationKey
{
	   float time;
		aiVector3D Position;
		aiQuaternion Rotation;
};


 struct BoneData
    {  
		string name;
		string parentName;
		aiVector3D Position;
		aiQuaternion Rotation;
		int numPos;
		int numRot;
		vector<AnimationKey> keys;
		

 };

 
 vector<Surface> anim_surf;


void LoadBones(int MeshIndex, const aiMesh* pMesh)
{

	
    for (int i = 0 ; i < pMesh->mNumBones ; i++)
	{                
        int BoneIndex = 0;        
        string BoneName(pMesh->mBones[i]->mName.data);

        if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) 
		{
            BoneIndex = m_NumBones;
            m_NumBones++;            
            m_BoneMapping[BoneName] = BoneIndex;
        }
        else {
            BoneIndex = m_BoneMapping[BoneName];
        }                      
      }    

	


}

const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName)
{
    for (int i = 0 ; i < pAnimation->mNumChannels ; i++) {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];
        
        if (string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }
    
    return NULL;
}




struct aiNode *findnode(struct aiNode *node, char *name)
{
	int i;
	if (!strcmp(name, node->mName.data))
		return node;
	for (i = 0; i < node->mNumChildren; i++) {
		struct aiNode *found = findnode(node->mChildren[i], name);
		if (found)
			return found;
	}
	return NULL;
}

struct aiBone* findbone( string name)
{
	int i;
	for (int i=0;i<scene->mNumMeshes ;i++)
	{
        aiMesh* paiMesh=scene->mMeshes[i];
		
	
	     for (int x=0;x < paiMesh->mNumBones ;x++)
		 {
			 aiBone* mBone=paiMesh->mBones[x];
			 string bonename=string(mBone->mName.data);
			 if (strcmp(name.c_str(), bonename.c_str()))
	     	 return mBone;
		 }
	
	}
	return NULL;
}




	  vector<BoneData> bones;


void ReadNodeHeirarchy( const aiNode* pNode)
{    
    string NodeName(pNode->mName.data);
    string ParentName="none";


	if(pNode->mParent)
	{
		 ParentName=string(pNode->mParent->mName.data);
	} 
	BoneData bone;
	bone.name=NodeName;
	bone.parentName=ParentName;
	bone.numPos=0;
	bone.numRot=0;	
		
		

	pNode->mTransformation.DecomposeNoScaling(bone.Rotation,bone.Position);
	


    const aiAnimation* pAnimation = scene->mAnimations[0];
    const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

	
     
          
  
		 

    
    if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) 
	{
          int BoneIndex = m_BoneMapping[NodeName];
		
		      int numKeys = pNodeAnim->mNumRotationKeys;

			    bone.numPos=numKeys;
		        bone.numRot=numKeys;


           int currKeyIdx;
            aiVectorKey currPosKey;
            aiQuatKey currRotKey;

			 for (currKeyIdx = 0; currKeyIdx < numKeys; ++currKeyIdx )
            {

				currPosKey = pNodeAnim->mPositionKeys[currKeyIdx];
				currRotKey = pNodeAnim->mRotationKeys[currKeyIdx];
				float frameTime =currRotKey.mTime;

				aiVector3D kfPos;

		
				if ( currKeyIdx < pNodeAnim->mNumPositionKeys ) 
					{
						kfPos = currPosKey.mValue;
						
					}
					else {
						kfPos = bone.Position;
						
					}

					AnimationKey key;
					key.Position=kfPos;
					key.Rotation=currRotKey.mValue;
					key.time=frameTime;
				    bone.keys.push_back(key);

			 }

    }

	bones.push_back(bone);

    
    for (int i = 0 ; i < pNode->mNumChildren ; i++) 
	{
        ReadNodeHeirarchy( pNode->mChildren[i]);
    }
}

bool BoneCompare( Weight a, Weight b)
{
	return a.vert_weight > b.vert_weight;
}

int main(int argc, char* argv[])
{

	 std::cout << " "<<endl;
	 std::cout << "http://djokergames.wordpress.com \n";
	 std::cout << " @DjokerSoft 2014 \n";
	 std::cout << "\n";
	 std::cout << "Usage is  <infile> -Parameters ... \n";
	 std::cout << "\n";
	 std::cout <<" -s --Splits large meshes into smaller sub-meshes.\n";
//	 std::cout <<" -t --Calculates the tangents and bitangents.\n";
	 std::cout <<" -b --Export bones.\n";
	 std::cout <<" -x --Export mesh to xml file.\n";
	 std::cout <<" -l --Convert to left-handed geometry.\n";
	 std::cout <<" -o --A postprocessing step to reduce the number of meshes.\n";
	 std::cout <<" -pt --This step splits meshes with more than one primitive .\n";
	 std::cout << "\n";

	if (argc<=1)
	{
		 std::cout << "Not enough or invalid arguments, please try again.\n";
		return 0;
	}
	
	 int count;
	 bool exportbones=false;
	 bool exportxml=false;

	 int flags=aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_GenUVCoords | aiProcess_JoinIdenticalVertices  ;

    cout << " "<<endl;
    cout << "Command-line arguments used:\n";
    for( count = 1; count < argc; count++ )
	{
       //  cout << "  argv[" << count << "]   "
          //      << argv[count] << "\n";

		 string value=string(argv[count]);

	if (value=="-x")
	{
		  std::cout << "Save  Mesh to xml\n";
		 exportxml=true;
	}	 

	if (value=="-s")
	{
		  std::cout << "Split Large Mesh\n";
		 flags|=  aiProcess_SplitLargeMeshes;
	}
	if (value=="-t")
	{
		 std::cout << "Calculates the tangents\n";
		 flags|= aiProcess_CalcTangentSpace;
	} 
	if (value=="-pt")
	{
		 std::cout << "Splits meshes \n";
		 flags|= aiProcess_SortByPType;
	} 
	if (value=="-o")
	{
		 std::cout << "Optimization meshes \n";
		 flags|= aiProcess_OptimizeMeshes;
		 flags|=aiProcess_OptimizeGraph;
	} 	
	if (value=="-l")
	{
		 std::cout << "Convert To Left Handed mesh \n";
		 flags|= aiProcess_ConvertToLeftHanded;
		 
	} 	
	if (value=="-b")
	{
		 std::cout << "Save bones on mesh \n";
		 exportbones=true;
		 
	}
	
	

	}
std::cout << " \n"<<endl;

	char* filename=argv[1];

string savefile=	removeExtension(string(filename));
savefile+=".h3d";
string savexmlfile=	removeExtension(string(filename));
savexmlfile+=".xml";
		


	

	
	 
	 std::cout << "Process mesh"<<endl;
	 std::cout << " "<<endl;
		scene = aiImportFile(filename,flags);
		// If the import failed, report it
	if( !scene)
	{
		 std::cout << " Operation Faill ... "<<endl;
		 std::cout << " "<<endl;
		 std::cout << importer.GetErrorString()<<endl;
		 std::cout << " "<<endl;
		
		 std::cout << "     By Luis Santos AKA DJOKER ... "<<endl;
		return false;
	}

	FILE* xmlfile=0;
	if(exportxml)  xmlfile=fopen(savexmlfile.c_str(),"w");


	FILE* f=Write_File(savefile.c_str());
//primeiro escreve o material e as texturas
	Write_String(f, "H3D");
	Write_Int(f,	1478);
	Write_Int(f,	scene->mNumMaterials);
//	Write_Int(f,	scene->mNumTextures);

	if(exportxml)fprintf (xmlfile, "<Model>\n");

	 std::cout << " BEGIN WRITE MATERIALS..."<<endl;
	
	if(exportxml) 	fprintf (xmlfile, "<Materials>\n");

	for (int i=0;i<=scene->mNumMaterials-1;i++)
	{
	if(exportxml) 		fprintf (xmlfile, "<Layer>\n");


			

		Write_Int(f,	0);//flags
		  
		 // printf("count [%i] textures DIFFUSE  \n",scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE));
		//  printf("count [%i] textures LIGHTMAP  \n",scene->mMaterials[i]->GetTextureCount(aiTextureType_LIGHTMAP));
		
		   Write_Float(f,1.0f);
		   Write_Float(f,1.0f);
		   Write_Float(f,1.0f);
		   Write_Float(f,1.0f);
		  int dnum=scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE);
		 // printf("Num [%d] textures DIFFUSE  \n",dnum);
		  int lnum=scene->mMaterials[i]->GetTextureCount(aiTextureType_LIGHTMAP);
		//  printf("Num [%d] textures LIGHTMAP  \n",lnum);

if(exportxml) 	fprintf (xmlfile, "<Brush flags=\"%i\" r=\"%f\" g=\"%f\" b=\"%f\" alpha=\"%f\"/>\n",0,1,1,1,1);

	
		  if(dnum>=1)     
		  {
			   aiString path;  
			   scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE,0,&path);
			   Write_String(f, path.C_Str());
		  if(exportxml)      fprintf (xmlfile, "<Texture filename=\"%s\"/>\n",path.C_Str());
			   std::cout << "  texture " << path.C_Str()<<" DIFFUSE"<<endl;

		  }
		  	 //use lightmap
		  Write_Int(f,	lnum);

		  if(lnum>=1)
		  {

			   aiString path;  
			   scene->mMaterials[i]->GetTexture(aiTextureType_LIGHTMAP,0,&path);
			   Write_String(f, path.C_Str());
			   std::cout << "  texture  " << path.C_Str()<<" LIGHTMAP"<<endl;
			 
		  }
	

if(exportxml) fprintf (xmlfile, "</Layer>\n");

	}
    std::cout << " END WRITE MATERIALS"<<endl;
	if(exportxml) fprintf (xmlfile, "</Materials>\n");
	m_NumBones=0;

	//trabalhamos  as meches
	Write_Int(f,	scene->mNumMeshes);//numero of meshs
//	printf("\n");
	//printf("Write [%i] meshes  \n",scene->mNumMeshes);
	//printf("\n");
	

	if(exportxml) fprintf (xmlfile, "<Mesh>\n");
	for (int i=0;i<=scene->mNumMeshes-1;i++)
	{
		aiMesh* mesh=scene->mMeshes[i];
		if(mesh==NULL) break;
		if(exportbones)
		{
			LoadBones(i,mesh);
		}
	
		

		if (mesh->mName.length<=0)
		{

            string name="Surface_"+std::to_string(i);
			Write_String(f,   name.c_str());
		if(exportxml) 	fprintf (xmlfile, "<Surface name=\"%s\" ",name.c_str());	

		} else
		{
		Write_String(f, mesh->mName.C_Str());
	    if(exportxml) 	fprintf (xmlfile, "<Surface Name=\"%s\" ",mesh->mName.C_Str());	
		}

		
		


		 Write_Int(f,0);//flags
		 Write_Int(f,mesh->mMaterialIndex);
		 Write_Int(f,mesh->mNumVertices);
	     Write_Int(f,mesh->mNumFaces);
		 Write_Int(f,mesh->GetNumUVChannels());


	if(exportxml) 	 fprintf (xmlfile, " Flags=\"%i\" materialID=\"%i\" numVertices=\"%i\" numFaces=\"%i\" numUvChannels=\"%i\">\n",0,mesh->mMaterialIndex,mesh->mNumVertices,mesh->mNumFaces,mesh->GetNumUVChannels());	

	

	//	    printf("Mesh Name[%s] [%i]vertices, [%i]faces, [%i] UVChannels,  \n",mesh->mName.C_Str(),mesh->mNumVertices,mesh->mNumFaces,mesh->GetNumUVChannels());

		  std::cout << " Mesh Name:" << mesh->mName.C_Str() << 
            " Vertices:" << mesh->mNumVertices <<
            " Faces: " << mesh->mNumFaces << 
            " UVChannels: " <<mesh->GetNumUVChannels() << endl;

		 
			 if(exportxml)  fprintf (xmlfile, "<Vertices>\n");	
		 for (int x=0;x<=mesh->mNumVertices-1;x++)
		 {
			 if(exportxml) 	  fprintf (xmlfile, "<Vertex ");
			 Write_Float(f,mesh->mVertices[x].x);
			 Write_Float(f,mesh->mVertices[x].y);
			 Write_Float(f,mesh->mVertices[x].z);
			 Write_Float(f,mesh->mNormals[x].x);
			 Write_Float(f,mesh->mNormals[x].y);
			 Write_Float(f,mesh->mNormals[x].z);
			 Write_Float(f,mesh->mTextureCoords[0][x].x);
			 Write_Float(f,mesh->mTextureCoords[0][x].y);

		       if(exportxml) 
			   {
				   fprintf (xmlfile, " x=\"%f\" y=\"%f\" z=\"%f\" ",mesh->mVertices[x].x,mesh->mVertices[x].y,mesh->mVertices[x].z);
			       fprintf (xmlfile, " nx=\"%f\" ny=\"%f\" nz=\"%f\" ",mesh->mNormals[x].x,mesh->mNormals[x].y,mesh->mNormals[x].z);
				  fprintf (xmlfile, " uv0=\"%f\" tv0=\"%f\"",mesh->mTextureCoords[0][x].x,mesh->mTextureCoords[0][x].y);
			   }

			 if(mesh->GetNumUVChannels()==2)
			 {
				  Write_Float(f,mesh->mTextureCoords[1][x].x);
			      Write_Float(f,mesh->mTextureCoords[1][x].y);
			if(exportxml) fprintf (xmlfile, " uv1=\"%f\" tv1=\"%f\"",mesh->mTextureCoords[1][x].x,mesh->mTextureCoords[1][x].y);
			 }
			 if(exportxml) fprintf (xmlfile, "/>\n");
		 }
	     if(exportxml) 	 fprintf (xmlfile, "</Vertices>\n");	
         if(exportxml)  fprintf (xmlfile, "<Faces>\n");	
		  for (int x=0;x<=mesh->mNumFaces-1;x++)
		 {
	
			Write_Int(f, mesh->mFaces[x].mIndices[0]);
			Write_Int(f, mesh->mFaces[x].mIndices[1]);
			Write_Int(f, mesh->mFaces[x].mIndices[2]);
			if(exportxml) 		  fprintf (xmlfile, "<Face ");
			if(exportxml) fprintf (xmlfile, " a=\"%i\" b=\"%i\" c=\"%i\"",mesh->mFaces[x].mIndices[0],mesh->mFaces[x].mIndices[1],mesh->mFaces[x].mIndices[2]);
			if(exportxml) fprintf (xmlfile, "/>\n");
		  }
  if(exportxml)    fprintf (xmlfile, "</Faces>\n");	

if(exportxml) 	 fprintf (xmlfile, "</Surface>\n");	
	}
	if(exportxml) fprintf (xmlfile, "</Mesh>\n");

	if(exportbones)
	{
		

	 ReadNodeHeirarchy(scene->mRootNode);
     const aiAnimation* anim = scene->mAnimations[0];
	 Write_Int(f,bones.size());
	 float framesPerSecond=anim->mTicksPerSecond;
	 float duration=anim->mDuration;
	 Write_Float(f,framesPerSecond);
	 Write_Float(f,duration);
	 
	if(exportxml)   fprintf (xmlfile, "<Animation Fps=\"%f\" Dutation=\"%f\">\n",framesPerSecond,duration);

//printf("Begin write BONES\n");
	    std::cout << "BEGIN WRITE JOINTS"<<endl;

	 for (int i=0;i<bones.size();i++)
	 {
		 
		 

		 Write_String(f,bones[i].name.c_str());
		 Write_String(f,bones[i].parentName.c_str());
	

		   std::cout << " Joint Name:" << bones[i].name.c_str() << 
            " Parent Name :" << bones[i].parentName.c_str()  << endl;

	if(exportxml) 	   fprintf (xmlfile, "<Joint Name=\"%s\" Parent=\"%s\" NumKeys=\"%i\"",bones[i].name.c_str(),bones[i].parentName.c_str() ,bones[i].keys.size());

	

	        Write_Int(f,bones[i].keys.size());
		    Write_Float(f,bones[i].Position.x);
			Write_Float(f,bones[i].Position.y);
			Write_Float(f,bones[i].Position.z);
			Write_Float(f,bones[i].Rotation.x);
			Write_Float(f,bones[i].Rotation.y);
			Write_Float(f,bones[i].Rotation.z);
			Write_Float(f,bones[i].Rotation.w);

 if(exportxml)   fprintf (xmlfile, "  px=\"%f\"py=\"%f\" pz=\"%f\"",bones[i].Position.x,bones[i].Position.y,bones[i].Position.z);
if(exportxml)    fprintf (xmlfile, " rx=\"%f\" ry=\"%f\" rz=\"%f\" rw=\"%f\">\n",bones[i].Rotation.x,bones[i].Rotation.y,bones[i].Rotation.z,bones[i].Rotation.w);




	if(exportxml) 	fprintf (xmlfile, "<Keys>\n");

			for(int x=0;x<bones[i].keys.size();x++)
			{

				AnimationKey key=	bones[i].keys[x];

				Write_Float(f,key.time);				
				Write_Float(f,key.Position.x);
				Write_Float(f,key.Position.y);
				Write_Float(f,key.Position.z);
				Write_Float(f,key.Rotation.x);
			Write_Float(f,key.Rotation.y);
			Write_Float(f,key.Rotation.z);
			Write_Float(f,key.Rotation.w);
	
			if(exportxml) 
			{
			fprintf (xmlfile, "<Key");
			fprintf (xmlfile, " time=\"%f\"",key.time);
	   	    fprintf (xmlfile, " px=\"%f\" py=\"%f\" pz=\"%f\"",key.Position.x,key.Position.y,key.Position.z);
    		fprintf (xmlfile, " rx=\"%f\" ry=\"%f\" rz=\"%f\" rw=\"%f\"",key.Rotation.x,key.Rotation.y,key.Rotation.z,key.Rotation.w);
			fprintf (xmlfile, "/>\n");
			}
			}
			
		if(exportxml) 	fprintf (xmlfile, "</Keys>\n");
			
		if(exportxml) fprintf (xmlfile, "</Joint>\n");
	 }
	 
	  if(exportxml) fprintf (xmlfile, "</Animation>\n");

std::cout << "END WRITE JOINTS"<<endl;
//printf("Begin write BONES\n");
std::cout << " "<<endl;
std::cout << "BEGIN WRITE BONES"<<endl;
	
/*
if(exportxml) 	 fprintf (xmlfile, "<Bones>\n");	

    anim_surf.resize(scene->mNumMeshes+1);
	
	for (int i=0;i<scene->mNumMeshes ;i++)
	{
         aiMesh* paiMesh=scene->mMeshes[i];
	
		 anim_surf[i].vertexWeights.resize(paiMesh->mNumVertices+1);

		 
			 for (int v=0; v < paiMesh->mNumVertices;v++)
			 {
				 anim_surf[i].vertexWeights[v].numbones=0;
				 anim_surf[i].vertexWeights[v].weights.resize(4);
				 anim_surf[i].vertexWeights[v].vertex=paiMesh->mVertices[v];
				 anim_surf[i].vertexWeights[v].weights[0].vert_bone_no=-1;
				 anim_surf[i].vertexWeights[v].weights[0].vert_weight=0.0f;
				 anim_surf[i].vertexWeights[v].weights[1].vert_bone_no=-1;
				 anim_surf[i].vertexWeights[v].weights[1].vert_weight=0.0f;  
				 anim_surf[i].vertexWeights[v].weights[2].vert_bone_no=-1;
				 anim_surf[i].vertexWeights[v].weights[2].vert_weight=0.0f;
				 anim_surf[i].vertexWeights[v].weights[3].vert_bone_no=-1;
				 anim_surf[i].vertexWeights[v].weights[3].vert_weight=0.0f;
			  }


		 int BoneIndex = 0;        
       

	     for (int x=0;x < paiMesh->mNumBones ;x++)
		 {

			   string BoneName(paiMesh->mBones[x]->mName.data);

			    if (m_BoneMapping.find(BoneName) != m_BoneMapping.end()) 
             	{
                int BoneIndex = m_BoneMapping[BoneName];

			   std::cout << " Bone Name:" << BoneName.c_str() <<  " Index:" << x <<  " BoneIndex: " << BoneIndex << endl;
			    for (int v=0; v < paiMesh->mBones[x]->mNumWeights;v++)
			   {
			    int vid= paiMesh->mBones[x]->mWeights[v].mVertexId;
 			    float bo_vert_w= paiMesh->mBones[x]->mWeights[v].mWeight;
				
				for (int count=0; v < 4;count++)
				{
					if (anim_surf[i].vertexWeights[vid].weights[count].vert_bone_no==-1)
					{
						
						std::cout << " Bone Name:" << BoneName.c_str() <<  " count :" << anim_surf[i].vertexWeights[vid].numbones << endl;

						anim_surf[i].vertexWeights[vid].weights[count].vert_bone_no=BoneIndex;
						anim_surf[i].vertexWeights[vid].weights[count].vert_weight=bo_vert_w;
						anim_surf[i].vertexWeights[vid].numbones++;

						

						break;
					}
				}

				std::sort( anim_surf[i].vertexWeights[vid].weights.begin(), anim_surf[i].vertexWeights[vid].weights.end(), BoneCompare);
				

			 }
			}
			 
		}
	}
//*************************************************************

	 for (int i=0;i<scene->mNumMeshes ;i++)
	 {
         aiMesh* paiMesh=scene->mMeshes[i];
	
		
				if(exportxml) fprintf (xmlfile, "<Skin Surface=\"%i\"\>\n",i);

				for (int b=0;b<paiMesh->mNumVertices;b++)
				{
				if(exportxml)	
				{
					if (anim_surf[i].vertexWeights[b].numbones!=0)fprintf (xmlfile, "<BoneVertex VertexID=\"%i\" Bones=\"%i\">\n",b,anim_surf[i].vertexWeights[b].numbones);
				}
			 Write_Int(f,anim_surf[i].vertexWeights[b].numbones);
			 for (int j=0;j<anim_surf[i].vertexWeights[b].numbones;j++)
			 {
				 	 Write_Int(f,anim_surf[i].vertexWeights[b].weights[j].vert_bone_no);
		        	 Write_Float(f,anim_surf[i].vertexWeights[b].weights[j].vert_weight);
		
				
			if(exportxml) if (anim_surf[i].vertexWeights[b].numbones!=0)	 fprintf (xmlfile, "<Bone%i Id=\"%i\" Weights=\"%f\"\>\n",j,anim_surf[i].vertexWeights[b].weights[j].vert_bone_no,anim_surf[i].vertexWeights[b].weights[j].vert_weight);

			 }

		if (anim_surf[i].vertexWeights[b].numbones!=0)	if(exportxml) fprintf (xmlfile, "</BoneVertex>\n");
			
			
				}
           
				 if(exportxml) fprintf (xmlfile, "<Skin>\n");
			}
		*/

    if(exportxml) fprintf (xmlfile, "<Skin>\n");
	for (int i=0;i<scene->mNumMeshes ;i++)
	{
         aiMesh* paiMesh=scene->mMeshes[i];
		 Write_Int(f,paiMesh->mNumBones);
		 string name;
	 
		if (paiMesh->mName.length<=0)
		{
             name="Surface_"+std::to_string(i);
		} else
		{
		name=string(paiMesh->mName.C_Str());
		}

		if(exportxml) fprintf (xmlfile, "<Surface Index=\"%i\" NumBones=\"%i\">\n",i,paiMesh->mNumBones);

		std::cout << " Mesh Name:" << name.c_str() <<" Num Bones:" << paiMesh->mNumBones << endl;
	     for (int x=0;x < paiMesh->mNumBones ;x++)
		 {
			 string name=string(paiMesh->mBones[x]->mName.data);
			 std::cout << "    Bone Name:" << name.c_str() <<  " Num Weights :" << paiMesh->mBones[x]->mNumWeights << endl;

    		 Write_String(f, name.c_str());
			 Write_Int(f,paiMesh->mBones[x]->mNumWeights);


		 if(exportxml)	fprintf (xmlfile, "<Bone Name=\"%s\" NumWeights=\"%i\">\n",name.c_str(),paiMesh->mBones[x]->mNumWeights);



		 	 for (int v=0; v < paiMesh->mBones[x]->mNumWeights;v++)
			 {
				 Write_Int(f, paiMesh->mBones[x]->mWeights[v].mVertexId);
				 Write_Float(f, paiMesh->mBones[x]->mWeights[v].mWeight);

				 if(exportxml) fprintf (xmlfile, "<Vertex Id=\"%i\" Weight=\"%f\"\/>\n", paiMesh->mBones[x]->mWeights[v].mVertexId,paiMesh->mBones[x]->mWeights[v].mWeight);

			 }
 		
          if(exportxml) fprintf (xmlfile, "</Bone>\n");

		 }
		 
		 if(exportxml) fprintf (xmlfile, "</Surface>\n");
	}
	if(exportxml) fprintf (xmlfile, "</Skin>\n");

	
	//printf("End write BONES\n");
std::cout << "END WRITE BONES"<<endl;
std::cout << " "<<endl;
	} else
	{
	 Write_Int(f,0);
	}

		
if(exportxml) fprintf (xmlfile, "</Model>\n");
if(exportxml) Close_File(xmlfile);

		Close_File(f);
		aiReleaseImport(scene);
		 std::cout << "\n";
		 std::cout << " Operation succeeded ... \n";
		 std::cout << "     By Luis Santos AKA DJOKER ... \n";


	return 0;
}

