
#include <map>
#include <algorithm>
#include <sstream>
#include <array>

#include <imgui/imgui.h>
#include "ProfilerTask.h"

#undef max
#undef min

namespace LegitProfiler
{
  inline v2 Vec2(ImVec2 vec)
  {
    return v2(vec.x, vec.y);
  }
  class ProfilerGraph
  {
  public:
    int frameWidth;
    int frameSpacing;
    bool useColoredLegendText;

    ProfilerGraph(size_t framesCount)
    {
      frames.resize(framesCount);
      for (auto &frame : frames)
        frame.tasks.reserve(100);
      frameWidth = 3;
      frameSpacing = 1;
      useColoredLegendText = false;
    }

    void LoadFrameData(const LegitProfiler::Task *tasks, size_t count)
    {
      auto &currFrame = frames[currFrameIndex];
      currFrame.tasks.resize(0);
      for (size_t taskIndex = 0; taskIndex < count; taskIndex++)
      {
        if (taskIndex == 0)
          currFrame.tasks.push_back(tasks[taskIndex]);
        else
        {
          if (tasks[taskIndex - 1].color != tasks[taskIndex].color || tasks[taskIndex - 1].name != tasks[taskIndex].name)
          {
            currFrame.tasks.push_back(tasks[taskIndex]);
          }
          else
          {
            currFrame.tasks.back().endTime = tasks[taskIndex].endTime;
          }
        }
      }
      currFrame.taskStatsIndex.resize(currFrame.tasks.size());

      for (size_t taskIndex = 0; taskIndex < currFrame.tasks.size(); taskIndex++)
      {
        auto &task = currFrame.tasks[taskIndex];
        auto it = taskNameToStatsIndex.find(task.name);
        if (it == taskNameToStatsIndex.end())
        {
          taskNameToStatsIndex[task.name] = taskStats.size();
          TaskStats taskStat;
          taskStat.maxTime = -1.0;
          taskStats.push_back(taskStat);
        }
        currFrame.taskStatsIndex[taskIndex] = taskNameToStatsIndex[task.name];
      }
      currFrameIndex = (currFrameIndex + 1) % frames.size();

      RebuildTaskStats(currFrameIndex, 300);
    }

    void RenderTimings(int graphWidth, int legendWidth, int height, int frameIndexOffset)
    {
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      const v2 widgetPos = Vec2(ImGui::GetCursorScreenPos());
      RenderGraph(drawList, widgetPos, v2((f32)graphWidth, (f32)height), frameIndexOffset);
      RenderLegend(drawList, widgetPos + v2((f32)graphWidth, 0.0f), v2((f32)legendWidth, (f32)height), frameIndexOffset);
      ImGui::Dummy(ImVec2(float(graphWidth + legendWidth), float(height)));
    }

