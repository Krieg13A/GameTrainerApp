#include "PositionTracker.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>

PositionTracker::PositionTracker() 
    : nextEnemyId(1), maxTrackingDistance(100.0), trajectoryTimeout(5.0),
      minPositionsForTrajectory(3), deathAnalysisRadius(200.0), visibilityThreshold(0.5) {
}

PositionTracker::~PositionTracker() {
    Reset();
}

void PositionTracker::Initialize() {
    std::cout << "[PositionTracker] Initializing enemy position tracking..." << std::endl;
    Reset();
    std::cout << "[PositionTracker] Position tracking initialized" << std::endl;
}

void PositionTracker::SetTrackingDistance(double distance) {
    maxTrackingDistance = std::max(10.0, distance);
    std::cout << "[PositionTracker] Max tracking distance set to " << maxTrackingDistance << " pixels" << std::endl;
}

void PositionTracker::SetTrajectoryTimeout(double timeout) {
    trajectoryTimeout = std::max(0.5, timeout);
    std::cout << "[PositionTracker] Trajectory timeout set to " << trajectoryTimeout << " seconds" << std::endl;
}

void PositionTracker::SetMinPositionsForTrajectory(int minPositions) {
    minPositionsForTrajectory = std::max(1, minPositions);
    std::cout << "[PositionTracker] Min positions for trajectory set to " << minPositionsForTrajectory << std::endl;
}

void PositionTracker::UpdateEnemyPositions(const std::vector<EnemyDetection>& detections, double timestamp) {
    CleanupOldTrajectories(timestamp);
    
    for (const auto& detection : detections) {
        EnemyPosition position;
        position.position = detection.center;
        position.timestamp = timestamp;
        position.confidence = detection.confidence;
        position.isVisible = true;
        
        EnemyTrajectory* trajectory = FindEnemyTrajectory(position.position, timestamp);
        
        if (trajectory) {
            position.enemyId = trajectory->enemyId;
            UpdateTrajectory(*trajectory, position);
        } else {
            position.enemyId = AssignEnemyId(position.position, timestamp);
            
            EnemyTrajectory newTrajectory;
            newTrajectory.enemyId = position.enemyId;
            newTrajectory.positions.push_back(position);
            newTrajectory.firstSeen = timestamp;
            newTrajectory.lastSeen = timestamp;
            newTrajectory.isActive = true;
            newTrajectory.movementSpeed = 0.0;
            newTrajectory.movementPattern = "stationary";
            
            enemyTrajectories[position.enemyId] = newTrajectory;
        }
    }
    
    for (auto& [enemyId, trajectory] : enemyTrajectories) {
        if (trajectory.isActive && trajectory.positions.size() >= minPositionsForTrajectory) {
            CalculateMovementSpeed(trajectory);
            trajectory.movementPattern = AnalyzeMovementPattern(trajectory);
            PredictNextPosition(trajectory);
        }
    }
}

std::vector<EnemyTrajectory> PositionTracker::GetActiveTrajectories() const {
    std::vector<EnemyTrajectory> activeTrajectories;
    
    for (const auto& [enemyId, trajectory] : enemyTrajectories) {
        if (trajectory.isActive) {
            activeTrajectories.push_back(trajectory);
        }
    }
    
    return activeTrajectories;
}

std::vector<EnemyTrajectory> PositionTracker::GetAllTrajectories() const {
    std::vector<EnemyTrajectory> allTrajectories;
    
    for (const auto& [enemyId, trajectory] : enemyTrajectories) {
        allTrajectories.push_back(trajectory);
    }
    
    return allTrajectories;
}

EnemyTrajectory* PositionTracker::FindEnemyTrajectory(const cv::Point2f& position, double timestamp) {
    EnemyTrajectory* closestTrajectory = nullptr;
    double closestDistance = maxTrackingDistance;
    
    for (auto& [enemyId, trajectory] : enemyTrajectories) {
        if (!trajectory.isActive) continue;
        
        if (timestamp - trajectory.lastSeen > trajectoryTimeout) {
            trajectory.isActive = false;
            continue;
        }
        
        for (const auto& pos : trajectory.positions) {
            double distance = cv::norm(position - pos.position);
            if (distance < closestDistance) {
                closestDistance = distance;
                closestTrajectory = &trajectory;
            }
        }
    }
    
    return closestTrajectory;
}

