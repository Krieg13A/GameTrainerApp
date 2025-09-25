#include "ReviewInterface.h"
#include <iostream>
#include <iomanip>

ReviewInterface::ReviewInterface() 
    : currentClipIndex(-1), currentPlaybackTime(0.0), isPlaying(false) {
}

ReviewInterface::~ReviewInterface() {
    if (concentrationTracker.IsInReviewMode()) {
        concentrationTracker.EndReview();
    }
    if (gameplayAnalyzer.IsAnalyzing()) {
        gameplayAnalyzer.EndAnalysis();
    }
}

void ReviewInterface::LoadClips(const std::string& sessionId) {
    
    clips.clear();
    
    GameplayClip clip1;
    clip1.clipId = sessionId + "_death_001";
    clip1.filename = "death_clip_001.mp4";
    clip1.duration = 15.5;
    clip1.description = "Death at 2:15 - Enemy behind cover";
    clip1.timestamp = 135.0;
    
    ShotAnalysis shot1;
    shot1.timestamp = 5.2;
    shot1.hit = false;
    shot1.aimOffset = 25.5;
    shot1.recoilCompensation = 0.8;
    shot1.distanceToTarget = 150.0;
    shot1.reactionTime = 0.35;
    shot1.weaponType = "AK-47";
    clip1.shots.push_back(shot1);
    
    ShotAnalysis shot2;
    shot2.timestamp = 8.7;
    shot2.hit = false;
    shot2.aimOffset = 45.2;
    shot2.recoilCompensation = 1.4;
    shot2.distanceToTarget = 120.0;
    shot2.reactionTime = 0.28;
    shot2.weaponType = "AK-47";
    clip1.shots.push_back(shot2);
    
    clips.push_back(clip1);
    
    GameplayClip clip2;
    clip2.clipId = sessionId + "_miss_001";
    clip2.filename = "miss_clip_001.mp4";
    clip2.duration = 8.2;
    clip2.description = "Missed shot at 1:30 - Poor aim";
    clip2.timestamp = 90.0;
    
    ShotAnalysis shot3;
    shot3.timestamp = 3.1;
    shot3.hit = false;
    shot3.aimOffset = 60.8;
    shot3.recoilCompensation = 0.6;
    shot3.distanceToTarget = 200.0;
    shot3.reactionTime = 0.42;
    shot3.weaponType = "AWP";
    clip2.shots.push_back(shot3);
    
    clips.push_back(clip2);
    
    GameplayClip clip3;
    clip3.clipId = sessionId + "_kill_001";
    clip3.filename = "kill_clip_001.mp4";
    clip3.duration = 12.0;
    clip3.description = "Successful kill at 3:45 - Good positioning";
    clip3.timestamp = 225.0;
    
    ShotAnalysis shot4;
    shot4.timestamp = 4.5;
    shot4.hit = true;
    shot4.aimOffset = 8.2;
    shot4.recoilCompensation = 1.0;
    shot4.distanceToTarget = 80.0;
    shot4.reactionTime = 0.18;
    shot4.weaponType = "M4A4";
    clip3.shots.push_back(shot4);
    
    clips.push_back(clip3);
    
    std::cout << "[ReviewInterface] Loaded " << clips.size() << " clips for session: " << sessionId << std::endl;
}

void ReviewInterface::AddClip(const GameplayClip& clip) {
    clips.push_back(clip);
}

std::vector<GameplayClip> ReviewInterface::GetClips() const {
    return clips;
}

