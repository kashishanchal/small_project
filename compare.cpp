// compare.cpp
// Simple comparator: normalize strings, compute Levenshtein distance and similarity
// Usage:
//  compare_tool --protected-dir /home/user/Desktop/protected --threshold 0.7 --input "text"
//  or: echo "text" | compare_tool --protected-dir ...

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::string normalize(const std::string &s) {
    // simple normalization: lowercase, map common leet, remove non-alnum (keep spaces), collapse spaces
    std::string out;
    out.reserve(s.size());
    for (unsigned char ch : s) {
        // map basic leet -> letter
        char c = ch;
        if (c == '0') c = 'o';
        else if (c == '1') c = 'l';
        else if (c == '3') c = 'e';
        else if (c == '4') c = 'a';
        else if (c == '5') c = 's';
        else if (c == '7') c = 't';
        else if (c == '@') c = 'a';
        else if (c == '$') c = 's';
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (std::isalnum(static_cast<unsigned char>(c))) {
            out.push_back(c);
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            // normalize spaces to single space
            if (out.empty() || out.back() != ' ') out.push_back(' ');
        }
        // other punctuation skipped
    }
    // trim trailing spaces
    while (!out.empty() && out.back() == ' ') out.pop_back();
    // collapse multiple spaces already handled
    return out;
}

static int levenshtein_distance(const std::string &a, const std::string &b) {
    if (a.empty()) return static_cast<int>(b.size());
    if (b.empty()) return static_cast<int>(a.size());
    size_t n = a.size(), m = b.size();
    std::vector<int> prev(m + 1), cur(m + 1);
    for (size_t j = 0; j <= m; ++j) prev[j] = static_cast<int>(j);
    for (size_t i = 1; i <= n; ++i) {
        cur[0] = static_cast<int>(i);
        for (size_t j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            cur[j] = std::min({ prev[j] + 1, cur[j - 1] + 1, prev[j - 1] + cost });
        }
        prev.swap(cur);
    }
    return prev[m];
}

int main(int argc, char **argv) {
    std::string protected_dir;
    double threshold = 0.7;
    std::string input;

    // simple arg parsing
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--protected-dir" && i + 1 < argc) { protected_dir = argv[++i]; }
        else if (a == "--threshold" && i + 1 < argc) { threshold = std::stod(argv[++i]); }
        else if (a == "--input" && i + 1 < argc) { input = argv[++i]; }
        else if (a == "--help") {
            std::cout << "compare_tool --protected-dir <dir> --threshold <0..1> [--input \"text\"]\n";
            return 0;
        }
    }

    if (protected_dir.empty()) {
        // default to ~/Desktop/protected
        const char *home = getenv("HOME");
        if (!home) {
            std::cerr << "HOME not set; provide --protected-dir" << std::endl;
            return 2;
        }
        protected_dir = std::string(home) + "/Desktop/protected";
    }

    // read stdin if no input arg
    if (input.empty()) {
        std::string stdin_text;
        std::string line;
        bool has = false;
        while (std::getline(std::cin, line)) { stdin_text += line; stdin_text += '\n'; has = true; }
        if (has) input = stdin_text;
    }

    if (input.empty()) {
        std::cerr << "No input text provided (use --input or pipe text)" << std::endl;
        return 3;
    }

    std::string norm_input = normalize(input);
    if (norm_input.empty()) {
        std::cout << "NO_MATCH 0 " << "" << std::endl;
        return 0;
    }

    double best_score = 0.0;
    std::string best_path;

    try {
        for (auto &p : fs::directory_iterator(protected_dir)) {
            if (!p.is_regular_file()) continue;
            std::ifstream ifs(p.path());
            if (!ifs) continue;
            std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            std::string norm = normalize(content);
            if (norm.empty()) continue;
            int dist = levenshtein_distance(norm_input, norm);
            int maxlen = std::max(static_cast<int>(norm_input.size()), static_cast<int>(norm.size()));
            double sim = 1.0 - (double)dist / (double)maxlen;
            if (sim > best_score) { best_score = sim; best_path = p.path().string(); }
        }
    } catch (const std::exception &e) {
        std::cerr << "Error reading protected dir: " << e.what() << std::endl;
        return 4;
    }

    int pct = static_cast<int>(best_score * 100.0 + 0.5);
    if (best_score >= threshold) {
        std::cout << "MATCH " << pct << " " << best_path << std::endl;
        return 0; // indicate match
    }
    std::cout << "NO_MATCH " << pct << " " << best_path << std::endl;
    return 1;
}
