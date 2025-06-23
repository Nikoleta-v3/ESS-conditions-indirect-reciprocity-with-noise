#include "Norms.hpp"
#include "Game.hpp"

int main() {

    // 1. Leading Eight Self Cooperation Rate
    std::vector<Norm> l8_norms = {Norm::L1(), Norm::L2(), Norm::L3(), Norm::L4(),
                                  Norm::L5(), Norm::L6(), Norm::L7(), Norm::L8()};

    double perception_error = 0.0;
    double mu_e = 0.0;

    for (const auto& norm : l8_norms) {
        Game sim(0.01, perception_error, mu_e, norm);
        double self_coop = sim.resident_coop;
        assert (self_coop > 0.98);
    }

    // 2. Leading Eight Should Be Nash Equilibria
    for (const auto& norm : l8_norms) {
        double assessment_error = 0.01;
        double benefit = 1.0;
        double cost = 0.1;

        Game sim(assessment_error, perception_error, mu_e, norm);

        // Self payoff
        double self_payoff = (benefit - cost) * sim.resident_coop;
        bool isNash = true;

        // Mutant payoff
        for (int i=0; i < 16; i++) {
            // Define Mutant Norm
            ActionRule invader = ActionRule::MakeDeterministicRule(i);

            // Calculate Payoff
            auto [H,coop_mut_to_res,coop_res_to_mut] = sim.calc_invader_stats(invader);
            double invader_payoff = benefit * coop_res_to_mut - cost * coop_mut_to_res;

            // Check Nash Equilibrium
            if (invader_payoff - self_payoff > 1e-6) {
                isNash = false;
                std::cout << invader.Inspect() << std::endl;
                std::cout << "Invader payoff: " << invader_payoff << std::endl;
                std::cout << "Resident payoff: " << self_payoff << std::endl;
                std::cout << "Coop Resident Towards Mutant: " << coop_res_to_mut << std::endl;
            }
        }
        assert (isNash);
    }

    // 3. AllD Action Rule should be Nash Equilibrium Regardless of the Assessment Error
    for (int j=0; j < 256; j++) {
        double assessment_error2 = 0.01;
        double benefit = 1.0, cost = 0.1;
        AssessmentRule RuleS = AssessmentRule::MakeDeterministicRule(j);
        Norm AllD{ RuleS, ActionRule::ALLD()};

        Game sim(assessment_error2, perception_error, mu_e, AllD);
        // Self payoff
        double self_payoff = (benefit - cost) * sim.resident_coop;
        bool isNash = true;

        // Mutant and their payoff
        for (int i=0; i < 16; i++) {
            // Define Mutant Norm
            ActionRule invader = ActionRule::MakeDeterministicRule(i);
            auto [H,coop_mut_to_res,coop_res_to_mut] = sim.calc_invader_stats(invader);
            double invader_payoff = benefit * coop_res_to_mut - cost * coop_mut_to_res;

            // Check Nash Equilibrium
            if (invader_payoff - self_payoff > 1e-6) {
                isNash = false;
            }
        }
        assert (isNash);
    }

    // 4. AllD Action Rule is ESS. Check with Delta v for all contexts
    for (int j = 0; j < 256; j++) {
        double assessment_error2 = 0.01;
        double benefit = 1.0, cost = 0.1;
        constexpr Reputation B = Reputation::B, G = Reputation::G;
        constexpr Action C = Action::C, D = Action::D;

        AssessmentRule RuleS = AssessmentRule::MakeDeterministicRule(j);
        Norm AllD = Norm(RuleS, ActionRule::ALLD());

        Game sim(assessment_error2, perception_error, mu_e, AllD);
        bool isESS = true;

        Norm AllD_r = AllD.RescaleWithError(assessment_error2, perception_error, mu_e);

        // Mutant and their payoff
        double delta_v = sim.calc_delta_v(benefit, cost);
        // std::cout << "Delta_u: " << delta_v << std::endl;

        std::vector<std::pair<Reputation, Reputation>> pairs = {
            {G, G},
            {G, B},
            {B, G},
            {B, B}
        };

        for (const auto& [X, Y] : pairs) {
            bool condition = (AllD_r.assessment_rule(X, Y, C) - 
                              AllD_r.assessment_rule(X, Y, D)) * delta_v < cost;

            if (condition != true) {
                isESS = false;
            }
        }
        assert(isESS);
    }

    // 5. All Leading Eight Action Rules are ESS. Check with Delta v for all contexts
    for (const auto& norm : l8_norms) {
        constexpr Reputation B = Reputation::B, G = Reputation::G;
        constexpr Action C = Action::C, D = Action::D;
        double assessment_error2 = 0.01;
        double benefit = 1.0, cost = 0.6;
        Game sim(assessment_error2, perception_error, mu_e, norm);

        // Mutant and their payoff
        double delta_u = sim.calc_delta_v(benefit, cost);
        Norm norm_r = norm.RescaleWithError(assessment_error2, perception_error, mu_e);

        std::vector<std::pair<Reputation, Reputation>> pairs = {
            {G, G},
            {G, B},
            {B, G},
            {B, B}
        };
        for (const auto& [X, Y] : pairs) {
            double lhs = (norm_r.assessment_rule(X, Y, C) - norm_r.assessment_rule(X, Y, D)) * delta_u;
            double rhs = cost;

            if (norm_r.action_rule(X, Y) == 1.0) {
                assert(lhs > rhs);
            }
            else {
                assert(lhs < rhs);
            }
        }
    }

    // 6. For all norms, calc_delta_v and calc_delta_v2 should be equal in the presence of implementation errors
    for (int j = 0; j < 4096; j++) {
        double assessment_error2 = 0.01;
        double perception_error = 0.03;
        double mu_e = 0.1;
        double benefit = 1.0, cost = 0.1;
        Norm norm = Norm::ConstructFromID(j);
        Game sim(assessment_error2, perception_error, mu_e, norm);

        double delta_v = sim.calc_delta_v(benefit, cost);
        double delta_v2 = sim.calc_delta_v2(benefit, cost);
        // std::cout << "Norm ID: " << norm.ID() << ", Delta_v: " << delta_v << ", Delta_v2: " << delta_v2 << std::endl;
        assert(std::abs(delta_v - delta_v2) < 1e-6);
    }

    // 7. Secondary sixteen should be ESS if b/c > 2.0
    for (int i = 1; i <= 16; i++) {
        double assessment_error = 0.01;
        double perception_error = 0.0;
        double mu_e = 0.0;

        Norm norm = Norm::SecondarySixteen(i);
        Game sim(assessment_error, perception_error, mu_e, norm);
        bool isESS_3 = sim.isESS(3.0, 1.0);
        assert(isESS_3);
        bool isESS_15 = sim.isESS(1.5, 1.0);
        assert(!isESS_15);
    }
}