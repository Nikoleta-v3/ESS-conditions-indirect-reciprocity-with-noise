#include "Norms.hpp"

int main() {
    constexpr Reputation B = Reputation::B, G = Reputation::G;
    constexpr Action C = Action::C, D = Action::D;

    // TESTS FOR ACTION RULE

    // Test the definition of action rules using arrays
    std::array<double, 4> actions = {0.0, 1.0, 0.0, 1.0};
    ActionRule disc(actions);

    assert (disc(G, G) == 1.0);
    assert (disc(G, B) == 0.0);

    // Test the definition of action rules using maps
    std::map<std::pair<Reputation, Reputation>, double> actions_map = {
        {{B, B}, 0.0},
        {{B, G}, 1.0},
        {{G, B}, 0.0},
        {{G, G}, 1.0}
    };

    ActionRule disc_map(actions_map);
    assert (disc_map(G, G) == 1.0);
    assert (disc_map(G, B) == 0.0);

    // Test isDeterministic
    assert (disc.IsDeterministic());

    // Test ID
    assert (disc.ID() == 10);

    // Test MakeDeterministic
    ActionRule AllD = ActionRule::MakeDeterministicRule(0);
    for (size_t i = 0; i < 4; i++) {
        assert (AllD.coop_probs[i] == 0.0);
    }

    ActionRule AllC = ActionRule::MakeDeterministicRule(15);
    for (size_t i = 0; i < 4; i++) {
        assert (AllC.coop_probs[i] == 1.0);
    }

    // Test pre define actions rules & operations
    assert (disc == ActionRule::DISC());

    assert (AllC == ActionRule::ALLC());

    assert (AllD == ActionRule::ALLD());

    assert (AllD != AllC);


    // TESTS FOR ASSESSMENT RULE

    // Test the definition of assessment rules using arrays
    std::array<double, 8> assessments = {0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
    AssessmentRule image_scoring(assessments);

    assert (image_scoring(G, G, C) == 1.0);
    assert (image_scoring(G, B, D) == 0.0);

    // Test with assignment error
    AssessmentRule RuleWithErrors = image_scoring.RescaleWithError(0.1);

    assert (RuleWithErrors(G, G, C) == 0.9);
    assert (RuleWithErrors(G, B, D) == 0.1);

    // Test with perception error
    // Defection is misrecognized as cooperation
    AssessmentRule RuleWithErrors2 = image_scoring.RescaleWithError(0.0, 0.1);

    assert (RuleWithErrors2(G, G, C) == 1.0);
    assert (RuleWithErrors2(G, B, D) == 0.1);

    // Test both errors
    AssessmentRule RuleWithErrors3 = image_scoring.RescaleWithError(0.1, 0.1);

    assert (RuleWithErrors3(G, G, C) == 0.9);
    assert (RuleWithErrors3(G, B, D) == 2.0 * 0.1 * 0.9);

    // Test IsDeterministic
    assert (image_scoring.IsDeterministic());

    // Test ID
    assert (image_scoring.ID() == 170);

    // Test MakeDeterministic
    AssessmentRule AllGood = AssessmentRule::MakeDeterministicRule(255);
    for (size_t i = 0; i < 8; i++) {
        assert (AllGood.good_probs[i] == 1.0);
    }

    // Test pre define assessment rules & operations
    assert (AllGood == AssessmentRule::AllGood());

    // std::cout << AllGood.Inspect() << std::endl;

    // TESTS FOR NORMS

    // Test strategy definition

    Norm image_scoring_norm(image_scoring, disc);

    ActionRule S = image_scoring_norm.action_rule;
    AssessmentRule NormS = image_scoring_norm.assessment_rule;

    assert (NormS(G, G, C) == 1.0);
    assert (S(G, G) == 1.0);

    Norm is_with_error = image_scoring_norm.RescaleWithError(0.1, 0.0, 0.05);
    assert (is_with_error.action_rule(G, G) == 0.95);
    assert (is_with_error.assessment_rule(G, G, C) == 0.9);

    assert (image_scoring_norm.ID() == 0b10101010'1010);

    // Reverse
    int ID = 0b10101010'1010;

    Norm strategy_from_id = Norm::ConstructFromID(ID);
    assert (strategy_from_id.ID() == ID);
    assert (strategy_from_id.assessment_rule == NormS);
    assert (strategy_from_id.action_rule == S);
}