void ReviewInterface::PlayClip(int clipIndex) {
    if (clipIndex < 0 || clipIndex >= clips.size()) {
        std::cout << "[ReviewInterface] Invalid clip index" << std::endl;
        return;
    }
    
    currentClipIndex = clipIndex;
    currentPlaybackTime = 0.0;
    isPlaying = true;
    
    const GameplayClip& clip = clips[currentClipIndex];
    concentrationTracker.StartReview(clip.clipId);
    gameplayAnalyzer.StartAnalysis(clip.clipId);
    
    for (const auto& shot : clip.shots) {
        gameplayAnalyzer.AnalyzeShot(shot);
    }
    
    std::cout << "\n=== PLAYING CLIP " << (clipIndex + 1) << " ===" << std::endl;
    std::cout << "Description: " << clip.description << std::endl;
    std::cout << "Duration: " << clip.duration << " seconds" << std::endl;
    std::cout << "File: " << clip.filename << std::endl;
    std::cout << "Shots in clip: " << clip.shots.size() << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  1 = Mark Low Focus at current time" << std::endl;
    std::cout << "  2 = Mark Medium Focus at current time" << std::endl;
    std::cout << "  3 = Mark High Focus at current time" << std::endl;
    std::cout << "  a = Analyze current moment (AI feedback)" << std::endl;
    std::cout << "  s = Show concentration timeline" << std::endl;
    std::cout << "  t = Show technical analysis" << std::endl;
    std::cout << "  n = Next clip" << std::endl;
    std::cout << "  p = Previous clip" << std::endl;
    std::cout << "  q = Quit review" << std::endl;
    std::cout << "\nPlayback: " << std::fixed << std::setprecision(1) 
              << currentPlaybackTime << "s / " << clip.duration << "s" << std::endl;
}

void ReviewInterface::PauseClip() {
    isPlaying = false;
    std::cout << "[ReviewInterface] Playback paused" << std::endl;
}

void ReviewInterface::SeekToTime(double timestamp) {
    if (currentClipIndex >= 0 && currentClipIndex < clips.size()) {
        currentPlaybackTime = timestamp;
        std::cout << "[ReviewInterface] Seeked to " << timestamp << "s" << std::endl;
    }
}

void ReviewInterface::NextClip() {
    if (currentClipIndex < clips.size() - 1) {
        if (concentrationTracker.IsInReviewMode()) {
            concentrationTracker.EndReview();
        }
        if (gameplayAnalyzer.IsAnalyzing()) {
            gameplayAnalyzer.EndAnalysis();
        }
        PlayClip(currentClipIndex + 1);
    } else {
        std::cout << "[ReviewInterface] No more clips" << std::endl;
    }
}

void ReviewInterface::PreviousClip() {
    if (currentClipIndex > 0) {
        if (concentrationTracker.IsInReviewMode()) {
            concentrationTracker.EndReview();
        }
        if (gameplayAnalyzer.IsAnalyzing()) {
            gameplayAnalyzer.EndAnalysis();
        }
        PlayClip(currentClipIndex - 1);
    } else {
        std::cout << "[ReviewInterface] No previous clips" << std::endl;
    }
}

void ReviewInterface::MarkCurrentConcentration(FocusLevel level, const std::string& reason) {
    if (!isPlaying || currentClipIndex < 0) {
        std::cout << "[ReviewInterface] No clip is currently playing" << std::endl;
        return;
    }
    
    concentrationTracker.MarkConcentrationAtTime(currentPlaybackTime, level, reason);
}

