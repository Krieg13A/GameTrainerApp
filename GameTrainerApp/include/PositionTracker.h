#pragma once
#include "EnemyDetector.h"
#include <vector>
#include <map>
#include <string>

struct EnemyPosition {
    cv::Point2f position;
    double timestamp;
    double confidence;
    std::string enemyId;
    bool isVisible;
};

struct EnemyTrajectory {
    std::string enemyId;
    std::vector<EnemyPosition> positions;
    double firstSeen;
    double lastSeen;
    bool isActive;
    cv::Point2f predictedNextPosition;
    double movementSpeed;
    std::string movementPattern; 
};

struct DeathAnalysis {
    double deathTime;
    cv::Point2f deathPosition;
    std::vector<EnemyPosition> nearbyEnemies;
    std::string deathCause; 
    double enemyDistance;
    std::string enemyWeapon;
    bool enemyWasVisible;
};

class PositionTracker {
private:
    std::map<std::string, EnemyTrajectory> enemyTrajectories;
    std::vector<DeathAnalysis> deathAnalyses;
    int nextEnemyId;
    
    double maxTrackingDistance;
    double trajectoryTimeout;
    int minPositionsForTrajectory;
    
    double deathAnalysisRadius;
    double visibilityThreshold;
    
public:
    PositionTracker();
    ~PositionTracker();
    
    void Initialize();
    void SetTrackingDistance(double distance);
    void SetTrajectoryTimeout(double timeout);
    void SetMinPositionsForTrajectory(int minPositions);
    
    void UpdateEnemyPositions(const std::vector<EnemyDetection>& detections, double timestamp);
    std::vector<EnemyTrajectory> GetActiveTrajectories() const;
    std::vector<EnemyTrajectory> GetAllTrajectories() const;
    
    EnemyTrajectory* FindEnemyTrajectory(const cv::Point2f& position, double timestamp);
    std::string AssignEnemyId(const cv::Point2f& position, double timestamp);
    void UpdateTrajectory(EnemyTrajectory& trajectory, const EnemyPosition& position);
    
    void AnalyzeDeath(const cv::Point2f& deathPosition, double timestamp);
    std::vector<DeathAnalysis> GetDeathAnalyses() const;
    DeathAnalysis* FindDeathAnalysis(double timestamp);
    
    void CalculateMovementSpeed(EnemyTrajectory& trajectory);
    void PredictNextPosition(EnemyTrajectory& trajectory);
    std::string AnalyzeMovementPattern(const EnemyTrajectory& trajectory);
    
    bool WasEnemyVisibleAtTime(const std::string& enemyId, double timestamp);
    std::vector<EnemyPosition> GetEnemiesNearPosition(const cv::Point2f& position, double radius, double timestamp);
    
    void CleanupOldTrajectories(double currentTimestamp);
    void Reset();
    
    void PrintTrajectoryInfo() const;
    void PrintDeathAnalyses() const;
    void SaveTrajectoryData(const std::string& filename);
    void LoadTrajectoryData(const std::string& filename);
    
    void SetDeathAnalysisRadius(double radius);
    void SetVisibilityThreshold(double threshold);
};
