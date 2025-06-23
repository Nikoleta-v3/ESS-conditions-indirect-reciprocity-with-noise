#ifndef GAME_H
#define GAME_H

#include "Norms.hpp"


class Game {
    public:
        double assessment_error;
        double perception_error;
        double mu_e;
        Norm norm;
        Norm r_norm;
        double equilibrium_state;
        double resident_coop;

        static constexpr Reputation B = Reputation::B, G = Reputation::G;
        static constexpr Action C = Action::C, D = Action::D;

        Game(double assessment_error, double perception_error, double mu_e, const Norm& norm)
            : assessment_error(assessment_error), perception_error(perception_error), mu_e(mu_e), norm(norm),
              r_norm(norm.RescaleWithError(assessment_error, perception_error, mu_e)),
              equilibrium_state(calc_equilibrium_state()),
              resident_coop(calc_self_coop_resident()) {}


        double calc_equilibrium_state() {
            const AssessmentRule R = r_norm.assessment_rule;
            const ActionRule S = r_norm.action_rule;

            double RS_GG = R(G, G, C) * S(G, G) + R(G, G, D) * (1.0 - S(G, G));
            double RS_GB = R(G, B, C) * S(G, B) + R(G, B, D) * (1.0 - S(G, B));
            double RS_BG = R(B, G, C) * S(B, G) + R(B, G, D) * (1.0 - S(B, G));
            double RS_BB = R(B, B, C) * S(B, B) + R(B, B, D) * (1.0 - S(B, B));
            double c2 = RS_GG - RS_GB - RS_BG + RS_BB;
            double c1 = RS_GB + RS_BG - 2.0 * RS_BB - 1;
            double c0 = RS_BB;

            if (std::abs(c2) < 1e-9) {
                return -c0 / c1;
            } else {
                return (-c1 - std::sqrt(c1 * c1 - 4.0 * c2 * c0)) / (2.0 * c2);
            }
        }

        double calc_self_coop_resident () {
            ActionRule S = r_norm.action_rule;
            double c1 = equilibrium_state * equilibrium_state * S(G, G);
            double c2 = equilibrium_state * (1.0 - equilibrium_state) * (S(G, B) + S(B, G));
            double c3 = (1.0 - equilibrium_state) * (1.0 - equilibrium_state) * S(B, B);
            return c1 + c2 + c3;
        };

        std::tuple<double, double, double> calc_invader_stats(const ActionRule& invader_strategy) const {
            ActionRule S_mut = invader_strategy.RescaleWithError(mu_e);
            double H = calc_equilibrium_state_mutant(S_mut);
            double coop_invader_to_resident = 0.0;
            {
                double c1 = equilibrium_state * H * S_mut(G, G);
                double c2 = (1.0 - equilibrium_state) * H * S_mut(G, B);
                double c3 = equilibrium_state * (1.0 -  H) * S_mut(B, G);
                double c4 = (1.0 - equilibrium_state) * (1.0 - H) * S_mut(B, B);
                coop_invader_to_resident = c1 + c2 + c3 + c4;
            }
            double coop_resident_to_invader = 0.0;
            {
                ActionRule S = r_norm.action_rule;
                double c1 = equilibrium_state * H * S(G, G);
                double c2 = equilibrium_state * (1.0 - H) * S(G, B);
                double c3 = (1.0 - equilibrium_state) * H * S(B, G);
                double c4 = (1.0 - equilibrium_state) * (1.0 - H) * S(B, B);
                coop_resident_to_invader = c1 + c2 + c3 + c4;
            }
            return std::make_tuple(H, coop_invader_to_resident, coop_resident_to_invader);
        }

        double calc_delta_v(double benefit, double cost) const {
            AssessmentRule R = r_norm.assessment_rule;
            ActionRule S = r_norm.action_rule;

            double Num1 = benefit * (equilibrium_state * (S(G, G) - S(G, B))
                                    + (1.0 - equilibrium_state) * (S(B, G) - S(B, B)));
            double Num2 = cost * (equilibrium_state * (S(G, G) - S(B, G))
                                + (1.0 - equilibrium_state) * (S(G, B) - S(B, B)));

            double RS_GG = R(G, G, C) * S(G, G) + R(G, G, D) * (1.0 - S(G, G));
            double RS_GB = R(G, B, C) * S(G, B) + R(G, B, D) * (1.0 - S(G, B));
            double RS_BG = R(B, G, C) * S(B, G) + R(B, G, D) * (1.0 - S(B, G));
            double RS_BB = R(B, B, C) * S(B, B) + R(B, B, D) * (1.0 - S(B, B));
            double Den = 1.0 - equilibrium_state * (RS_GG - RS_BG) - (1.0 - equilibrium_state) * (RS_GB - RS_BB);

            return (Num1 - Num2) / Den;
        }

