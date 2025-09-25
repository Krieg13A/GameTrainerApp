#include "CombatAnalyzer.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>

CombatAnalyzer::CombatAnalyzer() 
    : isRecording(false), combatThreshold(0.5), clipDuration(10.0),
      enemyDetectionCooldown(0.1), combatTimeout(3.0), minEnemiesForCombat(1) {
    ResetCombatState();
}

CombatAnalyzer::~CombatAnalyzer() {
    if (isRecording) {
        std::cout << "[CombatAnalyzer] Stopping active recording on destruction" << std::endl;
    }
}

bool CombatAnalyzer::Initialize() {
    std::cout << "[CombatAnalyzer] Initializing combat analysis system..." << std::endl;
    
    if (!enemyDetector.Initialize()) {
        std::cerr << "[CombatAnalyzer] Failed to initialize enemy detector" << std::endl;
        return false;
    }
    
    ResetCombatState();
    std::cout << "[CombatAnalyzer] Combat analysis system initialized successfully" << std::endl;
    return true;
}

void CombatAnalyzer::SetCombatThreshold(double threshold) {
    combatThreshold = std::max(0.0, std::min(1.0, threshold));
    std::cout << "[CombatAnalyzer] Combat threshold set to " << combatThreshold << std::endl;
}

void CombatAnalyzer::SetClipDuration(double duration) {
    clipDuration = std::max(1.0, duration);
    std::cout << "[CombatAnalyzer] Clip duration set to " << clipDuration << " seconds" << std::endl;
}

CombatState CombatAnalyzer::AnalyzeFrame(const cv::Mat& frame, double timestamp) {
    if (frame.empty()) {
        return currentCombatState;
    }
    
    std::vector<EnemyDetection> enemies = enemyDetector.DetectEnemies(frame);
    
    if (!enemies.empty()) {
        currentCombatState.lastEnemySeen = timestamp;
        currentCombatState.enemyCount = enemies.size();
        currentCombatState.activeEnemies = enemies;
        currentCombatState.combatIntensity = CalculateCombatIntensity(enemies);
        
        if (!currentCombatState.isActive) {
            if (enemies.size() >= minEnemiesForCombat && 
                currentCombatState.combatIntensity >= combatThreshold) {
                currentCombatState.isActive = true;
                currentCombatState.startTime = timestamp;
                std::cout << "[CombatAnalyzer] Combat started at " << timestamp << "s" << std::endl;
            }
        }
    } else {
        currentCombatState.enemyCount = 0;
        currentCombatState.activeEnemies.clear();
        currentCombatState.combatIntensity = 0.0;
        
        if (currentCombatState.isActive) {
            double timeSinceLastEnemy = timestamp - currentCombatState.lastEnemySeen;
            if (timeSinceLastEnemy > combatTimeout) {
                currentCombatState.isActive = false;
                std::cout << "[CombatAnalyzer] Combat ended at " << timestamp << "s" << std::endl;
            }
        }
    }
    
    return currentCombatState;
}

bool CombatAnalyzer::ShouldStartRecording(const CombatState& state) {
    return state.isActive && !isRecording && state.combatIntensity >= combatThreshold;
}

bool CombatAnalyzer::ShouldStopRecording(const CombatState& state) {
    if (!state.isActive && isRecording) {
        return true;
    }
    
    if (isRecording && (state.lastEnemySeen + combatTimeout < state.startTime + clipDuration)) {
        return true;
    }
    return false;
}

CombatClip CombatAnalyzer::StartRecording(const std::string& reason, double timestamp) {
    CombatClip clip;
    clip.clipId = GenerateClipId(timestamp);
    clip.startTime = timestamp;
    clip.endTime = timestamp + clipDuration;
    clip.triggerReason = reason;
    clip.enemies = currentCombatState.activeEnemies;
    clip.playerDied = false;
    clip.enemyKilled = false;
    clip.combatIntensity = currentCombatState.combatIntensity;
    clip.filename = clip.clipId + ".mp4";
    
    isRecording = true;
    
    std::cout << "[CombatAnalyzer] Started recording clip: " << clip.clipId << std::endl;
    std::cout << "[CombatAnalyzer] Reason: " << reason << std::endl;
    std::cout << "[CombatAnalyzer] Combat intensity: " << clip.combatIntensity << std::endl;
    
    return clip;
}

