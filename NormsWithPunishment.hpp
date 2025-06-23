#ifndef NormsWithPunishment_H
#define NormsWithPunishment_H

#include <vector>
#include <array>
#include <map>
#include <iostream>
#include <sstream>
#include <cassert>

enum class Action {
    D = 0,
    C = 1,
    P = 2
  };

enum class Reputation {
    B = 0,
    G = 1
  };

std::string ReputationToString(Reputation rep) {
    switch (rep) {
        case Reputation::B: return "B";
        case Reputation::G: return "G";
        default: return "Unknown";
    }
};

std::string ActionToString(Action act) {
    switch (act) {
        case Action::D: return "D";
        case Action::C: return "C";
        case Action::P: return "P";
        default: return "Unknown";
    }
};

Action IntToAction(int value) {
    switch (value) {
        case 0: return Action::D;
        case 1: return Action::C;
        case 2: return Action::P;
        default:
            throw std::out_of_range("Invalid integer for Action");
    }
};

class ActionRule {
    public:
        std::array<Action, 4> actions_vector;

        // Constructor using std::array
        ActionRule(const std::array<Action, 4> & actions_vector) : actions_vector(actions_vector) {}

        // Constructor using std::map
        ActionRule(const std::map<std::pair<Reputation, Reputation>, Action>& actions) {
            if (actions.size() != 4) {
                throw std::runtime_error("Unspecified actions");
            }

            actions_vector = {
                actions.at({Reputation::B, Reputation::B}),
                actions.at({Reputation::B, Reputation::G}),
                actions.at({Reputation::G, Reputation::B}),
                actions.at({Reputation::G, Reputation::G})
            };
        }

        Action operator()(Reputation r1, Reputation r2) const {
            if (r1 == Reputation::B && r2 == Reputation::B) return actions_vector[0];
            if (r1 == Reputation::B && r2 == Reputation::G) return actions_vector[1];
            if (r1 == Reputation::G && r2 == Reputation::B) return actions_vector[2];
            if (r1 == Reputation::G && r2 == Reputation::G) return actions_vector[3];
            throw std::runtime_error("Invalid reputation pair");
        }

        int ID() const {
            int id = 0;
            int power = 1;
            for (size_t i = 0; i < 4; ++i) {
                id += static_cast<int>(actions_vector[i]) * power;
                power *= 3;
            }
            return id;
        }

        static ActionRule MakeDeterministicRule(int id) {
            if (id < 0 || id > 80) {
                throw std::runtime_error("AssessmentRuleDet: id must be between 0 and 80");
            }
            std::array<Action, 4> actions_vec;
            for (size_t i = 0; i < actions_vec.size(); ++i) {
                actions_vec[i] = IntToAction(id % 3);
                id /= 3;
            }
            return ActionRule{ actions_vec };
        }

        static ActionRule DISC() {
            return ActionRule{ {Action::D, Action::C, Action::D, Action::C} };
        }
        static ActionRule ALLC() {
            return ActionRule{ {Action::C, Action::C, Action::C, Action::C} };
        }
        static ActionRule ALLD() {
            return ActionRule{ {Action::D, Action::D, Action::D, Action::D} };
        }
        static ActionRule ALLP() {
            return ActionRule{ {Action::P, Action::P, Action::P, Action::P} };
        }

        std::string Inspect() const {
            std::stringstream ss;
            ss << "ActionRule: " << ID() << std::endl;
            ss << "==============" << std::endl;
            for (size_t i = 0; i < 4; i++) {
                Reputation rep_d = static_cast<Reputation>(i / 2);
                Reputation rep_r = static_cast<Reputation>(i % 2);
                ss << "(" << ReputationToString(rep_d) << " -> " << ReputationToString(rep_r) << ") : " 
                   << ActionToString(actions_vector[i]) << "\n";
            }
            return ss.str();
        }
};

bool operator==(const ActionRule& t1, const ActionRule& t2) {
    return t1.actions_vector[0] == t2.actions_vector[0] &&
           t1.actions_vector[1] == t2.actions_vector[1] &&
           t1.actions_vector[2] == t2.actions_vector[2] &&
           t1.actions_vector[3] == t2.actions_vector[3];}

bool operator!=(const ActionRule& t1, const ActionRule& t2) { return !(t1 == t2);}

