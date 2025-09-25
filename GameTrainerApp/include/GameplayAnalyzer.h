#pragma once
#include <string>
#include <vector>
#include <map>

enum class AnalysisType {
    AIM_ANALYSIS,
    RECOIL_COMPENSATION,
    DISTANCE_CALCULATION,
    POSITIONING,
    REACTION_TIME,
    CROSSHAIR_PLACEMENT
};

enum class MistakeSeverity {
    MINOR = 1,
    MODERATE = 2,
    MAJOR = 3,
    CRITICAL = 4
};

struct TechnicalAnalysis {
    double timestamp; 
    AnalysisType type;
    MistakeSeverity severity;
    std::string description;
    std::string explanation;
    std::string suggestion;
    std::map<std::string, double> metrics;
};

struct ShotAnalysis {
    double timestamp;
    bool hit;
    double aimOffset; 
    double recoilCompensation;
    double distanceToTarget;
    double reactionTime;
    std::string weaponType;
    std::vector<TechnicalAnalysis> mistakes;
};

class GameplayAnalyzer {
private:
    std::string currentClipId;
    std::vector<ShotAnalysis> shotAnalyses;
    std::vector<TechnicalAnalysis> generalAnalyses;
    bool isAnalyzing;

public:
    GameplayAnalyzer();
    ~GameplayAnalyzer();
    
    void StartAnalysis(const std::string& clipId);
    void EndAnalysis();
    bool IsAnalyzing() const;
    
    void AnalyzeShot(const ShotAnalysis& shot);
    void AddMistakeToShot(double timestamp, const TechnicalAnalysis& mistake);
    
    TechnicalAnalysis AnalyzeAim(double timestamp, double aimOffset, double targetSize);
    TechnicalAnalysis AnalyzeRecoilCompensation(double timestamp, double compensation, double expectedCompensation);
    TechnicalAnalysis AnalyzeDistanceCalculation(double timestamp, double actualDistance, double calculatedDistance);
    TechnicalAnalysis AnalyzePositioning(double timestamp, const std::string& position, const std::string& optimalPosition);
    TechnicalAnalysis AnalyzeReactionTime(double timestamp, double reactionTime, double averageReactionTime);
    TechnicalAnalysis AnalyzeCrosshairPlacement(double timestamp, double crosshairOffset, double optimalPlacement);
    
    std::vector<ShotAnalysis> GetShotAnalyses() const;
    std::vector<TechnicalAnalysis> GetGeneralAnalyses() const;
    std::map<AnalysisType, int> GetMistakeStats() const;
    std::map<MistakeSeverity, int> GetSeverityStats() const;
    
    std::string GenerateDetailedFeedback() const;
    std::string GenerateImprovementPlan() const;
    std::string GeneratePerformanceSummary() const;
    
    void SaveAnalysisData(const std::string& filename);
    void LoadAnalysisData(const std::string& filename);
    
    std::string GetAnalysisTypeString(AnalysisType type) const;
    std::string GetSeverityString(MistakeSeverity severity) const;
    std::string GetClipId() const;
};
