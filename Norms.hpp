#ifndef Norms_H
#define Norms_H

#include <vector>
#include <array>
#include <map>
#include <iostream>
#include <sstream>
#include <cassert>

enum class Action {
    D = 0,
    C = 1
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
        default: return "Unknown";
    }
};

class ActionRule {
    public:
        std::array<double, 4> coop_probs;

        // Constructor using std::array
        ActionRule(const std::array<double, 4>& coop_probs) : coop_probs(coop_probs) {}

        // Constructor using std::map
        ActionRule(const std::map<std::pair<Reputation, Reputation>, double>& actions) {
            if (actions.size() != 4) {
                throw std::runtime_error("Unspecified actions");
            }

            coop_probs = {
                actions.at({Reputation::B, Reputation::B}),
                actions.at({Reputation::B, Reputation::G}),
                actions.at({Reputation::G, Reputation::B}),
                actions.at({Reputation::G, Reputation::G})
            };
        }

        double operator()(Reputation r1, Reputation r2) const {
            if (r1 == Reputation::B && r2 == Reputation::B) return coop_probs[0];
            if (r1 == Reputation::B && r2 == Reputation::G) return coop_probs[1];
            if (r1 == Reputation::G && r2 == Reputation::B) return coop_probs[2];
            if (r1 == Reputation::G && r2 == Reputation::G) return coop_probs[3];
            throw std::runtime_error("Invalid reputation pair");
        }

        bool IsDeterministic() const {
            if (( coop_probs[0] == 0.0 || coop_probs[0] == 1.0 ) &&
                ( coop_probs[1] == 0.0 || coop_probs[1] == 1.0 ) &&
                ( coop_probs[2] == 0.0 || coop_probs[2] == 1.0 ) &&
                ( coop_probs[3] == 0.0 || coop_probs[3] == 1.0 ) ) {
              return true;
            }
            return false;
        }

        int ID() const {
            if (!IsDeterministic()) { return -1; }
            int id = 0;
            for (size_t i = 0; i < 4; i++) {
                if (coop_probs[i] == 1.0) { id += 1 << i; }
            }
            return id;
        }

        static ActionRule MakeDeterministicRule(int id) {
            if (id < 0 || id > 15) {
              throw std::runtime_error("AssessmentRuleDet: id must be between 0 and 15");
            }
            std::array<double, 4> c_probs = {0.0};
            for (size_t i = 0; i < c_probs.size(); i++) {
              if ((id >> i) % 2) { c_probs[i] = 1.0; }
              else { c_probs[i] = 0.0; }
            }
            return ActionRule{ c_probs };
        }

        static ActionRule DISC() {
            return ActionRule{ {0.0, 1.0, 0.0, 1.0} };
        }
        static ActionRule ALLC() {
            return ActionRule{ {1.0, 1.0, 1.0, 1.0} };
        }
        static ActionRule ALLD() {
            return ActionRule{ {0.0, 0.0, 0.0, 0.0} };
        }

        std::string Inspect() const {
            std::stringstream ss;
            ss << "ActionRule: " << ID() << std::endl;
            ss << "==============" << std::endl;
            for (size_t i = 0; i < 4; i++) {
                Reputation rep_d = static_cast<Reputation>(i / 2);
                Reputation rep_r = static_cast<Reputation>(i % 2);
                ss << "(" << ReputationToString(rep_d) << " -> " << ReputationToString(rep_r) << ") : " 
                   << coop_probs[i] << "\n";
            }
            return ss.str();
        }

        ActionRule RescaleWithError(double implementation_error) const {
            std::array<double, 4> rescaled = {0.0};
            for (size_t i = 0; i < 4; i++) {
                rescaled[i] = coop_probs[i] * (1.0 - implementation_error);
            }
            return ActionRule{rescaled};
        }
};

bool operator==(const ActionRule& t1, const ActionRule& t2) {
    return t1.coop_probs[0] == t2.coop_probs[0] &&
           t1.coop_probs[1] == t2.coop_probs[1] &&
           t1.coop_probs[2] == t2.coop_probs[2] &&
           t1.coop_probs[3] == t2.coop_probs[3];}

bool operator!=(const ActionRule& t1, const ActionRule& t2) { return !(t1 == t2);}

class AssessmentRule {
    public:
        std::array<double, 8> good_probs;

        // Constructor using std::array
        AssessmentRule(const std::array<double, 8>& g_probs) {
            for (size_t i = 0; i < good_probs.size(); ++i) {
                good_probs[i] = g_probs[i];
            }
        }

