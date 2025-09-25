#include "GameplayAnalyzer.h"
#include "EventLogger.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cmath>

GameplayAnalyzer::GameplayAnalyzer() 
    : isAnalyzing(false) {
}

GameplayAnalyzer::~GameplayAnalyzer() {
    if (isAnalyzing) {
        EndAnalysis();
    }
}

void GameplayAnalyzer::StartAnalysis(const std::string& clipId) {
    currentClipId = clipId;
    shotAnalyses.clear();
    generalAnalyses.clear();
    isAnalyzing = true;
    
    std::cout << "[GameplayAnalyzer] Started analysis for clip: " << clipId << std::endl;
    std::cout << "[GameplayAnalyzer] Analyzing aim, recoil, positioning, and more..." << std::endl;
}

void GameplayAnalyzer::EndAnalysis() {
    if (isAnalyzing) {
        std::cout << "[GameplayAnalyzer] Completed analysis for clip: " << currentClipId << std::endl;
        std::cout << "[GameplayAnalyzer] Total shots analyzed: " << shotAnalyses.size() << std::endl;
        std::cout << "[GameplayAnalyzer] Total mistakes identified: " << generalAnalyses.size() << std::endl;
        
        std::string filename = currentClipId + "_analysis.csv";
        SaveAnalysisData(filename);
        
        isAnalyzing = false;
        currentClipId.clear();
    }
}

bool GameplayAnalyzer::IsAnalyzing() const {
    return isAnalyzing;
}

void GameplayAnalyzer::AnalyzeShot(const ShotAnalysis& shot) {
    if (!isAnalyzing) return;
    
    shotAnalyses.push_back(shot);
    
    if (shot.aimOffset > 10.0) {
        TechnicalAnalysis aimMistake = AnalyzeAim(shot.timestamp, shot.aimOffset, 20.0);
        AddMistakeToShot(shot.timestamp, aimMistake);
    }
    
    if (shot.recoilCompensation < 0.7 || shot.recoilCompensation > 1.3) {
        TechnicalAnalysis recoilMistake = AnalyzeRecoilCompensation(shot.timestamp, shot.recoilCompensation, 1.0);
        AddMistakeToShot(shot.timestamp, recoilMistake);
    }
    
    if (shot.reactionTime > 0.3) {
        TechnicalAnalysis reactionMistake = AnalyzeReactionTime(shot.timestamp, shot.reactionTime, 0.2);
        AddMistakeToShot(shot.timestamp, reactionMistake);
    }
    
    std::cout << "[GameplayAnalyzer] Analyzed shot at " << shot.timestamp << "s - " 
              << (shot.hit ? "HIT" : "MISS") << " - " << shot.mistakes.size() << " mistakes found" << std::endl;
}

void GameplayAnalyzer::AddMistakeToShot(double timestamp, const TechnicalAnalysis& mistake) {
    generalAnalyses.push_back(mistake);
    
    for (auto& shot : shotAnalyses) {
        if (std::abs(shot.timestamp - timestamp) < 0.1) {
            shot.mistakes.push_back(mistake);
            break;
        }
    }
}

