
#include <map>
#include <algorithm>
#include <sstream>
#include <array>
#include <imgui/imgui.h>

#include "profiler_gui.h"

#undef max
#undef min

#define RGBA_LE(col) (((col & 0xff000000) >> (3 * 8)) + ((col & 0x00ff0000) >> (1 * 8)) + ((col & 0x0000ff00) << (1 * 8)) + ((col & 0x000000ff) << (3 * 8)))

static uint32_t imguiText = RGBA_LE(0xF2F5FAFFu);
static uint32_t Colors[] = {
/*sunFlower*/ RGBA_LE(0xf1c40fffu),
/*peterRiver*/ RGBA_LE(0x3498dbffu),
/*turqoise*/ RGBA_LE(0x1abc9cffu),
/*wisteria*/ RGBA_LE(0x8e44adffu),
/*clouds*/ RGBA_LE(0xecf0f1ffu),
/*silver*/ RGBA_LE(0xbdc3c7ffu),
/*belizeHole*/ RGBA_LE(0x2980b9ffu),
/*greenSea*/ RGBA_LE(0x16a085ffu),
/*emerald*/ RGBA_LE(0x2ecc71ffu),
/*pomegranate*/ RGBA_LE(0xc0392bffu),
/*carrot*/ RGBA_LE(0xe67e22ffu),
/*pumpkin*/ RGBA_LE(0xd35400ffu),
/*nephritis*/ RGBA_LE(0x27ae60ffu),
/*orange*/ RGBA_LE(0xf39c12ffu),
/*amethyst*/ RGBA_LE(0x9b59b6ffu),
/*alizarin*/ RGBA_LE(0xe74c3cffu)
};

Profiler_Graph::Profiler_Graph(usize framesCount) {
  frames.resize(framesCount);
  for (auto &frame : frames)
    frame.tasks.reserve(100);
}

void Profiler_Graph::load_frame_data(Graph_Entry* tasks, usize count) {

  i32 color_idx = 0;
  auto &currFrame = frames[currFrameIndex];
  currFrame.tasks.resize(0);
    for (usize taskIndex = 0; taskIndex < count; taskIndex++) {
    if(!tasks[taskIndex].color) tasks[taskIndex].color = Colors[color_idx++];

    if (taskIndex == 0)
      currFrame.tasks.push_back(tasks[taskIndex]);
    else {
      if (tasks[taskIndex - 1].color != tasks[taskIndex].color || tasks[taskIndex - 1].name != tasks[taskIndex].name)
        currFrame.tasks.push_back(tasks[taskIndex]);
      else
      currFrame.tasks.back().endTime = tasks[taskIndex].endTime;
    }
  }
  currFrame.taskStatsIndex.resize(currFrame.tasks.size());

  for (usize taskIndex = 0; taskIndex < currFrame.tasks.size(); taskIndex++) {
    auto &task = currFrame.tasks[taskIndex];
    auto it = taskNameToStatsIndex.find(task.name);

    if (it == taskNameToStatsIndex.end()) {
      taskNameToStatsIndex[task.name] = taskStats.size();
      Task_Stats taskStat;
      taskStat.maxTime = -1.0;
      taskStats.push_back(taskStat);
    }
    currFrame.taskStatsIndex[taskIndex] = taskNameToStatsIndex[task.name];
  }
  
  currFrameIndex = (currFrameIndex + 1) % frames.size();
  rebuild_task_stats(currFrameIndex, 300);
}

void Profiler_Graph::render_timings(int graphWidth, int legendWidth, int height, int frameIndexOffset) {
  
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

    usize frameIndex = (endFrame - 1 - frameNumber + frames.size()) % frames.size();
    auto &frame = frames[frameIndex];

    for (usize taskIndex = 0; taskIndex < frame.tasks.size(); taskIndex++) {
      auto &task = frame.tasks[taskIndex];
      auto &stats = taskStats[frame.taskStatsIndex[taskIndex]];
      stats.maxTime = std::max(stats.maxTime, task.endTime - task.startTime);
    }
  }

  std::vector<usize> statPriorities;
  statPriorities.resize(taskStats.size());

  for(usize statIndex = 0; statIndex < taskStats.size(); statIndex++)
    statPriorities[statIndex] = statIndex;

  std::sort(statPriorities.begin(), statPriorities.end(), [this](usize left, usize right) {return taskStats[left].maxTime > taskStats[right].maxTime; });
  
  for (usize statNumber = 0; statNumber < taskStats.size(); statNumber++) {
    usize statIndex = statPriorities[statNumber];
    taskStats[statIndex].priorityOrder = statNumber;
  }
}