        AssessmentRule RescaleWithError(double assignment_error = 0, double perception_error = 0) const {
            std::array<double,8> rescaled = {0.0};
            for (size_t i = 0; i < 8; i++) {
              rescaled[i] = (1 - assignment_error) * good_probs[i] + assignment_error * (1 - good_probs[i]);
            }
            for (size_t i = 0; i < 8; i += 2) {
                rescaled[i] = (1 - perception_error) * rescaled[i] + perception_error * rescaled[i + 1];
              }
            return AssessmentRule{rescaled};
          }

        // Constructor using std::map
        AssessmentRule(const std::map<std::tuple<Reputation, Reputation, Action>, double>& g_probs) {
            if (g_probs.size() != 8) {
                throw std::runtime_error("AssessmentRule: g_probs must have 8 elements");
            }
            good_probs[0] = g_probs.at({Reputation::B, Reputation::B, Action::D});
            good_probs[1] = g_probs.at({Reputation::B, Reputation::B, Action::C});
            good_probs[2] = g_probs.at({Reputation::B, Reputation::G, Action::D});
            good_probs[3] = g_probs.at({Reputation::B, Reputation::G, Action::C});
            good_probs[4] = g_probs.at({Reputation::G, Reputation::B, Action::D});
            good_probs[5] = g_probs.at({Reputation::G, Reputation::B, Action::C});
            good_probs[6] = g_probs.at({Reputation::G, Reputation::G, Action::D});
            good_probs[7] = g_probs.at({Reputation::G, Reputation::G, Action::C});
        }

        double operator()(Reputation r1, Reputation r2, Action a) const {
            int index;

            if (r1 == Reputation::B && r2 == Reputation::B && a == Action::D) index = 0;
            else if (r1 == Reputation::B && r2 == Reputation::B && a == Action::C) index = 1;
            else if (r1 == Reputation::B && r2 == Reputation::G && a == Action::D) index = 2;
            else if (r1 == Reputation::B && r2 == Reputation::G && a == Action::C) index = 3;
            else if (r1 == Reputation::G && r2 == Reputation::B && a == Action::D) index = 4;
            else if (r1 == Reputation::G && r2 == Reputation::B && a == Action::C) index = 5;
            else if (r1 == Reputation::G && r2 == Reputation::G && a == Action::D) index = 6;
            else if (r1 == Reputation::G && r2 == Reputation::G && a == Action::C) index = 7;
            else throw std::runtime_error("Invalid reputation-action combination");

            return good_probs[index];
        }

        void Set(Reputation r1, Reputation r2, Action a, double p) {
            int index;
            if (r1 == Reputation::B && r2 == Reputation::B && a == Action::D) index = 0;
            else if (r1 == Reputation::B && r2 == Reputation::B && a == Action::C) index = 1;
            else if (r1 == Reputation::B && r2 == Reputation::G && a == Action::D) index = 2;
            else if (r1 == Reputation::B && r2 == Reputation::G && a == Action::C) index = 3;
            else if (r1 == Reputation::G && r2 == Reputation::B && a == Action::D) index = 4;
            else if (r1 == Reputation::G && r2 == Reputation::B && a == Action::C) index = 5;
            else if (r1 == Reputation::G && r2 == Reputation::G && a == Action::D) index = 6;
            else if (r1 == Reputation::G && r2 == Reputation::G && a == Action::C) index = 7;
            else throw std::runtime_error("Invalid reputation-action combination");

            good_probs[index] = p;
        }

        bool IsDeterministic() const {
            if (( good_probs[0] == 0.0 || good_probs[0] == 1.0) &&
                ( good_probs[1] == 0.0 || good_probs[1] == 1.0) &&
                ( good_probs[2] == 0.0 || good_probs[2] == 1.0) &&
                ( good_probs[3] == 0.0 || good_probs[3] == 1.0) &&
                ( good_probs[4] == 0.0 || good_probs[4] == 1.0) &&
                ( good_probs[5] == 0.0 || good_probs[5] == 1.0) &&
                ( good_probs[6] == 0.0 || good_probs[6] == 1.0) &&
                ( good_probs[7] == 0.0 || good_probs[7] == 1.0) ) {
              return true;
            }
            return false;
        }

        int ID() const {
            if (!IsDeterministic()) { return -1; }
            int id = 0;
            for (size_t i = 0; i < 8; i++) {
              if (good_probs[i] == 1.0) { id += 1 << i; }
            }
            return id;
        }

