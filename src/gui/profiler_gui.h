
#pragma once

#include <imgui/imgui.h>
#include "../lib/lib.h"

#include <map>

struct Graph_Entry {
    f32 startTime, endTime;
    u32 color;
    std::string name;

    f32 length() {
        return endTime - startTime;
    }
};

class Profiler_Graph {
public:    
    i32 frameWidth = 3, frameSpacing = 1;
    bool useColoredLegendText = false;

    Profiler_Graph(usize framesCount);
    void load_frame_data(Graph_Entry* tasks, usize count);
    void render_timings(i32 graphWidth, i32 legendWidth, i32 height, i32 frameIndexOffset);

private:
    void rebuild_task_stats(usize endFrame, usize framesCount);
    void render_graph(ImDrawList* drawList, v2 graphPos, v2 graphSize, usize frameIndexOffset);
    void render_legend(ImDrawList* drawList, v2 legendPos, v2 legendSize, usize frameIndexOffset);
    static void rect(ImDrawList* drawList, v2 minPoint, v2 maxPoint, uint32_t col, bool filled = true);
    static void text(ImDrawList* drawList, v2 point, uint32_t col, const char* text);
    static void triangle(ImDrawList* drawList, std::array<v2, 3> points, uint32_t col, bool filled = true);
    static void render_task_marker(ImDrawList* drawList, v2 leftMinPoint, v2 leftMaxPoint, v2 rightMinPoint, v2 rightMaxPoint, uint32_t col);

    struct Frame_Data {
        std::vector<Graph_Entry> tasks;
        std::vector<usize> taskStatsIndex;
    };

    struct Task_Stats {
        f32 maxTime;
        usize priorityOrder, onScreenIndex;
    };

    std::vector<Task_Stats> taskStats;
    std::map<std::string, usize> taskNameToStatsIndex;

    std::vector<Frame_Data> frames;
    usize currFrameIndex = 0;
};

struct Profiler_Window {
    
    Profiler_Window();
    void render();
    
    Profiler_Graph cpuGraph;

    f32 avgFrameTime = 1.0f;
    usize fpsFramesCount = 0;
    bool useColoredLegendText = true, stopProfiling = false;
    i32 frameOffset = 0, frameWidth = 3, frameSpacing = 1;

    using TimePoi32 = std::chrono::time_point<std::chrono::system_clock>;
    TimePoi32 prevFpsFrameTime;
};
