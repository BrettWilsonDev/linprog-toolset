#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <climits>
#include <sstream>

struct TardinessJob {
    int id;
    int processingTime;
    int dueDate;
    int picked;  // 0 = not picked, 1 = picked
    
    TardinessJob(int id, int processingTime, int dueDate) 
        : id(id), processingTime(processingTime), dueDate(dueDate), picked(0) {}
};

struct TardinessProblemData {
    std::string number;
    std::string sequence;
    int remainingTime;
    int dueDate;
    int overdue;
    int overdueDays;
    int currentOverdue;
    int totalOverdue;
    bool pruned;
    int bestCandidate;
    std::string bestCandidateLetter;
    bool isSolution;
    std::string candidateLetter;
    
    TardinessProblemData() : remainingTime(0), dueDate(0), overdue(0), overdueDays(0), 
                            currentOverdue(0), totalOverdue(0), pruned(false), 
                            bestCandidate(0), isSolution(false) {}
};

class TardinessScheduler {
private:
    std::vector<TardinessJob> jobs;
    int bestCandidate;
    std::vector<std::string> bestSequence;
    std::string bestCandidateLetter;
    int candidateCount;
    std::vector<TardinessProblemData> allProblems;

    int calculateRemainingTime() {
        int total = 0;
        for (const auto& job : jobs) {
            if (job.picked != 1) {
                total += job.processingTime;
            }
        }
        return total;
    }

    std::string getProblemNumber(const std::vector<std::string>& currentSequence, int jobId, int position) {
        if (currentSequence.empty()) {
            return std::to_string(jobId);
        } else {
            std::vector<std::string> parts;
            
            // Get the root job from first selection
            int firstJobId = std::stoi(currentSequence[0].substr(1, 1)); // Extract job ID from x14, x24, etc.
            parts.push_back(std::to_string(firstJobId));
            
            // For subsequent levels, determine the order based on available jobs
            std::vector<int> tempPicked(jobs.size(), 0);
            
            // Simulate the sequence to this point
            for (const auto& seqItem : currentSequence) {
                int seqJobId = std::stoi(seqItem.substr(1, 1)); // Extract job ID
                for (size_t j = 0; j < jobs.size(); ++j) {
                    if (jobs[j].id == seqJobId) {
                        tempPicked[j] = 1;
                        break;
                    }
                }
            }
            
            // Find available jobs at this level
            std::vector<int> availableJobs;
            for (size_t j = 0; j < jobs.size(); ++j) {
                if (tempPicked[j] == 0) {
                    availableJobs.push_back(jobs[j].id);
                }
            }
            std::sort(availableJobs.begin(), availableJobs.end());
            
            // Find position of current job in available jobs
            if (!availableJobs.empty()) {
                auto it = std::find(availableJobs.begin(), availableJobs.end(), jobId);
                if (it != availableJobs.end()) {
                    int jobPosition = std::distance(availableJobs.begin(), it) + 1;
                    parts.push_back(std::to_string(jobPosition));
                } else {
                    parts.push_back("1");
                }
            }
            
            std::string result = parts[0];
            for (size_t i = 1; i < parts.size(); ++i) {
                result += "." + parts[i];
            }
            return result;
        }
    }

