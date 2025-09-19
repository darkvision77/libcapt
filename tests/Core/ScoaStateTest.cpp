#include "libcapt/Compression/ScoaState.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace Capt;

TEST(ScoaStateTest, Simple) {
    uint8_t lines[][5] = {
        {1, 2, 3, 4, 5},
    };
    Compression::ScoaState state(sizeof(lines[0]));

    state.ProcessLine(lines[0]);
    EXPECT_THAT(state.Copy,   testing::ElementsAre(0, 0, 0, 0, 0));
    EXPECT_THAT(state.Repeat, testing::ElementsAre(1, 1, 1, 1, 1));
    EXPECT_THAT(state.Raw,    testing::ElementsAre(5, 4, 3, 2, 1));
    state.PrevLine = {lines[0], lines[0]+sizeof(lines[0])};

    state.ProcessLine(lines[0]);
    EXPECT_THAT(state.Copy,   testing::ElementsAre(5, 4, 3, 2, 1));
    EXPECT_THAT(state.Repeat, testing::ElementsAre(1, 1, 1, 1, 1));
    EXPECT_THAT(state.Raw,    testing::ElementsAre(0, 0, 0, 0, 0));

    state.ProcessLine(lines[0]);
    EXPECT_THAT(state.Copy,   testing::ElementsAre(5, 4, 3, 2, 1));
    EXPECT_THAT(state.Repeat, testing::ElementsAre(1, 1, 1, 1, 1));
    EXPECT_THAT(state.Raw,    testing::ElementsAre(0, 0, 0, 0, 0));
}

TEST(StateTest, General) {
    uint8_t lines[][7] = {
        {1, 2, 2, 4, 5, 5, 5},
        {5, 5, 5, 5, 5, 5, 5},
        {5, 5, 5, 5, 5, 5, 5},
    };
    Compression::ScoaState state(sizeof(lines[0]));

    state.ProcessLine(lines[0]);
    EXPECT_THAT(state.Copy,   testing::ElementsAre(0, 0, 0, 0, 0, 0, 0));
    EXPECT_THAT(state.Repeat, testing::ElementsAre(1, 2, 1, 1, 3, 2, 1));
    EXPECT_THAT(state.Raw,    testing::ElementsAre(1, 0, 2, 1, 0, 0, 1));
    state.PrevLine = {lines[0], lines[0]+sizeof(lines[0])};

    state.ProcessLine(lines[1]);
    EXPECT_THAT(state.Copy,   testing::ElementsAre(0, 0, 0, 0, 3, 2, 1));
    EXPECT_THAT(state.Repeat, testing::ElementsAre(7, 6, 5, 4, 3, 2, 1));
    EXPECT_THAT(state.Raw,    testing::ElementsAre(0, 0, 0, 0, 0, 0, 0));
    state.PrevLine = {lines[1], lines[1]+sizeof(lines[1])};

    state.ProcessLine(lines[2]);
    EXPECT_THAT(state.Copy,   testing::ElementsAre(7, 6, 5, 4, 3, 2, 1));
    EXPECT_THAT(state.Repeat, testing::ElementsAre(7, 6, 5, 4, 3, 2, 1));
    EXPECT_THAT(state.Raw,    testing::ElementsAre(0, 0, 0, 0, 0, 0, 0));
    state.PrevLine = {lines[2], lines[2]+sizeof(lines[2])};
}
