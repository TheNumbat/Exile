
#include <algorithm>
#include "profiler_gui.h"

#undef max
#undef min

static const Profile_Color color_order[] = {
    Profile_Color::alizarin, Profile_Color::greenSea, Profile_Color::pumpkin, 
    Profile_Color::silver, Profile_Color::turqoise, Profile_Color::belizeHole, 
    Profile_Color::nephritis, Profile_Color::clouds, Profile_Color::pomegranate, 
    Profile_Color::peterRiver, Profile_Color::sunFlower, Profile_Color::amethyst, 
    Profile_Color::carrot, Profile_Color::wisteria, Profile_Color::orange, 
    Profile_Color::emerald
};
static const u32 num_colors = sizeof(color_order)/sizeof(color_order[0]);

void Profiler_Graph::init(usize framesCount) {
    frames = vec<Frame_Data, MProfGui>::make(framesCount);
    frames.size = framesCount;
}

void Profiler_Graph::destroy() {
    frames.destroy();
    taskNameToStatsIndex.destroy();
    taskStats.destroy();
}

void Profiler_Graph::load_frame_data(Graph_Entry* tasks, usize count) {

    i32 color_idx = 0;
    auto &currFrame = frames[currFrameIndex];
    currFrame.tasks.destroy();
    currFrame.taskStatsIndex.destroy();

    for (usize taskIndex = 0; taskIndex < count; taskIndex++) {
        if(tasks[taskIndex].color == Profile_Color::none) tasks[taskIndex].color = color_order[color_idx++];
        if(color_idx == num_colors) color_idx = 0;

        if (taskIndex == 0)
            currFrame.tasks.push(tasks[taskIndex]);
        else {
            if (tasks[taskIndex - 1].color != tasks[taskIndex].color || tasks[taskIndex - 1].name != tasks[taskIndex].name)
                currFrame.tasks.push(tasks[taskIndex]);
            else
            currFrame.tasks.back().endTime = tasks[taskIndex].endTime;
        }
    }
    currFrame.taskStatsIndex = vec<usize, MProfGui>::make(currFrame.tasks.size);
    currFrame.taskStatsIndex.size = currFrame.tasks.size;

    for (usize taskIndex = 0; taskIndex < currFrame.tasks.size; taskIndex++) {
        auto &task = currFrame.tasks[taskIndex];
        usize* it = taskNameToStatsIndex.try_get(task.name);

        if (!it) {
            taskNameToStatsIndex.insert(task.name, taskStats.size);
            Task_Stats taskStat;
            taskStat.maxTime = -1.0;
            taskStats.push(taskStat);
        }
        currFrame.taskStatsIndex[taskIndex] = taskNameToStatsIndex.get(task.name);
    }
    
    currFrameIndex = (currFrameIndex + 1) % frames.size;
    rebuild_task_stats(currFrameIndex, 300);
}

void Profiler_Graph::render_timings(i32 graphWidth, i32 legendWidth, i32 height, i32 frameIndexOffset) {
    
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    const v2 widgetPos(p.x, p.y);
    render_graph(drawList, widgetPos, v2((f32)graphWidth, (f32)height), frameIndexOffset);
    render_legend(drawList, widgetPos + v2((f32)graphWidth, 0.0f), v2((f32)legendWidth, (f32)height), frameIndexOffset);
    ImGui::Dummy(ImVec2(f32(graphWidth + legendWidth), f32(height)));
}

void Profiler_Graph::rebuild_task_stats(usize endFrame, usize framesCount) {
    
    for (auto &taskStat : taskStats) {
        taskStat.maxTime = -1.0f;
        taskStat.priorityOrder = usize(-1);
        taskStat.onScreenIndex = usize(-1);
    }

    for (usize frameNumber = 0; frameNumber < framesCount; frameNumber++) {

        usize frameIndex = (endFrame - 1 - frameNumber + frames.size) % frames.size;
        auto &frame = frames[frameIndex];

        for (usize taskIndex = 0; taskIndex < frame.tasks.size; taskIndex++) {
            auto &task = frame.tasks[taskIndex];
            auto &stats = taskStats[frame.taskStatsIndex[taskIndex]];
            stats.maxTime = std::max(stats.maxTime, task.endTime - task.startTime);
        }
    }

    vec<usize> statPriorities = vec<usize>::make(taskStats.size);
    statPriorities.size = taskStats.size;

    for(usize statIndex = 0; statIndex < taskStats.size; statIndex++)
        statPriorities[statIndex] = statIndex;

    std::sort(statPriorities.begin(), statPriorities.end(), [this](usize left, usize right) {return taskStats[left].maxTime > taskStats[right].maxTime; });
    
    for (usize statNumber = 0; statNumber < taskStats.size; statNumber++) {
        usize statIndex = statPriorities[statNumber];
        taskStats[statIndex].priorityOrder = statNumber;
    }

    statPriorities.destroy();
}

