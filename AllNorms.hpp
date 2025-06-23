#include "Norms.hpp"
#include "Game.hpp"
#include <bitset>
#include <set>
#include <algorithm>


using IntVec = std::vector<int>;

std::vector<IntVec> create_self_symmetric_norms() {
    std::vector<IntVec> norms;
    
    for (int i = 0; i < (1 << 6); ++i) {
        std::bitset<6> bits(i);
        IntVec norm = {
            bits[0], bits[1],
            bits[2], bits[3],
            1 - bits[2], 1 - bits[3],
            1 - bits[0], 1 - bits[1],
            bits[4], bits[5],
            bits[4], bits[5]
        };
        norms.push_back(norm);
    }

    return norms;
}

bool is_flipped(const IntVec& a, const IntVec& b) {
    for (size_t i = 0; i < a.size(); ++i)
        if (b[i] != 1 - a[i])
            return false;
    return true;
}

std::vector<IntVec> generate_all_norms() {
    std::vector<IntVec> SNorms = create_self_symmetric_norms();
    std::vector<IntVec> NormsToCheck;

    for (int i = 0; i < (1 << 12); ++i) {
        std::bitset<12> bits(i);
        IntVec norm(12);
        for (int j = 0; j < 12; ++j)
            norm[j] = bits[j];

        bool to_keep = true;

        // Check if it's in SNorms
        if (find(SNorms.begin(), SNorms.end(), norm) != SNorms.end()) {
            to_keep = false;
        }

        // Check flipped version
        for (const auto& other_norm : NormsToCheck) {
            if (is_flipped(norm, other_norm)) {
                to_keep = false;
                break;
            }
        }

        if (to_keep)
            NormsToCheck.push_back(norm);
    }

    // Combine
    NormsToCheck.insert(NormsToCheck.end(), SNorms.begin(), SNorms.end());
    return NormsToCheck;
}
