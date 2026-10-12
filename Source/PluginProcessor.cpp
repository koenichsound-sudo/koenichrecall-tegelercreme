#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdlib>
#include <cmath>

juce::AudioProcessorValueTreeState::ParameterLayout CremeProcessor::makeLayout() {
 juce::AudioProcessorValueTreeState::ParameterLayout layout;
 for(const auto& s:creme::specs())
  layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{s.id,1},s.name,s.values,s.initial,
   juce::AudioParameterChoiceAttributes().withLabel(s.unit)));
 return layout;
}
CremeProcessor::CremeProcessor()
 : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),
 state(*this,nullptr,"KoenichCremeState",makeLayout()) {}
bool CremeProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
 return l.inputBuses.size()==1 && l.outputBuses.size()==1 && !l.getMainInputChannelSet().isDisabled()
  && l.getMainInputChannelSet()==l.getMainOutputChannelSet() && l.getMainInputChannelSet().size()<=64;
}
juce::AudioProcessorEditor* CremeProcessor::createEditor() {return new CremeEditor(*this);}
void CremeProcessor::getStateInformation(juce::MemoryBlock& dest) {
 auto tree=state.copyState();tree.setProperty("version",1,nullptr);
 if(auto xml=tree.createXml()) copyXmlToBinary(*xml,dest);
}
void CremeProcessor::setStateInformation(const void* data,int length) {
 if(data==nullptr || length<=0) return;
 auto xml=getXmlFromBinary(data,length);
 if(!xml || !xml->hasTagName(state.state.getType())) return;
 auto tree=juce::ValueTree::fromXml(*xml);
 if(!tree.isValid() || (int)tree.getProperty("version",0)!=1 || tree.getNumChildren()!=12) return;
 juce::StringArray seen;
 for(auto child:tree) {
  auto id=child.getProperty("id").toString();auto* parameter=state.getParameter(id);
  if(!child.hasType("PARAM") || !child.hasProperty("value") || parameter==nullptr || seen.contains(id)) return;
  auto value=child.getProperty("value").toString();const char* begin=value.toRawUTF8();char* end=nullptr;
  const double index=std::strtod(begin,&end);
  if(end==begin || *end!='\0' || !std::isfinite(index) || index<0 || index>parameter->getNormalisableRange().end || std::abs(index-std::round(index))>1.0e-6) return;
  seen.add(id);
 }
 state.replaceState(tree);
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {return new CremeProcessor;}
