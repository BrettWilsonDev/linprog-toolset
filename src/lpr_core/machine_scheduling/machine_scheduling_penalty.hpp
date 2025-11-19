#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <climits>
#include <sstream>

struct Job
{
    int id;
    int processingTime;
    int dueDate;
    int picked; // 0 = not picked, 1 = picked

    Job(int id, int processingTime, int dueDate)
        : id(id), processingTime(processingTime), dueDate(dueDate), picked(0) {}
};

struct ProblemData
{
    std::string number;
    std::string sequence;
    int remainingTime;
    int dueDate;
    int overdueDays;
    int penaltyRate;
    int jobPenalty;
    int currentPenalty;
    int totalPenalty;
    bool pruned;
    int bestCandidate;
    std::string bestCandidateLetter;
    bool isSolution;
    std::string candidateLetter;

    ProblemData() : remainingTime(0), dueDate(0), overdueDays(0), penaltyRate(0),
                    jobPenalty(0), currentPenalty(0), totalPenalty(0), pruned(false),
                    bestCandidate(0), isSolution(false) {}
};

class JobScheduler
{
private:
    std::vector<Job> jobs;
    std::vector<double> penalties;
    int bestCandidate;
    std::vector<std::string> bestSequence;
    std::string bestCandidateLetter;
    int candidateCount;
    std::vector<ProblemData> allProblems;
    mutable std::ostringstream oss;

    int calculateRemainingTime()
    {
        int total = 0;
        for (const auto &job : jobs)
        {
            if (job.picked != 1)
            {
                total += job.processingTime;
            }
        }
        return total;
    }

    // std::string getProblemNumber(const std::vector<std::string> &currentSequence, int jobId, int position)
    // {
    //     if (currentSequence.empty())
    //     {
    //         return std::to_string(jobId);
    //     }
    //     else
    //     {
    //         std::vector<std::string> parts;

    //         // Get the root job from first selection
    //         int firstJobId = std::stoi(currentSequence[0].substr(1, 1)); // Extract job ID from x14, x24, etc.
    //         parts.push_back(std::to_string(firstJobId));

    //         // For subsequent levels, determine the order based on available jobs
    //         std::vector<int> tempPicked(jobs.size(), 0);

    //         // Simulate the sequence to this point
    //         for (const auto &seqItem : currentSequence)
    //         {
    //             int seqJobId = std::stoi(seqItem.substr(1, 1)); // Extract job ID
    //             for (size_t j = 0; j < jobs.size(); ++j)
    //             {
    //                 if (jobs[j].id == seqJobId)
    //                 {
    //                     tempPicked[j] = 1;
    //                     break;
    //                 }
    //             }
    //         }

    //         // Find available jobs at this level
    //         std::vector<int> availableJobs;
    //         for (size_t j = 0; j < jobs.size(); ++j)
    //         {
    //             if (tempPicked[j] == 0)
    //             {
    //                 availableJobs.push_back(jobs[j].id);
    //             }
    //         }
    //         std::sort(availableJobs.begin(), availableJobs.end());

    //         // Find position of current job in available jobs
    //         if (!availableJobs.empty())
    //         {
    //             auto it = std::find(availableJobs.begin(), availableJobs.end(), jobId);
    //             if (it != availableJobs.end())
    //             {
    //                 int jobPosition = std::distance(availableJobs.begin(), it) + 1;
    //                 parts.push_back(std::to_string(jobPosition));
    //             }
    //             else
    //             {
    //                 parts.push_back("1");
    //             }
    //         }

    //         std::string result = parts[0];
    //         for (size_t i = 1; i < parts.size(); ++i)
    //         {
    //             result += "." + parts[i];
    //         }
    //         return result;
    //     }
    // }

