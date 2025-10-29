#pragma once
#include "plot.hpp"

class HeatmapPlot : public Plot {
public:
    enum Colormap { VIRIDIS, HOT };
    
private:
    std::vector<std::vector<float>> gridData; // data
    GLuint heatmapVAO, heatmapVBO;
    bool initialized = false;

    int rows = 0, cols = 0;
    Colormap cmap = VIRIDIS;

public:

    HeatmapPlot(GLFWwindow* _window, float _x, float _y, float _width, float _height,
                const char* vertexPath, const char* fragmentPath)
        : Plot(_window, _x, _y, _width, _height, vertexPath, fragmentPath) {}

    void setGridSize(int r, int c) {
        rows = r;
        cols = c;
        gridData.assign(rows, std::vector<float>(cols, 0.0f));
        computeDataBounds();
    }
    void setGridData(const std::vector<std::vector<float>>& newData) {
        if (newData.empty()) return;
        gridData = newData;
        rows = gridData.size();
        cols = gridData[0].size();
        computeDataBounds();
    }
    void computeDataBounds() override {
        if (rows == 0 || cols == 0) return;
        
        xMin = 0.0f;
        xMax = static_cast<float>(cols);
        yMin = 0.0f;
        yMax = static_cast<float>(rows);

        float xMargin = 0.0f;
        float yMargin = 0.0f;

        xMin -= xMargin; xMax += xMargin;
        yMin -= yMargin; yMax += yMargin;
    }

    void setColormap(Colormap m) { cmap = m; }

    void pushColumn(const std::vector<float>& newColumn) {
        if (rows == 0) return;
        if (newColumn.size() != rows) return;

        for (int r = 0; r < rows; r++) {
            gridData[r].push_back(newColumn[r]);
            if (gridData[r].size() > cols)
                gridData[r].erase(gridData[r].begin());
        }
        computeDataBounds();
    }

    void loadVertices() override {
        Plot::loadVertices();

        std::vector<Vertex> heatVertices;

        glm::vec2 p0 = dataToNDC(0,0);
        glm::vec2 p1 = dataToNDC(1,1);


        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                float val = gridData[i][j];

                glm::vec3 color = colormap(val);

                Vertex v;
                v.alpha = 1.0f;

                glm::vec2 pA = dataToNDC(j,   i);
                glm::vec2 pB = dataToNDC(j+1, i);
                glm::vec2 pC = dataToNDC(j+1, i+1);
                glm::vec2 pD = dataToNDC(j,   i+1);

                v.r = color.r; v.g = color.g; v.b = color.b;

                // triangle 1
                v.x = pA.x; v.y = pA.y; heatVertices.push_back(v);
                v.x = pB.x; v.y = pB.y; heatVertices.push_back(v);
                v.x = pC.x; v.y = pC.y; heatVertices.push_back(v);

                // triangle 2
                v.x = pA.x; v.y = pA.y; heatVertices.push_back(v);
                v.x = pC.x; v.y = pC.y; heatVertices.push_back(v);
                v.x = pD.x; v.y = pD.y; heatVertices.push_back(v);
            }
        }

        if (!initialized) {
            glGenVertexArrays(1, &heatmapVAO);
            glGenBuffers(1, &heatmapVBO);
            initialized = true;
        }

        glBindVertexArray(heatmapVAO);
        glBindBuffer(GL_ARRAY_BUFFER, heatmapVBO);
        glBufferData(GL_ARRAY_BUFFER, heatVertices.size()*sizeof(Vertex),
                     heatVertices.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    void renderItem(bool poly) override {
        Plot::renderItem(poly);
        shader_item.use();
        glBindVertexArray(heatmapVAO);
        glDrawArrays(GL_TRIANGLES, 0, rows*cols*6);
    }

private:
    glm::vec3 colormap(float x) {
        x = glm::clamp(x, 0.0f, 1.0f);

        if (cmap == VIRIDIS) {
            return glm::vec3(
                0.267f + x*0.678f,
                0.004f + x*0.824f,
                0.329f + x*0.219f
            );
        }
        if (cmap == HOT) {
            return glm::vec3(
                glm::min(x*3.0f,1.0f),
                glm::min(glm::max(x-0.33f,0.0f)*3.0f,1.0f),
                glm::min(glm::max(x-0.66f,0.0f)*3.0f,1.0f)
            );
        }
        return glm::vec3(x,x,x);
    }
};