std::string PositionTracker::AssignEnemyId(const cv::Point2f& position, double timestamp) {
    std::ostringstream oss;
    oss << "enemy_" << nextEnemyId++;
    return oss.str();
}

void PositionTracker::UpdateTrajectory(EnemyTrajectory& trajectory, const EnemyPosition& position) {
    trajectory.positions.push_back(position);
    trajectory.lastSeen = position.timestamp;
    
    double cutoffTime = position.timestamp - 10.0;
    trajectory.positions.erase(
        std::remove_if(trajectory.positions.begin(), trajectory.positions.end(),
                      [cutoffTime](const EnemyPosition& pos) {
                          return pos.timestamp < cutoffTime;
                      }),
        trajectory.positions.end()
    );
}

void PositionTracker::AnalyzeDeath(const cv::Point2f& deathPosition, double timestamp) {
    DeathAnalysis analysis;
    analysis.deathTime = timestamp;
    analysis.deathPosition = deathPosition;
    analysis.deathCause = "unknown";
    analysis.enemyDistance = -1.0;
    analysis.enemyWeapon = "unknown";
    analysis.enemyWasVisible = false;
    
    analysis.nearbyEnemies = GetEnemiesNearPosition(deathPosition, deathAnalysisRadius, timestamp);
    
    if (!analysis.nearbyEnemies.empty()) {
        double minDistance = std::numeric_limits<double>::max();
        EnemyPosition closestEnemy;
        
        for (const auto& enemy : analysis.nearbyEnemies) {
            double distance = cv::norm(deathPosition - enemy.position);
            if (distance < minDistance) {
                minDistance = distance;
                closestEnemy = enemy;
            }
        }
        
        analysis.enemyDistance = minDistance;
        analysis.enemyWasVisible = closestEnemy.isVisible;
        
        if (analysis.enemyWasVisible && analysis.enemyDistance < 100.0) {
            analysis.deathCause = "enemy_shot";
            analysis.enemyWeapon = "unknown";
        } else if (analysis.enemyDistance < 200.0) {
            analysis.deathCause = "enemy_shot_unseen";
        }
    }
    
    deathAnalyses.push_back(analysis);
    
    std::cout << "[PositionTracker] Death analyzed at " << timestamp << "s" << std::endl;
    std::cout << "[PositionTracker] Cause: " << analysis.deathCause << std::endl;
    std::cout << "[PositionTracker] Enemy distance: " << analysis.enemyDistance << " pixels" << std::endl;
    std::cout << "[PositionTracker] Enemy visible: " << (analysis.enemyWasVisible ? "YES" : "NO") << std::endl;
}

std::vector<DeathAnalysis> PositionTracker::GetDeathAnalyses() const {
    return deathAnalyses;
}

DeathAnalysis* PositionTracker::FindDeathAnalysis(double timestamp) {
    for (auto& analysis : deathAnalyses) {
        if (std::abs(analysis.deathTime - timestamp) < 1.0) {
            return &analysis;
        }
    }
    return nullptr;
}

void PositionTracker::CalculateMovementSpeed(EnemyTrajectory& trajectory) {
    if (trajectory.positions.size() < 2) {
        trajectory.movementSpeed = 0.0;
        return;
    }
    
    double totalDistance = 0.0;
    double totalTime = 0.0;
    
    for (size_t i = 1; i < trajectory.positions.size(); ++i) {
        const auto& prev = trajectory.positions[i - 1];
        const auto& curr = trajectory.positions[i];
        
        double distance = cv::norm(curr.position - prev.position);
        double timeDiff = curr.timestamp - prev.timestamp;
        
        if (timeDiff > 0) {
            totalDistance += distance;
            totalTime += timeDiff;
        }
    }
    
    trajectory.movementSpeed = totalTime > 0 ? totalDistance / totalTime : 0.0;
}

