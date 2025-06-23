#ifndef GAME_H
#define GAME_H

#include "NormsWithPunishment.hpp"


class Game {
    public:
        double assessment_error;
        double perception_error;
        Norm norm;
        Norm r_norm;
        double equilibrium_state;
        double resident_coop;
        double resident_punishment;

        static constexpr Reputation B = Reputation::B, G = Reputation::G;
        static constexpr Action C = Action::C, D = Action::D, P = Action::P;

        Game(double assessment_error, double perception_error, const Norm& norm)
            : assessment_error(assessment_error), perception_error(perception_error), norm(norm),
              r_norm(norm.RescaleWithError(assessment_error, perception_error)),
              equilibrium_state(calc_equilibrium_state()),
              resident_coop(calc_self_coop_resident()),
              resident_punishment(calc_self_punishment_resident()) {}


        double calc_equilibrium_state() {
            const AssessmentRule R = r_norm.assessment_rule;
            const ActionRule S = r_norm.action_rule;

            auto [RS_GG, RS_GB, RS_BG, RS_BB] = calc_RSs(R, S);

            double c2 = RS_GG - RS_GB - RS_BG + RS_BB;
            double c1 = RS_GB + RS_BG - 2.0 * RS_BB - 1.0;
            double c0 = RS_BB;

            if (std::abs(c2) < 1e-9) {
                return -c0 / c1;
            } else {
                return (-c1 - std::sqrt(c1 * c1 - 4.0 * c2 * c0)) / (2.0 * c2);
            }
        }

        double calc_self_coop_resident() {
            ActionRule S = r_norm.action_rule;
            double c1 = 0.0, c2 = 0.0, c3 = 0.0;

            if (S(G, G) == C) {
                c1 = equilibrium_state * equilibrium_state;
            }
            if (S(B, B) == C) {
                c3 = (1.0 - equilibrium_state) * (1.0 - equilibrium_state);
            }
            if (S(G, B) == C) {
                c2 += equilibrium_state * (1.0 - equilibrium_state);
            }
            if (S(B, G) == C) {
                c2 += equilibrium_state * (1.0 - equilibrium_state);
            }
            return c1 + c2 + c3;
        };


        double calc_self_punishment_resident () {
            ActionRule S = r_norm.action_rule;
            double c1 = 0.0, c2 = 0.0, c3 = 0.0;

            if (S(G, G) == P) {
                c1 = equilibrium_state * equilibrium_state;
            }
            if (S(B, B) == P) {
                c3 = (1.0 - equilibrium_state) * (1.0 - equilibrium_state);
            }
            if (S(G, B) == P) {
                c2 += equilibrium_state * (1.0 - equilibrium_state);
            }
            if (S(B, G) == P) {
                c2 += equilibrium_state * (1.0 - equilibrium_state);
            }
            return c1 + c2 + c3;
        };

