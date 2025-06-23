#include "NormsWithPunishment.hpp"
#include "GameWithPunishment.hpp"

int main() {
    constexpr Reputation B = Reputation::B, G = Reputation::G;
    constexpr Action C = Action::C, D = Action::D, P = Action::P;

    // AllD Action Rule should be Nash Equilibrium Regardless of the Assessment Error
    for (int j=0; j < 4096; j++) {
        double assessment_error = 0.0;
        double perception_error = 0.0;
        double benefit = 1.0, cost = 0.1, punishment = 0.1, punishment_cost = 0.05;
        AssessmentRule RuleS = AssessmentRule::MakeDeterministicRule(j);
        Norm AllD{ RuleS, ActionRule::ALLD()};

        Game sim(assessment_error, perception_error, AllD);
        // Self payoff
        double self_payoff = (benefit - cost) * sim.resident_coop;
        bool isNash = true;

        // Mutant and their payoff
        for (int i=0; i < 81; i++) {
            // Define Mutant Norm
            ActionRule invader = ActionRule::MakeDeterministicRule(i);
            auto [H,coop_mut_to_res,coop_res_to_mut,punishment_invader_to_resident,punishment_resident_to_invader] = sim.calc_invader_stats(invader);
            double invader_payoff = (benefit * coop_res_to_mut 
                                    - cost * coop_mut_to_res 
                                    - punishment * punishment_resident_to_invader 
                                    - punishment_cost * punishment_invader_to_resident);

            // Check Nash Equilibrium
            if (invader_payoff - self_payoff > 1e-6) {
                isNash = false;
            }
        }
        assert (isNash);
    }

    // AllD Action Rule is ESS. Check with Delta v for all contexts
    for (int j = 0; j < 4096; j++) {
        double benefit = 1.0, cost = 0.1, punishment = 0.1, punishment_cost = 0.05;
        double assessment_error = 0.01, perception_error = 0.0;
        constexpr Reputation B = Reputation::B, G = Reputation::G;
        constexpr Action C = Action::C, D = Action::D; Action P = Action::P;

        AssessmentRule RuleS = AssessmentRule::MakeDeterministicRule(j);
        Norm AllD = Norm(RuleS, ActionRule::ALLD());

        Game sim(assessment_error, perception_error, AllD);
        bool isESS = true;

        // Mutant and their payoff
        double delta_v = sim.calc_delta_v(benefit, cost, punishment, punishment_cost);

        std::vector<std::pair<Reputation, Reputation>> pairs = {
            {G, G},
            {G, B},
            {B, G},
            {B, B}
        };

        for (const auto& [X, Y] : pairs) {
            bool condition1 = (AllD.assessment_rule(X, Y, D) - 
                               AllD.assessment_rule(X, Y, C)) * delta_v > -cost;

            bool condition2 = (AllD.assessment_rule(X, Y, D) - 
                               AllD.assessment_rule(X, Y, P)) * delta_v > -punishment_cost;

            if (!condition1 || !condition2) {
                isESS = false;
                std::cout << "Delta_v: " << delta_v << std::endl;
            }
        }
        assert(isESS);
    }

    // check consistency with isESS and isESS2 methods
    {
        for (size_t i = 0; i < 4096; ++i) {
            for (size_t j = 0; j < 81; ++j) {
                AssessmentRule R = AssessmentRule::MakeDeterministicRule(i);
                ActionRule S = ActionRule::MakeDeterministicRule(j);
                Norm Resident{ R, S };

                double assessment_error = 0.01, perception_error = 0.0;
                double benefit = 1.0, cost = 0.1, punishment = 0.7, punishment_cost = 0.3;
                Game sim(assessment_error, perception_error, Resident);
                assert(sim.isESS(benefit, cost, punishment, punishment_cost) == sim.isESS2(benefit, cost, punishment, punishment_cost));
            }
        }
    }

    // Test an Instance of Class 1 for c > alpha
    // We set S(B, B) = D, R(B, B, D) = 0.0, R(B, B, C) = 0.0, R(B, B, P) = 0.0
    {
        double assessment_error = 0.01;
        double perception_error = 0.0;
        double benefit = 1.0, cost = 0.9, punishment = 0.1, punishment_cost = 0.05;

        std::map<std::tuple<Reputation, Reputation, Action>, double> actions_map = {
        {{Reputation::B, Reputation::B, Action::D}, 0.0},
        {{Reputation::B, Reputation::B, Action::C}, 0.0},
        {{Reputation::B, Reputation::B, Action::P}, 0.0},
        {{Reputation::B, Reputation::G, Action::D}, 0.0},
        {{Reputation::B, Reputation::G, Action::C}, 1.0},
        {{Reputation::B, Reputation::G, Action::P}, 0.0},
        {{Reputation::G, Reputation::B, Action::D}, 1.0},
        {{Reputation::G, Reputation::B, Action::C}, 0.0}, // Can be either 0 or 1
        {{Reputation::G, Reputation::B, Action::P}, 0.0}, // Can be either 0 or 1
        {{Reputation::G, Reputation::G, Action::D}, 0.0},
        {{Reputation::G, Reputation::G, Action::C}, 1.0},
        {{Reputation::G, Reputation::G, Action::P}, 0.0}};

        AssessmentRule RuleS(actions_map);


        ActionRule ActionS = {{D, C, D, C}};
        Norm class1_resident{ RuleS, ActionS };

        Game sim(assessment_error, perception_error, class1_resident);
        assert(sim.isESS(benefit, cost, punishment, punishment_cost));

        bool isESS = true;
        double delta_v = sim.calc_delta_v(benefit, cost, punishment, punishment_cost);
        // std::cout << "Delta_u: " << delta_v << std::endl;

        std::vector<std::pair<Reputation, Reputation>> pairs = {
            {G, G},
            {G, B},
            {B, G},
            {B, B}
        };

        for (const auto& [X, Y] : pairs) {

            double lhs1, lhs2;
            bool condition1, condition2;
            if (class1_resident.action_rule(X, Y) == C) {

                lhs1 = (class1_resident.assessment_rule(X, Y, C) - 
                               class1_resident.assessment_rule(X, Y, D)) * delta_v;

                lhs2 = (class1_resident.assessment_rule(X, Y, C) - 
                                class1_resident.assessment_rule(X, Y, P)) * delta_v;

                condition1 = lhs1 > cost;
                condition2 = lhs2 > cost - punishment_cost;
            }
            else if (class1_resident.action_rule(X, Y) == D) {

                lhs1 = (class1_resident.assessment_rule(X, Y, D) - 
                               class1_resident.assessment_rule(X, Y, C)) * delta_v;

                lhs2 = (class1_resident.assessment_rule(X, Y, D) - 
                                class1_resident.assessment_rule(X, Y, P)) * delta_v;

                condition1 = lhs1 > - cost;
                condition2 = lhs2 > - punishment_cost;
            }
            else if (class1_resident.action_rule(X, Y) == P) {

                lhs1 = (class1_resident.assessment_rule(X, Y, P) - 
                               class1_resident.assessment_rule(X, Y, C)) * delta_v;

                lhs2 = (class1_resident.assessment_rule(X, Y, P) - 
                                class1_resident.assessment_rule(X, Y, D)) * delta_v;

                condition1 = lhs1 > punishment_cost - cost;
                condition2 = lhs2 > punishment_cost;
            }

            if (!condition1 || !condition2) {
                isESS = false;
            }
        }
        assert(isESS);
    }

}