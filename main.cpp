#include <bits/stdc++.h>
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

    // Ranking set stores indices into students vector
    RankCmp cmp; cmp.pv = &students;
    set<int, RankCmp> ranking(cmp);

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
                // build ranking
                ranking.clear();
                for (int i = 0; i < (int)students.size(); ++i) {
                    ranking.insert(i);
                }
                // assign ranks
                int r = 1;
                for (int id : ranking) {
                    students[id].rank = r++;
                }
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
            students[id].score[code] = score;
            students[id].avg = compute_avg(students[id].score);
            // Do not change ranking order until FLUSH
        } else if (cmd == "FLUSH") {
            if (started) {
                // Rebuild ranking snapshot using current data
                ranking.clear();
                for (int i = 0; i < (int)students.size(); ++i) ranking.insert(i);
                int r = 1;
                for (int id : ranking) students[id].rank = r++;
            }
        } else if (cmd == "PRINTLIST") {
            if (!started) {
                // If not started, ranking not yet initialized; per problem, PRINTLIST occurs after START
                // But handle gracefully: treat current order as empty list printed nothing
            }
            int r = 1;
            for (int id : ranking) {
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