void CombatAnalyzer::StopRecording(CombatClip& clip, double timestamp) {
    clip.endTime = timestamp;
    clip.enemies = currentCombatState.activeEnemies;
    clip.combatIntensity = currentCombatState.combatIntensity;
    
    clip.playerDied = DetectPlayerDeath(cv::Mat());
    clip.enemyKilled = DetectEnemyKill(cv::Mat());
    
    recordedClips.push_back(clip);
    
    SaveCombatMetadata(clip);
    
    isRecording = false;
    
    std::cout << "[CombatAnalyzer] Stopped recording clip: " << clip.clipId << std::endl;
    std::cout << "[CombatAnalyzer] Duration: " << (clip.endTime - clip.startTime) << "s" << std::endl;
    std::cout << "[CombatAnalyzer] Player died: " << (clip.playerDied ? "YES" : "NO") << std::endl;
    std::cout << "[CombatAnalyzer] Enemy killed: " << (clip.enemyKilled ? "YES" : "NO") << std::endl;
}

std::vector<CombatClip> CombatAnalyzer::GetRecordedClips() const {
    return recordedClips;
}

std::string CombatAnalyzer::AnalyzeCombatEvent(const CombatClip& clip) {
    std::ostringstream analysis;
    
    analysis << "Combat Event Analysis:\n";
    analysis << "- Duration: " << (clip.endTime - clip.startTime) << "s\n";
    analysis << "- Combat Intensity: " << clip.combatIntensity << "\n";
    analysis << "- Enemies Detected: " << clip.enemies.size() << "\n";
    analysis << "- Player Died: " << (clip.playerDied ? "YES" : "NO") << "\n";
    analysis << "- Enemy Killed: " << (clip.enemyKilled ? "YES" : "NO") << "\n";
    
    if (clip.playerDied) {
        analysis << "\nEvent Type: DEATH\n";
        analysis << "Analysis: Player was eliminated during combat\n";
        analysis << "Recommendation: Review positioning and reaction time\n";
    } else if (clip.enemyKilled) {
        analysis << "\nEvent Type: KILL\n";
        analysis << "Analysis: Successfully eliminated enemy\n";
        analysis << "Recommendation: Study successful engagement patterns\n";
    } else {
        analysis << "\nEvent Type: ENGAGEMENT\n";
        analysis << "Analysis: Combat occurred but no elimination\n";
        analysis << "Recommendation: Review aim and decision making\n";
    }
    
    return analysis.str();
}

double CombatAnalyzer::CalculateCombatIntensity(const std::vector<EnemyDetection>& enemies) {
    if (enemies.empty()) return 0.0;
    
    double totalConfidence = 0.0;
    double proximityFactor = 0.0;
    
    for (const auto& enemy : enemies) {
        totalConfidence += enemy.confidence;
        
        cv::Point2f screenCenter(640, 360);
        double distance = cv::norm(enemy.center - screenCenter);
        double normalizedDistance = std::max(0.0, 1.0 - (distance / 500.0));
        proximityFactor += normalizedDistance;
    }
    
    double avgConfidence = totalConfidence / enemies.size();
    double avgProximity = proximityFactor / enemies.size();
    
    return (avgConfidence * 0.7 + avgProximity * 0.3);
}

bool CombatAnalyzer::DetectPlayerDeath(const cv::Mat& frame) {
    return currentCombatState.combatIntensity > 0.8 && currentCombatState.enemyCount > 0;
}

bool CombatAnalyzer::DetectEnemyKill(const cv::Mat& frame) {
    return currentCombatState.combatIntensity > 0.6 && currentCombatState.enemyCount == 0;
}

void CombatAnalyzer::ResetCombatState() {
    currentCombatState.isActive = false;
    currentCombatState.startTime = 0.0;
    currentCombatState.lastEnemySeen = 0.0;
    currentCombatState.enemyCount = 0;
    currentCombatState.combatIntensity = 0.0;
    currentCombatState.activeEnemies.clear();
}