void ReviewInterface::ShowConcentrationTimeline() {
    if (!concentrationTracker.IsInReviewMode()) {
        std::cout << "[ReviewInterface] No active review session" << std::endl;
        return;
    }
    
    std::cout << "\n=== CONCENTRATION TIMELINE ===" << std::endl;
    auto marks = concentrationTracker.GetConcentrationMarks();
    
    if (marks.empty()) {
        std::cout << "No concentration marks yet." << std::endl;
        return;
    }
    
    for (const auto& mark : marks) {
        std::cout << std::fixed << std::setprecision(1) 
                  << mark.timestamp << "s: " 
                  << concentrationTracker.GetFocusLevelString(mark.level);
        if (!mark.reason.empty()) {
            std::cout << " (" << mark.reason << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void ReviewInterface::GenerateConcentrationReport() {
    if (!concentrationTracker.IsInReviewMode()) {
        std::cout << "[ReviewInterface] No active review session" << std::endl;
        return;
    }
    
    std::cout << "\n=== CONCENTRATION REPORT ===" << std::endl;
    auto stats = concentrationTracker.GetConcentrationStats();
    
    int totalMarks = stats[FocusLevel::LOW_FOCUS] + 
                    stats[FocusLevel::MEDIUM_FOCUS] + 
                    stats[FocusLevel::HIGH_FOCUS];
    
    if (totalMarks == 0) {
        std::cout << "No concentration data available." << std::endl;
        return;
    }
    
    std::cout << "Total marks: " << totalMarks << std::endl;
    std::cout << "Low Focus: " << stats[FocusLevel::LOW_FOCUS] 
              << " (" << (stats[FocusLevel::LOW_FOCUS] * 100 / totalMarks) << "%)" << std::endl;
    std::cout << "Medium Focus: " << stats[FocusLevel::MEDIUM_FOCUS] 
              << " (" << (stats[FocusLevel::MEDIUM_FOCUS] * 100 / totalMarks) << "%)" << std::endl;
    std::cout << "High Focus: " << stats[FocusLevel::HIGH_FOCUS] 
              << " (" << (stats[FocusLevel::HIGH_FOCUS] * 100 / totalMarks) << "%)" << std::endl;
    std::cout << std::endl;
}

void ReviewInterface::ShowPerformanceAnalysis() {
    std::cout << "\n=== PERFORMANCE ANALYSIS ===" << std::endl;
    std::cout << "This would analyze:" << std::endl;
    std::cout << "- Accuracy during different focus levels" << std::endl;
    std::cout << "- Reaction time correlation with concentration" << std::endl;
    std::cout << "- Common mistakes during low-focus periods" << std::endl;
    std::cout << "- Improvement suggestions based on focus patterns" << std::endl;
    std::cout << std::endl;
}

void ReviewInterface::ShowCurrentClipInfo() {
    if (currentClipIndex >= 0 && currentClipIndex < clips.size()) {
        const GameplayClip& clip = clips[currentClipIndex];
        std::cout << "\nCurrent Clip: " << (currentClipIndex + 1) << "/" << clips.size() << std::endl;
        std::cout << "Description: " << clip.description << std::endl;
        std::cout << "Time: " << std::fixed << std::setprecision(1) 
                  << currentPlaybackTime << "s / " << clip.duration << "s" << std::endl;
    }
}

void ReviewInterface::ShowPlaybackControls() {
    std::cout << "\nControls: 1=Low Focus, 2=Medium Focus, 3=High Focus, s=Timeline, n=Next, p=Prev, q=Quit" << std::endl;
}

void ReviewInterface::AnalyzeCurrentMoment() {
    if (!isPlaying || currentClipIndex < 0) {
        std::cout << "[ReviewInterface] No clip is currently playing" << std::endl;
        return;
    }
    
    SimulateMomentAnalysis(currentPlaybackTime);
}

void ReviewInterface::ShowTechnicalAnalysis() {
    if (!gameplayAnalyzer.IsAnalyzing()) {
        std::cout << "[ReviewInterface] No active analysis session" << std::endl;
        return;
    }
    
    std::cout << gameplayAnalyzer.GenerateDetailedFeedback() << std::endl;
}

void ReviewInterface::GenerateTechnicalReport() {
    if (!gameplayAnalyzer.IsAnalyzing()) {
        std::cout << "[ReviewInterface] No active analysis session" << std::endl;
        return;
    }
    
    std::cout << gameplayAnalyzer.GeneratePerformanceSummary() << std::endl;
    std::cout << gameplayAnalyzer.GenerateDetailedFeedback() << std::endl;
}

void ReviewInterface::ShowImprovementPlan() {
    if (!gameplayAnalyzer.IsAnalyzing()) {
        std::cout << "[ReviewInterface] No active analysis session" << std::endl;
        return;
    }
    
    std::cout << gameplayAnalyzer.GenerateImprovementPlan() << std::endl;
}

void ReviewInterface::SimulateShotAnalysis(double timestamp) {
    std::cout << "\n=== AI SHOT ANALYSIS ===" << std::endl;
    std::cout << "Analyzing shot at " << timestamp << "s..." << std::endl;
    
    if (currentClipIndex >= 0 && currentClipIndex < clips.size()) {
        const auto& shots = clips[currentClipIndex].shots;
        for (const auto& shot : shots) {
            if (std::abs(shot.timestamp - timestamp) < 0.5) {
                std::cout << "Shot Analysis:" << std::endl;
                std::cout << "- Hit: " << (shot.hit ? "YES" : "NO") << std::endl;
                std::cout << "- Aim Error: " << shot.aimOffset << " pixels" << std::endl;
                std::cout << "- Recoil Compensation: " << shot.recoilCompensation << "x" << std::endl;
                std::cout << "- Distance to Target: " << shot.distanceToTarget << " units" << std::endl;
                std::cout << "- Reaction Time: " << shot.reactionTime << "s" << std::endl;
                std::cout << "- Weapon: " << shot.weaponType << std::endl;
                
                if (!shot.hit) {
                    std::cout << "\nWhy you missed:" << std::endl;
                    if (shot.aimOffset > 20.0) {
                        std::cout << "- Poor aim: " << shot.aimOffset << " pixels off target" << std::endl;
                    }
                    if (shot.recoilCompensation < 0.8 || shot.recoilCompensation > 1.2) {
                        std::cout << "- Recoil control: " << shot.recoilCompensation << "x compensation (should be ~1.0x)" << std::endl;
                    }
                    if (shot.reactionTime > 0.3) {
                        std::cout << "- Slow reaction: " << shot.reactionTime << "s (should be <0.25s)" << std::endl;
                    }
                }
                break;
            }
        }
    }
    std::cout << std::endl;
}

void ReviewInterface::SimulateMomentAnalysis(double timestamp) {
    std::cout << "\n=== AI MOMENT ANALYSIS ===" << std::endl;
    std::cout << "Analyzing gameplay at " << timestamp << "s..." << std::endl;
    
    if (timestamp < 3.0) {
        std::cout << "Analysis: Opening positioning" << std::endl;
        std::cout << "- You're taking a defensive angle" << std::endl;
        std::cout << "- Good crosshair placement at head level" << std::endl;
        std::cout << "- Consider pre-aiming common enemy positions" << std::endl;
    } else if (timestamp < 6.0) {
        std::cout << "Analysis: Enemy spotted" << std::endl;
        std::cout << "- Enemy appears in your field of view" << std::endl;
        std::cout << "- Reaction time: 0.28s (good)" << std::endl;
        std::cout << "- Crosshair placement needs adjustment" << std::endl;
    } else if (timestamp < 9.0) {
        std::cout << "Analysis: Combat engagement" << std::endl;
        std::cout << "- Multiple shots fired" << std::endl;
        std::cout << "- Recoil control inconsistent" << std::endl;
        std::cout << "- Consider burst firing for better accuracy" << std::endl;
    } else {
        std::cout << "Analysis: Post-combat" << std::endl;
        std::cout << "- Checking for additional threats" << std::endl;
        std::cout << "- Good situational awareness" << std::endl;
        std::cout << "- Maintain crosshair discipline" << std::endl;
    }
    std::cout << std::endl;
}

void ReviewInterface::ProcessUserInput(char input) {
    switch (input) {
        case '1':
            MarkCurrentConcentration(FocusLevel::LOW_FOCUS, "User marked during review");
            break;
        case '2':
            MarkCurrentConcentration(FocusLevel::MEDIUM_FOCUS, "User marked during review");
            break;
        case '3':
            MarkCurrentConcentration(FocusLevel::HIGH_FOCUS, "User marked during review");
            break;
        case 'a':
            AnalyzeCurrentMoment();
            break;
        case 's':
            ShowConcentrationTimeline();
            break;
        case 't':
            ShowTechnicalAnalysis();
            break;
        case 'n':
            NextClip();
            break;
        case 'p':
            PreviousClip();
            break;
        case 'q':
            if (concentrationTracker.IsInReviewMode()) {
                concentrationTracker.EndReview();
            }
            if (gameplayAnalyzer.IsAnalyzing()) {
                gameplayAnalyzer.EndAnalysis();
            }
            std::cout << "[ReviewInterface] Exiting review mode" << std::endl;
            break;
        default:
            std::cout << "[ReviewInterface] Unknown command: " << input << std::endl;
            break;
    }
}
