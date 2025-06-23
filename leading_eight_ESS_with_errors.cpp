#include "Norms.hpp"
#include "Game.hpp"

#include <fstream>


void writeCSV(const std::vector<std::tuple<int, int, double, bool, double, double, double>>& output,
              const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }

    file << "order,ID,h,isNash,assessment_error,perception_error,mu_e\n";

    for (const auto& row : output) {
        file << std::get<0>(row) << ","
             << std::get<1>(row) << ","
             << std::get<2>(row) << ","
             << std::get<3>(row) << ","
             << std::get<4>(row) << ","
             << std::get<5>(row) << ","
             << std::get<6>(row) << "\n";
    }

    file.close();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <location to save output>" << std::endl;
        return 1;
    }
    std::vector<Norm> l8_norms = {Norm::L1(), Norm::L2(), Norm::L3(), Norm::L4(),
                                 Norm::L5(), Norm::L6(), Norm::L7(), Norm::L8()};


    double benefit = 1.0;
    double cost = 0.8;
    constexpr double EPSILON = 1e-5;

    std::string base = std::string(argv[1]);
    std::string file = base + "leading_eight_ESS_with_errors.csv";

    std::vector<double> vector_errors;
    for (double i = 0.0; i < 0.1002; i += 0.002) {
        vector_errors.push_back(i);
    }

    std::vector<std::tuple<int, int, double, bool, double, double, double>> output;

    const std::vector<std::pair<Reputation, Reputation>> all_contexts = {
        {Reputation::G, Reputation::G},
        {Reputation::G, Reputation::B},
        {Reputation::B, Reputation::G},
        {Reputation::B, Reputation::B}
    };

    int order = 1;
    for (const auto& norm : l8_norms) {
        std::cout << "ID" << norm.ID() << std::endl;
        for (double assessment_error : vector_errors) {
            for (double perception_error : vector_errors) {
                for (double mu_e : vector_errors) {
                    Game sim(assessment_error, perception_error, mu_e, norm);
                    double h = sim.equilibrium_state;
                    bool isESS = sim.isESS(benefit, cost);
                    output.push_back(std::make_tuple(order, norm.ID(), h, isESS, assessment_error, perception_error, mu_e));
                }
            }
        }
    order++;
    }

    writeCSV(output, file);

    return 0;
}
