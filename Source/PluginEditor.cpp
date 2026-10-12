#include "PluginEditor.h"
#include <cmath>

namespace {
const juce::Colour cream(0xfff3e4bd), textColour(0xffd8e5eb), muted(0xff829cae), accent(0xffe9ba69);
constexpr float baseWidth=1440,baseHeight=730;
const std::array<juce::Point<float>,9> centres {{
 {340,213},{550,213},{760,213},{970,213},{1180,213},
 {445,505},{655,505},{865,505},{1075,505}
}};
void label(juce::Graphics& g,const juce::String& string,juce::Rectangle<float> rect,float size,juce::Colour colour=textColour,juce::Justification justify=juce::Justification::centred) {
 g.setColour(colour);g.setFont(juce::FontOptions(size));g.drawText(string,rect,justify);
}
juce::Point<float> polar(juce::Point<float> centre,float radius,float angle) {
 return centre+juce::Point<float>(radius*std::sin(angle),-radius*std::cos(angle));
}
void drawScale(juce::Graphics& g,const creme::Spec& spec,juce::Point<float> centre) {
 const bool fine=spec.values.size()==51;
 for(int i=0;i<spec.values.size();++i) {
  const float angle=creme::angleFor(spec,(float)i);
  const bool major=!fine || i%5==0;
  auto from=polar(centre,65,angle),to=polar(centre,major?73.0f:68.0f,angle);
  g.setColour(major?cream.withAlpha(0.8f):muted.withAlpha(0.45f));g.drawLine({from,to},major?2.0f:1.0f);
  if(major) {
   auto point=polar(centre,fine?88.0f:91.0f,angle);
   auto value=fine?juce::String(i/5):spec.values[i];
   label(g,value,{point.x-28,point.y-12,56,24},fine?15.0f:17.0f,cream);
  }
 }
}
}
CremeLook::CremeLook() {
 setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff102a3c));
 setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff496274));
 setColour(juce::ComboBox::textColourId,cream);
 setColour(juce::ComboBox::arrowColourId,accent);
 setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff122c3d));
 setColour(juce::PopupMenu::textColourId,cream);
 setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff315a76));
 setColour(juce::PopupMenu::highlightedTextColourId,juce::Colours::white);
 setColour(juce::TooltipWindow::backgroundColourId,juce::Colour(0xff152b3b));
 setColour(juce::TooltipWindow::textColourId,cream);
 setColour(juce::TextButton::buttonColourId,juce::Colour(0xff102737));
 setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff816036));
 setColour(juce::TextButton::textColourOffId,muted);
 setColour(juce::TextButton::textColourOnId,juce::Colours::white);
}
void CremeLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float position,float,float,juce::Slider& slider) {
 const auto index=(int)slider.getProperties()["specIndex"];
 const auto& spec=creme::specs()[(size_t)index];
 auto bounds=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(5);
 const float radius=juce::jmin(bounds.getWidth(),bounds.getHeight())*0.5f;
 const auto centre=bounds.getCentre();bounds={centre.x-radius,centre.y-radius,2*radius,2*radius};
 g.setColour(juce::Colours::black.withAlpha(0.65f));g.fillEllipse(bounds.translated(0,5).expanded(2));
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff555959),bounds.getTopLeft(),juce::Colour(0xff131b20),bounds.getBottomRight(),false));g.fillEllipse(bounds);
 g.setColour(juce::Colour(0xff050b0e));g.drawEllipse(bounds,2);
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff252b2d),bounds.getTopLeft(),juce::Colour(0xff0d1216),bounds.getBottomRight(),false));g.fillEllipse(bounds.reduced(6));
 float angle=creme::angleFor(spec,position*(float)(spec.values.size()-1));
 juce::Path handle;
 handle.addRoundedRectangle(-13,-radius+5,26,2*radius-10,9);
 handle.applyTransform(juce::AffineTransform::rotation(angle).translated(centre.x,centre.y));
 g.setColour(juce::Colour(0xff090e11));g.fillPath(handle);
 g.setColour(juce::Colour(0xff3c4142));g.strokePath(handle,juce::PathStrokeType(1.2f));
 const auto from=polar(centre,radius*0.1f,angle),to=polar(centre,radius*0.85f,angle);
 g.setColour(cream);g.drawLine({from,to},4);
 if(slider.hasKeyboardFocus(false) || slider.isMouseOverOrDragging()) {
  g.setColour(accent.withAlpha(0.75f));g.drawEllipse(bounds.expanded(3),1.4f);
 }
}
PositionSwitch::PositionSwitch(juce::RangedAudioParameter& parameter,const creme::Spec& s)
 :spec(s),attachment(parameter,[this](float value) {
  selected=juce::jlimit(0,spec.values.size()-1,juce::roundToInt(value));
  for(size_t i=0;i<buttons.size();++i) buttons[i]->setToggleState((int)i==selected,juce::dontSendNotification);
  repaint();
 }) {
 setComponentID(s.id);setName(s.name);
 for(int i=0;i<s.values.size();++i) {
  auto button=std::make_unique<juce::TextButton>(s.values[i]);
  button->setComponentID(juce::String(s.id)+"_"+juce::String(i));
  button->setName(juce::String(s.name)+" "+s.values[i]);
  button->setTooltip(juce::String::fromUTF8("Hardware-Position speichern. Audio bleibt unverändert."));
  button->onClick=[this,i] {attachment.setValueAsCompleteGesture((float)i);};
  addAndMakeVisible(*button);buttons.push_back(std::move(button));
 }
 attachment.sendInitialUpdate();
}
void PositionSwitch::resized() {
 const float rowHeight=((float)getHeight()-34)/(float)buttons.size();
 for(size_t i=0;i<buttons.size();++i) buttons[i]->setBounds(54,30+juce::roundToInt((float)i*rowHeight),getWidth()-66,juce::roundToInt(rowHeight)-4);
}
void PositionSwitch::paint(juce::Graphics& g) {
 auto area=getLocalBounds().toFloat();
 g.setColour(juce::Colour(0xff091f30));g.fillRoundedRectangle(area,8);
 g.setColour(juce::Colour(0xff29465b));g.drawRoundedRectangle(area.reduced(0.5f),8,1);
 const auto title=juce::String(spec.id)=="in"?"IN / BYPASS":juce::String(spec.id)=="sc_lowcut"?"SC LOWCUT":"SIGNAL ORDER";
 label(g,title,{0,5,(float)getWidth(),20},12,muted);
 const float row=((float)getHeight()-34)/(float)buttons.size();
 const float top=30+row*0.5f-2,bottom=top+row*(float)(buttons.size()-1);
 g.setColour(juce::Colour(0xff061019));g.fillRoundedRectangle(22,top-10,13,bottom-top+20,6);
 g.setColour(juce::Colour(0xff506574));g.drawLine(28,top,28,bottom,2);
 const float selectedY=top+row*(float)selected;
 g.setGradientFill(juce::ColourGradient(juce::Colours::white,19,selectedY-7,juce::Colour(0xff78828b),36,selectedY+8,false));
 g.fillRoundedRectangle(16,selectedY-7,25,14,4);
}
void PositionSwitch::mouseDown(const juce::MouseEvent& e) {
 if(e.position.x>=50 || e.position.y<30) return;
 auto row=((float)getHeight()-34)/(float)buttons.size();
 attachment.setValueAsCompleteGesture((float)juce::jlimit(0,(int)buttons.size()-1,(int)((e.position.y-30)/row)));
}
void CremeEditor::FrontPanel::paint(juce::Graphics& g) {
 g.setColour(juce::Colour(0xff08141f));g.fillAll();
 g.setGradientFill(juce::ColourGradient(juce::Colour(0xff143f61),0,90,juce::Colour(0xff071d31),baseWidth,590,false));
 g.fillRoundedRectangle(16,88,1408,586,14);
 for(int y=92;y<671;y+=4) {g.setColour(juce::Colours::white.withAlpha(y%3==0?0.018f:0.007f));g.drawHorizontalLine(y,25,1415);}
 g.setColour(juce::Colour(0xff456378));g.drawRoundedRectangle(16,88,1408,586,14,1.5f);
 for(float x:{30.0f,1410.0f}) for(float y:{103.0f,659.0f}) {
  g.setColour(juce::Colour(0xff050d14));g.fillEllipse(x-6,y-6,12,12);g.setColour(juce::Colour(0xff57616b));g.drawLine(x-3,y,x+3,y,1.3f);
 }
 label(g,"KOENICH RECALL",{36,24,290,30},24,cream,juce::Justification::centredLeft);
 label(g,juce::String::fromUTF8("TEGELER CRÈME"),{433,18,580,42},32,cream);
 label(g,"V1.0.1 / ARM64",{1180,29,220,24},15,muted,juce::Justification::centredRight);
 label(g,"HARDWARE SETTINGS",{36,55,290,18},11,muted,juce::Justification::centredLeft);
 for(size_t i=0;i<9;++i) {
  const auto& spec=creme::specs()[i];const auto centre=centres[i];drawScale(g,spec,centre);
  label(g,juce::String(spec.name).toUpperCase(),{centre.x-90,centre.y+100,180,24},15,cream);
 }
 label(g,"EQ",{1315,195,75,28},20,accent);
 label(g,"COMP",{1260,479,120,28},20,accent);
 label(g,"RECALL",{1232,517,176,23},14,cream);
 label(g,"AUDIO UNCHANGED",{1208,546,222,22},12,muted);
 label(g,juce::String::fromUTF8("Ziehen: gerastert   ·   Shift: fein   ·   Doppelklick: Standard"),{38,693,800,23},14,muted,juce::Justification::centredLeft);
 label(g,juce::String::fromUTF8("Wert beim Hovern  /  Threshold: nach links ziehen"),{810,693,584,23},14,muted,juce::Justification::centredRight);
}
CremeEditor::CremeEditor(CremeProcessor& p) :AudioProcessorEditor(p) {
 setLookAndFeel(&look);addAndMakeVisible(panel);panel.setSize((int)baseWidth,(int)baseHeight);
 for(size_t i=0;i<9;++i) {
  const auto& spec=creme::specs()[i];
  auto& knob=knobs[i];knob=std::make_unique<DetentKnob>();
  knob->setName(spec.name);knob->setComponentID(spec.id);knob->getProperties().set("specIndex",(int)i);
  knob->reverseHorizontal=spec.reverse;
  knob->setSliderStyle(spec.reverse?juce::Slider::RotaryHorizontalVerticalDrag:juce::Slider::RotaryVerticalDrag);knob->setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
  knob->setScrollWheelEnabled(false);knob->setDoubleClickReturnValue(true,(double)spec.initial);knob->setWantsKeyboardFocus(true);
  knob->setTooltip(juce::String(spec.name)+juce::String::fromUTF8(": gerastert ziehen; Shift = fein; Doppelklick = Standard."));
  panel.addAndMakeVisible(*knob);
  knobAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.state,spec.id,*knob));

 }
 for(size_t i=0;i<3;++i) {
  const auto& spec=creme::specs()[i+9];switches[i]=std::make_unique<PositionSwitch>(*p.state.getParameter(spec.id),spec);panel.addAndMakeVisible(*switches[i]);
 }
 setResizable(true,true);setResizeLimits(1008,511,2160,1095);getConstrainer()->setFixedAspectRatio(baseWidth/baseHeight);setSize(1296,657);
}
CremeEditor::~CremeEditor() {setLookAndFeel(nullptr);}
void CremeEditor::paint(juce::Graphics& g) {g.fillAll(juce::Colour(0xff08141f));}
void CremeEditor::resized() {
 panel.setTransform(juce::AffineTransform::scale((float)getWidth()/baseWidth));
 for(size_t i=0;i<9;++i) {
  const auto c=centres[i];knobs[i]->setBounds((int)c.x-61,(int)c.y-61,122,122);
 }
 switches[0]->setBounds(50,135,174,110);
 switches[1]->setBounds(50,285,174,142);
 switches[2]->setBounds(50,517,190,113);
}