    std::string getProblemNumber(const std::vector<std::string> &currentSequence, int nextJobId, int position)
    {
        if (currentSequence.empty())
        {
            return std::to_string(nextJobId);
        }

        std::vector<int> path;
        std::vector<int> picked(jobs.size(), 0);

        // Replay the entire currentSequence to determine the branch index at each level
        for (const auto &seqItem : currentSequence)
        {
            int jobId = std::stoi(seqItem.substr(1, 1)); // extract job number from x14, x33, etc.

            // Get available jobs at this point in history
            std::vector<int> available;
            for (size_t i = 0; i < jobs.size(); ++i)
            {
                if (!picked[i])
                {
                    available.push_back(jobs[i].id);
                }
            }
            std::sort(available.begin(), available.end());

            // Find where this jobId was in the sorted list → that's its branch number
            auto it = std::find(available.begin(), available.end(), jobId);
            if (it != available.end())
            {
                int branchPos = std::distance(available.begin(), it) + 1;
                path.push_back(branchPos);
            }

            // Mark as picked
            for (size_t i = 0; i < jobs.size(); ++i)
            {
                if (jobs[i].id == jobId)
                {
                    picked[i] = 1;
                    break;
                }
            }
        }

        // Now add the branch position for the *next* job (the one we're considering)
        std::vector<int> available;
        for (size_t i = 0; i < jobs.size(); ++i)
        {
            if (!picked[i])
            {
                available.push_back(jobs[i].id);
            }
        }
        std::sort(available.begin(), available.end());

        auto it = std::find(available.begin(), available.end(), nextJobId);
        if (it != available.end())
        {
            int branchPos = std::distance(available.begin(), it) + 1;
            path.push_back(branchPos);
        }
        else
        {
            path.push_back(1); // fallback
        }

        // Build dotted string: 4.3.1 etc.
        std::string result = std::to_string(path[0]);
        for (size_t i = 1; i < path.size(); ++i)
        {
            result += "." + std::to_string(path[i]);
        }
        return result;
    }

    void branch(int position, int currentPenalty, std::vector<std::string> currentSequence)
    {
        if (position == 0)
        {
            if (currentPenalty < bestCandidate)
            {
                candidateCount++;
                bestCandidateLetter = std::string(1, 'A' + candidateCount - 1);

                // Store the solution problem
                ProblemData problemData;
                problemData.number = getProblemNumber(currentSequence.size() > 0 ? std::vector<std::string>(currentSequence.begin(), currentSequence.end() - 1) : std::vector<std::string>(),
                                                      currentSequence.empty() ? 0 : std::stoi(currentSequence.back().substr(1, 1)), position);

                std::string seqStr = "";
                for (size_t i = 0; i < currentSequence.size(); ++i)
                {
                    if (i > 0)
                        seqStr += " & ";
                    seqStr += currentSequence[i];
                }
                problemData.sequence = seqStr;
                problemData.totalPenalty = currentPenalty;
                problemData.isSolution = true;
                problemData.candidateLetter = bestCandidateLetter;
                allProblems.push_back(problemData);

                bestCandidate = currentPenalty;
                bestSequence = currentSequence;
            }
            return;
        }

        int remainingTime = calculateRemainingTime();
        std::vector<std::tuple<int, int, int, int, bool>> evals;

        for (size_t i = 0; i < jobs.size(); ++i)
        {
            if (jobs[i].picked == 1)
            {
                continue;
            }
            int overdue = remainingTime - jobs[i].dueDate;
            int overdueDays = std::max(0, overdue);
            int jobPenalty = overdueDays * penalties[i];
            int nextPenalty = currentPenalty + jobPenalty;
            bool pruned = nextPenalty > bestCandidate;
            evals.push_back(std::make_tuple(nextPenalty, i, overdueDays, jobPenalty, pruned));
        }

        // Sort by job ID for consistent ordering
        std::sort(evals.begin(), evals.end(), [this](const auto &a, const auto &b)
                  { return jobs[std::get<1>(a)].id < jobs[std::get<1>(b)].id; });

        for (const auto &eval : evals)
        {
            int nextPenalty = std::get<0>(eval);
            int i = std::get<1>(eval);
            int overdueDays = std::get<2>(eval);
            int jobPenalty = std::get<3>(eval);
            bool pruned = std::get<4>(eval);

            int jobId = jobs[i].id;
            std::string problemNum = getProblemNumber(currentSequence, jobId, position);
            std::string newSequenceItem = "x" + std::to_string(jobId) + std::to_string(position);

            std::string displaySequence = "";
            std::vector<std::string> tempSeq = currentSequence;
            tempSeq.push_back(newSequenceItem);
            for (size_t j = 0; j < tempSeq.size(); ++j)
            {
                if (j > 0)
                    displaySequence += " & ";
                displaySequence += tempSeq[j];
            }

            // Store problem data
            ProblemData problemData;
            problemData.number = problemNum;
            problemData.sequence = displaySequence;
            problemData.remainingTime = remainingTime;
            problemData.dueDate = jobs[i].dueDate;
            problemData.overdueDays = overdueDays;
            problemData.penaltyRate = penalties[i];
            problemData.jobPenalty = jobPenalty;
            problemData.currentPenalty = currentPenalty;
            problemData.totalPenalty = nextPenalty;
            problemData.pruned = pruned;
            problemData.bestCandidate = pruned ? bestCandidate : 0;
            problemData.bestCandidateLetter = pruned ? bestCandidateLetter : "";
            problemData.isSolution = false;
            allProblems.push_back(problemData);

            if (pruned)
            {
                continue;
            }

            jobs[i].picked = 1;
            branch(position - 1, nextPenalty, tempSeq);
            jobs[i].picked = 0; // Backtrack
        }
    }