void Profiler_Graph::render_graph(ImDrawList* drawList, v2 graphPos, v2 graphSize, usize frameIndexOffset) {

  rect(drawList, graphPos, graphPos + graphSize, 0xffffffff, false);
  f32 maxFrameTime = 1.0f / 30.0f;
  f32 heightThreshold = 1.0f;

  for (usize frameNumber = 0; frameNumber < frames.size(); frameNumber++) {

    usize frameIndex = (currFrameIndex - frameIndexOffset - 1 - frameNumber + 2 * frames.size()) % frames.size();

    v2 framePos = graphPos + v2(graphSize.x - 1 - frameWidth - (frameWidth + frameSpacing) * frameNumber, graphSize.y - 1);
    if (framePos.x < graphPos.x + 1)
      break;
    
    v2 taskPos = framePos + v2(0.0f, 0.0f);
    auto &frame = frames[frameIndex];
    
    for (auto task : frame.tasks) {
      f32 taskStartHeight = (f32(task.startTime) / maxFrameTime) * graphSize.y;
      f32 taskEndHeight = (f32(task.endTime) / maxFrameTime) * graphSize.y;
      if (abs(taskEndHeight - taskStartHeight) > heightThreshold)
        rect(drawList, taskPos + v2(0.0f, (f32)-taskStartHeight), taskPos + v2((f32)frameWidth, (f32)-taskEndHeight), task.color, true);
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

  auto &currFrame = frames[(currFrameIndex - frameIndexOffset - 1 + 2 * frames.size()) % frames.size()];
  usize maxTasksCount = usize(legendSize.y / (markerRightRectHeight + markerRightRectSpacing));

  for (auto &taskStat : taskStats)
    taskStat.onScreenIndex = usize(-1);

  usize tasksToShow = std::min<usize>(taskStats.size(), maxTasksCount);
  usize tasksShownCount = 0;
  for (usize taskIndex = 0; taskIndex < currFrame.tasks.size(); taskIndex++) {
    
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
    render_task_marker(drawList, markerLeftRectMin, markerLeftRectMax, markerRightRectMin, markerRightRectMax, task.color);

    uint32_t textColor = useColoredLegendText ? task.color : imguiText;

    f32 taskTimeMs = f32(task.endTime - task.startTime);
    std::ostringstream timeText;
    timeText.precision(2);
    timeText << std::fixed << std::string("[") << (taskTimeMs * 1000.0f);

    text(drawList, markerRightRectMax + textMargin, textColor, timeText.str().c_str());
    text(drawList, markerRightRectMax + textMargin + v2(nameOffset, 0.0f), textColor, (std::string("ms] ") + task.name).c_str());
  }
}

void Profiler_Graph::rect(ImDrawList* drawList, v2 minPoint, v2 maxPoint, uint32_t col, bool filled) {

  if(filled)
    drawList->AddRectFilled(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
  else
    drawList->AddRect(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
}

void Profiler_Graph::text(ImDrawList* drawList, v2 point, uint32_t col, const char* text) {
  drawList->AddText(ImVec2(point.x, point.y), col, text);
}

void Profiler_Graph::triangle(ImDrawList* drawList, std::array<v2, 3> points, uint32_t col, bool filled) {
  if (filled)
    drawList->AddTriangleFilled(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y), ImVec2(points[2].x, points[2].y), col);
  else
    drawList->AddTriangle(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y), ImVec2(points[2].x, points[2].y), col);
}

void Profiler_Graph::render_task_marker(ImDrawList* drawList, v2 leftMinPoint, v2 leftMaxPoint, v2 rightMinPoint, v2 rightMaxPoint, uint32_t col) {

  rect(drawList, leftMinPoint, leftMaxPoint, col, true);
  rect(drawList, rightMinPoint, rightMaxPoint, col, true);

  std::array<ImVec2, 4> points = {
    ImVec2(leftMaxPoint.x, leftMinPoint.y),
    ImVec2(leftMaxPoint.x, leftMaxPoint.y),
    ImVec2(rightMinPoint.x, rightMaxPoint.y),
    ImVec2(rightMinPoint.x, rightMinPoint.y)
  };
  drawList->AddConvexPolyFilled(points.data(), int(points.size()), col);
}

Profiler_Window::Profiler_Window() : cpuGraph(300) {
  prevFpsFrameTime = std::chrono::system_clock::now();
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

  std::stringstream title;
  title.precision(2);
  title << std::fixed << "Legit profiler [" << 1.0f / avgFrameTime << "fps\t" << avgFrameTime * 1000.0f << "ms]###ProfilerWindow";
  ImGui::Begin(title.str().c_str(), 0, ImGuiWindowFlags_NoScrollbar);
  ImVec2 canvasSize = ImGui::GetContentRegionAvail();

  int sizeMargin = int(ImGui::GetStyle().ItemSpacing.y);
  int maxGraphHeight = 300;
  int availableGraphHeight = (int(canvasSize.y) - sizeMargin);
  int graphHeight = std::min(maxGraphHeight, availableGraphHeight);
  int legendWidth = 200;
  int graphWidth = int(canvasSize.x) - legendWidth;
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