TechnicalAnalysis GameplayAnalyzer::AnalyzeAim(double timestamp, double aimOffset, double targetSize) {
    TechnicalAnalysis analysis;
    analysis.timestamp = timestamp;
    analysis.type = AnalysisType::AIM_ANALYSIS;
    analysis.metrics["aim_offset"] = aimOffset;
    analysis.metrics["target_size"] = targetSize;
    
    double accuracy = std::max(0.0, 100.0 - (aimOffset / targetSize) * 100.0);
    analysis.metrics["accuracy_percentage"] = accuracy;
    
    if (aimOffset > targetSize * 2.0) {
        analysis.severity = MistakeSeverity::CRITICAL;
        analysis.description = "Severe aim error - completely missed target";
        analysis.explanation = "Your crosshair was " + std::to_string((int)aimOffset) + " pixels away from the target center";
        analysis.suggestion = "Practice tracking moving targets and improve muscle memory for precise aiming";
    } else if (aimOffset > targetSize) {
        analysis.severity = MistakeSeverity::MAJOR;
        analysis.description = "Major aim error - missed target by significant margin";
        analysis.explanation = "Aim was off by " + std::to_string((int)aimOffset) + " pixels, target size was " + std::to_string((int)targetSize);
        analysis.suggestion = "Focus on crosshair placement and practice micro-adjustments";
    } else if (aimOffset > targetSize * 0.5) {
        analysis.severity = MistakeSeverity::MODERATE;
        analysis.description = "Moderate aim error - hit edge of target";
        analysis.explanation = "Aim was " + std::to_string((int)aimOffset) + " pixels off center";
        analysis.suggestion = "Work on precision aiming and reduce mouse sensitivity if needed";
    } else {
        analysis.severity = MistakeSeverity::MINOR;
        analysis.description = "Minor aim error - close to center";
        analysis.explanation = "Small deviation of " + std::to_string((int)aimOffset) + " pixels from optimal";
        analysis.suggestion = "Good aim, continue practicing for consistency";
    }
    
    return analysis;
}

TechnicalAnalysis GameplayAnalyzer::AnalyzeRecoilCompensation(double timestamp, double compensation, double expectedCompensation) {
    TechnicalAnalysis analysis;
    analysis.timestamp = timestamp;
    analysis.type = AnalysisType::RECOIL_COMPENSATION;
    analysis.metrics["actual_compensation"] = compensation;
    analysis.metrics["expected_compensation"] = expectedCompensation;
    
    double compensationError = std::abs(compensation - expectedCompensation);
    analysis.metrics["compensation_error"] = compensationError;
    
    if (compensationError > 0.5) {
        analysis.severity = MistakeSeverity::MAJOR;
        analysis.description = "Poor recoil control - significant over/under compensation";
        analysis.explanation = "Compensated " + std::to_string(compensation) + "x when " + std::to_string(expectedCompensation) + "x was needed";
        analysis.suggestion = "Practice recoil patterns in training mode and memorize weapon-specific compensation";
    } else if (compensationError > 0.3) {
        analysis.severity = MistakeSeverity::MODERATE;
        analysis.description = "Moderate recoil control issue";
        analysis.explanation = "Compensation was " + std::to_string(compensationError) + "x off from optimal";
        analysis.suggestion = "Focus on consistent recoil control and timing";
    } else {
        analysis.severity = MistakeSeverity::MINOR;
        analysis.description = "Good recoil control";
        analysis.explanation = "Compensation was close to optimal";
        analysis.suggestion = "Maintain current recoil control technique";
    }
    
    return analysis;
}

TechnicalAnalysis GameplayAnalyzer::AnalyzeDistanceCalculation(double timestamp, double actualDistance, double calculatedDistance) {
    TechnicalAnalysis analysis;
    analysis.timestamp = timestamp;
    analysis.type = AnalysisType::DISTANCE_CALCULATION;
    analysis.metrics["actual_distance"] = actualDistance;
    analysis.metrics["calculated_distance"] = calculatedDistance;
    
    double distanceError = std::abs(actualDistance - calculatedDistance);
    analysis.metrics["distance_error"] = distanceError;
    
    if (distanceError > actualDistance * 0.3) {
        analysis.severity = MistakeSeverity::MAJOR;
        analysis.description = "Poor distance calculation - significant lead error";
        analysis.explanation = "Calculated " + std::to_string(calculatedDistance) + " units lead, actual was " + std::to_string(actualDistance);
        analysis.suggestion = "Practice estimating target speed and distance, use visual cues for better calculation";
    } else if (distanceError > actualDistance * 0.15) {
        analysis.severity = MistakeSeverity::MODERATE;
        analysis.description = "Moderate distance calculation error";
        analysis.explanation = "Distance calculation was " + std::to_string(distanceError) + " units off";
        analysis.suggestion = "Improve target speed estimation and practice leading moving targets";
    } else {
        analysis.severity = MistakeSeverity::MINOR;
        analysis.description = "Good distance calculation";
        analysis.explanation = "Distance calculation was accurate";
        analysis.suggestion = "Continue practicing for consistency";
    }
    
    return analysis;
}