CombatState CombatAnalyzer::GetCurrentCombatState() const {
    return currentCombatState;
}

bool CombatAnalyzer::IsRecording() const {
    return isRecording;
}

void CombatAnalyzer::SetEnemyDetectionCooldown(double cooldown) {
    enemyDetectionCooldown = std::max(0.0, cooldown);
    std::cout << "[CombatAnalyzer] Enemy detection cooldown set to " << enemyDetectionCooldown << "s" << std::endl;
}

void CombatAnalyzer::SetCombatTimeout(double timeout) {
    combatTimeout = std::max(0.5, timeout);
    std::cout << "[CombatAnalyzer] Combat timeout set to " << combatTimeout << "s" << std::endl;
}

void CombatAnalyzer::SetMinEnemiesForCombat(int minEnemies) {
    minEnemiesForCombat = std::max(1, minEnemies);
    std::cout << "[CombatAnalyzer] Minimum enemies for combat set to " << minEnemiesForCombat << std::endl;
}

std::string CombatAnalyzer::GenerateClipId(double timestamp) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
    localtime_s(&tm, &time);
    
    std::ostringstream oss;
    oss << "combat_"
        << std::put_time(&tm, "%Y%m%d_%H%M%S")
        << "_" << std::fixed << std::setprecision(0) << timestamp;
    
    return oss.str();
}

void CombatAnalyzer::SaveCombatMetadata(const CombatClip& clip) {
    std::string filename = clip.clipId + "_metadata.csv";
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "[CombatAnalyzer] Failed to save metadata to " << filename << std::endl;
        return;
    }
    
    file << "clip_id,start_time,end_time,duration,trigger_reason,combat_intensity,player_died,enemy_killed,enemy_count\n";
    file << clip.clipId << ","
         << clip.startTime << ","
         << clip.endTime << ","
         << (clip.endTime - clip.startTime) << ","
         << clip.triggerReason << ","
         << clip.combatIntensity << ","
         << (clip.playerDied ? "true" : "false") << ","
         << (clip.enemyKilled ? "true" : "false") << ","
         << clip.enemies.size() << "\n";
    
    file.close();
    std::cout << "[CombatAnalyzer] Saved combat metadata to " << filename << std::endl;
}

void CombatAnalyzer::LoadCombatMetadata(const std::string& sessionId) {
    std::cout << "[CombatAnalyzer] Loading combat metadata for session: " << sessionId << std::endl;
}

void CombatAnalyzer::PrintCombatState() const {
    std::cout << "\n=== COMBAT STATE ===" << std::endl;
    std::cout << "Active: " << (currentCombatState.isActive ? "YES" : "NO") << std::endl;
    std::cout << "Start Time: " << currentCombatState.startTime << "s" << std::endl;
    std::cout << "Last Enemy Seen: " << currentCombatState.lastEnemySeen << "s" << std::endl;
    std::cout << "Enemy Count: " << currentCombatState.enemyCount << std::endl;
    std::cout << "Combat Intensity: " << currentCombatState.combatIntensity << std::endl;
    std::cout << "Recording: " << (isRecording ? "YES" : "NO") << std::endl;
    std::cout << std::endl;
}

void CombatAnalyzer::PrintRecordedClips() const {
    std::cout << "\n=== RECORDED CLIPS ===" << std::endl;
    std::cout << "Total clips: " << recordedClips.size() << std::endl;
    
    for (size_t i = 0; i < recordedClips.size(); ++i) {
        const auto& clip = recordedClips[i];
        std::cout << (i + 1) << ". " << clip.clipId << std::endl;
        std::cout << "   Duration: " << (clip.endTime - clip.startTime) << "s" << std::endl;
        std::cout << "   Reason: " << clip.triggerReason << std::endl;
        std::cout << "   Intensity: " << clip.combatIntensity << std::endl;
        std::cout << "   Player Died: " << (clip.playerDied ? "YES" : "NO") << std::endl;
        std::cout << "   Enemy Killed: " << (clip.enemyKilled ? "YES" : "NO") << std::endl;
    }
    std::cout << std::endl;
}
