#include "Norms.hpp"
#include "Game.hpp"
#include "AllNorms.hpp"
#include <fstream>

void writeCSV(const std::vector<std::tuple<int, int, double, double, double, double,
                                           double, double, double, double, double,
                                           double, double, double>>& output, const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    file << "ID,order,assessment_error,perception_error,mu_e,benefit,cost,SID,BB,BG,GB,GG,selfpay,mutantpay\n";

    for (const auto& row : output) {
        file << std::get<0>(row) << ","
             << std::get<1>(row) << ","
             << std::get<2>(row) << ","
             << std::get<3>(row) << ","
             << std::get<4>(row) << ","
             << std::get<5>(row) << ","
             << std::get<6>(row) << ","
             << std::get<7>(row) << ","
             << std::get<8>(row) << ","
             << std::get<9>(row) << ","
             << std::get<10>(row) << ","
             << std::get<11>(row) << ","
             << std::get<12>(row) << ","
             << std::get<13>(row) << "\n";
    }

    file.close();
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <location to save output>" << std::endl;
        return 1;
    }
    double benefit = 1.0;
    std::vector costs = {0.2, 0.6};
    double assessment_error = 0.1;
    double perception_error = 0.1;
    double mu_e = 0.1;

    constexpr Reputation B = Reputation::B, G = Reputation::G;
    constexpr Action C = Action::C, D = Action::D;

    std::string base = std::string(argv[1]);
    std::string file = base + "L6_L3_payoff_difference.csv";

    std::vector<Norm> norms = {Norm::L3(), Norm::L6()};

    std::vector<std::tuple<int, int, double, double, double, double, double, double,
                           double, double, double, double, double, double>> output;
    int order = 1;
    for (const auto& norm : norms) {
        ActionRule S = norm.action_rule;
        int sid = S.ID();
        // std::cout << "Norm ID: " << sid << std::endl;
        for (double cost : costs){

            Game sim(assessment_error, perception_error, mu_e, norm);

            double r_benefit = (1.0 - mu_e) * benefit;
            double r_cost = (1.0 - mu_e) * cost;

            // Self payoff
            double self_payoff = (r_benefit - r_cost) * sim.resident_coop;

            for (int i = 0; i < 16; ++i) {
                if (sid != i) {
                ActionRule invader = ActionRule::MakeDeterministicRule(i);

                auto [H,coop_mut_to_res,coop_res_to_mut] = sim.calc_invader_stats(invader);
                double invader_payoff = r_benefit * coop_res_to_mut - r_cost * coop_mut_to_res;;

                output.push_back(std::make_tuple(norm.ID(),
                                order,
                                assessment_error,
                                perception_error,
                                mu_e,
                                benefit,
                                cost,
                                invader.ID(),
                                invader(B, B),
                                invader(B, G),
                                invader(G, B),
                                invader(G, G),
                                self_payoff,
                                invader_payoff));
                }
            }
        }
        order++;
    }
    writeCSV(output, file);
    return 0;
}