void PositionTracker::PredictNextPosition(EnemyTrajectory& trajectory) {
    if (trajectory.positions.size() < 2) {
        trajectory.predictedNextPosition = trajectory.positions.empty() ? 
            cv::Point2f(0, 0) : trajectory.positions.back().position;
        return;
    }
    
    const auto& lastPos = trajectory.positions.back();
    const auto& secondLastPos = trajectory.positions[trajectory.positions.size() - 2];
    
    cv::Point2f velocity = lastPos.position - secondLastPos.position;
    double timeDiff = lastPos.timestamp - secondLastPos.timestamp;
    
    if (timeDiff > 0) {
        velocity *= (1.0 / timeDiff);
        trajectory.predictedNextPosition = lastPos.position + velocity * 0.1; 
    } else {
        trajectory.predictedNextPosition = lastPos.position;
    }
}

std::string PositionTracker::AnalyzeMovementPattern(const EnemyTrajectory& trajectory) {
    if (trajectory.positions.size() < minPositionsForTrajectory) {
        return "insufficient_data";
    }
    
    double totalVariance = 0.0;
    cv::Point2f avgPosition(0, 0);
    
    for (const auto& pos : trajectory.positions) {
        avgPosition += pos.position;
    }
    avgPosition *= (1.0 / trajectory.positions.size());
    
    for (const auto& pos : trajectory.positions) {
        cv::Point2f diff = pos.position - avgPosition;
        totalVariance += cv::norm(diff);
    }
    totalVariance /= trajectory.positions.size();
    
    if (trajectory.movementSpeed < 5.0) {
        return "stationary";
    } else if (totalVariance < 20.0) {
        return "moving_straight";
    } else if (totalVariance > 50.0) {
        return "erratic";
    } else {
        return "moving";
    }
}

bool PositionTracker::WasEnemyVisibleAtTime(const std::string& enemyId, double timestamp) {
    auto it = enemyTrajectories.find(enemyId);
    if (it == enemyTrajectories.end()) {
        return false;
    }
    
    const auto& trajectory = it->second;
    
    for (const auto& pos : trajectory.positions) {
        if (std::abs(pos.timestamp - timestamp) < 0.5 && pos.isVisible) {
            return true;
        }
    }
    
    return false;
}

std::vector<EnemyPosition> PositionTracker::GetEnemiesNearPosition(const cv::Point2f& position, double radius, double timestamp) {
    std::vector<EnemyPosition> nearbyEnemies;
    
    for (const auto& [enemyId, trajectory] : enemyTrajectories) {
        for (const auto& pos : trajectory.positions) {
            if (std::abs(pos.timestamp - timestamp) < 1.0) {
                double distance = cv::norm(position - pos.position);
                if (distance <= radius) {
                    nearbyEnemies.push_back(pos);
                }
            }
        }
    }
    
    return nearbyEnemies;
}

void PositionTracker::CleanupOldTrajectories(double currentTimestamp) {
    for (auto& [enemyId, trajectory] : enemyTrajectories) {
        if (currentTimestamp - trajectory.lastSeen > trajectoryTimeout) {
            trajectory.isActive = false;
        }
    }
}

void PositionTracker::Reset() {
    enemyTrajectories.clear();
    deathAnalyses.clear();
    nextEnemyId = 1;
}

void PositionTracker::PrintTrajectoryInfo() const {
    std::cout << "\n=== ENEMY TRAJECTORIES ===" << std::endl;
    std::cout << "Total trajectories: " << enemyTrajectories.size() << std::endl;
    
    int activeCount = 0;
    for (const auto& [enemyId, trajectory] : enemyTrajectories) {
        if (trajectory.isActive) activeCount++;
        
        std::cout << "Enemy " << enemyId << ":" << std::endl;
        std::cout << "  Active: " << (trajectory.isActive ? "YES" : "NO") << std::endl;
        std::cout << "  Positions: " << trajectory.positions.size() << std::endl;
        std::cout << "  Movement Speed: " << trajectory.movementSpeed << " pixels/s" << std::endl;
        std::cout << "  Pattern: " << trajectory.movementPattern << std::endl;
        std::cout << "  First Seen: " << trajectory.firstSeen << "s" << std::endl;
        std::cout << "  Last Seen: " << trajectory.lastSeen << "s" << std::endl;
    }
    
    std::cout << "Active trajectories: " << activeCount << std::endl;
    std::cout << std::endl;
}