TechnicalAnalysis GameplayAnalyzer::AnalyzePositioning(double timestamp, const std::string& position, const std::string& optimalPosition) {
    TechnicalAnalysis analysis;
    analysis.timestamp = timestamp;
    analysis.type = AnalysisType::POSITIONING;
    
    if (position != optimalPosition) {
        analysis.severity = MistakeSeverity::MODERATE;
        analysis.description = "Suboptimal positioning";
        analysis.explanation = "You were at " + position + " when " + optimalPosition + " would have been better";
        analysis.suggestion = "Study map layouts and practice positioning for better angles and cover";
    } else {
        analysis.severity = MistakeSeverity::MINOR;
        analysis.description = "Good positioning";
        analysis.explanation = "Position was optimal for the situation";
        analysis.suggestion = "Maintain good positioning awareness";
    }
    
    return analysis;
}

TechnicalAnalysis GameplayAnalyzer::AnalyzeReactionTime(double timestamp, double reactionTime, double averageReactionTime) {
    TechnicalAnalysis analysis;
    analysis.timestamp = timestamp;
    analysis.type = AnalysisType::REACTION_TIME;
    analysis.metrics["reaction_time"] = reactionTime;
    analysis.metrics["average_reaction_time"] = averageReactionTime;
    
    if (reactionTime > averageReactionTime * 1.5) {
        analysis.severity = MistakeSeverity::MAJOR;
        analysis.description = "Slow reaction time";
        analysis.explanation = "Reacted in " + std::to_string(reactionTime) + "s (average: " + std::to_string(averageReactionTime) + "s)";
        analysis.suggestion = "Practice reaction time exercises and ensure good sleep/focus";
    } else if (reactionTime > averageReactionTime * 1.2) {
        analysis.severity = MistakeSeverity::MODERATE;
        analysis.description = "Below average reaction time";
        analysis.explanation = "Reaction was slower than usual";
        analysis.suggestion = "Focus on anticipation and pre-aiming common angles";
    } else {
        analysis.severity = MistakeSeverity::MINOR;
        analysis.description = "Good reaction time";
        analysis.explanation = "Reacted quickly to the situation";
        analysis.suggestion = "Maintain good reaction time with practice";
    }
    
    return analysis;
}

TechnicalAnalysis GameplayAnalyzer::AnalyzeCrosshairPlacement(double timestamp, double crosshairOffset, double optimalPlacement) {
    TechnicalAnalysis analysis;
    analysis.timestamp = timestamp;
    analysis.type = AnalysisType::CROSSHAIR_PLACEMENT;
    analysis.metrics["crosshair_offset"] = crosshairOffset;
    analysis.metrics["optimal_placement"] = optimalPlacement;
    
    if (crosshairOffset > 50.0) {
        analysis.severity = MistakeSeverity::MAJOR;
        analysis.description = "Poor crosshair placement";
        analysis.explanation = "Crosshair was " + std::to_string(crosshairOffset) + " pixels from optimal head level";
        analysis.suggestion = "Practice keeping crosshair at head level and common angles";
    } else if (crosshairOffset > 25.0) {
        analysis.severity = MistakeSeverity::MODERATE;
        analysis.description = "Suboptimal crosshair placement";
        analysis.explanation = "Crosshair placement could be improved";
        analysis.suggestion = "Focus on head-level crosshair placement";
    } else {
        analysis.severity = MistakeSeverity::MINOR;
        analysis.description = "Good crosshair placement";
        analysis.explanation = "Crosshair was well positioned";
        analysis.suggestion = "Maintain good crosshair placement habits";
    }
    
    return analysis;
}