        double calc_delta_v2(double benefit, double cost) const {
            // conduct rescaling of mu_e against assessment_rule, benefit, and cost
            ActionRule S = norm.action_rule;  // S is not rescaled
            AssessmentRule R = r_norm.assessment_rule;
            {
                double ggc = (1.0 - mu_e) * R(G, G, C) + mu_e * R(G, G, D);
                R.Set(G, G, C, ggc);
                double gbc = (1.0 - mu_e) * R(G, B, C) + mu_e * R(G, B, D);
                R.Set(G, B, C, gbc);
                double bgc = (1.0 - mu_e) * R(B, G, C) + mu_e * R(B, G, D);
                R.Set(B, G, C, bgc);
                double bbc = (1.0 - mu_e) * R(B, B, C) + mu_e * R(B, B, D);
                R.Set(B, B, C, bbc);
            }
            double r_benefit = (1.0 - mu_e) * benefit;
            double r_cost = (1.0 - mu_e) * cost;

            double RS_GG = R(G, G, C) * S(G, G) + R(G, G, D) * (1.0 - S(G, G));
            double RS_GB = R(G, B, C) * S(G, B) + R(G, B, D) * (1.0 - S(G, B));
            double RS_BG = R(B, G, C) * S(B, G) + R(B, G, D) * (1.0 - S(B, G));
            double RS_BB = R(B, B, C) * S(B, B) + R(B, B, D) * (1.0 - S(B, B));

            double h = 0.0;
            {
                double c2 = RS_GG - RS_GB - RS_BG + RS_BB;
                double c1 = RS_GB + RS_BG - 2.0 * RS_BB - 1;
                double c0 = RS_BB;

                if (std::abs(c2) < 1e-9) {
                    h = -c0 / c1;
                } else {
                    h = (-c1 - std::sqrt(c1 * c1 - 4.0 * c2 * c0)) / (2.0 * c2);
                }
            }

            double Num1 = r_benefit * (h * (S(G, G) - S(G, B))
                                    + (1.0 - h) * (S(B, G) - S(B, B)));
            double Num2 = r_cost * (h * (S(G, G) - S(B, G))
                                + (1.0 - h) * (S(G, B) - S(B, B)));

            double Den = 1.0 - h * (RS_GG - RS_BG) - (1.0 - h) * (RS_GB - RS_BB);

            return (Num1 - Num2) / Den;
        }

        bool isESS(double benefit, double cost) const {
            double self_payoff = (benefit - cost) * resident_coop;
            for (int i=0; i < 16; i++) {
                if (norm.action_rule.ID() == i) {
                    continue; // Skip the resident strategy
                }
                // Define Mutant Norm
                ActionRule invader = ActionRule::MakeDeterministicRule(i);

                // Calculate Payoff
                auto [H,coop_mut_to_res,coop_res_to_mut] = calc_invader_stats(invader);
                double invader_payoff = benefit * coop_res_to_mut - cost * coop_mut_to_res;

                // Check Nash Equilibrium
                if (invader_payoff > self_payoff) {
                    return false;
                }
            }
            return true;
        }

    private:
        double calc_equilibrium_state_mutant(const ActionRule& invader_strategy) const {
            const AssessmentRule R = r_norm.assessment_rule;
            const ActionRule S = invader_strategy;

            double RS_GG = R(G, G, C) * S(G, G) + R(G, G, D) * (1.0 - S(G, G));
            double RS_GB = R(G, B, C) * S(G, B) + R(G, B, D) * (1.0 - S(G, B));
            double RS_BG = R(B, G, C) * S(B, G) + R(B, G, D) * (1.0 - S(B, G));
            double RS_BB = R(B, B, C) * S(B, B) + R(B, B, D) * (1.0 - S(B, B));

            double num = equilibrium_state * RS_BG + (1.0 - equilibrium_state) * RS_BB;
            double den = (1.0 - equilibrium_state * RS_GG +  equilibrium_state * RS_BG
                - (1.0 - equilibrium_state) * RS_GB + (1.0 - equilibrium_state) * RS_BB);
            return num / den;
        }
};

#endif