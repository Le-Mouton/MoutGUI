#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include "ttf-reader.hpp"
#include <map>

class Text : public Item {

	float displayScale; 

	std::map<std::string, int> dictTTF;



public:

	std::string text;
	float max_x;
	int precision = 24;

	Text (GLFWwindow* _window, float _x, float _y, float _displayScale, const char* vertexPath,const char* fragmentPath, std::string _text) :
	Item(_window, vertexPath, fragmentPath), displayScale(_displayScale), text(_text) {
		x = _x;
		y = _y;

		xpos = x;
		ypos = y;

		dictTTF["!"] = 1;
		dictTTF["#"] = 6;
		dictTTF["$"] = 7;
		dictTTF["%"] = 8;
		dictTTF["&"] = 9;
		dictTTF["'"] = 10;
		dictTTF["("] = 11;
		dictTTF[")"] = 12;
		dictTTF["*"] = 13;
		dictTTF["+"] = 14;
		dictTTF[","] = 15;
		dictTTF["-"] = 16;
		dictTTF["."] = 17;
		dictTTF["/"] = 18;
		dictTTF["0"] = 19;
		dictTTF["1"] = 20;
		dictTTF["2"] = 21;
		dictTTF["3"] = 22;
		dictTTF["4"] = 23;
		dictTTF["5"] = 24;
		dictTTF["6"] = 25;
		dictTTF["7"] = 26;
		dictTTF["8"] = 27;
		dictTTF["9"] = 28;
		dictTTF[":"] = 29;
		dictTTF[";"] = 30;
		dictTTF["<"] = 31;
		dictTTF["="] = 32;
		dictTTF[">"] = 33;
		dictTTF["?"] = 34;
		dictTTF["@"] = 35;
		dictTTF["A"] = 36;
		dictTTF["B"] = 37;
		dictTTF["C"] = 38;
		dictTTF["D"] = 39;
		dictTTF["E"] = 40;
		dictTTF["F"] = 41;
		dictTTF["G"] = 42;
		dictTTF["H"] = 43;
		dictTTF["I"] = 44;
		dictTTF["J"] = 45;
		dictTTF["K"] = 46;
		dictTTF["L"] = 47;
		dictTTF["M"] = 48;
		dictTTF["N"] = 49;
		dictTTF["O"] = 50;
		dictTTF["P"] = 51;
		dictTTF["Q"] = 52;
		dictTTF["R"] = 53;
		dictTTF["S"] = 54;
		dictTTF["T"] = 55;
		dictTTF["U"] = 56;
		dictTTF["V"] = 57;
		dictTTF["W"] = 58;
		dictTTF["X"] = 59;
		dictTTF["Y"] = 60;
		dictTTF["Z"] = 61;
		dictTTF["["] = 62;
		dictTTF["\\"] = 63;
		dictTTF["]"] = 64;
		dictTTF["^"] = 65;
		dictTTF["_"] = 66;
		dictTTF["`"] = 67;
		dictTTF["a"] = 68;
		dictTTF["b"] = 69;
		dictTTF["c"] = 70;
		dictTTF["d"] = 71;
		dictTTF["e"] = 72;
		dictTTF["f"] = 73;
		dictTTF["g"] = 74;
		dictTTF["h"] = 75;
		dictTTF["i"] = 76;
		dictTTF["j"] = 77;
		dictTTF["k"] = 78;
		dictTTF["l"] = 79;
		dictTTF["m"] = 80;
		dictTTF["n"] = 81;
		dictTTF["o"] = 82;
		dictTTF["p"] = 83;
		dictTTF["q"] = 84;
		dictTTF["r"] = 85;
		dictTTF["s"] = 86;
		dictTTF["t"] = 87;
		dictTTF["u"] = 88;
		dictTTF["v"] = 89;
		dictTTF["w"] = 90;
		dictTTF["x"] = 91;
		dictTTF["y"] = 92;
		dictTTF["z"] = 93;

	}
	void loadVertices(){

		vertices.clear();

		x = xpos;
		y = ypos;

	    float letterSpacing = displayScale * 0.1f;
	    float scale = 2.0f / w;
	    
	    float currentX = x;
	    
	    for(int i = 0; i < text.size(); i++){
	        std::string letter(1, text[i]);
	        if(letter == " "){
	        	currentX += 1.5f * letterSpacing;
	        	continue;
	        }
	        auto it = dictTTF.find(letter);
	        if (it == dictTTF.end()) continue;
	        
	        int indice = it->second;
	        GlyphData g = loadTTFGlyph("fonts/arial.ttf", indice);
	        
	        auto glyphVerts = buildFilledGlyph(g, scale * displayScale, currentX, y, precision);
	        vertices.insert(vertices.end(), glyphVerts.begin(), glyphVerts.end());
	        
	        if (!g.points.empty()) {
	            float minX = g.points[0].x;
	            float maxX = g.points[0].x;
	            
	            for (const GlyphPoint& pt : g.points) {
	                minX = std::min(minX, (float)pt.x);
	                maxX = std::max(maxX, (float)pt.x);
	            }
	            float glyphWidth = (maxX - minX) * scale * displayScale;
	            currentX += glyphWidth + letterSpacing;
	            max_x = currentX;
	        }
	    }
	}
	void renderSetUp(){
	    glGenVertexArrays(1, &VAO);
	    glGenBuffers(1, &VBO);
	    glBindVertexArray(VAO);
	    glBindBuffer(GL_ARRAY_BUFFER, VBO);
	    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
	    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	    glEnableVertexAttribArray(0);
	    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
	    glEnableVertexAttribArray(1);
	    glBindVertexArray(0);
	}
	void renderItem(bool poly){

        glEnable(GL_STENCIL_TEST);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        glStencilFunc(GL_ALWAYS, 0, 1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glStencilFunc(GL_NOTEQUAL, 0, 1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        glDisable(GL_STENCIL_TEST);

	}

	bool mouse_button_callback(GLFWwindow* /*window*/, int /*button*/, int /*action*/, int /*mods*/) { return false;}
	bool cursor_position_callback(GLFWwindow* /*window*/, double /*xpos*/, double /*ypos*/) { return false;}

};