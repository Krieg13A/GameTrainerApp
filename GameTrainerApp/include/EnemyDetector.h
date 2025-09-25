#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>

struct EnemyDetection {
    cv::Rect boundingBox; 
    double confidence;
    std::string enemyType;
    cv::Point2f center;
    double timestamp;
};

struct CombatEvent {
    double startTime;
    double endTime;
    std::vector<EnemyDetection> enemies;
    bool playerDied;
    bool enemyKilled;
    std::string eventType; 
};

class EnemyDetector {
private:
    cv::CascadeClassifier playerCascade;
    cv::HOGDescriptor hog;
    bool isInitialized;
    double detectionThreshold;
    std::vector<EnemyDetection> recentDetections;
    
    double minDetectionConfidence;
    int maxDetectionsPerFrame;
    double detectionCooldown;
    
public:
    EnemyDetector();
    ~EnemyDetector();
    
    bool Initialize();
    bool LoadDetectionModels(const std::string& cascadePath, const std::string& hogPath);
    
    std::vector<EnemyDetection> DetectEnemies(const cv::Mat& frame);
    std::vector<EnemyDetection> DetectPlayers(const cv::Mat& frame);
    std::vector<EnemyDetection> DetectBots(const cv::Mat& frame);
    
    bool IsCombatActive(const std::vector<EnemyDetection>& detections);
    CombatEvent AnalyzeCombatEvent(const std::vector<EnemyDetection>& detections, double timestamp);
    
    std::vector<EnemyDetection> FilterDetections(const std::vector<EnemyDetection>& detections);
    bool ValidateDetection(const EnemyDetection& detection, const cv::Mat& frame);
    
    void SetDetectionThreshold(double threshold);
    void SetMinConfidence(double confidence);
    void SetMaxDetections(int maxDetections);
    
    cv::Rect ExpandBoundingBox(const cv::Rect& box, double factor = 1.2);
    double CalculateDetectionConfidence(const cv::Mat& region);
    std::string ClassifyEnemyType(const cv::Mat& region);
    
    cv::Mat DrawDetections(const cv::Mat& frame, const std::vector<EnemyDetection>& detections);
    void SaveDetectionFrame(const cv::Mat& frame, const std::vector<EnemyDetection>& detections, const std::string& filename);
    
    bool IsInitialized() const;
    void Reset();
};