        static AssessmentRule MakeDeterministicRule(int id) {
            if (id < 0 || id > 256) {
              throw std::runtime_error("AssessmentRuleDet: id must be between 0 and 15");
            }
            std::array<double, 8> good_probs = {0.0};
            for (size_t i = 0; i < good_probs.size(); i++) {
              if ((id >> i) % 2) { good_probs[i] = 1.0; }
              else { good_probs[i] = 0.0; }
            }
            return AssessmentRule(good_probs);
        }

        static AssessmentRule AllGood() {
            return AssessmentRule::MakeDeterministicRule(0b11111111);
        }
        static AssessmentRule AllBad() {
            return AssessmentRule::MakeDeterministicRule(0b00000000);
        }
        static AssessmentRule ImageScoring() {
            return AssessmentRule{{0,1,0,1,0,1,0,1}};
        }
        static AssessmentRule KeepRecipient() {
            return AssessmentRule{{0,0,1,1,0,0,1,1}};
        }

        std::string Inspect() const {
            std::stringstream ss;
            ss << "AssessmentRule: " << ID() << std::endl;
            ss << "===================" << std::endl;
            for (size_t i = 0; i < 8; i++) {
              Reputation rep_d = static_cast<Reputation>(i / 4);
              Reputation rep_r = static_cast<Reputation>((i / 2) % 2);
              Action act = static_cast<Action>(i % 2);
              ss << "(" << ReputationToString(rep_d) << " -> " << ReputationToString(rep_r) << ", "
              << ActionToString(act) << ") : " << good_probs[i] << "\n";
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
           t1.good_probs[7] == t2.good_probs[7];
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
    
        bool IsDeterministic() const {
            return action_rule.IsDeterministic() && assessment_rule.IsDeterministic();
        }

        int ID() const {
            if (!IsDeterministic()) {
                return -1;
                }
                int id = 0;
                id += assessment_rule.ID() << 4;
                id += action_rule.ID();
            return id;
        }

        static Norm ConstructFromID(int id) {
            if (id < 0 || id >= (1 << 20)) {
                throw std::runtime_error("Norm: id must be between 0 and 2^20-1");
            }
            int Rd_id = (id >> 4) & 0xFF;
            int P_id = id & 0xF;
                return Norm(AssessmentRule::MakeDeterministicRule(Rd_id),
                            ActionRule::MakeDeterministicRule(P_id));
        }

        Norm RescaleWithError(double assignment_error, double perception_error, double mu_e) const {
            return Norm{assessment_rule.RescaleWithError(assignment_error, perception_error), action_rule.RescaleWithError(mu_e)};
        }

        static Norm L1() {
            return Norm({{0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0}},
                        {{1.0, 1.0, 0.0, 1.0}});
          }
        static Norm L2() {
            return Norm({{0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0}},
                        {{1.0, 1.0, 0.0, 1.0}});
          }
        static Norm L3() {
            return Norm({{1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0}},
                        {{0.0, 1.0, 0.0, 1.0}});
          }
        static Norm L4() {
            return Norm({{1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0}},
                        {{0.0, 1.0, 0.0, 1.0}});
          }
        static Norm L5() {
            return Norm({{1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0}},
                        {{0.0, 1.0, 0.0, 1.0}});
          }
        static Norm L6() {
            return Norm({{1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0}},
                        {{0.0, 1.0, 0.0, 1.0}});
          }
        static Norm L7() {
            return Norm({{0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0}},
                        {{0.0, 1.0, 0.0, 1.0}});
          }
        static Norm L8() {
            return Norm({{0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0}},
                        {{0.0, 1.0, 0.0, 1.0}});
          }

        static Norm SecondarySixteen(int i) {
            if (i <= 0 || i > 16) { throw std::runtime_error("Norm: i must be between 1 and 16"); }
            double R_GB_C = static_cast<double>( ((i - 1) >> 3) & 0b1 );
            double R_BG_C = static_cast<double>( ((i - 1) >> 2) & 0b1 );
            double R_BB_C = static_cast<double>( ((i - 1) >> 1) & 0b1 );
            double R_BB_D = static_cast<double>( ((i - 1) >> 0) & 0b1 );
            double P_BB = 0.0;
            if (R_BB_C == 1.0 && R_BB_D == 0.0) { P_BB = 1.0; }

            return Norm({{R_BB_D, R_BB_C, 1, R_BG_C, 1, R_GB_C, 0, 1}},
                        {{P_BB, 0, 0, 1}});
        }

};

#endif