class AssessmentRule {
    public:
        std::array<double, 12> good_probs;

        // Constructor using std::array
        explicit AssessmentRule(const std::array<double, 12>& g_probs) {
            for (size_t i = 0; i < good_probs.size(); ++i) {
                good_probs[i] = g_probs[i];
            }
        }

        AssessmentRule RescaleWithError(double assignment_error = 0, double perception_error = 0) const {
            std::array<double,12> rescaled = {0.0};
            for (size_t i = 0; i < 12; i++) {
                rescaled[i] = (1 - assignment_error) * good_probs[i] + assignment_error * (1 - good_probs[i]);
            }
            for (size_t i = 0; i < 12; i += 3) {
                rescaled[i] = (1 - perception_error) * rescaled[i] + perception_error * rescaled[i + 1];
              }
            return AssessmentRule{rescaled};
          }

        // Constructor using std::map
        explicit AssessmentRule(const std::map<std::tuple<Reputation, Reputation, Action>, double>& g_probs) {
            if (g_probs.size() != 12) {
                throw std::runtime_error("AssessmentRule: g_probs must have 12 elements");
            }
            good_probs[0] = g_probs.at({Reputation::B, Reputation::B, Action::D});
            good_probs[1] = g_probs.at({Reputation::B, Reputation::B, Action::C});
            good_probs[2] = g_probs.at({Reputation::B, Reputation::B, Action::P});
            good_probs[3] = g_probs.at({Reputation::B, Reputation::G, Action::D});
            good_probs[4] = g_probs.at({Reputation::B, Reputation::G, Action::C});
            good_probs[5] = g_probs.at({Reputation::B, Reputation::G, Action::P});
            good_probs[6] = g_probs.at({Reputation::G, Reputation::B, Action::D});
            good_probs[7] = g_probs.at({Reputation::G, Reputation::B, Action::C});
            good_probs[8] = g_probs.at({Reputation::G, Reputation::B, Action::P});
            good_probs[9] = g_probs.at({Reputation::G, Reputation::G, Action::D});
            good_probs[10] = g_probs.at({Reputation::G, Reputation::G, Action::C});
            good_probs[11] = g_probs.at({Reputation::G, Reputation::G, Action::P});
        }

        double operator()(Reputation r1, Reputation r2, Action a) const {
            int index;

            if (r1 == Reputation::B && r2 == Reputation::B && a == Action::D) index = 0;
            else if (r1 == Reputation::B && r2 == Reputation::B && a == Action::C) index = 1;
            else if (r1 == Reputation::B && r2 == Reputation::B && a == Action::P) index = 2;
            else if (r1 == Reputation::B && r2 == Reputation::G && a == Action::D) index = 3;
            else if (r1 == Reputation::B && r2 == Reputation::G && a == Action::C) index = 4;
            else if (r1 == Reputation::B && r2 == Reputation::G && a == Action::P) index = 5;
            else if (r1 == Reputation::G && r2 == Reputation::B && a == Action::D) index = 6;
            else if (r1 == Reputation::G && r2 == Reputation::B && a == Action::C) index = 7;
            else if (r1 == Reputation::G && r2 == Reputation::B && a == Action::P) index = 8;
            else if (r1 == Reputation::G && r2 == Reputation::G && a == Action::D) index = 9;
            else if (r1 == Reputation::G && r2 == Reputation::G && a == Action::C) index = 10;
            else if (r1 == Reputation::G && r2 == Reputation::G && a == Action::P) index = 11;
            else throw std::runtime_error("Invalid reputation-action combination");

            return good_probs[index];
        }

        void Set(Reputation r1, Reputation r2, Action a, double p) {
            int index;
            if (r1 == Reputation::B && r2 == Reputation::B && a == Action::D) index = 0;
            else if (r1 == Reputation::B && r2 == Reputation::B && a == Action::C) index = 1;
            else if (r1 == Reputation::B && r2 == Reputation::B && a == Action::P) index = 2;
            else if (r1 == Reputation::B && r2 == Reputation::G && a == Action::D) index = 3;
            else if (r1 == Reputation::B && r2 == Reputation::G && a == Action::C) index = 4;
            else if (r1 == Reputation::B && r2 == Reputation::G && a == Action::P) index = 5;
            else if (r1 == Reputation::G && r2 == Reputation::B && a == Action::D) index = 6;
            else if (r1 == Reputation::G && r2 == Reputation::B && a == Action::C) index = 7;
            else if (r1 == Reputation::G && r2 == Reputation::B && a == Action::P) index = 8;
            else if (r1 == Reputation::G && r2 == Reputation::G && a == Action::D) index = 9;
            else if (r1 == Reputation::G && r2 == Reputation::G && a == Action::C) index = 10;
            else if (r1 == Reputation::G && r2 == Reputation::G && a == Action::P) index = 11;
            else throw std::runtime_error("Invalid reputation-action combination");

            good_probs[index] = p;
        }

