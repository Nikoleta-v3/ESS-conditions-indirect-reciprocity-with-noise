cmake_minimum_required(VERSION 3.7)
project(indirect_recip)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADER_FILES Norms.hpp AllNorms.hpp Game.hpp)

add_executable(test_game test_game.cpp ${HEADER_FILES})

add_executable(test_norms test_norms.cpp ${HEADER_FILES})

add_executable(test_norms_with_punishment test_norms_with_punishment.cpp NormsWithPunishment.hpp)

add_executable(test_game_with_punishment test_game_with_punishment.cpp NormsWithPunishment.hpp GameWithPunishment.hpp)

add_executable(main_nash_search_with_P main_nash_search_with_P.cpp NormsWithPunishment.hpp GameWithPunishment.hpp)

add_executable(leading_eight_with_errors leading_eight_ESS_with_errors.cpp ${HEADER_FILES})

add_executable(equalizers_norms equalizers_norms.cpp ${HEADER_FILES})

add_executable(L6_L3_payoff_difference L6_L3_payoff_difference.cpp ${HEADER_FILES})