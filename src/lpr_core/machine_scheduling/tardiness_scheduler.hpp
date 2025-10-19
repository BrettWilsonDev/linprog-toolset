#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <climits>
#include <sstream>

struct TardinessJob
{
    int id;
    int processingTime;
    int dueDate;
    int picked; // 0 = not picked, 1 = picked

    TardinessJob(int id, int processingTime, int dueDate)
        : id(id), processingTime(processingTime), dueDate(dueDate), picked(0) {}
};

struct TardinessProblemData
{
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

class TardinessScheduler
{
private:
    std::vector<TardinessJob> jobs;
    int bestCandidate;
    std::vector<std::string> bestSequence;
    std::string bestCandidateLetter;
    int candidateCount;
    std::vector<TardinessProblemData> allProblems;
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

    std::string getProblemNumber(const std::vector<std::string> &currentSequence, int jobId, int position)
    {
        if (currentSequence.empty())
        {
            return std::to_string(jobId);
        }
        else
        {
            std::vector<std::string> parts;

            // Get the root job from first selection
            int firstJobId = std::stoi(currentSequence[0].substr(1, 1)); // Extract job ID from x14, x24, etc.
            parts.push_back(std::to_string(firstJobId));

            // For subsequent levels, determine the order based on available jobs
            std::vector<int> tempPicked(jobs.size(), 0);

            // Simulate the sequence to this point
            for (const auto &seqItem : currentSequence)
            {
                int seqJobId = std::stoi(seqItem.substr(1, 1)); // Extract job ID
                for (size_t j = 0; j < jobs.size(); ++j)
                {
                    if (jobs[j].id == seqJobId)
                    {
                        tempPicked[j] = 1;
                        break;
                    }
                }
            }

            // Find available jobs at this level
            std::vector<int> availableJobs;
            for (size_t j = 0; j < jobs.size(); ++j)
            {
                if (tempPicked[j] == 0)
                {
                    availableJobs.push_back(jobs[j].id);
                }
            }
            std::sort(availableJobs.begin(), availableJobs.end());

            // Find position of current job in available jobs
            if (!availableJobs.empty())
            {
                auto it = std::find(availableJobs.begin(), availableJobs.end(), jobId);
                if (it != availableJobs.end())
                {
                    int jobPosition = std::distance(availableJobs.begin(), it) + 1;
                    parts.push_back(std::to_string(jobPosition));
                }
                else
                {
                    parts.push_back("1");
                }
            }

            std::string result = parts[0];
            for (size_t i = 1; i < parts.size(); ++i)
            {
                result += "." + parts[i];
            }
            return result;
        }
    }

