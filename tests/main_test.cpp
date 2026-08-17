#include "test_framework.hpp"

int main() {
    return epfd::test::TestRegistry::instance().runAll();
}
