#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
using namespace std;

struct Student {
    string name;
    char gender; // 'M' or 'F'
    int clazz{};
    int score[9]{};
    int avg{}; // floor average of 9 scores
    int rank{}; // last flushed rank (1-based)
};

// Compute avg as floor of sum/9
static inline int compute_avg(const int sc[9]) {
    long long s = 0;
    for (int i = 0; i < 9; ++i) s += sc[i];
    return int(s / 9);
}

// Comparator for ranking: higher avg first; then compare scores 0..8 descending; then name ascending
struct RankCmp {
    const vector<Student>* pv; // pointer to students storage
    bool operator()(int i, int j) const {
        if (i == j) return false;
        const Student &a = (*pv)[i], &b = (*pv)[j];
        if (a.avg != b.avg) return a.avg > b.avg;
        for (int k = 0; k < 9; ++k) {
            if (a.score[k] != b.score[k]) return a.score[k] > b.score[k];
        }
        return a.name < b.name;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Storage: students in vector, lookup by name to index
    vector<Student> students;
    students.reserve(10000);
    unordered_map<string, int> idx; idx.reserve(16384);

    bool started = false;

    // Live ranking set stores indices into students vector (continuously maintained on UPDATE)
    RankCmp cmp; cmp.pv = &students;
    set<int, RankCmp> live_ranking(cmp);
    // Snapshot order from last START/FLUSH for stable printing and query
    vector<int> snapshot_order;

    string cmd;
    while (cin >> cmd) {
        if (cmd == "ADD") {
            string name; char gender; int clazz; int sc[9];
            cin >> name >> gender >> clazz;
            for (int i = 0; i < 9; ++i) cin >> sc[i];
            if (started) {
                cout << "[Error]Cannot add student now." << '\n';
                continue;
            }
            if (idx.find(name) != idx.end()) {
                cout << "[Error]Add failed." << '\n';
                continue;
            }
            Student s;
            s.name = name;
            s.gender = gender;
            s.clazz = clazz;
            for (int i = 0; i < 9; ++i) s.score[i] = sc[i];
            s.avg = compute_avg(s.score);
            s.rank = 0;
            int id = (int)students.size();
            students.push_back(s);
            idx[name] = id;
        } else if (cmd == "START") {
            if (!started) {
                live_ranking.clear();
                for (int i = 0; i < (int)students.size(); ++i) live_ranking.insert(i);
                snapshot_order.clear(); snapshot_order.reserve(students.size());
                for (int id : live_ranking) snapshot_order.push_back(id);
                int r = 1; for (int id : snapshot_order) students[id].rank = r++;
                started = true;
            }
        } else if (cmd == "UPDATE") {
            string name; int code; int score;
            cin >> name >> code >> score;
            auto it = idx.find(name);
            if (it == idx.end()) {
                cout << "[Error]Update failed." << '\n';
                continue;
            }
            int id = it->second;
            if (started) live_ranking.erase(id); // maintain live ranking position
            students[id].score[code] = score;
            students[id].avg = compute_avg(students[id].score);
            if (started) live_ranking.insert(id);
            // Snapshot order and displayed ranks remain unchanged until FLUSH
        } else if (cmd == "FLUSH") {
            if (started) {
                // Refresh snapshot from live ranking and update displayed ranks
                snapshot_order.clear(); snapshot_order.reserve(students.size());
                for (int id : live_ranking) snapshot_order.push_back(id);
                int r = 1; for (int id : snapshot_order) students[id].rank = r++;
            }
        } else if (cmd == "PRINTLIST") {
            if (!started) {
                // If not started, ranking not yet initialized; per problem, PRINTLIST occurs after START
                // But handle gracefully: treat current order as empty list printed nothing
            }
            int r = 1;
            for (int id : snapshot_order) {
                const Student &s = students[id];
                cout << r << ' ' << s.name << ' ' << (s.gender == 'M' ? "male" : "female")
                     << ' ' << s.clazz << ' ' << s.avg << '\n';
                ++r;
            }
        } else if (cmd == "QUERY") {
            string name; cin >> name;
            auto it = idx.find(name);
            if (it == idx.end()) {
                cout << "[Error]Query failed." << '\n';
            } else {
                int id = it->second;
                cout << "STUDENT " << students[id].name << " NOW AT RANKING " << students[id].rank << '\n';
            }
        } else if (cmd == "END") {
            break;
        } else {
            // Unknown command: ignore
        }
    }

    return 0;
}