        std::tuple<double, double, double, double, double> calc_invader_stats(const ActionRule& invader_strategy) const {
            ActionRule S_mut = invader_strategy;
            double H = calc_equilibrium_state_mutant(S_mut);
            ActionRule S = norm.action_rule;
        
            double coop_invader_to_resident = 0.0;
            {   double c1 = 0.0, c2 = 0.0, c3 = 0.0, c4 = 0.0;
                if (S_mut(G, G) == C) {
                    c1 = equilibrium_state * H;
                }
                if (S_mut(G, B) == C) {
                    c2 = (1.0 - equilibrium_state) * H;
                }
                if (S_mut(B, G) == C) {
                    c3 = equilibrium_state * (1.0 - H);
                }
                if (S_mut(B, B) == C) {
                    c4 = (1.0 - equilibrium_state) * (1.0 - H);
                }
                coop_invader_to_resident = c1 + c2 + c3 + c4;
            }
            double coop_resident_to_invader = 0.0;
            {   double c1 = 0.0, c2 = 0.0, c3 = 0.0, c4 = 0.0;
                if (S(G, G) == C) {
                    c1 = equilibrium_state * H;
                }
                if (S(G, B) == C) {
                    c2 = equilibrium_state * (1.0 - H);
                }
                if (S(B, G) == C) {
                    c3 = (1.0 - equilibrium_state) * H;
                }
                if (S(B, B) == C) {
                    c4 = (1.0 - equilibrium_state) * (1.0 - H);
                }
                coop_resident_to_invader = c1 + c2 + c3 + c4;
            }
            double punishment_invader_to_resident = 0.0;
            {   double c1 = 0.0, c2 = 0.0, c3 = 0.0, c4 = 0.0;
                if (S_mut(G, G) == P) {
                    c1 = equilibrium_state * H;
                }
                if (S_mut(G, B) == P) {
                    c2 = (1.0 - equilibrium_state) * H;
                }
                if (S_mut(B, G) == P) {
                    c3 = equilibrium_state * (1.0 - H);
                }
                if (S_mut(B, B) == P) {
                    c4 = (1.0 - equilibrium_state) * (1.0 - H);
                }
                punishment_invader_to_resident = c1 + c2 + c3 + c4;
            }
            double punishment_resident_to_invader = 0.0;
            {   double c1 = 0.0, c2 = 0.0, c3 = 0.0, c4 = 0.0;
                if (S(G, G) == P) {
                    c1 = equilibrium_state * H;
                }
                if (S(G, B) == P) {
                    c2 = equilibrium_state * (1.0 - H);
                }
                if (S(B, G) == P) {
                    c3 = (1.0 - equilibrium_state) * H;
                }
                if (S(B, B) == P) {
                    c4 = (1.0 - equilibrium_state) * (1.0 - H);
                }
                punishment_resident_to_invader = c1 + c2 + c3 + c4;
            }
            return std::make_tuple(H,
                                   coop_invader_to_resident,
                                   coop_resident_to_invader,
                                   punishment_invader_to_resident,
                                   punishment_resident_to_invader);
        }

        double calc_delta_v(double benefit, double cost, double punishment, double punishment_cost) const {
            AssessmentRule R = r_norm.assessment_rule;
            ActionRule S = r_norm.action_rule;

            double Num1 = 0.0, Num2 = 0.0, Num3 = 0.0, Num4 = 0.0;

            double b_term = 0.0;
            if (S(G, G) == C) b_term += equilibrium_state;
            if (S(G, B) == C) b_term -= equilibrium_state;
            if (S(B, G) == C) b_term += (1.0 - equilibrium_state);
            if (S(B, B) == C) b_term -= (1.0 - equilibrium_state);
            Num1 = benefit * b_term;

            double c_term = 0.0;
            if (S(G, G) == C) c_term += equilibrium_state;
            if (S(B, G) == C) c_term -= equilibrium_state;
            if (S(G, B) == C) c_term += (1.0 - equilibrium_state);
            if (S(B, B) == C) c_term -= (1.0 - equilibrium_state);
            Num2 = cost * c_term;

            double p_term = 0.0;
            if (S(G, G) == P) p_term += equilibrium_state;
            if (S(G, B) == P) p_term -= equilibrium_state;
            if (S(B, G) == P) p_term += (1.0 - equilibrium_state);
            if (S(B, B) == P) p_term -= (1.0 - equilibrium_state);
            Num3 = punishment * p_term;

            double pc_term = 0.0;
            if (S(G, G) == P) pc_term += equilibrium_state;
            if (S(B, G) == P) pc_term -= equilibrium_state;
            if (S(G, B) == P) pc_term += (1.0 - equilibrium_state);
            if (S(B, B) == P) pc_term -= (1.0 - equilibrium_state);
            Num4 = punishment_cost * pc_term;

            auto [RS_GG, RS_GB, RS_BG, RS_BB] = calc_RSs(R, S);
            double Den = 1.0 - equilibrium_state * (RS_GG - RS_BG) - (1.0 - equilibrium_state) * (RS_GB - RS_BB);

            return (Num1 - Num2 - Num3 - Num4) / Den;
        }

    
        bool isESS(double benefit, double cost, double punishment, double punishment_cost) const {
            double self_payoff = (benefit - cost) * resident_coop - (punishment + punishment_cost) * resident_punishment;
            for (int i=0; i < 81; i++) {
                if (norm.action_rule.ID() == i) {
                    continue; // Skip the resident strategy
                }
                // Define Mutant Norm
                ActionRule invader = ActionRule::MakeDeterministicRule(i);

                // Calculate Payoff
                auto [H,coop_mut_to_res,coop_res_to_mut,punishment_invader_to_resident,punishment_resident_to_invader] = calc_invader_stats(invader);
                double invader_payoff = (benefit * coop_res_to_mut 
                                         - cost * coop_mut_to_res 
                                         - punishment * punishment_resident_to_invader 
                                         - punishment_cost * punishment_invader_to_resident);

                // Check Nash Equilibrium
                if (invader_payoff > self_payoff) {
                    return false;
                }
            }
            return true;
        }

