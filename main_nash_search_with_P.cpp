#include "NormsWithPunishment.hpp"
#include "GameWithPunishment.hpp"
#include <fstream>


int JudgeClass( const Norm& norm ) {
    constexpr Reputation G = Reputation::G, B = Reputation::B;
    constexpr Action C = Action::C, D = Action::D, P = Action::P;
    const auto S = norm.action_rule;
    const auto R = norm.assessment_rule;

    std::string s = ActionToString( S(G,G) ) + ActionToString( S(G,B) ) + ActionToString( S(B,G) );
    const std::map<std::string, int> class_map = {
        {"CDC", 1},
        {"CPC", 2},
        {"CDD", 3},
        {"CPD", 4},
        {"CDP", 5},
        {"CPP", 6}
    };
    // if key is not found, return 0 (AllD)
    if (class_map.find(s) == class_map.end()) {
        return 0;
    }
    return class_map.at(s);
}

std::vector<int> EnumerateCESS(double benefit, double cost, double punishment, double punishment_cost) {
    const double assessment_error = 0.001;
    const double perception_error = 0.0;

    std::vector<int> class_counts(7, 0); // 0 is for others
    for (size_t i = 0; i < 4096ul; ++i) {
        AssessmentRule R = AssessmentRule::MakeDeterministicRule(i);
        for (size_t j = 0; j < 81; ++j) {
            ActionRule S = ActionRule::MakeDeterministicRule(j);
            Norm norm{ R, S };
            Game sim(assessment_error, perception_error, norm);
            if ( sim.resident_coop > 0.99 && sim.isESS(benefit, cost, punishment, punishment_cost) ) {
                if (sim.equilibrium_state >= 0.5) {  // to remove GB-symmetry
                    int c = JudgeClass(norm);
                    class_counts[c]++;
                }
            }
        }
    }
    return class_counts;
}

int main() {
    // when c > alpha
    double benefit = 3.0, cost = 1.0, punishment = 0.7, punishment_cost = 0.3;
    auto counts = EnumerateCESS(benefit, cost, punishment, punishment_cost);
    for (size_t i = 0; i < counts.size(); ++i) {
        std::cout << "Class " << i << ": " << counts[i] << std::endl;
    }
    std::vector<int> expected_counts = {0, 32, 16, 128, 64, 64, 32};
    assert(expected_counts == counts);

    // when benefit is low, class 3 and 5 disappear
    benefit = 1.5, cost = 1.0, punishment = 0.7, punishment_cost = 0.3;
    counts = EnumerateCESS(benefit, cost, punishment, punishment_cost);
    for (size_t i = 0; i < counts.size(); ++i) {
        std::cout << "Class " << i << ": " << counts[i] << std::endl;
    }
    expected_counts = {0, 32, 16, 0, 64, 0, 32};
    assert(expected_counts == counts);

    // when beta is low, class 4 and 6 disappear
    benefit = 1.5, cost = 1.0, punishment = 0.2, punishment_cost = 0.3;
    counts = EnumerateCESS(benefit, cost, punishment, punishment_cost);
    for (size_t i = 0; i < counts.size(); ++i) {
        std::cout << "Class " << i << ": " << counts[i] << std::endl;
    }
    expected_counts = {0, 32, 16, 0, 0, 0, 0};
    assert(expected_counts == counts);

    // when c > alpha
    benefit = 3.0, cost = 1.0, punishment = 0.7, punishment_cost = 1.3;
    counts = EnumerateCESS(benefit, cost, punishment, punishment_cost);
    for (size_t i = 0; i < counts.size(); ++i) {
        std::cout << "Class " << i << ": " << counts[i] << std::endl;
    }
    expected_counts = {0, 128, 32, 256, 64, 64, 16};
    assert(expected_counts == counts);

    // when benefit and beta are low, class 3 and 4 disappear
    benefit = 1.5, cost = 1.0, punishment = 0.2, punishment_cost = 1.3;
    counts = EnumerateCESS(benefit, cost, punishment, punishment_cost);
    for (size_t i = 0; i < counts.size(); ++i) {
        std::cout << "Class " << i << ": " << counts[i] << std::endl;
    }
    expected_counts = {0, 128, 32, 0, 0, 64, 16};
    assert(expected_counts == counts);

    return 0;
}