    void branch(int position, int currentOverdue, std::vector<std::string> currentSequence)
    {
        if (position == 0)
        {
            if (currentOverdue < bestCandidate)
            {
                candidateCount++;
                bestCandidateLetter = std::string(1, 'A' + candidateCount - 1);

                // Store the solution problem
                TardinessProblemData problemData;
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

        for (size_t i = 0; i < jobs.size(); ++i)
        {
            if (jobs[i].picked == 1)
            {
                continue;
            }
            int overdue = remainingTime - jobs[i].dueDate;
            int overdueDays = std::max(0, overdue);
            int nextOverdue = currentOverdue + overdueDays;
            bool pruned = nextOverdue > bestCandidate;
            evals.push_back(std::make_tuple(nextOverdue, i, overdue, overdueDays, pruned));
        }

        // Sort by job ID for consistent ordering
        std::sort(evals.begin(), evals.end(), [this](const auto &a, const auto &b)
                  { return jobs[std::get<1>(a)].id < jobs[std::get<1>(b)].id; });

        for (const auto &eval : evals)
        {
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
            for (size_t j = 0; j < tempSeq.size(); ++j)
            {
                if (j > 0)
                    displaySequence += " & ";
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

            if (pruned)
            {
                continue;
            }

            jobs[i].picked = 1;
            branch(position - 1, nextOverdue, tempSeq);
            jobs[i].picked = 0; // Backtrack
        }
    }

    std::pair<int, std::vector<std::string>> runGreedy()
    {
        int tempBest = INT_MAX;
        int position = static_cast<int>(jobs.size());
        int pickedOverdue = 0;
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

            int minOverdue = INT_MAX;
            int bestI = -1;
            for (size_t i = 0; i < jobs.size(); ++i)
            {
                if (pickedJobs[i] == 1)
                {
                    continue;
                }
                int overdue = remainingTime - jobs[i].dueDate;
                int nextOverdue = pickedOverdue + std::max(0, overdue);
                if (nextOverdue < minOverdue)
                {
                    minOverdue = nextOverdue;
                    bestI = static_cast<int>(i);
                }
            }
            if (bestI == -1)
            {
                break;
            }
            pickedOverdue = minOverdue;
            pickedJobs[bestI] = 1;
            sequence.push_back("x" + std::to_string(jobs[bestI].id) + std::to_string(position));
            position--;
        }

        if (pickedOverdue < tempBest)
        {
            tempBest = pickedOverdue;
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
    TardinessScheduler()
        : bestCandidate(INT_MAX), candidateCount(0) {}

    TardinessScheduler(const std::vector<std::vector<double>> &jobData)
        : bestCandidate(INT_MAX), candidateCount(0)
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
        int initialOverdue = greedyResult.first;
        std::vector<std::string> initialSequence = greedyResult.second;

        bestCandidate = initialOverdue;
        bestSequence = initialSequence;
        candidateCount = 1;
        bestCandidateLetter = "A";

        oss << "Initial greedy sum tardiness: " << initialOverdue << "\n";
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
        std::sort(allProblems.begin(), allProblems.end(), [this](const TardinessProblemData &a, const TardinessProblemData &b)
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
                oss << "Total overdue = " << problem.totalOverdue << " days *\n";
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
                oss << "Total overdue = " << problem.currentOverdue << "+" << problem.overdueDays
                    << " = " << problem.totalOverdue << " days\n";

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
        oss << "Best sum tardiness: " << bestCandidate << " " << bestCandidateLetter << "\n";
        oss << "Best sequence (backward positions): ";
        for (size_t i = 0; i < bestSequence.size(); ++i)
        {
            if (i > 0)
                oss << " ";
            oss << bestSequence[i];
        }
        oss << "\n";
    }
    // Usage example function
    inline void runTardinessSchedulerExample()
    {
        oss << "TARDINESS SCHEDULER - EXAMPLE\n";
        oss << std::string(80, '=') << "\n";

        std::vector<std::vector<int>> jobData = {
            {1, 6, 8},  // Job 1: processing time 6, due date 8
            {2, 4, 4},  // Job 2: processing time 4, due date 4
            {3, 5, 12}, // Job 3: processing time 5, due date 12
            {4, 8, 16}  // Job 4: processing time 8, due date 16
        };

        solve();
    }
};

class MachineSchedulingTardiness
{
private:
    bool isConsoleOutput;
    std::ostringstream oss;

public:
    MachineSchedulingTardiness(bool isConsoleOutput = false) : isConsoleOutput(isConsoleOutput) {}
    ~MachineSchedulingTardiness() = default;

    // Get collected output
    std::string getCollectedOutput() const
    {
        return oss.str();
    }

    std::string getJSON()
    {
        return to_d3_json(oss.str());
    }

    void runTardinessScheduler(const std::vector<std::vector<double>> &jobData)
    {
        oss << "TARDINESS SCHEDULER\n";
        oss << std::string(80, '=') << "\n";

        TardinessScheduler solver(jobData);

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
        std::string total_overdue;
        std::string status;
        std::string eliminated_by;
        std::string branch_on;
        std::vector<Node *> children;

        // For root
        std::string initial_tardiness;
        std::string initial_sequence;
        std::string best_tardiness;
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
        if (!node->total_overdue.empty())
        {
            os << ",\"total_overdue\":\"" << json_escape(node->total_overdue) << "\"";
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
            if (!node->initial_tardiness.empty())
            {
                os << ",\"initial_tardiness\":\"" << json_escape(node->initial_tardiness) << "\"";
            }
            if (!node->initial_sequence.empty())
            {
                os << ",\"initial_sequence\":\"" << json_escape(node->initial_sequence) << "\"";
            }
            if (!node->best_tardiness.empty())
            {
                os << ",\"best_tardiness\":\"" << json_escape(node->best_tardiness) << "\"";
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
            if (line.empty() || line.find("=====") == 0 || line == "TARDINESS SCHEDULER")
            {
                ++i;
                continue;
            }
            if (line.find("Initial greedy sum tardiness:") == 0)
            {
                root->initial_tardiness = trim(line.substr(30));
                ++i;
                continue;
            }
            if (line.find("Initial sequence (forward):") == 0)
            {
                root->initial_sequence = trim(line.substr(27));
                ++i;
                continue;
            }
            if (line.find("Best sum tardiness:") == 0)
            {
                root->best_tardiness = trim(line.substr(20));
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

                // Parse next lines
                if (++i < lines.size())
                {
                    new_node->variables = trim(lines[i]);
                }
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
                    std::string total_line = trim(lines[i]);
                    size_t eq_pos = total_line.find("=");
                    if (eq_pos != std::string::npos)
                    {
                        new_node->total_overdue = trim(total_line.substr(eq_pos + 1));
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
                // Skip the separator
                ++i;

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
};