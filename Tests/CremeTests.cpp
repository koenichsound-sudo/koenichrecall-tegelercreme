#include "../Source/PluginEditor.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <cstring>
#include <iostream>
#include <stdexcept>

static void require(bool condition,const char* message) {if(!condition) throw std::runtime_error(message);}
static juce::Component* find(juce::Component& c,const juce::String& id) {
 if(c.getComponentID()==id) return &c;
 for(auto* child:c.getChildren()) if(auto* result=find(*child,id)) return result;
 return nullptr;
}
template<class T> void passthrough(juce::AudioProcessor& p,int channels,int samples) {
 juce::AudioBuffer<T> buffer(channels,samples),original(channels,samples);
 for(int c=0;c<channels;++c) for(int i=0;i<samples;++i) buffer.setSample(c,i,(T)std::sin(c+i*0.163));
 if(samples>=3) {buffer.setSample(0,0,(T)-0.0);buffer.setSample(0,1,std::numeric_limits<T>::denorm_min());buffer.setSample(0,2,std::numeric_limits<T>::infinity());}
 original.makeCopyOf(buffer);juce::MidiBuffer midi;p.processBlock(buffer,midi);
 for(int c=0;c<channels;++c) require(std::memcmp(original.getReadPointer(c),buffer.getReadPointer(c),(size_t)samples*sizeof(T))==0,"Audio changed");
}
static void saveRestore(CremeProcessor& a,CremeProcessor& b) {
 juce::MemoryBlock state;a.getStateInformation(state);b.setStateInformation(state.getData(),(int)state.getSize());
 for(const auto& s:creme::specs()) require(std::abs(a.state.getRawParameterValue(s.id)->load()-b.state.getRawParameterValue(s.id)->load())<1e-6f,"State recall mismatch");
}
static juce::MouseEvent event(juce::Component& c,juce::Point<float> point,juce::Point<float> down,bool drag) {
 const auto now=juce::Time::getCurrentTime();
 return {juce::Desktop::getInstance().getMainMouseSource(),point,juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier),1,0,0,0,0,&c,&c,now,down,now,1,drag};
}
int main(int argc,char** argv) {
 juce::ScopedJuceInitialiser_GUI init;
 try {
  CremeProcessor a,b;
  require(a.getParameters().size()==12,"Must expose 12 recall parameters");
  const std::array<int,12> counts{6,6,6,6,51,51,6,5,4,2,3,2};
  for(size_t i=0;i<12;++i) {
   const auto& s=creme::specs()[i];auto* p=a.state.getParameter(s.id);
   require(p!=nullptr && p->getNumSteps()==counts[i],"Discrete parameter step count");
   for(int step=0;step<counts[i];++step) {
    p->setValueNotifyingHost(p->convertTo0to1((float)step));
    require(juce::roundToInt(a.state.getRawParameterValue(s.id)->load())==step,"Detent index mismatch");
    require(p->getText(p->getValue(),100)==s.values[step],"Host displays wrong physical value");saveRestore(a,b);
   }
  }
  require(creme::specs()[1].values==juce::StringArray{"20","30","60","100","140","200"},"Low frequencies");
  require(creme::specs()[2].values==juce::StringArray{"10","12","16","18","20","24"},"High frequencies");
  require(creme::specs()[6].values==juce::StringArray{"0.1","0.3","1","3","10","30"},"Attack values");
  require(creme::specs()[7].values==juce::StringArray{"0.1","0.3","0.8","1.2","AUTO"},"User release values override PDF");
  require(creme::specs()[8].values==juce::StringArray{"1.5","2","4","10"},"Ratio values");
  for(int n=0;n<=50;++n) for(size_t i:{4u,5u}) require(std::abs(creme::specs()[i].values[n].getDoubleValue()-n*0.2)<1e-6,"0.2 increments");
  require(creme::angleFor(creme::specs()[5],0)>creme::angleFor(creme::specs()[5],50),"Threshold scale must be reversed");
  require(std::abs(creme::angleFor(creme::specs()[4],25))<1e-6f && std::abs(creme::angleFor(creme::specs()[5],25))<1e-6f,"Output and Threshold 5 at noon");
  juce::MemoryBlock valid;b.getStateInformation(valid);
  for(const auto& invalid:{"nan","oops","-1","100","0.25"}) {
   auto tree=a.state.copyState();tree.setProperty("version",1,nullptr);tree.getChild(0).setProperty("value",invalid,nullptr);
   juce::MemoryBlock data;juce::AudioProcessor::copyXmlToBinary(*tree.createXml(),data);b.setStateInformation(data.getData(),(int)data.getSize());
   juce::MemoryBlock after;b.getStateInformation(after);require(after==valid,"Invalid state mutated valid settings");
  }
  b.setStateInformation("bad",3);{juce::MemoryBlock after;b.getStateInformation(after);require(after==valid,"Corrupt state mutated settings");}
  for(int channels:{1,2,8,64}) {
   auto set=channels==1?juce::AudioChannelSet::mono():channels==2?juce::AudioChannelSet::stereo():juce::AudioChannelSet::discreteChannels(channels);
   juce::AudioProcessor::BusesLayout layout;layout.inputBuses.add(set);layout.outputBuses.add(set);require(a.setBusesLayout(layout),"Matching channel layout rejected");
   for(double rate:{44100.0,48000.0,96000.0}) for(int mode:{0,1}) {
    a.state.getParameter("in")->setValueNotifyingHost((float)mode);a.prepareToPlay(rate,1024);
    for(int size:{0,1,64,1024}) {passthrough<float>(a,channels,size);passthrough<double>(a,channels,size);}a.releaseResources();
   }
  }
  {
   std::unique_ptr<juce::AudioProcessorEditor> editor(b.createEditor());
   for(size_t i=0;i<9;++i) {
    const auto& s=creme::specs()[i];auto* knob=dynamic_cast<DetentKnob*>(find(*editor,s.id));auto* combo=dynamic_cast<juce::ComboBox*>(find(*editor,juce::String(s.id)+"_value"));
    require(knob && !combo,"Knob missing or selector still present");require(knob->getInterval()==1,"Knob does not snap to detents");
    for(int step=0;step<s.values.size();++step) {
     knob->setValue(step,juce::sendNotificationSync);require(knob->getTooltip()==juce::String(s.name)+": "+creme::display(s,step),"Hover value mismatch");
     require(juce::roundToInt(b.state.getRawParameterValue(s.id)->load())==step,"Knob/host synchronization");
    }
    b.state.getParameter(s.id)->setValueNotifyingHost(0);require(knob->getValue()==0,"Host/knob synchronization");
    const auto down=juce::Point<float>(61,61);knob->mouseDown(event(*knob,down,down,false));knob->mouseDrag(event(*knob,{61,-89},down,true));knob->mouseUp(event(*knob,{61,-89},down,true));
    require(knob->getValue()>0,"Vertical mouse drag unresponsive");
    require(std::abs(knob->getValue()-std::round(knob->getValue()))<1e-6,"Mouse drag produced intermediate value");
    if(i==5) {
     knob->setValue(25,juce::sendNotificationSync);
     knob->mouseDown(event(*knob,down,down,false));knob->mouseDrag(event(*knob,{-29,61},down,true));knob->mouseUp(event(*knob,{-29,61},down,true));
     require(knob->getValue()>25,"Threshold left drag must increase value");
     require(creme::angleFor(s,(float)knob->getValue())<0,"Threshold left drag must turn left");
     knob->setValue(25,juce::sendNotificationSync);
     knob->mouseDown(event(*knob,down,down,false));knob->mouseDrag(event(*knob,{151,61},down,true));knob->mouseUp(event(*knob,{151,61},down,true));
     require(knob->getValue()<25,"Threshold right drag must decrease value");
    }
    require(knob->hitTest(61,61) && !knob->hitTest(0,0),"Knob hit testing");
   }
   for(size_t i=9;i<12;++i) {
    const auto& s=creme::specs()[i];
    for(int n=0;n<s.values.size();++n) {
     auto* button=dynamic_cast<juce::TextButton*>(find(*editor,juce::String(s.id)+"_"+juce::String(n)));require(button!=nullptr,"Missing switch position");
     button->onClick();require(juce::roundToInt(b.state.getRawParameterValue(s.id)->load())==n && button->getToggleState(),"Switch position not recalled");
    }
   }
   for(size_t i=0;i<12;++i) {const auto& s=creme::specs()[i];auto* p=a.state.getParameter(s.id);p->setValueNotifyingHost(p->convertTo0to1((float)((i+2)%(size_t)s.values.size())));}
   saveRestore(a,b);
   for(size_t i=0;i<9;++i) {const auto& s=creme::specs()[i];auto* k=dynamic_cast<juce::Slider*>(find(*editor,s.id));require(std::abs(k->getValue()-b.state.getRawParameterValue(s.id)->load())<1e-6,"Open GUI recall");}
  }
  if(argc>1) {
   CremeProcessor defaults;std::unique_ptr<juce::AudioProcessorEditor> editor(defaults.createEditor());
   juce::FileOutputStream stream{juce::File(argv[1])};require(stream.openedOk(),"Preview file");stream.setPosition(0);require(stream.truncate().wasOk(),"Preview truncate");
   juce::PNGImageFormat png;require(png.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds(),true,1.5f),stream),"Preview PNG");
  }
  std::cout<<"PASS: 12 parameters, all detents, exact physical labels, 0.2 steps, reversed Threshold, recall, invalid states, GUI synchronization, dynamic hover values, no selectors, Threshold left/right drags, mouse drags, switch positions, float/double bit-exact passthrough (1/2/8/64 channels, 44.1/48/96 kHz, IN and BYPASS).\n";
  if(argc>2) {
   juce::AudioPluginFormatManager manager;manager.addFormat(std::make_unique<juce::VST3PluginFormat>());
   juce::OwnedArray<juce::PluginDescription> types;manager.getFormat(0)->findAllTypesForFile(types,argv[2]);require(types.size()==1,"VST3 scan");
   juce::String error;auto one=manager.createPluginInstance(*types[0],48000,512,error),two=manager.createPluginInstance(*types[0],48000,512,error);require(one && two,"VST3 instantiation");
   require(one->getParameters().size()>=12,"Hosted parameters missing");
   for(size_t i=0;i<12;++i) {const auto& s=creme::specs()[i];one->getParameters()[(int)i]->setValueNotifyingHost((float)(s.values.size()-2)/(float)(s.values.size()-1));}
   one->prepareToPlay(48000,512);passthrough<float>(*one,2,512);one->releaseResources();
   juce::MemoryBlock chunk;one->getStateInformation(chunk);two->setStateInformation(chunk.getData(),(int)chunk.getSize());
   for(int i=0;i<12;++i) require(std::abs(one->getParameters()[i]->getValue()-two->getParameters()[i]->getValue())<1e-6f,"Hosted VST3 recall");
   std::unique_ptr<juce::AudioProcessorEditor> editor(two->createEditorAndMakeActive());require(editor!=nullptr,"Hosted editor");
   std::cout<<"PASS: compiled VST3 scan, two instances, saved-state transfer, audio passthrough and native editor creation.\n";
  }
  return 0;
 } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
