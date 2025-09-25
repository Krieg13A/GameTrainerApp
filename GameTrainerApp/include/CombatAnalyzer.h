#pragma once
#include "EnemyDetector.h"
#include <vector>
#include <string>
#include <memory>

struct CombatState {
    bool isActive;
    double startTime;
    double lastEnemySeen;
    int enemyCount;
    double combatIntensity; // 0.0 to 1.0
    std::vector<EnemyDetection> activeEnemies;
};

struct CombatClip {
    std::string clipId;
    double startTime;
    double endTime;
    std::string triggerReason; // "enemy_detected", "player_died", "combat_started"
    std::vector<EnemyDetection> enemies;
    bool playerDied;
    bool enemyKilled;
    double combatIntensity;
    std::string filename; // Video file path
};

class CombatAnalyzer {
private:
    EnemyDetector enemyDetector;
    CombatState currentCombatState;
    std::vector<CombatClip> recordedClips;
    bool isRecording;
    double combatThreshold;
    double clipDuration;
    
    // Combat detection parameters
    double enemyDetectionCooldown;
    double combatTimeout;
    int minEnemiesForCombat;
    
public:
    CombatAnalyzer();
    ~CombatAnalyzer();
    
    // Initialization
    bool Initialize();
    void SetCombatThreshold(double threshold);
    void SetClipDuration(double duration);
    
    // Combat detection and analysis
    CombatState AnalyzeFrame(const cv::Mat& frame, double timestamp);
    bool ShouldStartRecording(const CombatState& state);
    bool ShouldStopRecording(const CombatState& state);
    
    // Clip management
    CombatClip StartRecording(const std::string& reason, double timestamp);
    void StopRecording(CombatClip& clip, double timestamp);
    std::vector<CombatClip> GetRecordedClips() const;
    
    // Combat event analysis
    std::string AnalyzeCombatEvent(const CombatClip& clip);
    double CalculateCombatIntensity(const std::vector<EnemyDetection>& enemies);
    bool DetectPlayerDeath(const cv::Mat& frame);
    bool DetectEnemyKill(const cv::Mat& frame);
    
    // State management
    void ResetCombatState();
    CombatState GetCurrentCombatState() const;
    bool IsRecording() const;
    
    // Configuration
    void SetEnemyDetectionCooldown(double cooldown);
    void SetCombatTimeout(double timeout);
    void SetMinEnemiesForCombat(int minEnemies);
    
    // Utility
    std::string GenerateClipId(double timestamp);
    void SaveCombatMetadata(const CombatClip& clip);
    void LoadCombatMetadata(const std::string& sessionId);
    
    // Debug
    void PrintCombatState() const;
    void PrintRecordedClips() const;
};