void Profiler_Graph::render_graph(ImDrawList* drawList, v2 graphPos, v2 graphSize, usize frameIndexOffset) {

    rect(drawList, graphPos, graphPos + graphSize, 0xffffffff, false);
    f32 maxFrameTime = 1.0f / 30.0f;
    f32 heightThreshold = 1.0f;

    for (usize frameNumber = 0; frameNumber < frames.size; frameNumber++) {

        usize frameIndex = (currFrameIndex - frameIndexOffset - 1 - frameNumber + 2 * frames.size) % frames.size;

        v2 framePos = graphPos + v2(graphSize.x - 1 - frameWidth - (frameWidth + frameSpacing) * frameNumber, graphSize.y - 1);
        if (framePos.x < graphPos.x + 1)
            break;
        
        v2 taskPos = framePos + v2(0.0f, 0.0f);
        auto &frame = frames[frameIndex];
        
        for (auto task : frame.tasks) {
            f32 taskStartHeight = (f32(task.startTime) / maxFrameTime) * graphSize.y;
            f32 taskEndHeight = (f32(task.endTime) / maxFrameTime) * graphSize.y;
            if (abs(taskEndHeight - taskStartHeight) > heightThreshold)
                rect(drawList, taskPos + v2(0.0f, (f32)-taskStartHeight), taskPos + v2((f32)frameWidth, (f32)-taskEndHeight), (u32)task.color, true);
        }
    }
}

void Profiler_Graph::render_legend(ImDrawList* drawList, v2 legendPos, v2 legendSize, usize frameIndexOffset) {

    f32 markerLeftRectMargin = 3.0f;
    f32 markerLeftRectWidth = 5.0f;
    f32 maxFrameTime = 1.0f / 30.0f;
    f32 markerMidWidth = 30.0f;
    f32 markerRightRectWidth = 10.0f;
    f32 markerRigthRectMargin = 3.0f;
    f32 markerRightRectHeight = 10.0f;
    f32 markerRightRectSpacing = 4.0f;
    f32 nameOffset = 40.0f;
    v2 textMargin = v2(5.0f, -3.0f);

    auto &currFrame = frames[(currFrameIndex - frameIndexOffset - 1 + 2 * frames.size) % frames.size];
    usize maxTasksCount = usize(legendSize.y / (markerRightRectHeight + markerRightRectSpacing));

    for (auto &taskStat : taskStats)
        taskStat.onScreenIndex = usize(-1);

    usize tasksToShow = std::min<usize>(taskStats.size, maxTasksCount);
    usize tasksShownCount = 0;
    for (usize taskIndex = 0; taskIndex < currFrame.tasks.size; taskIndex++) {
        
        auto &task = currFrame.tasks[taskIndex];
        auto &stat = taskStats[currFrame.taskStatsIndex[taskIndex]];

        if (stat.priorityOrder >= tasksToShow)
            continue;

        if (stat.onScreenIndex == usize(-1))
            stat.onScreenIndex = tasksShownCount++;
        else
            continue;

        f32 taskStartHeight = (f32(task.startTime) / maxFrameTime) * legendSize.y;
        f32 taskEndHeight = (f32(task.endTime) / maxFrameTime) * legendSize.y;

        v2 markerLeftRectMin = legendPos + v2(markerLeftRectMargin, legendSize.y);
        v2 markerLeftRectMax = markerLeftRectMin + v2(markerLeftRectWidth, 0.0f);
        markerLeftRectMin.y -= taskStartHeight;
        markerLeftRectMax.y -= taskEndHeight;

        v2 markerRightRectMin = legendPos + v2(markerLeftRectMargin + markerLeftRectWidth + markerMidWidth, legendSize.y - markerRigthRectMargin - (markerRightRectHeight + markerRightRectSpacing) * stat.onScreenIndex);
        v2 markerRightRectMax = markerRightRectMin + v2(markerRightRectWidth, -markerRightRectHeight);
        render_task_marker(drawList, markerLeftRectMin, markerLeftRectMax, markerRightRectMin, markerRightRectMax, (u32)task.color);

        u32 textColor = (u32)(useColoredLegendText ? task.color : Profile_Color::text);

        f32 taskTimeMs = f32(task.endTime - task.startTime);

        string label = scratch_format("[% ", taskTimeMs * 1000.0f);
        label.cut(6);

        text(drawList, markerRightRectMax + textMargin, textColor, label.c_str);
        text(drawList, markerRightRectMax + textMargin + v2(nameOffset, 0.0f), textColor, scratch_format("ms] %", task.name).c_str);
    }
}

