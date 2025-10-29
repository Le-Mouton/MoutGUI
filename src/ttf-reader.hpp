#pragma once
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <numeric>
#include <glm/glm.hpp>
#include "item.hpp"


struct TTHeader { uint16_t numTables, searchRange, entrySelector, rangeShift; };
struct TableEntry { char tag[4]; uint32_t checkSum, offset, length; };

struct GlyphPoint {
    int16_t x, y;
    bool onCurve;
};

struct GlyphData {
    std::vector<GlyphPoint> points;
    std::vector<uint16_t>   endPtsOfContours;
};

inline uint16_t readU16(std::ifstream &f){ uint8_t b[2]; f.read((char*)b,2); return (b[0]<<8)|b[1]; }
inline int16_t  readS16(std::ifstream &f){ uint8_t b[2]; f.read((char*)b,2); return (int16_t)((b[0]<<8)|b[1]); }
inline uint32_t readU32(std::ifstream &f){ uint8_t b[4]; f.read((char*)b,4); return (b[0]<<24)|(b[1]<<16)|(b[2]<<8)|b[3]; }

// -----------------------------------------
// Chargement d’un glyphe simple (.ttf)
// -----------------------------------------
inline GlyphData loadTTFGlyph(const std::string& path, int glyphIndex = 1) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Font not found: " + path);
    f.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    // ---- Tables ----
    f.seekg(4);
    TTHeader headTbl{};
    headTbl.numTables = readU16(f);
    f.seekg(12, std::ios::beg);

    std::vector<TableEntry> tables(headTbl.numTables);
    for (int i = 0; i < headTbl.numTables; ++i) {
        f.read(tables[i].tag, 4);
        tables[i].checkSum = readU32(f);
        tables[i].offset   = readU32(f);
        tables[i].length   = readU32(f);
    }

    auto find = [&](const char* tag)->uint32_t{
        for (auto& t: tables)
            if (std::string(t.tag,4)==std::string(tag,4))
                return t.offset;
        return 0u;
    };

    uint32_t glyfOffset = find("glyf");
    uint32_t locaOffset = find("loca");
    uint32_t headOffset = find("head");
    uint32_t maxpOffset = find("maxp");
    if (!glyfOffset || !locaOffset || !headOffset || !maxpOffset)
        throw std::runtime_error("Missing required tables in TTF");

    // ---- head = indexToLocFormat ----
    f.seekg(headOffset + 50, std::ios::beg);
    int16_t indexToLocFormat = readS16(f);

    // ---- maxp = numGlyphs ----
    f.seekg(maxpOffset + 4, std::ios::beg);
    uint16_t numGlyphs = readU16(f);
    if (glyphIndex < 0 || glyphIndex >= numGlyphs) glyphIndex = 0;

    // ---- loca table ----
    std::vector<uint32_t> loca(numGlyphs + 1);
    f.seekg(locaOffset, std::ios::beg);
    if (indexToLocFormat == 0) {
        for (int i = 0; i <= numGlyphs; ++i) loca[i] = readU16(f) * 2;
    } else {
        for (int i = 0; i <= numGlyphs; ++i) loca[i] = readU32(f);
    }

    // ---- glyf data ----
    uint32_t offset = glyfOffset + loca[glyphIndex];
    f.seekg(offset, std::ios::beg);
    int16_t numberOfContours = readS16(f);

    (void)readS16(f); (void)readS16(f);
    (void)readS16(f); (void)readS16(f);

    GlyphData gd;
    if (numberOfContours <= 0) return gd; // ignore composites

    // ---- Contours ----
    gd.endPtsOfContours.resize(numberOfContours);
    for (int i = 0; i < numberOfContours; ++i)
        gd.endPtsOfContours[i] = readU16(f);

    // ---- Instructions ----
    uint16_t instructionLength = readU16(f);
    f.seekg(instructionLength, std::ios::cur);

    int numPoints = gd.endPtsOfContours.back() + 1;

    // ---- Flags ----
    std::vector<uint8_t> flags;
    flags.reserve(numPoints);
    for (int i = 0; i < numPoints; ) {
        uint8_t flag = f.get();
        flags.push_back(flag);
        ++i;
        if (flag & 0x08) {
            uint8_t repeat = f.get();
            for (int j = 0; j < repeat; ++j) { flags.push_back(flag); ++i; }
        }
    }

    // ---- X coords ----
    std::vector<int16_t> xv(numPoints);
    int16_t xPos = 0;
    for (int i = 0; i < numPoints; ++i) {
        uint8_t fl = flags[i];
        if (fl & 0x02) {
            uint8_t dx = f.get();
            xPos += (fl & 0x10) ? dx : -dx;
        } else if (!(fl & 0x10)) {
            xPos += readS16(f);
        }
        xv[i] = xPos;
    }

    // ---- Y coords ----
    std::vector<int16_t> yv(numPoints);
    int16_t yPos = 0;
    for (int i = 0; i < numPoints; ++i) {
        uint8_t fl = flags[i];
        if (fl & 0x04) {
            uint8_t dy = f.get();
            yPos += (fl & 0x20) ? dy : -dy;
        } else if (!(fl & 0x20)) {
            yPos += readS16(f);
        }
        yv[i] = yPos;
    }

    // ---- Points ----
    gd.points.reserve(numPoints);
    for (int i = 0; i < numPoints; ++i)
        gd.points.push_back({xv[i], yv[i], bool(flags[i] & 0x01)});

    return gd;
}