void PositionTracker::PrintDeathAnalyses() const {
    std::cout << "\n=== DEATH ANALYSES ===" << std::endl;
    std::cout << "Total deaths analyzed: " << deathAnalyses.size() << std::endl;
    
    for (size_t i = 0; i < deathAnalyses.size(); ++i) {
        const auto& analysis = deathAnalyses[i];
        std::cout << "Death " << (i + 1) << ":" << std::endl;
        std::cout << "  Time: " << analysis.deathTime << "s" << std::endl;
        std::cout << "  Position: (" << analysis.deathPosition.x << ", " << analysis.deathPosition.y << ")" << std::endl;
        std::cout << "  Cause: " << analysis.deathCause << std::endl;
        std::cout << "  Enemy Distance: " << analysis.enemyDistance << " pixels" << std::endl;
        std::cout << "  Enemy Visible: " << (analysis.enemyWasVisible ? "YES" : "NO") << std::endl;
        std::cout << "  Nearby Enemies: " << analysis.nearbyEnemies.size() << std::endl;
    }
    std::cout << std::endl;
}

void PositionTracker::SaveTrajectoryData(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[PositionTracker] Failed to save trajectory data to " << filename << std::endl;
        return;
    }
    
    file << "enemy_id,timestamp,x,y,confidence,is_visible\n";
    
    for (const auto& [enemyId, trajectory] : enemyTrajectories) {
        for (const auto& pos : trajectory.positions) {
            file << enemyId << ","
                 << pos.timestamp << ","
                 << pos.position.x << ","
                 << pos.position.y << ","
                 << pos.confidence << ","
                 << (pos.isVisible ? "true" : "false") << "\n";
        }
    }
    
    file.close();
    std::cout << "[PositionTracker] Saved trajectory data to " << filename << std::endl;
}

void PositionTracker::LoadTrajectoryData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "[PositionTracker] No existing trajectory data found" << std::endl;
        return;
    }
    
    std::string line;
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string enemyId, timestampStr, xStr, yStr, confidenceStr, visibleStr;
        
        if (std::getline(iss, enemyId, ',') &&
            std::getline(iss, timestampStr, ',') &&
            std::getline(iss, xStr, ',') &&
            std::getline(iss, yStr, ',') &&
            std::getline(iss, confidenceStr, ',') &&
            std::getline(iss, visibleStr)) {
            
            EnemyPosition pos;
            pos.enemyId = enemyId;
            pos.timestamp = std::stod(timestampStr);
            pos.position.x = std::stof(xStr);
            pos.position.y = std::stof(yStr);
            pos.confidence = std::stod(confidenceStr);
            pos.isVisible = (visibleStr == "true");
            
            if (enemyTrajectories.find(enemyId) == enemyTrajectories.end()) {
                EnemyTrajectory trajectory;
                trajectory.enemyId = enemyId;
                trajectory.firstSeen = pos.timestamp;
                trajectory.lastSeen = pos.timestamp;
                trajectory.isActive = false;
                enemyTrajectories[enemyId] = trajectory;
            }
            
            enemyTrajectories[enemyId].positions.push_back(pos);
        }
    }
    
    file.close();
    std::cout << "[PositionTracker] Loaded trajectory data from " << filename << std::endl;
}

void PositionTracker::SetDeathAnalysisRadius(double radius) {
    deathAnalysisRadius = std::max(10.0, radius);
    std::cout << "[PositionTracker] Death analysis radius set to " << deathAnalysisRadius << " pixels" << std::endl;
}

void PositionTracker::SetVisibilityThreshold(double threshold) {
    visibilityThreshold = std::max(0.0, std::min(1.0, threshold));
    std::cout << "[PositionTracker] Visibility threshold set to " << visibilityThreshold << std::endl;
}
