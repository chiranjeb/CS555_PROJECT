#include "surfaces.h"
#include "hitablelist.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "transformations.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class SceneSetup {

public:
    SceneSetup(){};
    SceneSetup(string fileName);
    SceneSetup(string fileName, int nx, int ny, int ns);
    SceneSetup(string fileName, float distToFocus, float aperature, float fov, 
                    float aspect, vec3 lookfrom, vec3 lookat, vec3 vup); 

    SceneSetup(string fileName,  int nx, int ny, int ns, float distToFocus,
                    float aperature, float fov, float aspect, vec3 lookfrom, 
                    vec3 lookat, vec3 vup); 

    void readFileLines(string filename, vector<string>& fileLines);
    void parseFileLines(vector<string>& fileLines);
    

private:
    void setupPixelParams(string line);
    void setupCameraParams(string line);
    void setupHitable(string line);
    void setupTexture(string textureInfo, texture *tex);
    void setupMaterial(string materialInfo, material *mat); 
 
    int nx, ny, ns;
    float distToFocus, aperture, fov, aspect;
    vec3 lookfrom, lookat, vup;
    camera cam;
    vector<hitable*> list;

};

SceneSetup::SceneSetup(string fileName) 
   {
    vector<string> fileLines;
   
    readFileLines(fileName, fileLines);
    parseFileLines(fileLines);
   }

void SceneSetup::readFileLines(string fileName)
   {
    ifstream fin;
    string line;
    fin.clear();
    fin.open(fileName);
   
    if(fin.is_open())
       { 
        while(getline(fin, line))
           {
            fileLines.push_back(line);
           }
   
       }
    fin.close();
   }

void SceneSetup::parseFileLines(vector<string>& fileLines);
   { 
    for(int i = 0; i < fileLines.size(); i++) 
       {
        if(fileLines[i][0] == 'p')
           {
            setupPixelParams(fileLines[i]);
           } 
        else if(fileLines[i][0] == 'c')
           {
            setupCameraParams(fileLines[i]); 
           }
        else
           {
            setupHitable(fileLines[i]);
           }
       }  
   }

void SceneSetup::setupHitable(string line)
   {
    vec3 translation, color;
    float rotation;
    texture *tex; 
    material *mat;
    hitable *surface;
    int pos=0;    
    string token, delimiter = ":";

    // skip idenifying prolog 
    pos = line.find(delimiter); 
    line.erase(0, pos+delimiter.length());      

    // read and process texture info
    pos = line.find(delimiter); 
    token = line.substr(0, pos);
    line.erase(0, pos+delimiter.length());      
    setupTexture(token, tex);

    // read and process material info
    pos = line.find(delimiter); 
    token = line.substr(0, pos);
    line.erase(0, pos+delimiter.length());      
    setupMaterial(token, mat);
   
   }

void SceneSetup::setupTexture(string token, texture *tex) 
   {
    string delimiter = "(";
    int pos = token.find(delimiter);
    string subToken = token.substr(0,pos);
    token.erase(0, pos+delimiter.length());

    if(subToken.compare("constantTexture"))
       {
        // remove initial '('
        string delimiter = "(";
        int pos = token.find(delimiter);
        token.erase(0, pos+delimiter.length());

        float r,g,b;
        delimiter = ")";
        pos = token.find(delimiter);
        subToken = token.substr(0,pos);

        delimiter = ",";

        pos = subToken.find(delimiter); 
        r = stof(subToken.substr(0, pos));
        subToken.erase(0, pos+delimiter.length());      
            
        pos = subToken.find(delimiter); 
        g = stof(subToken.substr(0, pos));
        subToken.erase(0, pos+delimiter.length());      

        b = stof(subToken);
        tex = new constantTexture(vec3(r,g,b));
       }         
    else if(subToken.compare("checkerTexture"))
       {
	texture *one, *other;
 	int parenCount = 0;
        bool isClosed = false, beenOpen = false;
 
	int i = 0;
        while(!isClosed) 
           {
            if(token[i] == '(')
               {
                beenOpen = true;
                parenCount++;
               }
            if(token[i] == ')')
               {
                parenCount--;
               }
            if(parenCount == 0 && beenOpen == true) 
               {
                isClosed = true;
               }
            i++;
           }
	setupTexture(token.substr(0, i), one);
        setupTexture(token.substr(i,token.length()));

       }
    else if(token.substr(0,token.find("(")).compare("checkerNoise"))
       {


       }
    else if(token.substr(0,token.find("(")).compare("camo"))
       {

       }
    else if(token.substr(0,token.find("(")).compare("marble"))
       {

       }
   }