    std::pair<int, std::vector<std::string>> runGreedy()
    {
        int tempBest = INT_MAX;
        int position = static_cast<int>(jobs.size());
        int pickedPenalty = 0;
        std::vector<std::string> sequence;
        std::vector<int> pickedJobs(jobs.size(), 0);

        while (position > 0)
        {
            int remainingTime = 0;
            for (size_t i = 0; i < jobs.size(); ++i)
            {
                if (pickedJobs[i] != 1)
                {
                    remainingTime += jobs[i].processingTime;
                }
            }

            int minPenalty = INT_MAX;
            int bestI = -1;
            for (size_t i = 0; i < jobs.size(); ++i)
            {
                if (pickedJobs[i] == 1)
                {
                    continue;
                }
                int overdue = remainingTime - jobs[i].dueDate;
                int overdueDays = std::max(0, overdue);
                int jobPenalty = overdueDays * penalties[i];
                int nextPenalty = pickedPenalty + jobPenalty;
                if (nextPenalty < minPenalty)
                {
                    minPenalty = nextPenalty;
                    bestI = static_cast<int>(i);
                }
            }
            if (bestI == -1)
            {
                break;
            }
            pickedPenalty = minPenalty;
            pickedJobs[bestI] = 1;
            sequence.push_back("x" + std::to_string(jobs[bestI].id) + std::to_string(position));
            position--;
        }

        if (pickedPenalty < tempBest)
        {
            tempBest = pickedPenalty;
        }

        // Reverse to forward order
        std::reverse(sequence.begin(), sequence.end());
        return std::make_pair(tempBest, sequence);
    }

    std::vector<int> parseNumberForSorting(const std::string &number)
    {
        std::vector<int> parts;
        std::stringstream ss(number);
        std::string part;
        while (std::getline(ss, part, '.'))
        {
            parts.push_back(std::stoi(part));
        }
        return parts;
    }

public:
    JobScheduler()
        : bestCandidate(INT_MAX), candidateCount(0) {}

    JobScheduler(const std::vector<std::vector<double>> &jobData, const std::vector<double> &penaltyRates)
        : penalties(penaltyRates), bestCandidate(INT_MAX), candidateCount(0)
    {

        for (const auto &jobInfo : jobData)
        {
            jobs.emplace_back(jobInfo[0], jobInfo[1], jobInfo[2]);
        }
    }

    std::string getCollectedOutput() const
    {
        return oss.str();
    }