    /*void bla()
    {

    }*/
  private:
    void RebuildTaskStats(size_t endFrame, size_t framesCount)
    {
      for (auto &taskStat : taskStats)
      {
        taskStat.maxTime = -1.0f;
        taskStat.priorityOrder = size_t(-1);
        taskStat.onScreenIndex = size_t(-1);
      }

      for (size_t frameNumber = 0; frameNumber < framesCount; frameNumber++)
      {
        size_t frameIndex = (endFrame - 1 - frameNumber + frames.size()) % frames.size();
        auto &frame = frames[frameIndex];
        for (size_t taskIndex = 0; taskIndex < frame.tasks.size(); taskIndex++)
        {
          auto &task = frame.tasks[taskIndex];
          auto &stats = taskStats[frame.taskStatsIndex[taskIndex]];
          stats.maxTime = std::max(stats.maxTime, task.endTime - task.startTime);
        }
      }
      std::vector<size_t> statPriorities;
      statPriorities.resize(taskStats.size());
      for(size_t statIndex = 0; statIndex < taskStats.size(); statIndex++)
        statPriorities[statIndex] = statIndex;

      std::sort(statPriorities.begin(), statPriorities.end(), [this](size_t left, size_t right) {return taskStats[left].maxTime > taskStats[right].maxTime; });
      for (size_t statNumber = 0; statNumber < taskStats.size(); statNumber++)
      {
        size_t statIndex = statPriorities[statNumber];
        taskStats[statIndex].priorityOrder = statNumber;
      }
    }
    void RenderGraph(ImDrawList *drawList, v2 graphPos, v2 graphSize, size_t frameIndexOffset)
    {
      Rect(drawList, graphPos, graphPos + graphSize, 0xffffffff, false);
      float maxFrameTime = 1.0f / 30.0f;
      float heightThreshold = 1.0f;

      for (size_t frameNumber = 0; frameNumber < frames.size(); frameNumber++)
      {
        size_t frameIndex = (currFrameIndex - frameIndexOffset - 1 - frameNumber + 2 * frames.size()) % frames.size();

        v2 framePos = graphPos + v2(graphSize.x - 1 - frameWidth - (frameWidth + frameSpacing) * frameNumber, graphSize.y - 1);
        if (framePos.x < graphPos.x + 1)
          break;
        v2 taskPos = framePos + v2(0.0f, 0.0f);
        auto &frame = frames[frameIndex];
        for (auto task : frame.tasks)
        {
          float taskStartHeight = (float(task.startTime) / maxFrameTime) * graphSize.y;
          float taskEndHeight = (float(task.endTime) / maxFrameTime) * graphSize.y;
          //taskMaxCosts[task.name] = std::max(taskMaxCosts[task.name], task.endTime - task.startTime);
          if (abs(taskEndHeight - taskStartHeight) > heightThreshold)
            Rect(drawList, taskPos + v2(0.0f, (f32)-taskStartHeight), taskPos + v2((f32)frameWidth, (f32)-taskEndHeight), (u32)task.color, true);
        }
      }
    }
    void RenderLegend(ImDrawList *drawList, v2 legendPos, v2 legendSize, size_t frameIndexOffset)
    {
      float markerLeftRectMargin = 3.0f;
      float markerLeftRectWidth = 5.0f;
      float maxFrameTime = 1.0f / 30.0f;
      float markerMidWidth = 30.0f;
      float markerRightRectWidth = 10.0f;
      float markerRigthRectMargin = 3.0f;
      float markerRightRectHeight = 10.0f;
      float markerRightRectSpacing = 4.0f;
      float nameOffset = 38.0f;
      v2 textMargin = v2(5.0f, -3.0f);

      auto &currFrame = frames[(currFrameIndex - frameIndexOffset - 1 + 2 * frames.size()) % frames.size()];
      size_t maxTasksCount = size_t(legendSize.y / (markerRightRectHeight + markerRightRectSpacing));

      for (auto &taskStat : taskStats)
      {
        taskStat.onScreenIndex = size_t(-1);
      }

      size_t tasksToShow = std::min<size_t>(taskStats.size(), maxTasksCount);
      size_t tasksShownCount = 0;
      for (size_t taskIndex = 0; taskIndex < currFrame.tasks.size(); taskIndex++)
      {
        auto &task = currFrame.tasks[taskIndex];
        auto &stat = taskStats[currFrame.taskStatsIndex[taskIndex]];

        if (stat.priorityOrder >= tasksToShow)
          continue;

        if (stat.onScreenIndex == size_t(-1))
        {
          stat.onScreenIndex = tasksShownCount++;
        }
        else
          continue;
        float taskStartHeight = (float(task.startTime) / maxFrameTime) * legendSize.y;
        float taskEndHeight = (float(task.endTime) / maxFrameTime) * legendSize.y;

        v2 markerLeftRectMin = legendPos + v2(markerLeftRectMargin, legendSize.y);
        v2 markerLeftRectMax = markerLeftRectMin + v2(markerLeftRectWidth, 0.0f);
        markerLeftRectMin.y -= taskStartHeight;
        markerLeftRectMax.y -= taskEndHeight;

        v2 markerRightRectMin = legendPos + v2(markerLeftRectMargin + markerLeftRectWidth + markerMidWidth, legendSize.y - markerRigthRectMargin - (markerRightRectHeight + markerRightRectSpacing) * stat.onScreenIndex);
        v2 markerRightRectMax = markerRightRectMin + v2(markerRightRectWidth, -markerRightRectHeight);
        RenderTaskMarker(drawList, markerLeftRectMin, markerLeftRectMax, markerRightRectMin, markerRightRectMax, (u32)task.color);

        u32 textColor = (u32)(useColoredLegendText ? task.color : Color::imguiText);

        float taskTimeMs = float(task.endTime - task.startTime);
        std::ostringstream timeText;
        timeText.precision(2);
        timeText << std::fixed << std::string("[") << (taskTimeMs * 1000.0f);

        Text(drawList, markerRightRectMax + textMargin, textColor, timeText.str().c_str());
        Text(drawList, markerRightRectMax + textMargin + v2(nameOffset, 0.0f), textColor, (std::string("ms] ") + task.name).c_str());
      }
    }
    static void Rect(ImDrawList *drawList, v2 minPoint, v2 maxPoint, uint32_t col, bool filled = true)
    {
      if(filled)
        drawList->AddRectFilled(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
      else
        drawList->AddRect(ImVec2(minPoint.x, minPoint.y), ImVec2(maxPoint.x, maxPoint.y), col);
    }
    static void Text(ImDrawList *drawList, v2 point, uint32_t col, const char *text)
    {
      drawList->AddText(ImVec2(point.x, point.y), col, text);
    }
    static void Triangle(ImDrawList *drawList, std::array<v2, 3> points, uint32_t col, bool filled = true)
    {
      if (filled)
        drawList->AddTriangleFilled(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y), ImVec2(points[2].x, points[2].y), col);
      else
        drawList->AddTriangle(ImVec2(points[0].x, points[0].y), ImVec2(points[1].x, points[1].y), ImVec2(points[2].x, points[2].y), col);
    }
    static void RenderTaskMarker(ImDrawList *drawList, v2 leftMinPoint, v2 leftMaxPoint, v2 rightMinPoint, v2 rightMaxPoint, uint32_t col)
    {
      Rect(drawList, leftMinPoint, leftMaxPoint, col, true);
      Rect(drawList, rightMinPoint, rightMaxPoint, col, true);
      std::array<ImVec2, 4> points = {
        ImVec2(leftMaxPoint.x, leftMinPoint.y),
        ImVec2(leftMaxPoint.x, leftMaxPoint.y),
        ImVec2(rightMinPoint.x, rightMaxPoint.y),
        ImVec2(rightMinPoint.x, rightMinPoint.y)
      };
      drawList->AddConvexPolyFilled(points.data(), int(points.size()), col);
    }
    struct FrameData
    {
      std::vector<LegitProfiler::Task> tasks;
      std::vector<size_t> taskStatsIndex;
    };

