
#include "dbg_gui.h"

void DbgGui::Profiler() {

    map<Location,LegitProfiler::Task,Mframe> accum;
    
    Profiler::iterate_timings([&accum](Profiler::thread_id id, Profiler::Timing_Node n) {
        if(id != std::this_thread::get_id()) return;
        LegitProfiler::Task& t = accum.get_or_insert(n.loc);
        t.name = n.loc.func;
        t.endTime += Profiler::ms(n.self_time);
    });

    vec<LegitProfiler::Task,Mframe> data = vec<LegitProfiler::Task,Mframe>::make(3);
    
    for(auto& t : accum) data.push(t.value);
    
    std::sort(data.begin(), data.end(), [](auto l, auto r) {
        return l.endTime < r.endTime;
    });

    for(u32 i = 1; i < data.size; i++) {
        data[i].startTime = data[i-1].endTime;
    }

    profiler.cpuGraph.LoadFrameData(data.data, data.size);
    profiler.Render();
}