    void solve()
    {
        // Run greedy first
        auto greedyResult = runGreedy();
        int initialPenalty = greedyResult.first;
        std::vector<std::string> initialSequence = greedyResult.second;

        bestCandidate = initialPenalty;
        bestSequence = initialSequence;
        candidateCount = 1;
        bestCandidateLetter = "A";

        oss << "Initial greedy total penalty: " << initialPenalty << "\n";
        oss << "Initial sequence (forward): ";
        for (size_t i = 0; i < initialSequence.size(); ++i)
        {
            if (i > 0)
                oss << " ";
            oss << initialSequence[i];
        }
        oss << "\n\n";

        // Run branch-and-bound
        branch(static_cast<int>(jobs.size()), 0, std::vector<std::string>());

        // Sort problems by their number for logical display order
        std::sort(allProblems.begin(), allProblems.end(), [this](const ProblemData &a, const ProblemData &b)
                  {
            std::vector<int> partsA = parseNumberForSorting(a.number);
            std::vector<int> partsB = parseNumberForSorting(b.number);
            return partsA < partsB; });

        // Print all problems in order
        for (const auto &problem : allProblems)
        {
            oss << "====================\n";
            if (problem.isSolution)
            {
                oss << "Problem " << problem.number << "\n";
                oss << "Total penalty = " << problem.totalPenalty << " *\n";
            }
            else
            {
                oss << "Problem " << problem.number << "\n";
                oss << problem.sequence << "\n";

                // Build time required string
                std::string timeStr = "";
                bool first = true;
                for (const auto &job : jobs)
                {
                    if (job.picked != 1)
                    {
                        if (!first)
                            timeStr += "+";
                        timeStr += std::to_string(job.processingTime);
                        first = false;
                    }
                }

                oss << "Time required = " << timeStr << " = " << problem.remainingTime << " days\n";
                oss << "Overdue = " << problem.remainingTime << "-" << problem.dueDate
                    << " = " << problem.overdueDays << " days\n";
                oss << "Penalty = " << problem.overdueDays << " * " << problem.penaltyRate
                    << " = " << problem.jobPenalty << "\n";
                oss << "Total penalty = " << problem.currentPenalty << "+" << problem.jobPenalty
                    << " = " << problem.totalPenalty << "\n";

                if (problem.pruned)
                {
                    oss << "Eliminated by Candidate " << problem.bestCandidate << " "
                        << problem.bestCandidateLetter << "\n";
                }
                else
                {
                    // Extract last job from sequence for branching info
                    std::string lastJob = problem.sequence;
                    size_t lastAmp = lastJob.find_last_of('&');
                    if (lastAmp != std::string::npos)
                    {
                        lastJob = lastJob.substr(lastAmp + 2); // +2 to skip "& "
                    }
                    std::string jobIdStr = lastJob.substr(1, 1); // Extract job ID

                    // Count number of jobs in sequence
                    int jobCount = 1;
                    for (char c : problem.sequence)
                    {
                        if (c == '&')
                            jobCount++;
                    }

                    oss << "Branching on x" << jobIdStr << jobCount << "\n";
                }
            }
        }

        oss << "\n";
        oss << "Best total penalty: " << bestCandidate << " " << bestCandidateLetter << "\n";
        oss << "Best sequence (backward positions): ";
        for (size_t i = 0; i < bestSequence.size(); ++i)
        {
            if (i > 0)
                oss << " ";
            oss << bestSequence[i];
        }
        oss << "\n";
    }
};

class MachineSchedulingPenalty
{
private:
    bool isConsoleOutput;
    std::ostringstream oss;

public:
    MachineSchedulingPenalty(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput) {}
    ~MachineSchedulingPenalty() = default;

    // Get collected output
    std::string getCollectedOutput() const
    {
        return oss.str();
    }

    std::string getJSON()
    {
        return to_d3_json(oss.str());
    }

    void runPenaltyScheduler(const std::vector<std::vector<double>> &jobData, const std::vector<double> &penaltyRates)
    {
        oss << "PENALTY SCHEDULER\n";
        oss << std::string(80, '=') << "\n";

        JobScheduler solver(jobData, penaltyRates);

        solver.solve();

        oss << solver.getCollectedOutput();

        if (isConsoleOutput)
        {
            std::cout << oss.str();

            std::cout << to_d3_json(oss.str());
        }
    }

    struct Node
    {
        std::string name;
        std::string variables;
        std::string time_required;
        std::string overdue;
        std::string penalty;
        std::string total_penalty;
        std::string status;
        std::string eliminated_by;
        std::string branch_on;
        std::vector<Node *> children;

