#include "NormsWithPunishment.hpp"

int main() {
    constexpr Reputation B = Reputation::B, G = Reputation::G;
    constexpr Action C = Action::C, D = Action::D, P = Action::P;

    // TESTS FOR ACTION RULE

    // Test the definition of action rules using arrays
    std::array<Action, 4> actions = {D, C, D, C};
    ActionRule disc(actions);

    assert (disc(G, G) == C);
    assert (disc(G, B) == D);

    // Test the definition of action rules using maps
    std::map<std::pair<Reputation, Reputation>, Action> actions_map = {
        {{B, B}, D},
        {{B, G}, C},
        {{G, B}, D},
        {{G, G}, C}
    };

    ActionRule disc_map(actions_map);
    assert (disc_map(G, G) == C);
    assert (disc_map(G, B) == D);

    // Test punishment action
    std::map<std::pair<Reputation, Reputation>, Action> actions_map_punishment = {
        {{B, B}, D},
        {{B, G}, P},
        {{G, B}, D},
        {{G, G}, C}
    };
    ActionRule disc_map_punishment(actions_map_punishment);
    assert (disc_map_punishment(G, G) == C);
    assert (disc_map_punishment(G, B) == D);
    assert (disc_map_punishment(B, G) == P);
    assert (disc_map_punishment(B, B) == D);

    // Test ID
    std::array<Action, 4> alld_actions = {D, D, D, D};
    ActionRule AllD(alld_actions);
    assert (AllD.ID() == 0);

    std::array<Action, 4> allp_actions = {P, P, P, P};
    ActionRule AllP(allp_actions);
    assert (AllP.ID() == 80);

    std::array<Action, 4> allc_actions = {C, C, C, C};
    ActionRule AllC(allc_actions);
    assert (AllC.ID() == 40);

    // Test MakeDeterministic
    ActionRule AllD_from_make = ActionRule::MakeDeterministicRule(0);
    assert (AllD == AllD_from_make);

    ActionRule AllC_from_make = ActionRule::MakeDeterministicRule(40);
    assert (AllC == AllC_from_make);

    ActionRule AllP_from_make = ActionRule::MakeDeterministicRule(80);
    assert (AllP == AllP_from_make);

    // Test pre define actions rules & operations
    assert (disc == ActionRule::DISC());

    assert (AllC == ActionRule::ALLC());

    assert (AllD == ActionRule::ALLD());

    assert (AllP == ActionRule::ALLP());

    assert (AllD != AllC);

    // std::cout << AllP.Inspect() << std::endl;

   // TESTS FOR ASSESSMENT RULE

    // Test the definition of assessment rules using arrays
    std::array<double, 12> assessments = {0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0};
    AssessmentRule image_scoring(assessments);

    assert (image_scoring(G, G, C) == 1.0);
    assert (image_scoring(G, B, D) == 0.0);

    std::array<double, 12> assessments_punishment = {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0};
    AssessmentRule only_good_when_punishing(assessments_punishment);

    assert (only_good_when_punishing(G, G, P) == 1.0);
    assert (only_good_when_punishing(G, B, P) == 1.0);
    assert (only_good_when_punishing(B, G, P) == 1.0);
    assert (only_good_when_punishing(B, B, P) == 1.0);

    // Test ID
    assert (image_scoring.ID() == 1170);

    // Test pre define assessment rules & operations
    assert (image_scoring == AssessmentRule::ImageScoring());

    // Test with assignment error
    AssessmentRule RuleWithErrors = image_scoring.RescaleWithError(0.1);

    assert (RuleWithErrors(G, G, C) == 0.9);
    assert (RuleWithErrors(G, B, D) == 0.1);

    AssessmentRule all_good = AssessmentRule::AllGood();
    assert (all_good.ID() == 4095);
    AssessmentRule all_bad = AssessmentRule::AllBad();
    assert (all_bad.ID() == 0);
    // std::cout << image_scoring.Inspect() << std::endl;

    // TESTS FOR NORMS

    // Test strategy definition
    Norm image_scoring_norm(image_scoring, disc);

    ActionRule S = image_scoring_norm.action_rule;
    AssessmentRule NormS = image_scoring_norm.assessment_rule;

    assert (NormS(G, G, C) == 1);
    assert (S(G, G) == C);

    assert (image_scoring_norm.ID() == 0b10010010010'0011110);

    // Reverse
    int ID = 0b10010010010'0011110;

    Norm strategy_from_id = Norm::ConstructFromID(ID);
    assert (strategy_from_id.ID() == ID);
    assert (strategy_from_id.assessment_rule == NormS);
    assert (strategy_from_id.action_rule == S);

    Norm is_with_error = image_scoring_norm.RescaleWithError(0.1, 0.0);
    assert (is_with_error.action_rule(G, G) == C);
    assert (is_with_error.assessment_rule(G, G, C) == 0.9);

}