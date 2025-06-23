#include "Norms.hpp"
#include "Game.hpp"
#include "AllNorms.hpp"
#include <fstream>

void writeCSV(const std::vector<std::tuple<int, int, double, double, double, double, double, double>>& output, const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    file << "order,SID,GG,GB,BG,BB,selfpay,mutantpay\n";

    for (const auto& row : output) {
        file << std::get<0>(row) << ","
             << std::get<1>(row) << ","
             << std::get<2>(row) << ","
             << std::get<3>(row) << ","
             << std::get<4>(row) << ","
             << std::get<5>(row) << ","
             << std::get<6>(row) << ","
             << std::get<7>(row) << "\n";
    }

    file.close();
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <location to save output>" << std::endl;
        return 1;
    }
    double assessment_error = 0.01;
    double perception_error = 0.0;
    double mu_e = 0.0;
    double benefit = 1.0;
    double cost = 0.1;

    constexpr Reputation B = Reputation::B, G = Reputation::G;
    constexpr Action C = Action::C, D = Action::D;

    std::string base = std::string(argv[1]);
    std::string file = base + "equalizers_payoffs.csv";

    std::vector<Norm> norms;

    // Equalizer Generous Scoring
    std::map<std::pair<Reputation, Reputation>, double> actions_map = {
        {{B, B}, 0.0},
        {{B, G}, 1.0},
        {{G, B}, 0.0},
        {{G, G}, 1.0}
    };

    ActionRule disc(actions_map);

    std::map<std::tuple<Reputation, Reputation, Action>, double> assessments_map = {
        {{B, B, D}, 1.0 - cost / ((1 - 2 * assessment_error) * benefit)},
        {{B, B, C}, 1.0},
        {{B, G, D}, 1.0 - cost / ((1 - 2 * assessment_error) * benefit)},
        {{B, G, C}, 1.0},
        {{G, B, D}, 1.0 - cost / ((1 - 2 * assessment_error) * benefit)},
        {{G, B, C}, 1.0},
        {{G, G, D}, 1.0 - cost / ((1 - 2 * assessment_error) * benefit)},
        {{G, G, C}, 1.0}
    };
    AssessmentRule generous_scoring_assessment(assessments_map);
    Norm generous_scoring(generous_scoring_assessment, disc);

    // Equalizer Cautious Generous Scoring
    std::map<std::tuple<Reputation, Reputation, Action>, double> assessments_map2 = {
        {{B, B, D}, 0.0},
        {{B, B, C}, cost / ((1 - 2 * assessment_error) * benefit)},
        {{B, G, D}, 1 - cost / ((1 - 2 * assessment_error) * benefit)},
        {{B, G, C}, 1},
        {{G, B, D}, 0},
        {{G, B, C}, cost / ((1 - 2 * assessment_error) * benefit)},
        {{G, G, D}, 1 - cost / ((1 - 2 * assessment_error) * benefit)},
        {{G, G, C}, 1}
    };
    AssessmentRule cautious_generous_scoring_assessment(assessments_map2);
    Norm cautious_generous_scoring(cautious_generous_scoring_assessment, disc);

    norms.push_back(generous_scoring);
    norms.push_back(cautious_generous_scoring);


    std::vector<std::tuple<int, int, double, double, double, double, double, double>> output;
    int order = 1;
    for (const auto& norm : norms) {
        Game sim(assessment_error, perception_error, mu_e, norm);

        double self_payoff = (benefit - cost) * sim.resident_coop;

        for (int i = 0; i < 16; ++i) {
            ActionRule invader = ActionRule::MakeDeterministicRule(i);

            auto [H,coop_mut_to_res,coop_res_to_mut] = sim.calc_invader_stats(invader);
            double invader_payoff = benefit * coop_res_to_mut - cost * coop_mut_to_res;;

            output.push_back(std::make_tuple(order,
                             invader.ID(),
                             invader(B, B),
                             invader(B, G),
                             invader(G, B),
                             invader(G, G),
                             self_payoff,
                             invader_payoff));
        }
        order++;
    }

    writeCSV(output, file);
    return 0;
}