        // For root
        std::string initial_penalty;
        std::string initial_sequence;
        std::string best_penalty;
        std::string best_sequence;
    };

    std::string json_escape(const std::string &s)
    {
        std::string res;
        for (char c : s)
        {
            if (c == '\"')
                res += "\\\"";
            else if (c == '\\')
                res += "\\\\";
            else
                res += c;
        }
        return res;
    }

    void to_json(std::ostringstream &os, Node *node, bool is_root = false)
    {
        os << "{";
        os << "\"name\":\"" << json_escape(node->name) << "\"";
        if (!node->variables.empty())
        {
            os << ",\"variables\":\"" << json_escape(node->variables) << "\"";
        }
        if (!node->time_required.empty())
        {
            os << ",\"time_required\":\"" << json_escape(node->time_required) << "\"";
        }
        if (!node->overdue.empty())
        {
            os << ",\"overdue\":\"" << json_escape(node->overdue) << "\"";
        }
        if (!node->penalty.empty())
        {
            os << ",\"penalty\":\"" << json_escape(node->penalty) << "\"";
        }
        if (!node->total_penalty.empty())
        {
            os << ",\"total_penalty\":\"" << json_escape(node->total_penalty) << "\"";
        }
        if (!node->status.empty())
        {
            os << ",\"status\":\"" << json_escape(node->status) << "\"";
        }
        if (!node->eliminated_by.empty())
        {
            os << ",\"eliminated_by\":\"" << json_escape(node->eliminated_by) << "\"";
        }
        if (!node->branch_on.empty())
        {
            os << ",\"branch_on\":\"" << json_escape(node->branch_on) << "\"";
        }
        if (is_root)
        {
            if (!node->initial_penalty.empty())
            {
                os << ",\"initial_penalty\":\"" << json_escape(node->initial_penalty) << "\"";
            }
            if (!node->initial_sequence.empty())
            {
                os << ",\"initial_sequence\":\"" << json_escape(node->initial_sequence) << "\"";
            }
            if (!node->best_penalty.empty())
            {
                os << ",\"best_penalty\":\"" << json_escape(node->best_penalty) << "\"";
            }
            if (!node->best_sequence.empty())
            {
                os << ",\"best_sequence\":\"" << json_escape(node->best_sequence) << "\"";
            }
        }
        if (!node->children.empty())
        {
            os << ",\"children\":[";
            for (size_t i = 0; i < node->children.size(); ++i)
            {
                to_json(os, node->children[i]);
                if (i < node->children.size() - 1)
                    os << ",";
            }
            os << "]";
        }
        os << "}";
    }

    void delete_tree(Node *node)
    {
        if (node)
        {
            for (auto child : node->children)
            {
                delete_tree(child);
            }
            delete node;
        }
    }

    std::string trim(const std::string &str)
    {
        size_t first = str.find_first_not_of(" \t");
        if (first == std::string::npos)
            return "";
        size_t last = str.find_last_not_of(" \t");
        return str.substr(first, (last - first + 1));
    }

    int get_level(const std::string &prob_id)
    {
        int dots = 0;
        for (char c : prob_id)
        {
            if (c == '.')
                ++dots;
        }
        return dots + 1;
    }

