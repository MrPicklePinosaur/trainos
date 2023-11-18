#ifndef __USER_TESTER_H__
#define __USER_TESTER_H__

#define TEST(expr) { if (expr) { println("\033[32m[PASSED]\033[0m "#expr); } else { println("\033[31m[FAILED]\033[0m "#expr); } }
#define _TEST(expr1, op, expr2) { if (expr1 op expr2) { println("\033[32m[PASSED]\033[0m "#expr1" "#op" "#expr2); } else { println("\033[31m[FAILED]\033[0m "#expr1" "#op" "#expr2); } }
#define TEST_EQ(expr1, expr2) _TEST(expr1, ==, expr2)

typedef void(*TestFn)(void);

void testHarness();

void testString();
void testCbuf();
void testList();
void testMap();
void testHashmap();
void testAlloc();

void testParser();
void testDijkstra();

void testNameserver();
void testSensor();

void testTraindata();
#endif // __USER_TESTER_H__
