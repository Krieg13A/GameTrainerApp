#pragma once
#include <string>
#include <vector>
#include "ConcentrationTracker.h"
#include "GameplayAnalyzer.h"

struct GameplayClip {
    std::string clipId;
    std::string filename;
    double duration; 
    std::string description;
    double timestamp;
    std::vector<ShotAnalysis> shots;
};

class ReviewInterface {
private:
    ConcentrationTracker concentrationTracker;
    GameplayAnalyzer gameplayAnalyzer;
    std::vector<GameplayClip> clips;
    int currentClipIndex;
    double currentPlaybackTime;
    bool isPlaying;

public:
    ReviewInterface();
    ~ReviewInterface();
    
    void LoadClips(const std::string& sessionId);
    void AddClip(const GameplayClip& clip);
    std::vector<GameplayClip> GetClips() const;
    
    void PlayClip(int clipIndex);
    void PauseClip();
    void SeekToTime(double timestamp);
    void NextClip();
    void PreviousClip();
    
    void MarkCurrentConcentration(FocusLevel level, const std::string& reason = "");
    void AnalyzeCurrentMoment();
    void ShowTechnicalAnalysis();
    
    void ShowConcentrationTimeline();
    void GenerateConcentrationReport();
    void GenerateTechnicalReport();
    void ShowPerformanceAnalysis();
    void ShowImprovementPlan();
    
    void ShowCurrentClipInfo();
    void ShowPlaybackControls();
    void ProcessUserInput(char input);
    
    void SimulateShotAnalysis(double timestamp);
    void SimulateMomentAnalysis(double timestamp);
};
