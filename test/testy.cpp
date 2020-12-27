#include <iostream>
#include <whackamole.hpp>
#include <helpers.hpp>
#include <states.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;
using namespace CppUnit;

#define CPPUNIT_ASSERT_EQUAL_ARR(ARR1, ARR2, LEN) for (int p=0; p<LEN; p++) CPPUNIT_ASSERT_EQUAL_MESSAGE(string("Expected array: ") + charr_to_str(ARR1, LEN) + string(" but got: ") + charr_to_str(ARR2, LEN), ARR1[p], ARR2[p])
#define CPPUNIT_ASSERT_EQUAL_INTARR(CURR_STATE, ARR1, ARR2, LEN) for (int p=0; p<LEN; p++) CPPUNIT_ASSERT_EQUAL_MESSAGE(string("For current state of ") + string("" + std::to_string(CURR_STATE)) + string(" the expected array is ") + intarr_to_str(ARR1, LEN) + string(", but got: ") + intarr_to_str(ARR2, LEN), ARR1[p], ARR2[p])

class WhackaTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE( WhackaTests );
    CPPUNIT_TEST( testIntToBase3 );
    CPPUNIT_TEST( testBase3ToInt );
    CPPUNIT_TEST( testTransitionsAndRewards );
    CPPUNIT_TEST( testSomethingElse );
    CPPUNIT_TEST_SUITE_END();

private:
    char* base3_buf;
    char* rewards;
    static const int num_test_states = 10;
    unsigned short int test_states_decimal[num_test_states] = {0, 1, 2, 3, 4, 18, 6530, 6560, 2222, 14};
    char gold_states_base3[num_test_states][NUM_HOLES] = {
        {0,0,0,0,0,0,0,0}, // 0
        {1,0,0,0,0,0,0,0}, // 1
        {2,0,0,0,0,0,0,0}, // 2
        {0,1,0,0,0,0,0,0}, // 3
        {1,1,0,0,0,0,0,0}, // 4
        {0,0,2,0,0,0,0,0}, // 18 
        {2,1,2,1,2,2,2,2}, // 6530
        {2,2,2,2,2,2,2,2}, // 6560
        {2,2,0,1,0,0,0,1}, // 2222
        {2,1,1,0,0,0,0,0} // 14
    };
    // the last element in each list is a transition to self and represents hitting no hole.
    unsigned short int transition_golds[num_test_states][NUM_HOLES+1] = {
        {0,0,0,0,0,0,0,0,0}, // 0: 0 => 0
        {0,1,1,1,1,1,1,1,1}, // 1: 1 => 0
        {0,2,2,2,2,2,2,2,2}, // 2: 200000000 => 00000000,    rest 200000000 => 200000000
        {3,0,3,3,3,3,3,3,3}, // 3: 010000000 => 01000000 / 00000000 / rest 010000000
        {3,1,4,4,4,4,4,4,4},  // 4:  110000000 => 01000000(3) / 100000000(1) / rest 11000000(4)
        {18,18,0,18,18,18,18,18,18}, // 18: 002000000 => 00000000(0) if second hole hit / rest 00200000
        {6528,6527,6512,6503,6368,6044,5072,2156,6530}, // 6530: 21212222 => 01212222 etc.
        {6558,6554,6542,6506,6398,6074,5102,2186,6560}, // 6560: 22222222 => 02222222 etc.
        {2220,2216,2222,2195,2222,2222,2222,35,2222},   // 2222: 22010001 => 02010001 etc.
        {12,11,5,14,14,14,14,14,14}             // 14:   21100000 => 01100000 / 20100000 / 21000000, rest 21100000

    };
    char gold_rewards[num_test_states][NUM_HOLES+1] = {
        {MISS, MISS, MISS, MISS, MISS, MISS, MISS, MISS, REWARD},     // 0
        {PENALTY, MISS, MISS, MISS, MISS, MISS, MISS, MISS, REWARD},  // 1
        {REWARD, MISS, MISS, MISS, MISS, MISS, MISS, MISS, IDLE_BUT_WRONG},   // 2
        {MISS, PENALTY, MISS, MISS, MISS, MISS, MISS, MISS, REWARD},  // 3
        {PENALTY, PENALTY, MISS, MISS, MISS, MISS, MISS, MISS, REWARD}, // 4
        {MISS, MISS, REWARD, MISS, MISS, MISS, MISS, MISS, IDLE_BUT_WRONG}, // 18
        {REWARD, PENALTY, REWARD, PENALTY, REWARD, REWARD, REWARD, REWARD, IDLE_BUT_WRONG}, // 6530
        {REWARD, REWARD, REWARD, REWARD, REWARD, REWARD, REWARD, REWARD, IDLE_BUT_WRONG}, // 6560
        {REWARD, REWARD, MISS, PENALTY, MISS, MISS, MISS, PENALTY, IDLE_BUT_WRONG}, // 2222
        {REWARD, PENALTY, PENALTY, MISS, MISS, MISS, MISS, MISS, IDLE_BUT_WRONG} // 14
    };

public:
    void setUp() {
        cout << "Setting up...\n";
        this->base3_buf = new char[NUM_HOLES];
        this->rewards = new char[NUM_HOLES+1];

    }
    void tearDown() {
        cout << "Tearing down...\n";
        delete this->base3_buf;
        delete this->rewards;
    }

protected:
    void testIntToBase3() {
        for (int q=0; q<num_test_states; q++) {
            to_base3_buf(this->test_states_decimal[q], this->base3_buf, NUM_HOLES);
            CPPUNIT_ASSERT_EQUAL_ARR(this->base3_buf, gold_states_base3[q], NUM_HOLES);
        }
    }
    void testBase3ToInt() {
        for (int q=0; q<num_test_states; q++) {
            CPPUNIT_ASSERT_EQUAL(base3_to_int(gold_states_base3[q], NUM_HOLES), test_states_decimal[q]);
        }
    }
    void testTransitionsAndRewards() {
        for (int t=0; t<num_test_states; t++) {
            unsigned short int* transitions = new unsigned short int[NUM_HOLES+1];
            unsigned short int curr_state = this->test_states_decimal[t];
            all_transitions_and_rewards(curr_state, transitions,
                                        this->rewards, this->base3_buf, NUM_HOLES);
            CPPUNIT_ASSERT_EQUAL_INTARR(curr_state, this->transition_golds[t], transitions, NUM_HOLES+1);
            CPPUNIT_ASSERT_EQUAL_ARR(this->gold_rewards[t], this->rewards, NUM_HOLES+1);
            delete transitions;
        }

    }
    void testSomethingElse() {
        CPPUNIT_ASSERT( 1 == 1 );
    }

};

int dosth() {
    return -1;
}

CPPUNIT_TEST_SUITE_REGISTRATION(WhackaTests);

int main() {
    cout << "Hello, tests\n";
    CppUnit::TextUi::TestRunner testrunner;
    CPPUNIT_NS::TestResult testresult;
    testrunner.addTest (CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest ());
    testrunner.run();
}
