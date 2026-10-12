#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

namespace creme {
struct Spec {
 const char* id;
 const char* name;
 juce::StringArray values;
 const char* unit;
 int initial;
 float startDegrees, endDegrees;
 bool reverse;
};
inline juce::StringArray fineSteps() {
 juce::StringArray result;
 for(int i=0;i<=50;++i) result.add(juce::String(i*0.2,1));
 return result;
}
inline const std::array<Spec,12>& specs() {
 static const std::array<Spec,12> data {{
  {"low_boost","Low Boost",{"0","1","2","3","4","5"},"",0,-75,75,false},
  {"low_freq","Low Frequency",{"20","30","60","100","140","200"},"Hz",0,-75,75,false},
  {"high_freq","High Frequency",{"10","12","16","18","20","24"},"kHz",0,-75,75,false},
  {"high_boost","High Boost",{"0","1","2","3","4","5"},"",0,-75,75,false},
  {"output","Output",fineSteps(),"",0,-135,135,false},
  {"threshold","Threshold",fineSteps(),"",0,-135,135,true},
  {"attack","Attack",{"0.1","0.3","1","3","10","30"},"ms",0,-75,75,false},
  {"release","Release",{"0.1","0.3","0.8","1.2","AUTO"},"s",0,-60,60,false},
  {"ratio","Ratio",{"1.5","2","4","10"},":1",0,-45,45,false},
  {"in","IN / Bypass",{"IN","BYPASS"},"",0,0,0,false},
  {"sc_lowcut","Sidechain Lowcut",{"FULL","120 Hz","60 Hz"},"",0,0,0,false},
  {"eq_order","EQ / Compressor Order",{"EQ -> COMP","COMP -> EQ"},"",0,0,0,false}
 }};
 return data;
}
inline juce::String display(const Spec& spec,int index) {
 auto value=spec.values[juce::jlimit(0,spec.values.size()-1,index)];
 if(value=="AUTO" || juce::String(spec.unit).isEmpty()) return value;
 return value+(juce::String(spec.unit)==":1"?"":" ")+spec.unit;
}
inline float angleFor(const Spec& spec,float index) {
 float n=juce::jlimit(0.0f,1.0f,index/(float)(spec.values.size()-1));
 if(spec.reverse) n=1-n;
 return juce::degreesToRadians(spec.startDegrees+n*(spec.endDegrees-spec.startDegrees));
}
}