std::vector<ShotAnalysis> GameplayAnalyzer::GetShotAnalyses() const {
    return shotAnalyses;
}

std::vector<TechnicalAnalysis> GameplayAnalyzer::GetGeneralAnalyses() const {
    return generalAnalyses;
}

std::map<AnalysisType, int> GameplayAnalyzer::GetMistakeStats() const {
    std::map<AnalysisType, int> stats;
    for (const auto& analysis : generalAnalyses) {
        stats[analysis.type]++;
    }
    return stats;
}

std::map<MistakeSeverity, int> GameplayAnalyzer::GetSeverityStats() const {
    std::map<MistakeSeverity, int> stats;
    for (const auto& analysis : generalAnalyses) {
        stats[analysis.severity]++;
    }
    return stats;
}

std::string GameplayAnalyzer::GenerateDetailedFeedback() const {
    std::ostringstream feedback;
    feedback << "\n=== DETAILED TECHNICAL FEEDBACK ===\n";
    
    auto mistakeStats = GetMistakeStats();
    auto severityStats = GetSeverityStats();
    
    feedback << "Total Mistakes Identified: " << generalAnalyses.size() << "\n";
    feedback << "Critical Issues: " << severityStats[MistakeSeverity::CRITICAL] << "\n";
    feedback << "Major Issues: " << severityStats[MistakeSeverity::MAJOR] << "\n";
    feedback << "Moderate Issues: " << severityStats[MistakeSeverity::MODERATE] << "\n";
    feedback << "Minor Issues: " << severityStats[MistakeSeverity::MINOR] << "\n\n";
    
    feedback << "Mistake Breakdown:\n";
    for (const auto& [type, count] : mistakeStats) {
        feedback << "- " << GetAnalysisTypeString(type) << ": " << count << " issues\n";
    }
    
    feedback << "\nTop Issues to Focus On:\n";
    int issueCount = 0;
    for (const auto& analysis : generalAnalyses) {
        if (analysis.severity >= MistakeSeverity::MAJOR && issueCount < 3) {
            feedback << (issueCount + 1) << ". " << analysis.description << "\n";
            feedback << "   " << analysis.explanation << "\n";
            feedback << "   Fix: " << analysis.suggestion << "\n\n";
            issueCount++;
        }
    }
    
    return feedback.str();
}

std::string GameplayAnalyzer::GenerateImprovementPlan() const {
    std::ostringstream plan;
    plan << "\n=== IMPROVEMENT PLAN ===\n";
    
    auto mistakeStats = GetMistakeStats();
    
    plan << "Priority Training Areas:\n";
    if (mistakeStats[AnalysisType::AIM_ANALYSIS] > 0) {
        plan << "1. AIM TRAINING\n";
        plan << "   - Practice precision aiming in training mode\n";
        plan << "   - Work on micro-adjustments and tracking\n";
        plan << "   - Consider adjusting mouse sensitivity\n\n";
    }
    
    if (mistakeStats[AnalysisType::RECOIL_COMPENSATION] > 0) {
        plan << "2. RECOIL CONTROL\n";
        plan << "   - Learn weapon-specific recoil patterns\n";
        plan << "   - Practice spray control in training\n";
        plan << "   - Focus on consistent compensation timing\n\n";
    }
    
    if (mistakeStats[AnalysisType::DISTANCE_CALCULATION] > 0) {
        plan << "3. LEADING TARGETS\n";
        plan << "   - Practice estimating target speed\n";
        plan << "   - Work on distance calculation\n";
        plan << "   - Study common movement patterns\n\n";
    }
    
    if (mistakeStats[AnalysisType::REACTION_TIME] > 0) {
        plan << "4. REACTION TIME\n";
        plan << "   - Practice reaction time exercises\n";
        plan << "   - Improve anticipation skills\n";
        plan << "   - Ensure good sleep and focus\n\n";
    }
    
    return plan.str();
}

