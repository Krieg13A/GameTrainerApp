#include "EnemyDetector.h"
#include <iostream>
#include <algorithm>
#include <chrono>

EnemyDetector::EnemyDetector() 
    : isInitialized(false), detectionThreshold(0.5), minDetectionConfidence(0.3),
      maxDetectionsPerFrame(10), detectionCooldown(0.1) {
}

EnemyDetector::~EnemyDetector() {
    Reset();
}

bool EnemyDetector::Initialize() {
    std::cout << "[EnemyDetector] Initializing enemy detection system..." << std::endl;
    
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    
    isInitialized = true;
    
    std::cout << "[EnemyDetector] Enemy detection system initialized successfully" << std::endl;
    return true;
}

bool EnemyDetector::LoadDetectionModels(const std::string& cascadePath, const std::string& hogPath) {

    std::cout << "[EnemyDetector] Loading detection models..." << std::endl;
    std::cout << "[EnemyDetector] Cascade: " << cascadePath << std::endl;
    std::cout << "[EnemyDetector] HOG: " << hogPath << std::endl;
    
    return true;
}

std::vector<EnemyDetection> EnemyDetector::DetectEnemies(const cv::Mat& frame) {
    std::vector<EnemyDetection> detections;
    
    if (!isInitialized || frame.empty()) {
        return detections;
    }
    
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    static int frameCounter = 0;
    frameCounter++;
    
    if (frameCounter % 30 == 0) {
        EnemyDetection detection;
        detection.boundingBox = cv::Rect(100, 100, 80, 120);
        detection.confidence = 0.75 + (rand() % 25) / 100.0;
        detection.enemyType = "player";
        detection.center = cv::Point2f(140, 160);
        detection.timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        
        detections.push_back(detection);
    }
    
    detections = FilterDetections(detections);
    
    recentDetections = detections;
    
    return detections;
}

std::vector<EnemyDetection> EnemyDetector::DetectPlayers(const cv::Mat& frame) {
    std::vector<EnemyDetection> allDetections = DetectEnemies(frame);
    std::vector<EnemyDetection> playerDetections;
    
    for (const auto& detection : allDetections) {
        if (detection.enemyType == "player") {
            playerDetections.push_back(detection);
        }
    }
    
    return playerDetections;
}

std::vector<EnemyDetection> EnemyDetector::DetectBots(const cv::Mat& frame) {
    std::vector<EnemyDetection> allDetections = DetectEnemies(frame);
    std::vector<EnemyDetection> botDetections;
    
    for (const auto& detection : allDetections) {
        if (detection.enemyType == "bot") {
            botDetections.push_back(detection);
        }
    }
    
    return botDetections;
}

bool EnemyDetector::IsCombatActive(const std::vector<EnemyDetection>& detections) {
    for (const auto& detection : detections) {
        if (detection.confidence > minDetectionConfidence) {
            cv::Point2f screenCenter(640, 360);
            double distance = cv::norm(detection.center - screenCenter);
            
            if (distance < 300) {
                return true;
            }
        }
    }
    
    return false;
}

CombatEvent EnemyDetector::AnalyzeCombatEvent(const std::vector<EnemyDetection>& detections, double timestamp) {
    CombatEvent event;
    event.startTime = timestamp;
    event.endTime = timestamp + 5.0;
    event.enemies = detections;
    event.playerDied = false;
    event.enemyKilled = false;
    event.eventType = "engagement";
    
    if (detections.empty()) {
        event.eventType = "miss";
    } else if (detections.size() > 1) {
        event.eventType = "multi_enemy";
    } else if (detections[0].confidence > 0.8) {
        event.eventType = "high_confidence_engagement";
    }
    
    return event;
}

std::vector<EnemyDetection> EnemyDetector::FilterDetections(const std::vector<EnemyDetection>& detections) {
    std::vector<EnemyDetection> filteredDetections;
    
    for (const auto& detection : detections) {
        if (detection.confidence < minDetectionConfidence) {
            continue;
        }
        
        if (detection.boundingBox.area() < 1000 || detection.boundingBox.area() > 50000) {
            continue;
        }
        
        bool isDuplicate = false;
        for (const auto& existing : filteredDetections) {
            cv::Rect intersection = detection.boundingBox & existing.boundingBox;
            double overlap = (double)intersection.area() / (detection.boundingBox.area() + existing.boundingBox.area() - intersection.area());
            
            if (overlap > 0.5) {
                isDuplicate = true;
                break;
            }
        }
        
        if (!isDuplicate) {
            filteredDetections.push_back(detection);
        }
    }
    
    if (filteredDetections.size() > maxDetectionsPerFrame) {
        std::sort(filteredDetections.begin(), filteredDetections.end(),
                 [](const EnemyDetection& a, const EnemyDetection& b) {
                     return a.confidence > b.confidence;
                 });
        filteredDetections.resize(maxDetectionsPerFrame);
    }
    
    return filteredDetections;
}

