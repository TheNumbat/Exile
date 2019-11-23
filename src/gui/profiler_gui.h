
#pragma once

#include <imgui/imgui.h>
#include "../lib/lib.h"

#define RGBA_LE(col) (((col & 0xff000000) >> (3 * 8)) + ((col & 0x00ff0000) >> (1 * 8)) + ((col & 0x0000ff00) << (1 * 8)) + ((col & 0x000000ff) << (3 * 8)))
enum class Profile_Color : u32 {
    none,
    turqoise = RGBA_LE(0x1abc9cffu),
    greenSea = RGBA_LE(0x16a085ffu),
    emerald = RGBA_LE(0x2ecc71ffu),
    nephritis = RGBA_LE(0x27ae60ffu),
    peterRiver = RGBA_LE(0x3498dbffu),
    belizeHole = RGBA_LE(0x2980b9ffu),
    amethyst = RGBA_LE(0x9b59b6ffu),
    wisteria = RGBA_LE(0x8e44adffu),
    sunFlower = RGBA_LE(0xf1c40fffu),
    orange = RGBA_LE(0xf39c12ffu),
    carrot = RGBA_LE(0xe67e22ffu),
    pumpkin = RGBA_LE(0xd35400ffu),
    alizarin = RGBA_LE(0xe74c3cffu),
    pomegranate = RGBA_LE(0xc0392bffu),
    clouds = RGBA_LE(0xecf0f1ffu),
    silver = RGBA_LE(0xbdc3c7ffu),
    text = RGBA_LE(0xF2F5FAFFu)
};
#undef RGBA_LE

struct Graph_Entry {
    f32 startTime = 0.0f, endTime = 0.0f;
    Profile_Color color = Profile_Color::none;
    literal name;

    f32 length() {
        return endTime - startTime;
    }
};

static constexpr char MProfGui_name[] = "MProfGui";
using MProfGui = Mallocator<MProfGui_name>;

struct Profiler_Graph {

    i32 frameWidth = 3, frameSpacing = 1;
    bool useColoredLegendText = false;

    void init(usize frames);
    void destroy();
    void load_frame_data(Graph_Entry* tasks, usize count);
    void render_timings(i32 graphWidth, i32 legendWidth, i32 height, i32 frameIndexOffset);

private:
    void rebuild_task_stats(usize endFrame, usize framesCount);
    void render_graph(ImDrawList* drawList, v2 graphPos, v2 graphSize, usize frameIndexOffset);
    void render_legend(ImDrawList* drawList, v2 legendPos, v2 legendSize, usize frameIndexOffset);
    static void rect(ImDrawList* drawList, v2 minPoint, v2 maxPoint, u32 col, bool filled = true);
    static void text(ImDrawList* drawList, v2 point, u32 col, const char* text);
    static void triangle(ImDrawList* drawList, v2 points[3], u32 col, bool filled = true);
    static void render_task_marker(ImDrawList* drawList, v2 leftMinPoint, v2 leftMaxPoint, v2 rightMinPoint, v2 rightMaxPoint, u32 col);

    struct Frame_Data {
        vec<Graph_Entry, MProfGui> tasks;
        vec<usize, MProfGui> taskStatsIndex;

        void destroy() {
            tasks.destroy();
            taskStatsIndex.destroy();
        }
    };

    struct Task_Stats {
        f32 maxTime;
        usize priorityOrder, onScreenIndex;
    };

    vec<Task_Stats, MProfGui> taskStats;
    vec<Frame_Data, MProfGui> frames;
    map<literal, usize, MProfGui> taskNameToStatsIndex;

    usize currFrameIndex = 0;
};

struct Profiler_Window {
    
    void init();
    void render();
    void destroy();
    
    Profiler_Graph cpuGraph;

    f32 avgFrameTime = 1.0f;
    usize fpsFramesCount = 0;
    bool useColoredLegendText = true, stopProfiling = false;
    i32 frameOffset = 0, frameWidth = 3, frameSpacing = 1;

    using Time_Point = std::chrono::time_point<std::chrono::system_clock>;
    Time_Point prevFpsFrameTime;
};


