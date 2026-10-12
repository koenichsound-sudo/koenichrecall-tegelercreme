#pragma once
#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

class CremeLook final : public juce::LookAndFeel_V4 {
public:
 CremeLook();
 void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};
class DetentKnob final : public juce::Slider {
public:
 bool reverseHorizontal=false;
 juce::String getTooltip() override {
  const auto& spec=creme::specs()[(size_t)(int)getProperties()["specIndex"]];
  return juce::String(spec.name)+": "+creme::display(spec,juce::roundToInt(getValue()));
 }
 void mouseDown(const juce::MouseEvent& e) override {
  dragOrigin=e.position;
  setMouseDragSensitivity(e.mods.isShiftDown()?1500:juce::jmax(140,(int)getMaximum()*6));
  juce::Slider::mouseDown(e);
 }
 void mouseDrag(const juce::MouseEvent& e) override {
  if(reverseHorizontal) juce::Slider::mouseDrag(e.withNewPosition(juce::Point<float>(2*dragOrigin.x-e.position.x,e.position.y)));
  else juce::Slider::mouseDrag(e);
 }
 bool hitTest(int x,int y) override {
  return getLocalBounds().toFloat().getCentre().getDistanceFrom({(float)x,(float)y})<=getWidth()*0.5f;
 }
private:
 juce::Point<float> dragOrigin;
};
class PositionSwitch final : public juce::Component {
public:
 PositionSwitch(juce::RangedAudioParameter&,const creme::Spec&);
 void paint(juce::Graphics&) override;
 void resized() override;
 void mouseDown(const juce::MouseEvent&) override;
private:
 const creme::Spec& spec;
 int selected=0;
 std::vector<std::unique_ptr<juce::TextButton>> buttons;
 juce::ParameterAttachment attachment;
};
class CremeEditor final : public juce::AudioProcessorEditor {
public:
 explicit CremeEditor(CremeProcessor&);
 ~CremeEditor() override;
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 class FrontPanel final : public juce::Component {
 public: void paint(juce::Graphics&) override;
 } panel;
 CremeLook look;
 std::array<std::unique_ptr<DetentKnob>,9> knobs;
 std::array<std::unique_ptr<PositionSwitch>,3> switches;
 std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> knobAttachments;
 juce::TooltipWindow tooltips{this,650};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CremeEditor)
};