    void branch(int position, int currentOverdue, std::vector<std::string> currentSequence) {
        if (position == 0) {
            if (currentOverdue < bestCandidate) {
                candidateCount++;
                bestCandidateLetter = std::string(1, 'A' + candidateCount - 1);
                
                // Store the solution problem
                TardinessProblemData problemData;
                problemData.number = getProblemNumber(currentSequence.size() > 0 ? 
                    std::vector<std::string>(currentSequence.begin(), currentSequence.end() - 1) : 
                    std::vector<std::string>(), 
                    currentSequence.empty() ? 0 : std::stoi(currentSequence.back().substr(1, 1)), position);
                
                std::string seqStr = "";
                for (size_t i = 0; i < currentSequence.size(); ++i) {
                    if (i > 0) seqStr += " & ";
                    seqStr += currentSequence[i];
                }
                problemData.sequence = seqStr;
                problemData.totalOverdue = currentOverdue;
                problemData.isSolution = true;
                problemData.candidateLetter = bestCandidateLetter;
                allProblems.push_back(problemData);
                
                bestCandidate = currentOverdue;
                bestSequence = currentSequence;
            }
            return;
        }

        int remainingTime = calculateRemainingTime();
        std::vector<std::tuple<int, int, int, int, bool>> evals;
        
        for (size_t i = 0; i < jobs.size(); ++i) {
            if (jobs[i].picked == 1) {
                continue;
            }
            int overdue = remainingTime - jobs[i].dueDate;
            int overdueDays = std::max(0, overdue);
            int nextOverdue = currentOverdue + overdueDays;
            bool pruned = nextOverdue > bestCandidate;
            evals.push_back(std::make_tuple(nextOverdue, i, overdue, overdueDays, pruned));
        }

        // Sort by job ID for consistent ordering
        std::sort(evals.begin(), evals.end(), [this](const auto& a, const auto& b) {
            return jobs[std::get<1>(a)].id < jobs[std::get<1>(b)].id;
        });

        for (const auto& eval : evals) {
            int nextOverdue = std::get<0>(eval);
            int i = std::get<1>(eval);
            int overdue = std::get<2>(eval);
            int overdueDays = std::get<3>(eval);
            bool pruned = std::get<4>(eval);
            
            int jobId = jobs[i].id;
            std::string problemNum = getProblemNumber(currentSequence, jobId, position);
            std::string newSequenceItem = "x" + std::to_string(jobId) + std::to_string(position);
            
            std::string displaySequence = "";
            std::vector<std::string> tempSeq = currentSequence;
            tempSeq.push_back(newSequenceItem);
            for (size_t j = 0; j < tempSeq.size(); ++j) {
                if (j > 0) displaySequence += " & ";
                displaySequence += tempSeq[j];
            }

            // Store problem data
            TardinessProblemData problemData;
            problemData.number = problemNum;
            problemData.sequence = displaySequence;
            problemData.remainingTime = remainingTime;
            problemData.dueDate = jobs[i].dueDate;
            problemData.overdue = overdue;
            problemData.overdueDays = overdueDays;
            problemData.currentOverdue = currentOverdue;
            problemData.totalOverdue = nextOverdue;
            problemData.pruned = pruned;
            problemData.bestCandidate = pruned ? bestCandidate : 0;
            problemData.bestCandidateLetter = pruned ? bestCandidateLetter : "";
            problemData.isSolution = false;
            allProblems.push_back(problemData);

            if (pruned) {
                continue;
            }

            jobs[i].picked = 1;
            branch(position - 1, nextOverdue, tempSeq);
            jobs[i].picked = 0; // Backtrack
        }
    }

    std::pair<int, std::vector<std::string>> runGreedy() {
        int tempBest = INT_MAX;
        int position = static_cast<int>(jobs.size());
        int pickedOverdue = 0;
        std::vector<std::string> sequence;
        std::vector<int> pickedJobs(jobs.size(), 0);
        
        while (position > 0) {
            int remainingTime = 0;
            for (size_t i = 0; i < jobs.size(); ++i) {
                if (pickedJobs[i] != 1) {
                    remainingTime += jobs[i].processingTime;
                }
            }
            
            int minOverdue = INT_MAX;
            int bestI = -1;
            for (size_t i = 0; i < jobs.size(); ++i) {
                if (pickedJobs[i] == 1) {
                    continue;
                }
                int overdue = remainingTime - jobs[i].dueDate;
                int nextOverdue = pickedOverdue + std::max(0, overdue);
                if (nextOverdue < minOverdue) {
                    minOverdue = nextOverdue;
                    bestI = static_cast<int>(i);
                }
            }
            if (bestI == -1) {
                break;
            }
            pickedOverdue = minOverdue;
            pickedJobs[bestI] = 1;
            sequence.push_back("x" + std::to_string(jobs[bestI].id) + std::to_string(position));
            position--;
        }
        
        if (pickedOverdue < tempBest) {
            tempBest = pickedOverdue;
        }
        
        // Reverse to forward order
        std::reverse(sequence.begin(), sequence.end());
        return std::make_pair(tempBest, sequence);
    }

    std::vector<int> parseNumberForSorting(const std::string& number) {
        std::vector<int> parts;
        std::stringstream ss(number);
        std::string part;
        while (std::getline(ss, part, '.')) {
            parts.push_back(std::stoi(part));
        }
        return parts;
    }

public:
    TardinessScheduler() 
        : bestCandidate(INT_MAX), candidateCount(0) {}


