#pragma once
#include "Parameters.h"

class CremeProcessor final : public juce::AudioProcessor {
public:
 CremeProcessor();
 juce::AudioProcessorValueTreeState state;
 const juce::String getName() const override {return "Koenich Recall - Tegeler Creme";}
 void prepareToPlay(double,int) override {}
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 // Deliberate bit-transparent in-place passthrough, regardless of recall settings.
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override {}
 void processBlock(juce::AudioBuffer<double>&,juce::MidiBuffer&) override {}
 bool supportsDoublePrecisionProcessing() const override {return true;}
 bool acceptsMidi() const override {return false;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return 0;}
 bool hasEditor() const override {return true;}
 juce::AudioProcessorEditor* createEditor() override;
 int getNumPrograms() override {return 1;}
 int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override {return "Recall";}
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CremeProcessor)
};
