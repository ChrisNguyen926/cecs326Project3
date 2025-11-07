// Build: g++ -std=c++17 -O2 BankersAlgo.cpp -o bankers
// Run:   ./bankers

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using std::cout;
using std::cin;
using std::string;
using std::vector;

using Matrix = vector<vector<int>>;
using Vector = vector<int>;

class BankersAlgorithm {
public:
    BankersAlgorithm(int n_processes, int n_resources,Vector available, Matrix maxClaim, Matrix allocation)
        : n(n_processes), m(n_resources),
          Available(std::move(available)),
          Max(std::move(maxClaim)),
          Allocation(std::move(allocation)) {
        recomputeNeed();
    }

    // Determines if there exists a sequence that lets all processes finish.
    bool Safety(vector<int>& safeSequence) const {
        Vector Work = Available;             
        vector<char> Finish(n, 0);           
        safeSequence.clear();

        bool progressed = true;
        while (progressed) {
            progressed = false;
            for (int i = 0; i < n; ++i) {
                if (Finish[i]) continue;
                if (canMeetNeed(i, Work)) {
                    for (int j = 0; j < m; ++j) Work[j] += Allocation[i][j];
                    Finish[i] = 1;
                    safeSequence.push_back(i);
                    progressed = true;
                }
            }
        }
        return std::count(Finish.begin(), Finish.end(), 1) == n;
    }

    enum class RequestResult { Granted, ExceedsNeed, NotAvailable, Unsafe };

    // Attempts grant a request for process pid. If granted, optionally returns a safe sequence.
    RequestResult Request(int pid, const Vector& req, vector<int>* safeAfter = nullptr) {
        for (int j = 0; j < m; ++j)
            if (req[j] > Need[pid][j]) return RequestResult::ExceedsNeed;
        for (int j = 0; j < m; ++j)
            if (req[j] > Available[j]) return RequestResult::NotAvailable;
        for (int j = 0; j < m; ++j) {
            Available[j]   -= req[j];
            Allocation[pid][j] += req[j];
            Need[pid][j]   -= req[j];
        }

        // Safety check
        vector<int> seq;
        bool safe = Safety(seq);
        if (safe) {
            if (safeAfter) *safeAfter = seq;
            return RequestResult::Granted;   // keep allocation
        }

        // Roll back
        for (int j = 0; j < m; ++j) {
            Available[j]   += req[j];
            Allocation[pid][j] -= req[j];
            Need[pid][j]   += req[j];
        }
        return RequestResult::Unsafe;
    }

    // Output formatting
    void PrintHeader() const {
        cout << "n = " << n << " # Number of processes\n";
        cout << "m = " << m << " # Number of resources types\n\n";
    }

    void PrintVectorLabeled(const string& label, const Vector& v) const {
        cout << "# " << label << "\n";
        cout << "[";
        for (int i = 0; i < (int)v.size(); ++i) {
            cout << v[i];
            if (i + 1 < (int)v.size()) cout << ", ";
        }
        cout << "]\n\n";
    }

    static void PrintMatrixBlock(const string& label, const Matrix& A) {
        cout << "# " << label << "\n";
        cout << "[";
        for (int i = 0; i < (int)A.size(); ++i) {
            cout << "[";
            for (int j = 0; j < (int)A[i].size(); ++j) {
                cout << A[i][j];
                if (j + 1 < (int)A[i].size()) cout << ", ";
            }
            cout << "]";
            if (i + 1 < (int)A.size()) cout << ",\n ";
        }
        cout << "]\n\n";
    }

    void PrintStateLikeScreenshot() const {
        PrintHeader();
        PrintVectorLabeled("Available Vector (initially total resources available)", Available);
        PrintMatrixBlock("Maximum Matrix", Max);
        PrintMatrixBlock("Allocation Matrix", Allocation);
        PrintMatrixBlock("Need Matrix (Max - Allocation)", Need);
    }

    int N() const { return n; }
    int M() const { return m; }

private:
    int n, m;
    Vector Available;
    Matrix Max, Allocation, Need;

    void recomputeNeed() {
        Need.assign(n, Vector(m, 0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < m; ++j)
                Need[i][j] = Max[i][j] - Allocation[i][j];
    }

    bool canMeetNeed(int pid, const Vector& Work) const {
        for (int j = 0; j < m; ++j)
            if (Need[pid][j] > Work[j]) return false;
        return true;
    }
};

static string SeqToString(const vector<int>& seq) {
    string s = "[";
    for (size_t i = 0; i < seq.size(); ++i) {
        s += std::to_string(seq[i]);
        if (i + 1 < seq.size()) s += ", ";
    }
    s += "]";
    return s;
}

int main() {
    int n = 5, m = 3;
    Vector available = {3, 3, 2};
    Matrix maxMat = {
        {7,5,3},
        {3,2,2},
        {9,0,2},
        {2,2,2},
        {4,3,3}
    };
    Matrix alloc = {
        {0,1,0},
        {2,0,0},
        {3,0,2},
        {2,1,1},
        {0,0,2}
    };

    BankersAlgorithm ba(n, m, available, maxMat, alloc);
    ba.PrintStateLikeScreenshot();

    while (true) {
        cout << "Banker's Algorithm Test Menu:\n";
        cout << "1. Check for safe sequence\n";
        cout << "2. User-defined resource request\n";
        cout << "3. Exit\n";
        cout << "Enter your choice (1-3): ";

        int choice;
        if (!(cin >> choice)) return 0;

        if (choice == 1) {
            vector<int> seq;
            bool safe = ba.Safety(seq);
            if (safe) {
                cout << "System is in a SAFE state.\n";
                cout << "Safe Sequence: " << SeqToString(seq) << "\n\n";
            } else {
                cout << "System is in an UNSAFE state.\n\n";
            }
        } else if (choice == 2) {
            int pid;
            cout << "Enter process ID (0-" << (ba.N() - 1) << "): ";
            cin >> pid;
            if (pid < 0 || pid >= ba.N()) { cout << "Invalid PID.\n\n"; continue; }

            Vector req(m, 0);
            cout << "Enter request for P" << pid << " (" << m << " integers): ";
            for (int j = 0; j < m; ++j) cin >> req[j];

            vector<int> seqAfter;
            auto res = ba.Request(pid, req, &seqAfter);

            if (res == BankersAlgorithm::RequestResult::Granted) {
                cout << "Request granted.\n";
                cout << "Safe Sequence: " << SeqToString(seqAfter) << "\n\n";
                ba.PrintStateLikeScreenshot();
            } else if (res == BankersAlgorithm::RequestResult::ExceedsNeed) {
                cout << "Error: Request exceeds remaining need for P" << pid << ".\n\n";
            } else if (res == BankersAlgorithm::RequestResult::NotAvailable) {
                cout << "Resources not available. Process P" << pid << " must wait.\n\n";
            } else {
                cout << "Error: Request would lead to an unsafe state.\n\n";
            }
        } else if (choice == 3) {
            break;
        } else {
            cout << "Invalid choice.\n\n";
        }
    }
    return 0;
}
