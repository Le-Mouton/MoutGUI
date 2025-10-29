#pragma once
#include "plot.hpp"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class LinePlot : public Plot {
protected:

    GLuint pointsVAO, pointsVBO;
    std::vector<Vertex> pointsVertices;
    float pointSize = 6.0f;
    
    std::vector<LineSeries> lineSeries;

    std::vector<std::deque<DataPoint>> lineData;
    
public:
    LinePlot(GLFWwindow* _window, float _x, float _y, float _width, float _height,
                 const char* vertexPath, const char* fragmentPath)
    : Plot(_window, _x, _y, _width, _height, vertexPath, fragmentPath) {
        if (title) title->text = "LinePlot";
    }
    
    ~LinePlot() {
        glDeleteVertexArrays(1, &pointsVAO);
        glDeleteBuffers(1, &pointsVBO);
        
        for (auto& series : lineSeries) {
            if (series.initialized) {
                glDeleteVertexArrays(1, &series.VAO);
                glDeleteBuffers(1, &series.VBO);
            }
        }
    }
    
	void addLineSeries(const std::deque<DataPoint>& data) {
	    lineData.push_back(data);
	    lineSeries.emplace_back();
	    if (autoScale) computeDataBounds();
	}

	void setSeriesData(size_t idx, const std::deque<DataPoint>& data) {
	    if (idx >= lineData.size()) return;
	    lineData[idx] = data;
	    if (autoScale) computeDataBounds();
	}

    void setLineWidth(float lineWidth){
		for (auto& series : lineSeries) {
			series.lineWidth = lineWidth;
		}
    }
    void setLineColor(float r, float g, float b){
		for (auto& series : lineSeries) {
			series.color = glm::vec3(r, g, b);
		}
    }
    
    void clearLineData() {
        lineData.clear();
        for (auto& series : lineSeries) {
            if (series.initialized) {
                glDeleteVertexArrays(1, &series.VAO);
                glDeleteBuffers(1, &series.VBO);
            }
        }
        lineSeries.clear();
    }
    
    void computeDataBounds() override {
        if (lineData.empty()) return;
        
        bool first = true;
        
        for (const auto& series : lineData) {
            for (const auto& pt : series) {
                if (first) {
                    xMin = xMax = pt.x;
                    yMin = yMax = pt.y;
                    first = false;
                } else {
                    xMin = std::min(xMin, pt.x);
                    xMax = std::max(xMax, pt.x);
                    yMin = std::min(yMin, pt.y);
                    yMax = std::max(yMax, pt.y);
                }
            }
        }
        
        float xMargin = (xMax - xMin) * 0.05f;
        float yMargin = (yMax - yMin) * 0.05f;
        xMin -= xMargin; xMax += xMargin;
        yMin -= yMargin; yMax += yMargin;
    }
    
	void loadVertices() override {
	    Plot::loadVertices();

	    for (size_t i = 0; i < lineData.size(); i++) {
	        if (i >= lineSeries.size()) lineSeries.emplace_back();

	        auto& series = lineSeries[i];
	        series.vertices.clear();

	        std::vector<DataPoint> sortedPts(lineData[i].begin(), lineData[i].end());
	        std::sort(sortedPts.begin(), sortedPts.end(),
	                  [](const DataPoint& a, const DataPoint& b){ return a.x < b.x; });

	        for (const auto& pt : sortedPts) {
	            glm::vec2 ndc = dataToNDC(pt.x, pt.y);
	            Vertex v{};
	            v.x = ndc.x; v.y = ndc.y;
	            v.r = series.color.r; v.g = series.color.g; v.b = series.color.b; v.alpha = 1.0f;
	            series.vertices.push_back(v);
	        }
	    }
	}
    
	void renderSetUp() override {
	    Plot::renderSetUp();
	    for (auto& series : lineSeries) {
	        if (!series.initialized) {
	            glGenVertexArrays(1, &series.VAO);
	            glGenBuffers(1, &series.VBO);
	            glBindVertexArray(series.VAO);
	            glBindBuffer(GL_ARRAY_BUFFER, series.VBO);
	            glBufferData(GL_ARRAY_BUFFER, series.vertices.size()*sizeof(Vertex),
	                         series.vertices.data(), GL_DYNAMIC_DRAW);
	            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	            glEnableVertexAttribArray(0);
	            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
	            glEnableVertexAttribArray(1);
	            series.initialized = true;
	        }
	    }
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
	    glBindVertexArray(0);
	}

	void renderItem(bool poly) override {
	    Plot::renderItem(poly);
	    shader_item.use();

	    for (auto& series : lineSeries) {
	        if (!series.initialized) {

	            glGenVertexArrays(1, &series.VAO);
	            glGenBuffers(1, &series.VBO);
	            glBindVertexArray(series.VAO);
	            glBindBuffer(GL_ARRAY_BUFFER, series.VBO);
	            glBufferData(GL_ARRAY_BUFFER, series.vertices.size()*sizeof(Vertex),
	                         series.vertices.data(), GL_DYNAMIC_DRAW);
	            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	            glEnableVertexAttribArray(0);
	            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
	            glEnableVertexAttribArray(1);
	            series.initialized = true;
	        }

	        if (series.vertices.size() >= 2) {
	            glBindBuffer(GL_ARRAY_BUFFER, series.VBO);
	            glBufferData(GL_ARRAY_BUFFER, series.vertices.size()*sizeof(Vertex),
	                         series.vertices.data(), GL_DYNAMIC_DRAW);
	            glBindVertexArray(series.VAO);
	            glLineWidth(series.lineWidth);
	            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)series.vertices.size());
	        }
	    }
	}
};