// -----------------------------------------
// Géométrie : outils & triangulation
// -----------------------------------------
inline float signedArea(const std::vector<glm::vec2>& P){
    double a = 0.0;
    for (size_t i=0;i<P.size();++i){
        const auto& p=P[i]; const auto& q=P[(i+1)%P.size()];
        a += double(p.x)*q.y - double(p.y)*q.x;
    }
    return float(a*0.5);
}

inline void ensureCCW(std::vector<glm::vec2>& P){
    if (signedArea(P) < 0.0f) std::reverse(P.begin(), P.end());
}

inline bool isConvexCCW(const glm::vec2& a,const glm::vec2& b,const glm::vec2& c){
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x) > 0.0f;
}

inline bool pointInTri(const glm::vec2& p,const glm::vec2& a,const glm::vec2& b,const glm::vec2& c){
    float A = (b.x-a.x)*(c.y-a.y)-(b.y-a.y)*(c.x-a.x);
    float s = ((a.y-c.y)*(p.x-c.x)+(c.x-a.x)*(p.y-c.y))/A;
    float t = ((c.y-b.y)*(p.x-c.x)+(b.x-c.x)*(p.y-c.y))/A;
    float u = 1.0f - s - t;
    return s>=0 && t>=0 && u>=0;
}

inline void appendCurve(std::vector<glm::vec2>& poly,
                        const glm::vec2& p0, const glm::vec2& c, const glm::vec2& p1,
                        int steps){
    for (int s=1; s<=steps; ++s){
        float t = float(s)/float(steps);
        float omt = 1.0f - t;
        glm::vec2 pt = omt*omt*p0 + 2.0f*omt*t*c + t*t*p1;
        poly.push_back(pt);
    }
}

inline std::vector<glm::vec2> tessellateContour(const std::vector<GlyphPoint>& raw,
                                                float scale, int steps) {
    if (raw.empty()) return {};

    std::vector<GlyphPoint> pts = raw;

    if (!pts.front().onCurve) {
        const auto& last = pts.back();
        pts.insert(pts.begin(), {
            int16_t((last.x + pts.front().x) / 2),
            int16_t((last.y + pts.front().y) / 2),
            true
        });
    }
    if (!pts.back().onCurve) {
        const auto& first = pts.front();
        const auto& last  = pts.back();
        pts.push_back({
            int16_t((last.x + first.x) / 2),
            int16_t((last.y + first.y) / 2),
            true
        });
    }
    std::vector<GlyphPoint> expanded;
    for (size_t i = 0; i < pts.size(); ++i) {
        expanded.push_back(pts[i]);
        const auto& a = pts[i];
        const auto& b = pts[(i + 1) % pts.size()];
        if (!a.onCurve && !b.onCurve) {
            expanded.push_back({
                int16_t((a.x + b.x) / 2),
                int16_t((a.y + b.y) / 2),
                true
            });
        }
    }

    auto P = [&](const GlyphPoint& g) { return glm::vec2(g.x * scale, g.y * scale); };

    std::vector<glm::vec2> poly;
    
    
    size_t i = 0;
    while (i < expanded.size()) {
        const GlyphPoint& a = expanded[i];
        const GlyphPoint& b = expanded[(i + 1) % expanded.size()];

        if (a.onCurve && b.onCurve) {
            poly.push_back(P(a));
            i++;
        } 
        else if (a.onCurve && !b.onCurve) {
            const GlyphPoint& c = expanded[(i + 2) % expanded.size()];

            if (!c.onCurve) {
                poly.push_back(P(a));
                i++;
                continue;
            }
            
            glm::vec2 p0 = P(a), pc = P(b), p1 = P(c);
            
            poly.push_back(p0);
            
            for (int s = 1; s < steps; ++s) {
                float t = float(s) / steps;
                float omt = 1.0f - t;
                glm::vec2 pt = omt*omt*p0 + 2.0f*omt*t*pc + t*t*p1;
                poly.push_back(pt);
            }
            
            i += 2;
        }
        else {
            poly.push_back(P(a));
            i++;
        }
    }

    if (!poly.empty() && glm::distance(poly.front(), poly.back()) > 1e-6f) {
        poly.push_back(poly.front());
    }

    return poly;
}