std::string GameplayAnalyzer::GeneratePerformanceSummary() const {
    std::ostringstream summary;
    summary << "\n=== PERFORMANCE SUMMARY ===\n";
    
    int totalShots = shotAnalyses.size();
    int hits = 0;
    double totalAccuracy = 0.0;
    
    for (const auto& shot : shotAnalyses) {
        if (shot.hit) hits++;
        totalAccuracy += shot.aimOffset;
    }
    
    double hitRate = totalShots > 0 ? (double)hits / totalShots * 100.0 : 0.0;
    double avgAccuracy = totalShots > 0 ? totalAccuracy / totalShots : 0.0;
    
    summary << "Total Shots: " << totalShots << "\n";
    summary << "Hits: " << hits << " (" << std::fixed << std::setprecision(1) << hitRate << "%)\n";
    summary << "Average Aim Error: " << avgAccuracy << " pixels\n";
    summary << "Total Mistakes: " << generalAnalyses.size() << "\n";
    
    return summary.str();
}

void GameplayAnalyzer::SaveAnalysisData(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[GameplayAnalyzer] Failed to save analysis data to " << filename << std::endl;
        return;
    }
    
    file << "timestamp,type,severity,description,explanation,suggestion\n";
    for (const auto& analysis : generalAnalyses) {
        file << analysis.timestamp << ","
             << GetAnalysisTypeString(analysis.type) << ","
             << GetSeverityString(analysis.severity) << ","
             << "\"" << analysis.description << "\","
             << "\"" << analysis.explanation << "\","
             << "\"" << analysis.suggestion << "\"\n";
    }
    
    file.close();
    std::cout << "[GameplayAnalyzer] Saved analysis data to " << filename << std::endl;
}

void GameplayAnalyzer::LoadAnalysisData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "[GameplayAnalyzer] No existing analysis data found" << std::endl;
        return;
    }
    
    generalAnalyses.clear();
    std::string line;
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string timestampStr, typeStr, severityStr, description, explanation, suggestion;
        
        if (std::getline(iss, timestampStr, ',') &&
            std::getline(iss, typeStr, ',') &&
            std::getline(iss, severityStr, ',')) {
            
            TechnicalAnalysis analysis;
            analysis.timestamp = std::stod(timestampStr);

            generalAnalyses.push_back(analysis);
        }
    }
    
    file.close();
    std::cout << "[GameplayAnalyzer] Loaded " << generalAnalyses.size() 
              << " analysis entries from " << filename << std::endl;
}

std::string GameplayAnalyzer::GetAnalysisTypeString(AnalysisType type) const {
    switch (type) {
        case AnalysisType::AIM_ANALYSIS: return "AIM_ANALYSIS";
        case AnalysisType::RECOIL_COMPENSATION: return "RECOIL_COMPENSATION";
        case AnalysisType::DISTANCE_CALCULATION: return "DISTANCE_CALCULATION";
        case AnalysisType::POSITIONING: return "POSITIONING";
        case AnalysisType::REACTION_TIME: return "REACTION_TIME";
        case AnalysisType::CROSSHAIR_PLACEMENT: return "CROSSHAIR_PLACEMENT";
        default: return "UNKNOWN";
    }
}

std::string GameplayAnalyzer::GetSeverityString(MistakeSeverity severity) const {
    switch (severity) {
        case MistakeSeverity::MINOR: return "MINOR";
        case MistakeSeverity::MODERATE: return "MODERATE";
        case MistakeSeverity::MAJOR: return "MAJOR";
        case MistakeSeverity::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string GameplayAnalyzer::GetClipId() const {
    return currentClipId;
}