    std::string to_d3_json(const std::string &output_str)
    {
        std::stringstream ss(output_str);
        std::string line;
        std::vector<std::string> lines;
        while (std::getline(ss, line))
        {
            lines.push_back(line);
        }

        Node *root = new Node();
        root->name = "root";

        std::vector<Node *> stack;
        stack.push_back(root);

        size_t i = 0;
        while (i < lines.size())
        {
            line = trim(lines[i]);
            if (line.empty() || line.find("=====") == 0 || line == "PENALTY SCHEDULER")
            {
                ++i;
                continue;
            }
            if (line.find("Initial greedy total penalty:") == 0)
            {
                root->initial_penalty = trim(line.substr(30));
                ++i;
                continue;
            }
            if (line.find("Initial sequence (forward):") == 0)
            {
                root->initial_sequence = trim(line.substr(27));
                ++i;
                continue;
            }
            if (line.find("Best total penalty:") == 0)
            {
                root->best_penalty = trim(line.substr(20));
                ++i;
                continue;
            }
            if (line.find("Best sequence (backward positions):") == 0)
            {
                root->best_sequence = trim(line.substr(36));
                ++i;
                continue;
            }
            if (line.find("Problem ") == 0)
            {
                std::string prob_id = trim(line.substr(8));
                int level = get_level(prob_id);

                Node *new_node = new Node();
                new_node->name = prob_id;

                bool parsed = false;
                if (++i < lines.size())
                {
                    new_node->variables = trim(lines[i]);
                    parsed = true;
                }

                bool is_solution = parsed && (new_node->variables.find("Total ") == 0);

                if (is_solution)
                {
                    size_t eq_pos = new_node->variables.find("=");
                    if (eq_pos != std::string::npos)
                    {
                        std::string total_str = trim(new_node->variables.substr(eq_pos + 1));
                        size_t star_pos = total_str.find("*");
                        if (star_pos != std::string::npos)
                        {
                            total_str = trim(total_str.substr(0, star_pos));
                        }
                        // No " days" in penalty
                        new_node->total_penalty = total_str;
                    }
                    new_node->status = "solution";
                }
                else
                {
                    if (++i < lines.size())
                    {
                        std::string time_line = trim(lines[i]);
                        size_t eq_pos = time_line.find("=");
                        if (eq_pos != std::string::npos)
                        {
                            new_node->time_required = trim(time_line.substr(eq_pos + 1));
                        }
                    }
                    if (++i < lines.size())
                    {
                        std::string over_line = trim(lines[i]);
                        size_t eq_pos = over_line.find("=");
                        if (eq_pos != std::string::npos)
                        {
                            new_node->overdue = trim(over_line.substr(eq_pos + 1));
                        }
                    }
                    if (++i < lines.size())
                    {
                        std::string penalty_line = trim(lines[i]);
                        size_t eq_pos = penalty_line.find("=");
                        if (eq_pos != std::string::npos)
                        {
                            new_node->penalty = trim(penalty_line.substr(eq_pos + 1));
                        }
                    }
                    if (++i < lines.size())
                    {
                        std::string total_line = trim(lines[i]);
                        size_t eq_pos = total_line.find("=");
                        if (eq_pos != std::string::npos)
                        {
                            new_node->total_penalty = trim(total_line.substr(eq_pos + 1));
                        }
                    }
                    if (++i < lines.size())
                    {
                        std::string action_line = trim(lines[i]);
                        if (action_line.find("Eliminated by ") == 0)
                        {
                            new_node->status = "eliminated";
                            new_node->eliminated_by = trim(action_line.substr(14));
                        }
                        else if (action_line.find("Branching on ") == 0)
                        {
                            new_node->status = "branching";
                            new_node->branch_on = trim(action_line.substr(13));
                        }
                    }
                }
                // Add to tree
                while (stack.size() > static_cast<size_t>(level))
                {
                    stack.pop_back();
                }
                if (!stack.empty())
                {
                    Node *parent = stack.back();
                    parent->children.push_back(new_node);
                }
                stack.push_back(new_node);
                ++i; // Skip the separator
            }
            else
            {
                ++i;
            }
        }

        std::ostringstream os;
        to_json(os, root, true);
        std::string json = os.str();

        delete_tree(root);
        return json;
    }

    // Usage example function
    inline void runPenaltySchedulerExample()
    {
        oss << "PENALTY SCHEDULER - EXAMPLE\n";
        oss << std::string(80, '=') << "\n";

        std::vector<std::vector<double>> jobData = {
            {1, 6, 10},
            {2, 6, 14},
            {3, 7, 15},
            {4, 11, 16}};

        std::vector<double> penaltyRates = {5, 6, 5, 6};

        JobScheduler solver(jobData, penaltyRates);
        solver.solve();

        oss << solver.getCollectedOutput();

        if (isConsoleOutput)
        {
            std::cout << oss.str();

            std::cout << to_d3_json(oss.str());
        }
    }
};