inline std::vector<Vertex> triangulateCCW(const std::vector<glm::vec2>& poly){
    std::vector<Vertex> out;
    if (poly.size() < 3) return out;

    std::vector<glm::vec2> P = poly;
    if (!P.empty() && P.front() == P.back()) P.pop_back();
    if (P.size() < 3) return out;

    std::vector<int> idx(P.size());
    std::iota(idx.begin(), idx.end(), 0);

    auto pushV = [&](const glm::vec2& q){
        Vertex v; 
        v.x = q.x; v.y = q.y; v.z = 0;
        v.r = 0.9f; v.g = 0.9f; v.b = 0.9f; v.alpha = 1.0f;
        out.push_back(v);
    };

    auto isConvex = [](const glm::vec2& a, const glm::vec2& b, const glm::vec2& c) -> bool {
        float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        return cross > -1e-6f;
    };

    auto pointInTriangle = [](const glm::vec2& p, const glm::vec2& a, 
                              const glm::vec2& b, const glm::vec2& c) -> bool {
        auto sign = [](const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) {
            return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
        };

        float d1 = sign(p, a, b);
        float d2 = sign(p, b, c);
        float d3 = sign(p, c, a);

        bool hasNeg = (d1 < -1e-6f) || (d2 < -1e-6f) || (d3 < -1e-6f);
        bool hasPos = (d1 > 1e-6f) || (d2 > 1e-6f) || (d3 > 1e-6f);

        return !(hasNeg && hasPos);
    };

    int guard = 0;
    int maxIterations = P.size() * P.size();

    while (idx.size() > 2 && guard++ < maxIterations) {
        bool cut = false;
        int n = idx.size();

        for (int k = 0; k < n; ++k) {
            int ia = idx[(k + n - 1) % n];
            int ib = idx[k];
            int ic = idx[(k + 1) % n];
            
            const glm::vec2& a = P[ia];
            const glm::vec2& b = P[ib];
            const glm::vec2& c = P[ic];

            float area = std::abs((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x));
            if (area < 1e-8f) {
                idx.erase(idx.begin() + k);
                cut = true;
                break;
            }

            if (!isConvex(a, b, c)) continue;

            bool hasPointInside = false;
            for (int j : idx) {
                if (j == ia || j == ib || j == ic) continue;
                if (pointInTriangle(P[j], a, b, c)) {
                    hasPointInside = true;
                    break;
                }
            }

            if (hasPointInside) continue;

            pushV(a); pushV(b); pushV(c);
            idx.erase(idx.begin() + k);
            cut = true;
            break;
        }

        if (!cut) {
            if (idx.size() > 2) {
                int ia = idx[0];
                int ib = idx[1];
                int ic = idx[2];
                pushV(P[ia]); pushV(P[ib]); pushV(P[ic]);
                idx.erase(idx.begin() + 1);
            } else {
                break;
            }
        }
    }

    return out;
}

inline std::vector<Vertex> buildFilledGlyph(const GlyphData& gd,
                                            float scale = 1.0f / 2048.0f,
                                            float offsetX = 0.0f,
                                            float offsetY = 0.0f,
                                            int steps = 32) {
    std::vector<Vertex> filled;
    if (gd.points.empty() || gd.endPtsOfContours.empty()) return filled;

    int start = 0;
    for (uint16_t end : gd.endPtsOfContours) {
        std::vector<GlyphPoint> contour(gd.points.begin() + start,
                                        gd.points.begin() + end + 1);
        start = end + 1;
        if (contour.empty()) continue;

        auto poly = tessellateContour(contour, scale, steps);
        if (poly.size() < 3) continue;
        
        float area = signedArea(poly);

        if (area < 0.0f) {
            std::reverse(poly.begin(), poly.end());
        }

        auto tris = triangulateCCW(poly);
        
        for (auto& v : tris) {
            v.x += offsetX;
            v.y += offsetY;
        }
        
        filled.insert(filled.end(), tris.begin(), tris.end());
    }

    return filled;
}