    TardinessScheduler(const std::vector<std::vector<int>>& jobData) 
        : bestCandidate(INT_MAX), candidateCount(0) {
        
        for (const auto& jobInfo : jobData) {
            jobs.emplace_back(jobInfo[0], jobInfo[1], jobInfo[2]);
        }
    }

    void solve() {
        // Run greedy first
        auto greedyResult = runGreedy();
        int initialOverdue = greedyResult.first;
        std::vector<std::string> initialSequence = greedyResult.second;
        
        bestCandidate = initialOverdue;
        bestSequence = initialSequence;
        candidateCount = 1;
        bestCandidateLetter = "A";

        std::cout << "Initial greedy sum tardiness: " << initialOverdue << std::endl;
        std::cout << "Initial sequence (forward): ";
        for (size_t i = 0; i < initialSequence.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << initialSequence[i];
        }
        std::cout << std::endl << std::endl;

        // Run branch-and-bound
        branch(static_cast<int>(jobs.size()), 0, std::vector<std::string>());

        // Sort problems by their number for logical display order
        std::sort(allProblems.begin(), allProblems.end(), [this](const TardinessProblemData& a, const TardinessProblemData& b) {
            std::vector<int> partsA = parseNumberForSorting(a.number);
            std::vector<int> partsB = parseNumberForSorting(b.number);
            return partsA < partsB;
        });

        // Print all problems in order
        for (const auto& problem : allProblems) {
            std::cout << "====================" << std::endl;
            if (problem.isSolution) {
                std::cout << "Problem " << problem.number << std::endl;
                std::cout << "Total overdue = " << problem.totalOverdue << " days *" << std::endl;
            } else {
                std::cout << "Problem " << problem.number << std::endl;
                std::cout << problem.sequence << std::endl;
                
                // Build time required string
                std::string timeStr = "";
                bool first = true;
                for (const auto& job : jobs) {
                    if (job.picked != 1) {
                        if (!first) timeStr += "+";
                        timeStr += std::to_string(job.processingTime);
                        first = false;
                    }
                }
                
                std::cout << "Time required = " << timeStr << " = " << problem.remainingTime << " days" << std::endl;
                std::cout << "Overdue = " << problem.remainingTime << "-" << problem.dueDate 
                         << " = " << problem.overdueDays << " days" << std::endl;
                std::cout << "Total overdue = " << problem.currentOverdue << "+" << problem.overdueDays 
                         << " = " << problem.totalOverdue << " days" << std::endl;
                
                if (problem.pruned) {
                    std::cout << "Eliminated by Candidate " << problem.bestCandidate << " " 
                             << problem.bestCandidateLetter << std::endl;
                } else {
                    // Extract last job from sequence for branching info
                    std::string lastJob = problem.sequence;
                    size_t lastAmp = lastJob.find_last_of('&');
                    if (lastAmp != std::string::npos) {
                        lastJob = lastJob.substr(lastAmp + 2); // +2 to skip "& "
                    }
                    std::string jobIdStr = lastJob.substr(1, 1); // Extract job ID
                    
                    // Count number of jobs in sequence
                    int jobCount = 1;
                    for (char c : problem.sequence) {
                        if (c == '&') jobCount++;
                    }
                    
                    std::cout << "Branching on x" << jobIdStr << jobCount << std::endl;
                }
            }
        }

        std::cout << std::endl;
        std::cout << "Best sum tardiness: " << bestCandidate << " " << bestCandidateLetter << std::endl;
        std::cout << "Best sequence (backward positions): ";
        for (size_t i = 0; i < bestSequence.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << bestSequence[i];
        }
        std::cout << std::endl;
    }
    // Usage example function
    inline void runTardinessSchedulerExample() {
        std::vector<std::vector<int>> jobData = {
            {1, 6, 8},   // Job 1: processing time 6, due date 8
            {2, 4, 4},   // Job 2: processing time 4, due date 4
            {3, 5, 12},  // Job 3: processing time 5, due date 12
            {4, 8, 16}   // Job 4: processing time 8, due date 16
        };
        
        TardinessScheduler scheduler(jobData);
        scheduler.solve();
    }
};