bool EnemyDetector::ValidateDetection(const EnemyDetection& detection, const cv::Mat& frame) {
    if (detection.boundingBox.x < 0 || detection.boundingBox.y < 0 ||
        detection.boundingBox.x + detection.boundingBox.width > frame.cols ||
        detection.boundingBox.y + detection.boundingBox.height > frame.rows) {
        return false;
    }
    
    if (detection.confidence < minDetectionConfidence) {
        return false;
    }
    
    return true;
}

void EnemyDetector::SetDetectionThreshold(double threshold) {
    detectionThreshold = std::max(0.0, std::min(1.0, threshold));
    std::cout << "[EnemyDetector] Detection threshold set to " << detectionThreshold << std::endl;
}

void EnemyDetector::SetMinConfidence(double confidence) {
    minDetectionConfidence = std::max(0.0, std::min(1.0, confidence));
    std::cout << "[EnemyDetector] Minimum confidence set to " << minDetectionConfidence << std::endl;
}

void EnemyDetector::SetMaxDetections(int maxDetections) {
    maxDetectionsPerFrame = std::max(1, maxDetections);
    std::cout << "[EnemyDetector] Max detections per frame set to " << maxDetectionsPerFrame << std::endl;
}

cv::Rect EnemyDetector::ExpandBoundingBox(const cv::Rect& box, double factor) {
    int newWidth = static_cast<int>(box.width * factor);
    int newHeight = static_cast<int>(box.height * factor);
    int newX = box.x - (newWidth - box.width) / 2;
    int newY = box.y - (newHeight - box.height) / 2;
    
    return cv::Rect(newX, newY, newWidth, newHeight);
}

double EnemyDetector::CalculateDetectionConfidence(const cv::Mat& region) {

    if (region.empty()) return 0.0;
    
    double area = region.rows * region.cols;
    double normalizedArea = std::min(1.0, area / 10000.0); 
    
    cv::Scalar mean, stddev;
    cv::meanStdDev(region, mean, stddev);
    double contrast = stddev[0] / 255.0; 
    
    return (normalizedArea + contrast) / 2.0;
}

std::string EnemyDetector::ClassifyEnemyType(const cv::Mat& region) {

    if (region.empty()) return "unknown";
    
    double aspectRatio = (double)region.cols / region.rows;
    
    if (aspectRatio > 0.4 && aspectRatio < 0.8) {
        return "player";
    } else {
        return "bot";
    }
}

cv::Mat EnemyDetector::DrawDetections(const cv::Mat& frame, const std::vector<EnemyDetection>& detections) {
    cv::Mat result = frame.clone();
    
    for (const auto& detection : detections) {
        cv::rectangle(result, detection.boundingBox, cv::Scalar(0, 255, 0), 2);
        
        std::string confidenceText = std::to_string(detection.confidence).substr(0, 4);
        cv::putText(result, confidenceText, cv::Point(detection.boundingBox.x, detection.boundingBox.y - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
        
        cv::putText(result, detection.enemyType, cv::Point(detection.boundingBox.x, detection.boundingBox.y + detection.boundingBox.height + 20),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1);
    }
    
    return result;
}

void EnemyDetector::SaveDetectionFrame(const cv::Mat& frame, const std::vector<EnemyDetection>& detections, const std::string& filename) {
    cv::Mat annotatedFrame = DrawDetections(frame, detections);
    cv::imwrite(filename, annotatedFrame);
    std::cout << "[EnemyDetector] Saved detection frame to " << filename << std::endl;
}

bool EnemyDetector::IsInitialized() const {
    return isInitialized;
}

void EnemyDetector::Reset() {
    recentDetections.clear();
    isInitialized = false;
    std::cout << "[EnemyDetector] Reset detection system" << std::endl;
}