        int ID() const {
            int id = 0;
            for (size_t i = 0; i < 12; i++) {
                if (good_probs[i] == 1.0) {
                    id += 1 << i;
                }
            }
            return id;
        }

        static AssessmentRule MakeDeterministicRule(int id) {
            if (id < 0 || id >= 4096) {
                throw std::runtime_error("AssessmentRuleDet: id must be between 0 and 4095");
            }

            std::array<double, 12> good_probs{};
            for (size_t i = 0; i < good_probs.size(); i++) {
                good_probs[i] = (id >> i) & 1;
            }

            return AssessmentRule(good_probs);
        }

        static AssessmentRule AllGood() {
            return AssessmentRule::MakeDeterministicRule(0b111111111111);
        }
        static AssessmentRule AllBad() {
            return AssessmentRule::MakeDeterministicRule(0b000000000000);
        }
        static AssessmentRule ImageScoring() {
            return AssessmentRule{{0.0,1.0,0.0,0.0,1.0,0.0,0.0,1.0,0.0,0.0,1.0,0.0}};
        }
        std::string Inspect() const {
            std::stringstream ss;
            ss << "AssessmentRule: " << ID() << std::endl;
            ss << "===================" << std::endl;

            for (size_t i = 0; i < 12; i++) {
                Reputation donor_rep = static_cast<Reputation>((i / 6) % 2);
                Reputation recip_rep = static_cast<Reputation>((i / 3) % 2);
                Action action = static_cast<Action>((i / 1) % 3);

                ss << "("
                << "Donor: " << ReputationToString(donor_rep) << ", "
                << "Recipient: " << ReputationToString(recip_rep) << ", "
                << "Action: " << ActionToString(action) << ") : "
                << good_probs[i] << "\n";
            }

            return ss.str();
        }
};

bool operator==(const AssessmentRule& t1, const AssessmentRule& t2) {
    return t1.good_probs[0] == t2.good_probs[0] &&
           t1.good_probs[1] == t2.good_probs[1] &&
           t1.good_probs[2] == t2.good_probs[2] &&
           t1.good_probs[3] == t2.good_probs[3] &&
           t1.good_probs[4] == t2.good_probs[4] &&
           t1.good_probs[5] == t2.good_probs[5] &&
           t1.good_probs[6] == t2.good_probs[6] &&
           t1.good_probs[7] == t2.good_probs[7] &&
           t1.good_probs[8] == t2.good_probs[8] &&
           t1.good_probs[9] == t2.good_probs[9] &&
           t1.good_probs[10] == t2.good_probs[10] &&
           t1.good_probs[11] == t2.good_probs[11];
}
bool operator!=(const AssessmentRule& t1, const AssessmentRule& t2) {
    return !(t1 == t2);
};

class Norm {
    public:
        ActionRule action_rule;
        AssessmentRule assessment_rule;

        Norm(const AssessmentRule& as_rule, const ActionRule& a_rule)
            : assessment_rule(as_rule), action_rule(a_rule) {}

    int ID() const {
        int id = 0;
        id += assessment_rule.ID() << 7;
        id += action_rule.ID();
        return id;
    }

    static Norm ConstructFromID(int id) {
        if (id < 0 || id >= (1 << 19)) {
            throw std::runtime_error("Norm: id must be between 0 and 2^19 - 1");
        }

        int assessment_id = id >> 7;
        int action_id = id & 0x7F;

        return Norm(AssessmentRule::MakeDeterministicRule(assessment_id),
                    ActionRule::MakeDeterministicRule(action_id));
    }

    Norm RescaleWithError(double assignment_error, double perception_error) const {
        return Norm{assessment_rule.RescaleWithError(assignment_error, perception_error), action_rule};
    }
};
#endif