        bool isESS2(double benefit, double cost, double punishment, double punishment_cost) const {
            std::vector<std::pair<Reputation, Reputation>> rep_pairs = {
                {G, G},
                {G, B},
                {B, G},
                {B, B}
            };
            double delta_v = calc_delta_v(benefit, cost, punishment, punishment_cost);

            for (const auto& [X, Y] : rep_pairs) {
                double lhs1 = 0.0, lhs2 = 0.0;
                bool condition1 = false, condition2 = false;
                const ActionRule S = r_norm.action_rule;
                const AssessmentRule R = r_norm.assessment_rule;
                if (S(X, Y) == C) {
                    lhs1 = (R(X, Y, C) - R(X, Y, D)) * delta_v;
                    lhs2 = (R(X, Y, C) - R(X, Y, P)) * delta_v;
                    condition1 = lhs1 > cost;
                    condition2 = lhs2 > cost - punishment_cost;
                }
                else if (S(X, Y) == D) {
                    lhs1 = (R(X, Y, D) - R(X, Y, C)) * delta_v;
                    lhs2 = (R(X, Y, D) - R(X, Y, P)) * delta_v;
                    condition1 = lhs1 > - cost;
                    condition2 = lhs2 > - punishment_cost;
                }
                else if (S(X, Y) == P) {
                    lhs1 = (R(X, Y, P) - R(X, Y, C)) * delta_v;
                    lhs2 = (R(X, Y, P) - R(X, Y, D)) * delta_v;
                    condition1 = lhs1 > punishment_cost - cost;
                    condition2 = lhs2 > punishment_cost;
                }

                if (!condition1 || !condition2) {
                    return false;
                }
            }
            return true;
        }

    private:
        double calc_equilibrium_state_mutant(const ActionRule& invader_strategy) const {
            const AssessmentRule R = r_norm.assessment_rule;
            const ActionRule S = invader_strategy;

            auto [RS_GG, RS_GB, RS_BG, RS_BB] = calc_RSs(R, S);

            double num = equilibrium_state * RS_BG + (1.0 - equilibrium_state) * RS_BB;
            double den = (1.0 - equilibrium_state * RS_GG +  equilibrium_state * RS_BG
                - (1.0 - equilibrium_state) * RS_GB + (1.0 - equilibrium_state) * RS_BB);
            return num / den;
        }

        std::tuple<double, double, double, double> calc_RSs(const AssessmentRule& R, const ActionRule& S) const {
            double RS_GG, RS_GB, RS_BG, RS_BB;
            if (S(G, G) == C) {
                RS_GG = R(G, G, C);
            } else if (S(G, G) == P) {
                RS_GG = R(G, G, P);
            } else if (S(G, G) == D) {
                RS_GG = R(G, G, D);
            };

            if (S(G, B) == C) {
                RS_GB = R(G, B, C);
            } else if (S(G, B) == P) {
                RS_GB = R(G, B, P);
            } else if (S(G, B) == D) {
                RS_GB = R(G, B, D);
            };

            if (S(B, G) == C) {
                RS_BG = R(B, G, C);
            } else if (S(B, G) == P) {
                RS_BG = R(B, G, P);
            } else if (S(B, G) == D) {
                RS_BG = R(B, G, D);
            };

            if (S(B, B) == C) {
                RS_BB = R(B, B, C);
            } else if (S(B, B) == P) {
                RS_BB = R(B, B, P);
            } else if (S(B, B) == D) {
                RS_BB = R(B, B, D);
            };
        return std::make_tuple(RS_GG, RS_GB, RS_BG, RS_BB);
        }

    };

#endif