void Profiler_Graph::rect(ImDrawList* drawList, v2 minPoint, v2 maxPoint, u32 col, bool filled) {

    if(filled)
        drawList->AddRectFilled(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
    else
        drawList->AddRect(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
}

void Profiler_Graph::text(ImDrawList* drawList, v2 point, u32 col, const char* text) {
    drawList->AddText(ImVec2(point.x, point.y), col, text);
}

void Profiler_Graph::triangle(ImDrawList* drawList, v2 points[3], u32 col, bool filled) {
    if (filled)
        drawList->AddTriangleFilled(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y), ImVec2(points[2].x, points[2].y), col);
    else
        drawList->AddTriangle(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y), ImVec2(points[2].x, points[2].y), col);
}

void Profiler_Graph::render_task_marker(ImDrawList* drawList, v2 leftMinPoint, v2 leftMaxPoint, v2 rightMinPoint, v2 rightMaxPoint, u32 col) {

    rect(drawList, leftMinPoint, leftMaxPoint, col, true);
    rect(drawList, rightMinPoint, rightMaxPoint, col, true);

    ImVec2 points[] = {
        ImVec2(leftMaxPoint.x, leftMinPoint.y),
        ImVec2(leftMaxPoint.x, leftMaxPoint.y),
        ImVec2(rightMinPoint.x, rightMaxPoint.y),
        ImVec2(rightMinPoint.x, rightMinPoint.y)
    };
    drawList->AddConvexPolyFilled(points, 4, col);
}

void Profiler_Window::init() {
    cpuGraph.init(300);
    prevFpsFrameTime = std::chrono::system_clock::now();
}

void Profiler_Window::destroy() {
    cpuGraph.destroy();
}

void Profiler_Window::render() {
        
    fpsFramesCount++;
    auto currFrameTime = std::chrono::system_clock::now();
    {
        f32 fpsDeltaTime = std::chrono::duration<f32>(currFrameTime - prevFpsFrameTime).count();
        if (fpsDeltaTime > 0.5f) {
            this->avgFrameTime = fpsDeltaTime / f32(fpsFramesCount);
            fpsFramesCount = 0;
            prevFpsFrameTime = currFrameTime;
        }
    }

    ImGui::Begin(scratch_format("Profiler [% fps % ms]###ProfileGraph", 1.0f / avgFrameTime, avgFrameTime * 1000.0f).c_str, 0, ImGuiWindowFlags_NoScrollbar);
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    i32 sizeMargin = int(ImGui::GetStyle().ItemSpacing.y);
    i32 maxGraphHeight = 300;
    i32 availableGraphHeight = (int(canvasSize.y) - sizeMargin);
    i32 graphHeight = std::min(maxGraphHeight, availableGraphHeight);
    i32 legendWidth = 200;
    i32 graphWidth = int(canvasSize.x) - legendWidth;
    cpuGraph.render_timings(graphWidth, legendWidth, graphHeight, frameOffset);
    if (graphHeight + sizeMargin + sizeMargin < canvasSize.y) {
        ImGui::Columns(2);
        usize textSize = 50;
        ImGui::Checkbox("Stop profiling", &stopProfiling);
        ImGui::Checkbox("Colored legend text", &useColoredLegendText);
        ImGui::DragInt("Frame offset", &frameOffset, 1.0f, 0, 400);
        ImGui::NextColumn();

        ImGui::SliderInt("Frame width", &frameWidth, 1, 4);
        ImGui::SliderInt("Frame spacing", &frameSpacing, 0, 2);
        ImGui::SliderFloat("Transparency", &ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w, 0.0f, 1.0f);
        ImGui::Columns(1);
    }

    if (!stopProfiling)
        frameOffset = 0;
    
    cpuGraph.frameWidth = frameWidth;
    cpuGraph.frameSpacing = frameSpacing;
    cpuGraph.useColoredLegendText = useColoredLegendText;
    ImGui::End();
}

