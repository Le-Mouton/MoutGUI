#pragma once
#include "plot.hpp"

class Scatter : public Plot {
protected:

    GLuint pointsVAO, pointsVBO;
    std::vector<Vertex> pointsVertices;
    float pointSize = 6.0f;
    
    std::vector<LineSeries> lineSeries;

    std::vector<DataPoint> scatterData;
    std::vector<std::vector<DataPoint>> lineData;
    
public:
    Scatter(GLFWwindow* _window, float _x, float _y, float _width, float _height,
                 const char* vertexPath, const char* fragmentPath)
    : Plot(_window, _x, _y, _width, _height, vertexPath, fragmentPath) {
        if (title) title->text = "Scatter Combined Plot";
    }
    
    ~Scatter() {
        glDeleteVertexArrays(1, &pointsVAO);
        glDeleteBuffers(1, &pointsVBO);
        
        for (auto& series : lineSeries) {
            if (series.initialized) {
                glDeleteVertexArrays(1, &series.VAO);
                glDeleteBuffers(1, &series.VBO);
            }
        }
    }
    
    void setScatterData(const std::vector<DataPoint>& data) {
        scatterData = data;
        if (autoScale) computeDataBounds();
    }
    
    void addLineSeries(const std::vector<DataPoint>& data, 
                      float lineWidth = 2.0f,
                      glm::vec3 color = glm::vec3(0.2f, 0.4f, 0.8f)) {
        lineData.push_back(data);
        
        LineSeries series;
        series.lineWidth = lineWidth;
        series.color = color;
        lineSeries.push_back(series);
        
        if (autoScale) computeDataBounds();
    }
    
    void clearScatterData() {
        scatterData.clear();
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
        if (scatterData.empty() && lineData.empty()) return;
        
        bool first = true;
        
        for (const auto& pt : scatterData) {
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
        
        // Points scatter
        pointsVertices.clear();
        for (const auto& pt : scatterData) {
            glm::vec2 ndc = dataToNDC(pt.x, pt.y);
            Vertex v;
            v.x = ndc.x; v.y = ndc.y;
            v.r = pt.r; v.g = pt.g; v.b = pt.b;
            v.alpha = 1.0f;
            pointsVertices.push_back(v);
        }
        
        // Lignes
        for (size_t i = 0; i < lineData.size(); i++) {
            if (i >= lineSeries.size()) continue;
            
            lineSeries[i].vertices.clear();
            
            std::vector<DataPoint> sortedPts = lineData[i];
            std::sort(sortedPts.begin(), sortedPts.end(),
                     [](const DataPoint& a, const DataPoint& b) { return a.x < b.x; });
            
            for (const auto& pt : sortedPts) {
                glm::vec2 ndc = dataToNDC(pt.x, pt.y);
                Vertex v;
                v.x = ndc.x; v.y = ndc.y;
                v.r = lineSeries[i].color.r;
                v.g = lineSeries[i].color.g;
                v.b = lineSeries[i].color.b;
                v.alpha = 1.0f;
                lineSeries[i].vertices.push_back(v);
            }
        }
    }
    
    void renderSetUp() override {
        Plot::renderSetUp();
        
        // Points
        glGenVertexArrays(1, &pointsVAO);
        glGenBuffers(1, &pointsVBO);
        glBindVertexArray(pointsVAO);
        glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
        glBufferData(GL_ARRAY_BUFFER, pointsVertices.size() * sizeof(Vertex),
                     pointsVertices.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        
        // Lignes
        for (auto& series : lineSeries) {
            glGenVertexArrays(1, &series.VAO);
            glGenBuffers(1, &series.VBO);
            glBindVertexArray(series.VAO);
            glBindBuffer(GL_ARRAY_BUFFER, series.VBO);
            glBufferData(GL_ARRAY_BUFFER, series.vertices.size() * sizeof(Vertex),
                        series.vertices.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
            glEnableVertexAttribArray(1);
            series.initialized = true;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    void renderItem(bool poly) override {
        Plot::renderItem(poly);
        
        shader_item.use();
        
        // Dessiner les lignes en premier (derrière)
        for (auto& series : lineSeries) {
            if (series.initialized && series.vertices.size() >= 2) {
                glBindBuffer(GL_ARRAY_BUFFER, series.VBO);
                glBufferData(GL_ARRAY_BUFFER, series.vertices.size() * sizeof(Vertex),
                            series.vertices.data(), GL_DYNAMIC_DRAW);
                
                glBindVertexArray(series.VAO);
                glLineWidth(series.lineWidth);
                glDrawArrays(GL_LINE_STRIP, 0, series.vertices.size());
            }
        }
        
        // Dessiner les points par-dessus
        if (!pointsVertices.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
            glBufferData(GL_ARRAY_BUFFER, pointsVertices.size() * sizeof(Vertex),
                        pointsVertices.data(), GL_DYNAMIC_DRAW);
            
            glBindVertexArray(pointsVAO);
            glPointSize(pointSize);
            glDrawArrays(GL_POINTS, 0, pointsVertices.size());
        }
    }
};