    struct TaskStats
    {
      double maxTime;
      size_t priorityOrder;
      size_t onScreenIndex;
    };
    std::vector<TaskStats> taskStats;
    std::map<std::string, size_t> taskNameToStatsIndex;

    std::vector<FrameData> frames;
    size_t currFrameIndex = 0;
  };

  class ProfilersWindow
  {
  public:
    ProfilersWindow():
      cpuGraph(300)
    {
      stopProfiling = false;
      frameOffset = 0;
      frameWidth = 3;
      frameSpacing = 1;
      useColoredLegendText = true;
      prevFpsFrameTime = std::chrono::system_clock::now();
      fpsFramesCount = 0;
      avgFrameTime = 1.0f;
    }
    void Render()
    {
      fpsFramesCount++;
      auto currFrameTime = std::chrono::system_clock::now();
      {
        float fpsDeltaTime = std::chrono::duration<float>(currFrameTime - prevFpsFrameTime).count();
        if (fpsDeltaTime > 0.5f)
        {
          this->avgFrameTime = fpsDeltaTime / float(fpsFramesCount);
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
      cpuGraph.RenderTimings(graphWidth, legendWidth, graphHeight, frameOffset);
      if (graphHeight + sizeMargin + sizeMargin < canvasSize.y)
      {
        ImGui::Columns(2);
        size_t textSize = 50;
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
    bool stopProfiling;
    int frameOffset;
    ProfilerGraph cpuGraph;
    int frameWidth;
    int frameSpacing;
    bool useColoredLegendText;
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
    TimePoint prevFpsFrameTime;
    size_t fpsFramesCount;
    float avgFrameTime;
  };
}

